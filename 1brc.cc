#include <fcntl.h>
#include <linux/mman.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <algorithm>
#include <format>
#include <iostream>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include "rapidhash.h"

struct Record {
  int min;
  int max;
  int sum;
  int count;
};

std::tuple<absl::string_view, int> ParseValue(absl::string_view line) {
  auto tail = line.size() - 1;

  if (line[tail - 5] == ';') {
    return {
        line.substr(0, tail - 5),
        -(line[tail - 3] * 100 + line[tail - 2] * 10 + line[tail] - '0' * 111)};
  }

  if (line[tail - 4] == ';') {
    if (line[tail - 3] == '-') {
      return {line.substr(0, tail - 4),
              -(line[tail - 2] * 10 + line[tail] - '0' * 11)};
    } else {
      return {
          line.substr(0, tail - 4),
          line[tail - 3] * 100 + line[tail - 2] * 10 + line[tail] - '0' * 111};
    }
  }

  return {line.substr(0, tail - 3),
          line[tail - 2] * 10 + line[tail] - '0' * 11};
}

struct StringHash {
  using is_transparent = void;

  size_t operator()(absl::string_view v) const {
    return rapidhash(v.data(), v.size());
  }
};

int main(int argc, char *agrv[]) {
  int fd = open("measurements.txt", O_RDONLY);
  struct stat file_stat;
  fstat(fd, &file_stat);

  size_t len = file_stat.st_size;
  const char *data = reinterpret_cast<const char *>(
      mmap(nullptr, len, PROT_READ, MAP_SHARED | MAP_HUGE_1GB | MAP_POPULATE,
           fd, 0));
  const char *end = data + len;

  absl::flat_hash_map<std::string, Record, StringHash> records;
  for (;;) {
    const char *newline_pos =
        reinterpret_cast<const char *>(memchr(data, '\n', end - data));
    if (!newline_pos) {
      break;
    }

    absl::string_view line(data, newline_pos - data);
    data = newline_pos + 1;

    auto [city, val] = ParseValue(line);

    auto it = records.find(city);
    if (it != records.end()) {
      auto &rec = it->second;
      rec.max = std::max(rec.max, val);
      rec.min = std::min(rec.min, val);
      rec.sum += val;
      rec.count += 1;
    } else {
      records[city] = {val, val, val, 1};
    }
  }

  std::vector<std::pair<std::string, Record>> results;
  for (const auto &kv : records) {
    results.emplace_back(kv);
  }
  std::sort(
      results.begin(), results.end(),
      [](const auto &lhs, const auto &rhs) { return lhs.first < rhs.first; });

  std::cout << "{";

  bool is_first = true;
  for (const auto &kv : results) {
    const auto &rec = kv.second;
    if (is_first) {
      std::cout << std::format("{}={:.1f}/{:.1f}/{:.1f}", kv.first,
                               rec.min / 10.0, rec.sum / 10.0 / rec.count,
                               rec.max / 10.0);
      is_first = false;
    } else {
      std::cout << std::format(", {}={:.1f}/{:.1f}/{:.1f}", kv.first,
                               rec.min / 10.0, rec.sum / 10.0 / rec.count,
                               rec.max / 10.0);
    }
  }

  std::cout << "}" << std::endl;

  return 0;
}
