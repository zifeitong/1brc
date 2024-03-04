#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

struct Record {
  double min;
  double max;
  double sum;
  int count;
};

int main(int argc, char *agrv[]) {
  std::ifstream in("measurements.txt");

  std::unordered_map<std::string, Record> records;

  for (std::string line; std::getline(in, line);) {
    auto pos = line.rfind(';');
    std::string city = line.substr(0, pos);
    double val = std::stod(line.substr(pos + 1));

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
