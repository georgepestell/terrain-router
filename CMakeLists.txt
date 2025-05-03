# Created by the script cgal_create_CMakeLists
# This is the CMake script for compiling a set of CGAL applications.

cmake_minimum_required(VERSION 3.1...3.23)

project( terrain-router )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(cmake/version_info.cpp.in version_info.cpp)

# GDAL and it's components
find_package(GDAL CONFIG REQUIRED)

find_package(fmt CONFIG REQUIRED)

find_package(GeographicLib REQUIRED)

find_package(simdjson REQUIRED)

find_package(CURL REQUIRED)

find_package(OpenCV REQUIRED)

include_directories( ${GeographicLib_INCLUDE_DIRS} )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# CGAL and its components
find_package( CGAL REQUIRED COMPONENTS Core)

include(FindEigen3)
include(CGAL_Eigen3_support)

include(FindTBB)
include(CGAL_TBB_support)


if ( NOT CGAL_FOUND )

  message(STATUS "This project requires the CGAL library, and will not be compiled.")
  return()  

endif()


# Boost and its components
find_package( Boost REQUIRED COMPONENTS filesystem program_options system )

if ( NOT Boost_FOUND )

  message(STATUS "This project requires the Boost library, and will not be compiled.")

  return()  

endif()

# include for local directory
include_directories( BEFORE include )

# include for local package

# Boost linking
add_definitions( "-DCGAL_USE_BOOST_FILESYSTEM" )
list(APPEND CGAL_3RD_PARTY_LIBRARIES ${Boost_FILESYSTEM_LIBRARY} )
add_definitions( "-DCGAL_USE_BOOST_PROGRAM_OPTIONS" )
list(APPEND CGAL_3RD_PARTY_LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY} )
add_definitions( "-DCGAL_USE_BOOST_SYSTEM" )
list(APPEND CGAL_3RD_PARTY_LIBRARIES ${Boost_SYSTEM_LIBRARY} )


# Creating entries for target: tsr-route
# ############################

file(GLOB TSR_SRC
  "./src/tsr/**.cpp"
  "./src/tsr/IO/**.cpp"
  "./src/tsr/API/**.cpp"
  "./src/tsr/Features/**.cpp"
)

add_library ( tsr "src/cmd.cpp" )
add_executable( tsr-route ${TSR_SRC} )

add_to_cached_list( CGAL_EXECUTABLE_TARGETS tsr-route)

# Link the executable to CGAL and third-party libraries
target_link_libraries( tsr PRIVATE CGAL::CGAL CGAL::CGAL_Core CGAL::Eigen3_support CGAL::TBB_support GDAL::GDAL fmt::fmt ${GeographicLib_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBS} TBB::tbb CURL::libcurl simdjson ${OpenCV_LIBS} )
target_link_libraries( tsr-route PRIVATE tsr CGAL::CGAL CGAL::CGAL_Core CGAL::Eigen3_support CGAL::TBB_support GDAL::GDAL fmt::fmt ${GeographicLib_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBS} TBB::tbb CURL::libcurl simdjson ${OpenCV_LIBS} )

