#!/bin/bash
set -e

mkdir -p tmp
rm -rf tmp/*

echo "Making it"

HL_TARGET=host-trace_all make -C build

echo "Generating Trace"
# grab a trace
# HL_JIT_TARGET=host-cuda
HL_TRACE_FILE=$(pwd)/tmp/trace.bin ./build/demo

# echo "Generating Function Images"
# ../util/build/HalideTraceDump -i tmp/trace.bin -t jpg

echo "Generating Videos"
cat tmp/trace.bin | ~/repositories/halide_sandbox/util/build/HalideTraceViz --size 768 1280 --timestep 10000 --decay 1 300 \
--rgb 2 \
--func simple_xyc | \
avconv -f rawvideo -pix_fmt bgr32 -s 768x1280 -i /dev/stdin -c:v h264 tmp/simple_xyc.mp4 


## OLD
# # cat tmp/trace.bin | ~/repositories/halide_sandbox/util/build/HalideTraceViz -s 768 1280 -t 5000 -d 10000 -h 4 -f simple_xyc 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 768x1280 -i /dev/stdin -c:v h264 tmp/simple_xyc.mp4 
# # cat tmp/trace.bin | ~/repositories/halide_sandbox/util/build/HalideTraceViz -s 50 83 -t 10 -d 10000 -h 4 -f reorder_yxc 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 50x83 -i /dev/stdin -c:v h264 tmp/reorder_yxc.mp4 &
# # cat tmp/trace.bin | ~/repositories/halide_sandbox/util/build/HalideTraceViz -s 50 83 -t 10 -d 10000 -h 4 -f reorder_cxy 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 50x83 -i /dev/stdin -c:v h264 tmp/reorder_cxy.mp4 &
# # cat tmp/trace.bin | ~/repositories/halide_sandbox/util/build/HalideTraceViz -s 50 83 -t 10 -d 10000 -h 4 -f vectorize_split 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 50x83 -i /dev/stdin -c:v h264 tmp/vectorize_split.mp4 &
# # cat tmp/trace.bin | ~/repositories/halide_sandbox/util/build/HalideTraceViz -s 160x160 -t 10 -d 1000 -h 4 -f tile_fuse_parallel 0 255 2 0 10 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 160x160 -i /dev/stdin -c:v h264 tmp/tile_fuse_parallel_1.mp4
# cat tmp/trace.bin | ~/repositories/halide_sandbox/util/build/HalideTraceViz -s 768 1280 -t 5000 -d 10000 -h 4 -f tile_fuse_parallel 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 768x1280 -i /dev/stdin -c:v h264 tmp/tile_fuse_parallel.mp4
# # cat tmp/trace.bin | ~/repositories/halide_sandbox/util/build/HalideTraceViz -s 768 1280 -t 5000 -d 10000 -h 4 -f my_gpu 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 768x1280 -i /dev/stdin -c:v h264 tmp/my_gpu.mp4
