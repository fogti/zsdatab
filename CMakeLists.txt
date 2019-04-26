cmake_minimum_required(VERSION 3.7)
project(zsdatab LANGUAGES CXX)

set(CMAKE_PREFIX "ZSdatab")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for binaries")
set(INSTALL_LIB_DIR "lib${LIB_SUFFIX}" CACHE PATH "Installation directory for libraries")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_CMAKE_DIR "${INSTALL_LIB_DIR}/cmake/${CMAKE_PREFIX}" CACHE PATH "Installation directory for CMake module files")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-plt -fno-rtti")

find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

include(CheckIncludeFiles)
check_include_file_cxx("execution" HAVE_CXXH_EXECUTION)
check_include_file_cxx("experimental/execution" HAVE_CXXH_EX_EXECUTION)
check_include_file_cxx("experimental/algorithm" HAVE_CXXH_EX_ALGORITHM)

configure_file(config.h.in "${PROJECT_BINARY_DIR}/config.h")
configure_file("cmake/Find${CMAKE_PREFIX}.cmake.in" "${PROJECT_BINARY_DIR}/Find${CMAKE_PREFIX}.cmake" @ONLY)

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" lib ${ZLIB_INCLUDE_DIRS})

function(src_compile_flags flag)
  set_property(SOURCE ${ARGN} APPEND_STRING PROPERTY COMPILE_FLAGS " ${flag}")
endfunction()

# WARNING: you must re-run cmake every time a file under lib/ is added
#          because we use file globbing
file(GLOB_RECURSE LibSources lib/*.cxx lib/*.cpp lib/*.C)
add_library(zsdatable SHARED zsdatable.hpp ${LibSources})
set_target_properties(zsdatable PROPERTIES VERSION "12.0.0" SOVERSION 12)
# use ${ZLIB_LIBRARIES} instead of ZLIB::ZLIB to make EXPORT happy (shouldn't depend on find_package(ZLIB))
target_link_libraries(zsdatable ${ZLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

src_compile_flags("-frtti" lib/transaction.cxx)

add_executable(zsdatab-entry entry.cxx)
target_link_libraries(zsdatab-entry zsdatable)

install(TARGETS zsdatable DESTINATION "${INSTALL_LIB_DIR}" EXPORT "${CMAKE_PREFIX}Targets")
install(TARGETS zsdatab-entry DESTINATION "${INSTALL_BIN_DIR}")
install(FILES zsdatable.hpp DESTINATION "${INSTALL_INCLUDE_DIR}")
install(FILES "cmake/${CMAKE_PREFIX}Config.cmake" "${PROJECT_BINARY_DIR}/Find${CMAKE_PREFIX}.cmake" DESTINATION "${INSTALL_CMAKE_DIR}")
install(EXPORT "${CMAKE_PREFIX}Targets" DESTINATION "${INSTALL_CMAKE_DIR}" NAMESPACE "${CMAKE_PREFIX}::" COMPONENT dev)

install(FILES zsdatable DESTINATION "${INSTALL_BIN_DIR}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
