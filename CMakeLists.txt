cmake_minimum_required(VERSION 3.16)

project(qtlinq
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

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

    target_link_libraries(tst_qtlinq
        PRIVATE
            qtlinq
            Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Test
    )

    add_test(NAME qtlinq_tests COMMAND tst_qtlinq -v2 -functions)
endif()
