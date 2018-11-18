cmake_minimum_required (VERSION 2.8.11)
set (CMAKE_CXX_STANDARD 11)

set ( TARGET hello_world)
project ( ${TARGET} )


set ( HALIDE_SOURCE_DIRECTORY "/root/packages/Halide" )
set ( HALIDE_LIB_DIRECTORY "/root/packages/Halide/lib" )
find_library(HalideLib Halide PATHS ${HALIDE_LIB_DIRECTORY} NO_DEFAULT_PATH)

include_directories("${HALIDE_SOURCE_DIRECTORY}/include" "${HALIDE_SOURCE_DIRECTORY}/tools")

add_executable(${TARGET} ${TARGET}.cpp)
target_link_libraries( ${TARGET} ${HalideLib} -lpthread ${CMAKE_DL_LIBS}) 