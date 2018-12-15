cmake_minimum_required (VERSION 3.5.1)
set (CMAKE_CXX_STANDARD 11)

set ( TARGET HalideTraceViz)
project ( ${TARGET} )




set ( HALIDE_SOURCE_DIRECTORY "/home/ubuntu/repositories/halide_build" )
set ( HALIDE_LIB_DIRECTORY "/home/ubuntu/repositories/halide_build/lib" )
set ( HALIDE_GIT "/home/ubuntu/repositories/Halide" )

find_library(HalideLib Halide PATHS ${HALIDE_LIB_DIRECTORY} NO_DEFAULT_PATH)

include_directories("${HALIDE_SOURCE_DIRECTORY}/include" "${HALIDE_GIT}/tools")


add_executable(${TARGET} HalideTraceViz.cpp)
target_link_libraries( ${TARGET} ${HalideLib} -lpthread -lpng ${CMAKE_DL_LIBS}) 
