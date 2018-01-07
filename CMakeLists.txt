cmake_minimum_required(VERSION 3.7)
project(zsdatab LANGUAGES CXX)

set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for binaries")
set(INSTALL_LIB_DIR "lib${LIB_SUFFIX}" CACHE PATH "Installation directory for libraries")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_CMAKE_DIR share/cmake/Modules CACHE PATH "Installation directory for CMake module files")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

include(CheckIncludeFiles)
check_include_file_cxx("execution" HAVE_CXXH_EXECUTION)
check_include_file_cxx("experimental/algorithm" HAVE_CXXH_EX_ALGORITHM)

configure_file(include/config.h.in "${PROJECT_BINARY_DIR}/config.h")
configure_file(FindZsdatable.cmake.in "${PROJECT_BINARY_DIR}/FindZsdatable.cmake" @ONLY)

include_directories("${PROJECT_BINARY_DIR}" 3rdparty/ThreadPool include lib lib/3rdparty/gzstream ${ZLIB_INCLUDE_DIRS})

# WARNING: you must re-run cmake every time a file under lib/ is added
#          because we use file globbing
file(GLOB_RECURSE LibSources lib/*.cxx lib/*.C)
add_library(zsdatable SHARED include/zsdatable.hpp ${LibSources})
set_target_properties(zsdatable PROPERTIES VERSION "11.0.0" SOVERSION 11)
target_link_libraries(zsdatable ZLIB::ZLIB ${CMAKE_THREAD_LIBS_INIT})

add_executable(zsdatab-entry entry.cxx)
target_link_libraries(zsdatab-entry zsdatable)

install(TARGETS zsdatable DESTINATION "${INSTALL_LIB_DIR}")
install(TARGETS zsdatab-entry DESTINATION "${INSTALL_BIN_DIR}")
install(FILES include/zsdatable.hpp DESTINATION "${INSTALL_INCLUDE_DIR}")
install(FILES "${PROJECT_BINARY_DIR}/FindZsdatable.cmake" DESTINATION "${INSTALL_CMAKE_DIR}")

install(FILES zsdatable DESTINATION "${INSTALL_BIN_DIR}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
