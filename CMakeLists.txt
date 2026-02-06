cmake_minimum_required(VERSION 3.20)
project(rcpp
    VERSION 0.1.0
    DESCRIPTION "C++26 Refinement Types Library"
    LANGUAGES CXX
)

# Require C++26
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(RCPP_BUILD_TESTS "Build test suite" ON)
option(RCPP_BUILD_EXAMPLES "Build assembly comparison examples" OFF)
option(RCPP_INSTALL "Generate install target" ON)

# Create header-only library
add_library(rcpp INTERFACE)
add_library(rcpp::rcpp ALIAS rcpp)

target_include_directories(rcpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Add reflection flag (required, GCC only)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(rcpp INTERFACE -freflection)
endif()

# Tests
if(RCPP_BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
    )
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()

# Examples
if(RCPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
if(RCPP_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Install headers
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Install targets
    install(TARGETS rcpp
        EXPORT rcppTargets
    )

    # Generate and install config files
    install(EXPORT rcppTargets
        FILE rcppTargets.cmake
        NAMESPACE rcpp::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rcpp
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rcppConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/rcppConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rcpp
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/rcppConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/rcppConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/rcppConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rcpp
    )
endif()
