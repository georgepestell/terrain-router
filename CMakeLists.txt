cmake_minimum_required(VERSION 3.26)

project(routingLib VERSION 2024.0
                   LANGUAGES CXX
                   HOMEPAGE_URL "https://github.com/georgepestell/terrain_sensitive_routing")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)

# find_package(Threads REQUIRED)
find_package(Boost 1.45.0 COMPONENTS filesystem regex)
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
  FetchContent_POPULATE(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR}
    EXCLUDE_FROM_ALL)
endif()

## LIBRARY
add_library(${PROJECT_NAME}
  src/routingLib/routing.cpp)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION})

target_include_directories(${PROJECT_NAME}
  PUBLIC include
  PRIVATE src
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE ${Boost_LIBRARIES})

target_compile_options(${PROJECT_NAME} 
  PRIVATE -Wall -Wextra -pedantic -Werror)

target_compile_features(${PROJECT_NAME} 
  PRIVATE cxx_std_20)


add_executable(${PROJECT_NAME}-exec src/routingLib/main.cpp)

target_link_libraries(${PROJECT_NAME}-exec PRIVATE ${PROJECT_NAME})

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
  EXPORT RoutingLibTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

## UNIT TESTS

add_executable(${PROJECT_NAME}-tests
  test/test-routing.cpp)

target_link_libraries(${PROJECT_NAME}-tests
  PRIVATE gtest_main ${PROJECT_NAME}::${PROJECT_NAME})

target_compile_options(${PROJECT_NAME}-tests
  PRIVATE -Wall -Wextra -pedantic -Werror)

target_compile_features(${PROJECT_NAME}-tests
  PRIVATE cxx_std_20)

enable_testing()

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}-tests)