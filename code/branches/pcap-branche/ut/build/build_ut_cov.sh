#/bin/bash

# compile target with coverage
make cov

# compile ut
make clean
make -j2

# run ut
./ut_tmas

# generate coverage data
./collect_cov_info.sh
