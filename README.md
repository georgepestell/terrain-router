- author: George Pestell (200007413)
- supervisor: Prof. Graham Kirby

# Terrain Sensitive Routing

# Building

## Release Version

This project uses CMAKE to configure the project, and ninja to build.

```bash
mkdir build && cd build

cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DTSR_TEST=OFF
```

For performance reasons, setting `CMAKE_BUILD_TYPE=Release` is highly reccommended. Note: this will disable TRACE logs, except in testing. See [#Testing] for more info on tests. 

To build, run:

```bash
# Build everything
ninja

# Build just the library
ninja tsr

# Build the cmd executable
ninja tsr-route
```
## Development / Debug

### Testing

To build tests, set `TSR_TEST=ON` in cmake configuration:

```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DTSR_TEST=ON
```
# Running

## Running The Tests

To run the tests, setting the cmake build flag `TSR_TEST` is essential to enable. It is also recommended to set  `CMAKE_BUILD_TYPE=Debug` for more verbose error logs.

To configure cmake with these settings, run:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DTSR_Test=ON ..
```
To run all of the tests generated, run:

```bash
./test-tsr
```
Specific tests can be specified using the `--gtest_filter` option. This does a REGEX match with tests named `<test_suite>.<test_case>`.

```bash
# Run all tests under a specific test suite
./test-tsr --gtest_filter="<test_suite>.*"

# Run a specific test case
./test-tsr --gtest_filter="<test_suite>.<test_case>"

# Run all tests beginning with testRead under the IOTests suite
./test-tsr --gtest_filter="IOTests.testRead*"
```
Run `./test-tsr --help` for more options.
