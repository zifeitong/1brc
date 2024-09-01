run:
  bazel build -c opt 1brc --fdo_instrument=./
  ./bazel-bin/1brc > /dev/null
  llvm-profdata merge *.profraw -o fdo.profdata 
  rm *.profraw
  bazel build -c opt 1brc --fdo_optimize=//:fdo_profile

## BOLT
## llvm-bolt bazel-bin/1brc -instrument -o bazel-bin/1brc.instrumented
## bazel-bin/1brc.instrumented
## llvm-bolt bazel-bin/1brc -o bazel-bin/1brc.bolt -data /tmp/prof.fdata -reorder-blocks=ext-tsp -reorder-functions=hfsort -split-functions -split-all-cold -split-eh -dyno-stats
