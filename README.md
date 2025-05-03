- author: George Pestell (200007413)
- supervisor: Prof. Graham Kirby

# System Requirements

The following is a list of required c++ libraries that need to be installed, including versions used in development:

- `GDAL (v3.4.3)` - GIS data processing
- `CGAL (v5.6.2)` - Delaunay triangulation and mesh processing
- `oneTBB (v2.13)` - Parallelization
- `OpenCV (4.10.0)` - Computer Vision
- `GeographicLib (v26.1.0)` - Point projection conversion
- `simdjson (v3.6.0)` - JSON parsing (automatically fetched by CMAKE)
- `Eigen3 (v3.3)` - parallelization for CGAL

# Compilation Instructions

To build the library and router, first setup the build directory:

```bash
$ mkdir build && cd build
```

Then configure with cmake, making sure to set configuration option values as appropriate:

```bash
  $ cmake .. -G Ninja -DCMAKE_BUILD_TYPE= -DTSR_TEST=
```

- `TSR_TEST=ON/OFF `
  Specify whether to build test suite.
- `CMAKE_BUILD_TYPE=Release/Debug`
  Specify build type. Release is far more performant, but Debug contains much more logging information.

The library, CLI app, and tests can then be compiled using one of the following:

```bash
  # Compile everything
  $ ninja

  # Compile just the library
  $ ninja tsr

  # Compile the tests
  $ ninja test-tsr

  # Compile the cli application
  $ ninja tsr-route
```

# Running instructions

Run the router with:

```bash
./tsr-route <start-lat> <start-lon> <end-lat> <end-lon>
```
