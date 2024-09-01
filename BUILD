cc_binary(
  name = "1brc",
  deps = [
    "@abseil-cpp//absl/strings",
    "@abseil-cpp//absl/log:check",
    "@abseil-cpp//absl/container:flat_hash_map",
    "@highway//:algo",
  ],
  srcs = [
    "1brc.cc",
    "o1hash.h",
    "cities.h",
    "mph.h",
  ],
)

fdo_profile(
    name = "fdo_profile",
    profile = "fdo.profdata",
)
