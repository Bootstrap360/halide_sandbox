cmake_minimum_required (VERSION 3.5.1)
set (CMAKE_CXX_STANDARD 11)

set ( TARGET demo)
project ( ${TARGET} )

set ( HALIDE_SOURCE_DIRECTORY "$ENV{HOME}/repositories/halide_build" )
set ( HALIDE_LIB_DIRECTORY "$ENV{HOME}/repositories/halide_build/lib" )
set ( HALIDE_GIT "$ENV{HOME}/repositories/Halide" )


file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
    "${HALIDE_GIT}/tools")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")


find_library(HalideLib Halide PATHS ${HALIDE_LIB_DIRECTORY} NO_DEFAULT_PATH)

include_directories("${HALIDE_SOURCE_DIRECTORY}/include" "${HALIDE_GIT}/tools")


add_executable(demo demo.cpp ${SOURCES})
find_package(PNG)
target_link_libraries( demo ${HalideLib} -lpthread -lpng ${CMAKE_DL_LIBS} -lz -ljpeg) 