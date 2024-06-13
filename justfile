run:
  bazel build -c opt 1brc --copt=-march=native --fdo_instrument=./
  ./bazel-bin/1brc > /dev/null
  llvm-profdata merge *.profraw -o fdo.profdata 
  rm *.profraw
  bazel build -c opt 1brc --copt=-march=native --fdo_optimize=//:fdo_profile
