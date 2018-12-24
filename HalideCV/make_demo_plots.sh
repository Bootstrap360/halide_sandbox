#!/bin/bash
set -e

mkdir -p tmp
rm -rf tmp/*

rm *.jpg &

echo "Making it"

HL_JIT_TARGET=host-trace_all HL_TARGET=host-trace_all make -C build

echo "Generating Trace"
# grab a trace
# HL_JIT_TARGET=host-cuda
HL_JIT_TARGET=host-trace_all HL_TRACE_FILE=$(pwd)/tmp/trace.bin ./build/demo

echo "Generating Function Images"
../util/build/HalideTraceDump -i tmp/trace.bin -t jpg

# echo "Generating Videos"

# zoom=2
# img_width=$(( 256 * ${zoom} * 2 + 32))
# img_height=$(( 256 * ${zoom} + 32))

# cat tmp/trace.bin | ~/repositories/halide_sandbox/util/build/HalideTraceViz --size ${img_width} ${img_height} --timestep 1000 --decay 2 300 --zoom ${zoom} --hold 50 \
# --gray --min 0 --max 256 --uninit 50 50 100 \
# --move 0 0 --func 'rmap_out:input' \
# --right  $((256 * ${zoom} + 32)) --func 'rmap_out' | \
# avconv -f rawvideo -pix_fmt bgr32 -s ${img_width}x${img_height} -i /dev/stdin -c:v h264 tmp/rmap_out.mp4 
