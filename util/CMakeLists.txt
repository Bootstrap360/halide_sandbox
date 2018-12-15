
cmake_minimum_required (VERSION 3.5.1)
set (CMAKE_CXX_STANDARD 11)

project ( halide_util )

set ( HALIDE_SOURCE_DIRECTORY "$ENV{HOME}/repositories/halide_build" )
set ( HALIDE_LIB_DIRECTORY "$ENV{HOME}/repositories/halide_build/lib" )
set ( HALIDE_GIT "$ENV{HOME}/repositories/Halide" )

find_library(HalideLib Halide PATHS ${HALIDE_LIB_DIRECTORY} NO_DEFAULT_PATH)

include_directories("${HALIDE_SOURCE_DIRECTORY}/include" "${HALIDE_GIT}/tools")


add_executable(HalideTraceViz HalideTraceViz.cpp)
find_package( PNG )
target_link_libraries( HalideTraceViz ${HalideLib} -lpthread -lpng ${CMAKE_DL_LIBS} -lz -ljpeg)


add_executable( HalideTraceDump HalideTraceDump.cpp HalideTraceUtils.cpp)
find_package( PNG )
target_link_libraries(  HalideTraceDump ${HalideLib} -lpthread -lpng ${CMAKE_DL_LIBS} -lz -ljpeg) 




# halide_project(HalideTraceViz "utils" HalideTraceViz.cpp)
# halide_project(HalideTraceDump "utils" HalideTraceDump.cpp HalideTraceUtils.cpp)
# halide_use_image_io(HalideTraceDump)
