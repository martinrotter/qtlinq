#################################################################
#
# For license of this file, see <project-root-folder>/LICENSE.md.
#
#################################################################

cmake_minimum_required(VERSION 3.16)

project(qtlinq
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(QTLINQ_BUILD_TESTS "Build qtlinq tests" ON)

# Header-only interface library
add_library(qtlinq INTERFACE)
target_include_directories(qtlinq
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# We need at least C++17 for std::optional.
target_compile_features(qtlinq INTERFACE cxx_std_17)

# Tests
if(QTLINQ_BUILD_TESTS)
  enable_testing()

  # Find Qt (Qt6 or Qt5, Core + Test)
  find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Test)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

  add_executable(tst_qtlinq tests/tst_qtlinq.cpp)
  add_executable(tst_qtlinq_bench tests/tst_qtlinq_bench.cpp)

  target_link_libraries(tst_qtlinq
    PRIVATE
      qtlinq
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Test
  )

  target_link_libraries(tst_qtlinq_bench
    PRIVATE
      qtlinq
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Test
  )

  add_test(NAME qtlinq_tests COMMAND tst_qtlinq)
  add_test(NAME qtlinq_tests_bench COMMAND tst_qtlinq_bench)
endif()
