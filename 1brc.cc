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
#include "hwy/contrib/algo/find-inl.h"
#include "o1hash.h"

constexpr int kMaxCityNameLength = 64;

namespace hn = hwy::HWY_NAMESPACE;

struct Record {
  int sum;
  int count;
  int min;
  int max;
};

struct StringHash {
  using is_transparent = void;

  size_t operator()(absl::string_view v) const {
    return o1hash(v.data(), v.size());
  }
};

const hn::ScalableTag<uint8_t> kTag;

int main(int argc, char *agrv[]) {
  int fd = open("measurements.txt", O_RDONLY);
  struct stat file_stat;
  fstat(fd, &file_stat);

  size_t len = file_stat.st_size;
  const char *data = reinterpret_cast<const char *>(
      mmap(nullptr, len, PROT_READ, MAP_PRIVATE | MAP_HUGE_1GB | MAP_POPULATE,
           fd, 0));

  absl::flat_hash_map<std::string, Record, StringHash> records;
  for (;;) {
    size_t pos =
        hn::Find(kTag, static_cast<uint8_t>(';'),
                 reinterpret_cast<const uint8_t *>(data), kMaxCityNameLength);
    if (pos == kMaxCityNameLength) {
      break;
    }
    absl::string_view city(data, pos);

    data += pos + 1;

    int val;
    if (data[1] == '.') {
      val = data[0] * 10 + data[2] - '0' * 11;
      data += 4;
    } else if (data[2] == '.') {
      if (data[0] == '-') {
        val = -(data[1] * 10 + data[3] - '0' * 11);
      } else {
        val = data[0] * 100 + data[1] * 10 + data[3] - '0' * 111;
      }
      data += 5;
    } else {
      val = -(data[1] * 100 + data[2] * 10 + data[4] - '0' * 111);
      data += 6;
    }

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
