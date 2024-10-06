#include <fcntl.h>
#include <linux/mman.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <algorithm>
#include <format>
#include <iostream>
#include <string_view>
#include <vector>

#include "cities.h"
#include "hwy/contrib/algo/find-inl.h"
#include "mph.h"

constexpr int kMaxCityNameLength = 64;

namespace hn = hwy::HWY_NAMESPACE;

struct Record {
  int sum;
  int count;
  int min;
  int max;
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

  std::vector<Record> records(city_names.size());

  for (;;) {
    size_t pos =
        hn::Find(kTag, static_cast<uint8_t>(';'),
                 reinterpret_cast<const uint8_t *>(data), kMaxCityNameLength);
    if (pos == kMaxCityNameLength) {
      break;
    }
    std::string_view city(data, pos);

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

    auto &rec = records[get_index(city)];
    rec.max = std::max(rec.max, val);
    rec.min = std::min(rec.min, val);
    rec.sum += val;
    rec.count += 1;
  }

  std::cout << "{";

  bool is_first = true;
  for (int i = 0; i < records.size(); ++i) {
    const auto &rec = records[i];
    const auto &name = get_name(i);
    if (is_first) {
      std::cout << std::format("{}={:.1f}/{:.1f}/{:.1f}", name, rec.min / 10.0,
                               rec.sum / 10.0 / rec.count, rec.max / 10.0);
      is_first = false;
    } else {
      std::cout << std::format(", {}={:.1f}/{:.1f}/{:.1f}", name,
                               rec.min / 10.0, rec.sum / 10.0 / rec.count,
                               rec.max / 10.0);
    }
  }

  std::cout << "}" << std::endl;

  return 0;
}
