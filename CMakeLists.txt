cmake_minimum_required(VERSION 3.25)

project(terrin_senstive_routing)

option(TSR_TEST "include test targets in build system" OFF)
option(TSR_DOWNLOAD_DEPS "download dependencies during cmake configure" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions ( -Wno-gnu-zero-variadic-macro-arguments )

# get current working branch
execute_process(
  COMMAND git describe --all --dirty
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE TSR_GIT_DESCRIPTION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# last commit hash
execute_process(
    COMMAND git log -1 --format=%H
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE TSR_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(TIMESTAMP TSR_TIMESTAMP "%Y%m%dT%H%MZ" UTC)

find_package(CGAL REQUIRED)
if (NOT CGAL_FOUND)
  message(FATAL_ERROR "CGAL not found, cannot proceed")
endif()

if (CGAL_VERSION VERSION_LESS "5.6")
  message(FATAL_ERROR "CGAL version \"${CGAL_VERSION}\" is too old, at least 5.6 is required")
endif()

# Boost
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(Boost_USE_STATIC_LINS ON)
find_package(Boost REQUIRED COMPONENTS
  program_options
  filesystem
  system
)

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281) # 10.2.1

 FetchContent_MakeAvailable(fmt)

set(TSR_SOURCE_FILES
  src/DTM.cpp
  src/logging.cpp
  src/IO.cpp
)

configure_file(src/version_info.cpp version_info.cpp)

add_library(tsr
  ${TSR_SOURCE_FILES}
)

find_package(TBB)
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

find_package(OpenCV REQUIRED)
find_package(TBB REQUIRED)

target_compile_definitions(tsr PUBLIC $<$<CONFIG:Debug>:TSR_DEBUG>)

target_include_directories(tsr
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${Boost_INCLUDE_DIRS}

  PRIVATE
    ${CGAL_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(tsr
  PUBLIC
    ${Boost_LIBRARIES}
    fmt::fmt

  PRIVATE
    CGAL::CGAL
    Eigen3::Eigen
    ${OpenCV_LIBS}
    TBB::tbb
)

add_executable(tsr-route
  src/cmd.cpp
)

target_compile_definitions(tsr-route 
  PRIVATE "-DSOURCE_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\""
  PUBLIC $<$<CONFIG:Debug>:TSR_DEBUG>
  )

target_include_directories(tsr-route
  PRIVATE 
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

target_link_libraries(tsr-route
  PRIVATE
    ${Boost_LIBRARIES}
    tsr
    fmt::fmt
    TBB::tbb
)

if (TSR_TEST)

  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()


  add_executable(test-tsr test/test-route.cpp)
  target_include_directories(test-tsr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(test-tsr PRIVATE Eigen3::Eigen ${OpenCV_LIBS} tsr GTest::gtest_main TBB::tbb)


  
  include(GoogleTest)
  gtest_discover_tests(test-tsr)

endif()

# Add additional compiler flags if required
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(tsr PRIVATE -Wall -Wextra -pedantic -frounding-math)
  target_compile_options(tsr-route PRIVATE -Wall -Wextra -pedantic -frounding-math)
endif()