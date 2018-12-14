cmake_minimum_required (VERSION 3.5.1)
set (CMAKE_CXX_STANDARD 11)

set ( TARGET demo)
project ( ${TARGET} )


file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${HALIDE_SOURCE_DIRECTORY}/tools")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

set ( HALIDE_SOURCE_DIRECTORY "/home/glen/repositories/Halide" )
set ( HALIDE_LIB_DIRECTORY "/home/glen/repositories/Halide/lib" )
find_library(HalideLib Halide PATHS ${HALIDE_LIB_DIRECTORY} NO_DEFAULT_PATH)

include_directories("${HALIDE_SOURCE_DIRECTORY}/include" "${HALIDE_SOURCE_DIRECTORY}/tools")


add_executable(demo demo.cpp ${SOURCES})
find_package(PNG)
target_link_libraries( demo ${HalideLib} -lpthread -lpng ${CMAKE_DL_LIBS}) 