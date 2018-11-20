cmake_minimum_required (VERSION 3.5.1)
set (CMAKE_CXX_STANDARD 11)

set ( TARGET StereoBM)
project ( ${TARGET} )


file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${HALIDE_SOURCE_DIRECTORY}/tools")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

set ( HALIDE_SOURCE_DIRECTORY "/root/packages/Halide" )
set ( HALIDE_LIB_DIRECTORY "/root/packages/Halide/lib" )
find_library(HalideLib Halide PATHS ${HALIDE_LIB_DIRECTORY} NO_DEFAULT_PATH)

include_directories("${HALIDE_SOURCE_DIRECTORY}/include" "${HALIDE_SOURCE_DIRECTORY}/tools")

message( "SOURCES = " ${SOURCES})

add_executable(${TARGET} main.cpp ${SOURCES})
target_link_libraries( ${TARGET} ${HalideLib} -lpthread -lpng ${CMAKE_DL_LIBS}) 