cmake_minimum_required (VERSION 3.5.1)
set (CMAKE_CXX_STANDARD 11)

set ( TARGET HalideTraceViz)
project ( ${TARGET} )




set ( HALIDE_SOURCE_DIRECTORY "/root/packages/Halide" )
set ( HALIDE_LIB_DIRECTORY "/root/packages/Halide/lib" )
find_library(HalideLib Halide PATHS ${HALIDE_LIB_DIRECTORY} NO_DEFAULT_PATH)

include_directories("${HALIDE_SOURCE_DIRECTORY}/include" "${HALIDE_SOURCE_DIRECTORY}/tools")

add_executable(${TARGET} HalideTraceViz.cpp)
target_link_libraries( ${TARGET} ${HalideLib} -lpthread -lpng ${CMAKE_DL_LIBS}) 