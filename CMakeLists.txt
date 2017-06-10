cmake_minimum_required(VERSION 3.7)
project(zsdatab LANGUAGES CXX)

set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for binaries")
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_CMAKE_DIR share/cmake/Modules CACHE PATH "Installation directory for CMake module files")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(FindZsdatable.cmake.in "${PROJECT_BINARY_DIR}/FindZsdatable.cmake")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(zsdatable SHARED src/context/common.cxx src/context/main.cxx src/context/operators.cxx src/context/wrappers.cxx src/join.cxx src/map.cxx src/metadata.cxx src/operators.cxx src/table.cxx src/transaction.cxx src/zsdatable.hpp)
set_target_properties(zsdatable PROPERTIES VERSION "6.0.0" SOVERSION 6)

add_executable(zsdatab-entry src/entry.cxx)
target_link_libraries(zsdatab-entry zsdatable)

install(TARGETS zsdatable DESTINATION "${INSTALL_LIB_DIR}")
install(TARGETS zsdatab-entry DESTINATION "${INSTALL_BIN_DIR}")
install(FILES src/zsdatable.hpp DESTINATION "${INSTALL_INCLUDE_DIR}")
install(FILES "${PROJECT_BINARY_DIR}/FindZsdatable.cmake" DESTINATION "${INSTALL_CMAKE_DIR}")

install(FILES zsdatable DESTINATION "${INSTALL_BIN_DIR}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
