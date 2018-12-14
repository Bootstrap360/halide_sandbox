#!/bin/bash
set -e

make_gif()
{
for f in tmp/frames_*.tif; do convert $f ${f/tif/gif}; done
gifsicle --delay $2 --colors 256 --loop tmp/frames*.gif > tmp/$1
convert -layers Optimize tmp/$1 $1
rm tmp/frames_*if
}

rm -rf tmp/*

echo "Generating Trace"
# grab a trace
# HL_JIT_TARGET=host-cuda 
HL_TRACE=3 HL_TRACE_FILE=$(pwd)/tmp/trace.bin ./build/demo

echo "Generating Videos"

# cat tmp/trace.bin | /home/glen/repositories/halide_sandbox/util/build/HalideTraceViz -s 50 83 -t 10 -d 10000 -h 4 -f simple 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 50x83 -i /dev/stdin -c:v h264 tmp/simple.mp4 &
# cat tmp/trace.bin | /home/glen/repositories/halide_sandbox/util/build/HalideTraceViz -s 50 83 -t 10 -d 10000 -h 4 -f reorder_yxc 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 50x83 -i /dev/stdin -c:v h264 tmp/reorder_yxc.mp4 &
# cat tmp/trace.bin | /home/glen/repositories/halide_sandbox/util/build/HalideTraceViz -s 50 83 -t 10 -d 10000 -h 4 -f reorder_cxy 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 50x83 -i /dev/stdin -c:v h264 tmp/reorder_cxy.mp4 &
# cat tmp/trace.bin | /home/glen/repositories/halide_sandbox/util/build/HalideTraceViz -s 50 83 -t 10 -d 10000 -h 4 -f vectorize_split 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 50x83 -i /dev/stdin -c:v h264 tmp/vectorize_split.mp4 &
# cat tmp/trace.bin | /home/glen/repositories/halide_sandbox/util/build/HalideTraceViz -s 160x160 -t 10 -d 1000 -h 4 -f tile_fuse_parallel 0 255 2 0 10 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 160x160 -i /dev/stdin -c:v h264 tmp/tile_fuse_parallel_1.mp4
# cat tmp/trace.bin | /home/glen/repositories/halide_sandbox/util/build/HalideTraceViz -s 768 1280 -t 5000 -d 10000 -h 4 -f tile_fuse_parallel 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 768x1280 -i /dev/stdin -c:v h264 tmp/tile_fuse_parallel_2.mp4
cat tmp/trace.bin | /home/glen/repositories/halide_sandbox/util/build/HalideTraceViz -s 768 1280 -t 5000 -d 10000 -h 4 -f my_gpu 0 255 2 0 1 2 0 0 1 0 0 1 0 0 | avconv -f rawvideo -pix_fmt bgr32 -s 768x1280 -i /dev/stdin -c:v h264 tmp/my_gpu.mp4
