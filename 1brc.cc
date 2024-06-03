#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include <algorithm>
#include <fcntl.h>
#include <format>
#include <iostream>
#include <linux/mman.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <vector>

struct Record {
  double min;
  double max;
  double sum;
  int count;
};

double ParseValue(absl::string_view s) {
  double sign = 1.0;
  if (s[0] == '-') {
    s = s.substr(1);
    sign = -1.0;
  }

  if (s.size() == 4) {
    return sign * ((s[0] - '0') * 10.0 + (s[1] - '0') + (s[3] - '0') / 10.0);
  } else {
    CHECK(s.size() == 3);
    return sign * ((s[0] - '0') + (s[2] - '0') / 10.0);
  }
}

int main(int argc, char *agrv[]) {
  int fd = open("measurements.txt", O_RDONLY);
  struct stat file_stat;
  fstat(fd, &file_stat);

  size_t len = file_stat.st_size;
  const char *data = reinterpret_cast<const char *>(
      mmap(nullptr, len, PROT_READ, MAP_SHARED | MAP_HUGE_1GB | MAP_POPULATE,
           fd, 0));
  const char *end = data + len;

  absl::flat_hash_map<std::string, Record> records;
  for (;;) {
    const char *newline_pos =
        reinterpret_cast<const char *>(memchr(data, '\n', end - data));
    if (!newline_pos) {
      break;
    }

    absl::string_view line(data, newline_pos - data);
    data = newline_pos + 1;

    auto pos = line.rfind(';');
    auto city = line.substr(0, pos);

    double val = ParseValue(line.substr(pos + 1));

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
      std::cout << std::format("{}={:.1f}/{:.1f}/{:.1f}", kv.first, rec.min,
                               rec.sum / rec.count, rec.max);
      is_first = false;
    } else {
      std::cout << std::format(", {}={:.1f}/{:.1f}/{:.1f}", kv.first, rec.min,
                               rec.sum / rec.count, rec.max);
    }
  }

  std::cout << "}" << std::endl;

  return 0;
}
