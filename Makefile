.PHONY: all debug release dev install clean test bench pytest pybench

all: release

debug:
	@mkdir -p build && cd build && cmake .. && make

release:
	@mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make

dev:
	@mkdir -p build && cd build && cmake -DBUILD_TEST=ON -DBUILD_BENCH=ON .. && make

install:
	@mkdir -p build && cd build && make install

clean:
	@rm -rf build

test:
	@bin/imagehash_test
	@bin/hash_test
	@bin/cache_test

bench:
	@bin/imagehash_bench
	@bin/hash_bench
	@bin/cache_bench

pytest:
	@python3 tests/python/pyimagehash.py

pybench:
	@python3 tests/python/pyimagehash.py bench
