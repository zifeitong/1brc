#include <fcntl.h>
#include <linux/mman.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <algorithm>
#include <array>
#include <format>
#include <iostream>
#include <string_view>
#include <vector>

#include "hwy/contrib/algo/find-inl.h"
#include "mph.h"

namespace hn = hwy::HWY_NAMESPACE;

using std::literals::operator""sv;
using hash_t = uint32_t;

constexpr int kMaxCityNameLength = 64;
const hn::ScalableTag<uint8_t> kTag;

struct Record {
  int sum;
  int count;
  int min;
  int max;
};

// Returns id for given city name.
static int city_id(std::string_view name);

// Returns name for given city id.
static std::string_view city_name(int id);

// Return total number of cities.
static std::size_t city_count();

int main(int argc, char *agrv[]) {
  int fd = open("measurements.txt", O_RDONLY);
  struct stat file_stat;
  fstat(fd, &file_stat);

  size_t len = file_stat.st_size;
  const char *data = reinterpret_cast<const char *>(
      mmap(nullptr, len, PROT_READ, MAP_PRIVATE | MAP_HUGE_1GB | MAP_POPULATE,
           fd, 0));

  std::vector<Record> records(city_count());

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

    auto &rec = records[city_id(city)];
    rec.max = std::max(rec.max, val);
    rec.min = std::max(rec.min, -val);
    rec.sum += val;
    rec.count += 1;
  }

  std::cout << "{";

  bool is_first = true;
  for (int i = 0; i < records.size(); ++i) {
    const auto &rec = records[i];
    const auto &name = city_name(i);
    if (is_first) {
      std::cout << std::format("{}={:.1f}/{:.1f}/{:.1f}", name, -rec.min / 10.0,
                               rec.sum / 10.0 / rec.count, rec.max / 10.0);
      is_first = false;
    } else {
      std::cout << std::format(", {}={:.1f}/{:.1f}/{:.1f}", name,
                               -rec.min / 10.0, rec.sum / 10.0 / rec.count,
                               rec.max / 10.0);
    }
  }

  std::cout << "}" << std::endl;

  return 0;
}

static constexpr auto _names = std::array{
    "Abha"sv,
    "Abidjan"sv,
    "Abéché"sv,
    "Accra"sv,
    "Addis Ababa"sv,
    "Adelaide"sv,
    "Aden"sv,
    "Ahvaz"sv,
    "Albuquerque"sv,
    "Alexandra"sv,
    "Alexandria"sv,
    "Algiers"sv,
    "Alice Springs"sv,
    "Almaty"sv,
    "Amsterdam"sv,
    "Anadyr"sv,
    "Anchorage"sv,
    "Andorra la Vella"sv,
    "Ankara"sv,
    "Antananarivo"sv,
    "Antsiranana"sv,
    "Arkhangelsk"sv,
    "Ashgabat"sv,
    "Asmara"sv,
    "Assab"sv,
    "Astana"sv,
    "Athens"sv,
    "Atlanta"sv,
    "Auckland"sv,
    "Austin"sv,
    "Baghdad"sv,
    "Baguio"sv,
    "Baku"sv,
    "Baltimore"sv,
    "Bamako"sv,
    "Bangkok"sv,
    "Bangui"sv,
    "Banjul"sv,
    "Barcelona"sv,
    "Bata"sv,
    "Batumi"sv,
    "Beijing"sv,
    "Beirut"sv,
    "Belgrade"sv,
    "Belize City"sv,
    "Benghazi"sv,
    "Bergen"sv,
    "Berlin"sv,
    "Bilbao"sv,
    "Birao"sv,
    "Bishkek"sv,
    "Bissau"sv,
    "Blantyre"sv,
    "Bloemfontein"sv,
    "Boise"sv,
    "Bordeaux"sv,
    "Bosaso"sv,
    "Boston"sv,
    "Bouaké"sv,
    "Bratislava"sv,
    "Brazzaville"sv,
    "Bridgetown"sv,
    "Brisbane"sv,
    "Brussels"sv,
    "Bucharest"sv,
    "Budapest"sv,
    "Bujumbura"sv,
    "Bulawayo"sv,
    "Burnie"sv,
    "Busan"sv,
    "Cabo San Lucas"sv,
    "Cairns"sv,
    "Cairo"sv,
    "Calgary"sv,
    "Canberra"sv,
    "Cape Town"sv,
    "Changsha"sv,
    "Charlotte"sv,
    "Chiang Mai"sv,
    "Chicago"sv,
    "Chihuahua"sv,
    "Chittagong"sv,
    "Chișinău"sv,
    "Chongqing"sv,
    "Christchurch"sv,
    "City of San Marino"sv,
    "Colombo"sv,
    "Columbus"sv,
    "Conakry"sv,
    "Copenhagen"sv,
    "Cotonou"sv,
    "Cracow"sv,
    "Da Lat"sv,
    "Da Nang"sv,
    "Dakar"sv,
    "Dallas"sv,
    "Damascus"sv,
    "Dampier"sv,
    "Dar es Salaam"sv,
    "Darwin"sv,
    "Denpasar"sv,
    "Denver"sv,
    "Detroit"sv,
    "Dhaka"sv,
    "Dikson"sv,
    "Dili"sv,
    "Djibouti"sv,
    "Dodoma"sv,
    "Dolisie"sv,
    "Douala"sv,
    "Dubai"sv,
    "Dublin"sv,
    "Dunedin"sv,
    "Durban"sv,
    "Dushanbe"sv,
    "Edinburgh"sv,
    "Edmonton"sv,
    "El Paso"sv,
    "Entebbe"sv,
    "Erbil"sv,
    "Erzurum"sv,
    "Fairbanks"sv,
    "Fianarantsoa"sv,
    "Flores,  Petén"sv,
    "Frankfurt"sv,
    "Fresno"sv,
    "Fukuoka"sv,
    "Gaborone"sv,
    "Gabès"sv,
    "Gagnoa"sv,
    "Gangtok"sv,
    "Garissa"sv,
    "Garoua"sv,
    "George Town"sv,
    "Ghanzi"sv,
    "Gjoa Haven"sv,
    "Guadalajara"sv,
    "Guangzhou"sv,
    "Guatemala City"sv,
    "Halifax"sv,
    "Hamburg"sv,
    "Hamilton"sv,
    "Hanga Roa"sv,
    "Hanoi"sv,
    "Harare"sv,
    "Harbin"sv,
    "Hargeisa"sv,
    "Hat Yai"sv,
    "Havana"sv,
    "Helsinki"sv,
    "Heraklion"sv,
    "Hiroshima"sv,
    "Ho Chi Minh City"sv,
    "Hobart"sv,
    "Hong Kong"sv,
    "Honiara"sv,
    "Honolulu"sv,
    "Houston"sv,
    "Ifrane"sv,
    "Indianapolis"sv,
    "Iqaluit"sv,
    "Irkutsk"sv,
    "Istanbul"sv,
    "Jacksonville"sv,
    "Jakarta"sv,
    "Jayapura"sv,
    "Jerusalem"sv,
    "Johannesburg"sv,
    "Jos"sv,
    "Juba"sv,
    "Kabul"sv,
    "Kampala"sv,
    "Kandi"sv,
    "Kankan"sv,
    "Kano"sv,
    "Kansas City"sv,
    "Karachi"sv,
    "Karonga"sv,
    "Kathmandu"sv,
    "Khartoum"sv,
    "Kingston"sv,
    "Kinshasa"sv,
    "Kolkata"sv,
    "Kuala Lumpur"sv,
    "Kumasi"sv,
    "Kunming"sv,
    "Kuopio"sv,
    "Kuwait City"sv,
    "Kyiv"sv,
    "Kyoto"sv,
    "La Ceiba"sv,
    "La Paz"sv,
    "Lagos"sv,
    "Lahore"sv,
    "Lake Havasu City"sv,
    "Lake Tekapo"sv,
    "Las Palmas de Gran Canaria"sv,
    "Las Vegas"sv,
    "Launceston"sv,
    "Lhasa"sv,
    "Libreville"sv,
    "Lisbon"sv,
    "Livingstone"sv,
    "Ljubljana"sv,
    "Lodwar"sv,
    "Lomé"sv,
    "London"sv,
    "Los Angeles"sv,
    "Louisville"sv,
    "Luanda"sv,
    "Lubumbashi"sv,
    "Lusaka"sv,
    "Luxembourg City"sv,
    "Lviv"sv,
    "Lyon"sv,
    "Madrid"sv,
    "Mahajanga"sv,
    "Makassar"sv,
    "Makurdi"sv,
    "Malabo"sv,
    "Malé"sv,
    "Managua"sv,
    "Manama"sv,
    "Mandalay"sv,
    "Mango"sv,
    "Manila"sv,
    "Maputo"sv,
    "Marrakesh"sv,
    "Marseille"sv,
    "Maun"sv,
    "Medan"sv,
    "Mek'ele"sv,
    "Melbourne"sv,
    "Memphis"sv,
    "Mexicali"sv,
    "Mexico City"sv,
    "Miami"sv,
    "Milan"sv,
    "Milwaukee"sv,
    "Minneapolis"sv,
    "Minsk"sv,
    "Mogadishu"sv,
    "Mombasa"sv,
    "Monaco"sv,
    "Moncton"sv,
    "Monterrey"sv,
    "Montreal"sv,
    "Moscow"sv,
    "Mumbai"sv,
    "Murmansk"sv,
    "Muscat"sv,
    "Mzuzu"sv,
    "N'Djamena"sv,
    "Naha"sv,
    "Nairobi"sv,
    "Nakhon Ratchasima"sv,
    "Napier"sv,
    "Napoli"sv,
    "Nashville"sv,
    "Nassau"sv,
    "Ndola"sv,
    "New Delhi"sv,
    "New Orleans"sv,
    "New York City"sv,
    "Ngaoundéré"sv,
    "Niamey"sv,
    "Nicosia"sv,
    "Niigata"sv,
    "Nouadhibou"sv,
    "Nouakchott"sv,
    "Novosibirsk"sv,
    "Nuuk"sv,
    "Odesa"sv,
    "Odienné"sv,
    "Oklahoma City"sv,
    "Omaha"sv,
    "Oranjestad"sv,
    "Oslo"sv,
    "Ottawa"sv,
    "Ouagadougou"sv,
    "Ouahigouya"sv,
    "Ouarzazate"sv,
    "Oulu"sv,
    "Palembang"sv,
    "Palermo"sv,
    "Palm Springs"sv,
    "Palmerston North"sv,
    "Panama City"sv,
    "Parakou"sv,
    "Paris"sv,
    "Perth"sv,
    "Petropavlovsk-Kamchatsky"sv,
    "Philadelphia"sv,
    "Phnom Penh"sv,
    "Phoenix"sv,
    "Pittsburgh"sv,
    "Podgorica"sv,
    "Pointe-Noire"sv,
    "Pontianak"sv,
    "Port Moresby"sv,
    "Port Sudan"sv,
    "Port Vila"sv,
    "Port-Gentil"sv,
    "Portland (OR)"sv,
    "Porto"sv,
    "Prague"sv,
    "Praia"sv,
    "Pretoria"sv,
    "Pyongyang"sv,
    "Rabat"sv,
    "Rangpur"sv,
    "Reggane"sv,
    "Reykjavík"sv,
    "Riga"sv,
    "Riyadh"sv,
    "Rome"sv,
    "Roseau"sv,
    "Rostov-on-Don"sv,
    "Sacramento"sv,
    "Saint Petersburg"sv,
    "Saint-Pierre"sv,
    "Salt Lake City"sv,
    "San Antonio"sv,
    "San Diego"sv,
    "San Francisco"sv,
    "San Jose"sv,
    "San José"sv,
    "San Juan"sv,
    "San Salvador"sv,
    "Sana'a"sv,
    "Santo Domingo"sv,
    "Sapporo"sv,
    "Sarajevo"sv,
    "Saskatoon"sv,
    "Seattle"sv,
    "Seoul"sv,
    "Seville"sv,
    "Shanghai"sv,
    "Singapore"sv,
    "Skopje"sv,
    "Sochi"sv,
    "Sofia"sv,
    "Sokoto"sv,
    "Split"sv,
    "St. John's"sv,
    "St. Louis"sv,
    "Stockholm"sv,
    "Surabaya"sv,
    "Suva"sv,
    "Suwałki"sv,
    "Sydney"sv,
    "Ségou"sv,
    "Tabora"sv,
    "Tabriz"sv,
    "Taipei"sv,
    "Tallinn"sv,
    "Tamale"sv,
    "Tamanrasset"sv,
    "Tampa"sv,
    "Tashkent"sv,
    "Tauranga"sv,
    "Tbilisi"sv,
    "Tegucigalpa"sv,
    "Tehran"sv,
    "Tel Aviv"sv,
    "Thessaloniki"sv,
    "Thiès"sv,
    "Tijuana"sv,
    "Timbuktu"sv,
    "Tirana"sv,
    "Toamasina"sv,
    "Tokyo"sv,
    "Toliara"sv,
    "Toluca"sv,
    "Toronto"sv,
    "Tripoli"sv,
    "Tromsø"sv,
    "Tucson"sv,
    "Tunis"sv,
    "Ulaanbaatar"sv,
    "Upington"sv,
    "Vaduz"sv,
    "Valencia"sv,
    "Valletta"sv,
    "Vancouver"sv,
    "Veracruz"sv,
    "Vienna"sv,
    "Vientiane"sv,
    "Villahermosa"sv,
    "Vilnius"sv,
    "Virginia Beach"sv,
    "Vladivostok"sv,
    "Warsaw"sv,
    "Washington, D.C."sv,
    "Wau"sv,
    "Wellington"sv,
    "Whitehorse"sv,
    "Wichita"sv,
    "Willemstad"sv,
    "Winnipeg"sv,
    "Wrocław"sv,
    "Xi'an"sv,
    "Yakutsk"sv,
    "Yangon"sv,
    "Yaoundé"sv,
    "Yellowknife"sv,
    "Yerevan"sv,
    "Yinchuan"sv,
    "Zagreb"sv,
    "Zanzibar City"sv,
    "Zürich"sv,
    "Ürümqi"sv,
    "İzmir"sv,
};

static constexpr hash_t o1hash(std::string_view s) {
  static_assert(HWY_IS_LITTLE_ENDIAN, "Only support little endian");

  auto len = s.size();

  if consteval {
    if (len >= 4) {
      uint32_t first = (std::bit_cast<uint8_t>(s[3]) << 24) +
                       (std::bit_cast<uint8_t>(s[2]) << 16) +
                       (std::bit_cast<uint8_t>(s[1]) << 8) +
                       std::bit_cast<uint8_t>(s[0]),
               middle = (std::bit_cast<uint8_t>(s[(len >> 1) + 1]) << 24) +
                        (std::bit_cast<uint8_t>(s[len >> 1]) << 16) +
                        (std::bit_cast<uint8_t>(s[(len >> 1) - 1]) << 8) +
                        std::bit_cast<uint8_t>(s[(len >> 1) - 2]),
               last = (std::bit_cast<uint8_t>(s[len - 1]) << 24) +
                      (std::bit_cast<uint8_t>(s[len - 2]) << 16) +
                      (std::bit_cast<uint8_t>(s[len - 3]) << 8) +
                      std::bit_cast<uint8_t>(s[len - 4]);
      return (first + last) * middle;
    } else if (len) {
      return (std::bit_cast<uint8_t>(s[0]) << 16) |
             (std::bit_cast<uint8_t>(s[len >> 1]) << 8) |
             std::bit_cast<uint8_t>(s[len - 1]);
    }
  } else {
    if (len >= 4) {
      auto *p = s.data();

      uint32_t first = *reinterpret_cast<const uint32_t *>(p),
               middle = *reinterpret_cast<const uint32_t *>(p + (len >> 1) - 2),
               last = *reinterpret_cast<const uint32_t *>(p + len - 4);
      return (first + last) * middle;
    } else if (len) {
      return (std::bit_cast<uint8_t>(s[0]) << 16) |
             (std::bit_cast<uint8_t>(s[len >> 1]) << 8) |
             std::bit_cast<uint8_t>(s[len - 1]);
    }
  }

  return 0;
}

static constexpr auto _table = []() {
  std::array<hash_t, std::size(_names)> values;
  size_t i = 0;
  for (auto &v : values) {
    auto name = _names[i++];
    v = o1hash(name);
  }
  return values;
}();

static int city_id(std::string_view name) {
  return mph::lookup<_table>(o1hash(name));
}

static std::string_view city_name(int id) { return _names[id]; }

static std::size_t city_count() { return _names.size(); }
