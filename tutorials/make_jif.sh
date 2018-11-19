# HalideTraceViz accepts Halide-generated binary tracing packets from
# stdin, and outputs them as raw 8-bit rgba32 pixel values to
# stdout. You should pipe the output of HalideTraceViz into a video
# encoder or player.

# E.g. to encode a video:
#  HL_TRACE=3 <command to make pipeline> && \
#  HL_TRACE_FILE=/dev/stdout <command to run pipeline> | \
#  HalideTraceViz -s 1920 1080 -t 10000 <the -f args> | \
#  avconv -f rawvideo -pix_fmt bgr32 -s 1920x1080 -i /dev/stdin -c:v h264 output.avi

# To just watch the trace instead of encoding a video replace the last
# line with something like:
#  mplayer -demuxer rawvideo -rawvideo w=1920:h=1080:format=rgba:fps=30 -idle -fixed-vo -

# The arguments to HalideTraceViz are:
#  -s width height: The size of the output frames. Defaults to 1920 x 1080.

#  -t timestep: How many Halide computations should be covered by each
#     frame. Defaults to 10000.
#  -d decay factor: How quickly should the yellow and blue highlights
#     decay over time
#  -h hold frames: How many frames to output after the end of the trace.
#     Defaults to 250.
#  -l func label x y n: When func is first touched, the label appears at
#     the given coordinates and fades in over n frames.

#  For each Func you want to visualize, also specify:
#  -f func_name min_value max_value color_dim blank zoom cost x y strides
#  where
#   func_name: The name of the func or input image. If you have multiple
#     pipelines that use Funcs or the same name, you can optionally
#     prefix this with the name of the containing pipeline like so:
#     pipeline_name:func_name

#   min_value: The minimum value taken on by the Func. Values less than
#     or equal to this will map to black

#   max_value: The maximum value taken on by the Func. Values greater
#     than or equal to this will map to white

#   color_dim: Which dimension of the Func corresponds to color
#     channels. Usually 2. Set it to -1 if you want to visualize the Func
#     as grayscale

#   blank: Should the output occupied by the Func be set to black on end
#     realization events. Zero for no, one for yes.

#   zoom: Each value of the Func will draw as a zoom x zoom box in the output

#   cost: How much time in the output video should storing one value to
#     this Func take. Relative to the timestep.

#   x, y: The position on the screen corresponding to the Func's 0, 0
#     coordinate.

## Example call from generate_figures_5.sh

# HL_TRACE=3 HL_TRACE_FILE=$(pwd)/tmp/trace.bin make -C ../.. tutorial_lesson_05_scheduling_1
# cat tmp/trace.bin | ../../bin/HalideTraceViz -s 192 192 -t 1 -d 10000 -h 4 -f gradient 0 6 -1 0 32 2 32 32 1 0 0 1 | avconv -f rawvideo -pix_fmt bgr32 -s 192x192 -i /dev/stdin tmp/frames_%04d.tif

#   strides: A matrix that maps the coordinates of the Func to screen
#     pixels. Specified column major. For example, 1 0 0 1 0 0
#     specifies that the Func has three dimensions where the
#     first one maps to screen-space x coordinates, the second
#     one maps to screen-space y coordinates, and the third one
#     does not affect screen-space coordinates.


make_gif()
{
for f in tmp/frames_*.tif; do convert $f ${f/tif/gif}; done
gifsicle --delay $2 --colors 256 --loop tmp/frames*.gif > tmp/$1
convert -layers Optimize tmp/$1 $1
rm tmp/frames_*if
}

rm -rf tmp
mkdir tmp

TRACE_FILE=$(pwd)/tmp/trace.bin

HL_TRACE=3 make -C ./build && \
HL_TRACE_FILE=${TRACE_FILE} ./build/tutorials


# min_value max_value color_dim blank zoom cost x y strides
MIN_VAL=-1
MAX_VAL=6
COLOR_DIM=-1
BLANK=0
ZOOM=10
COST_T=2
X=${ZOOM}
Y=${ZOOM}
STRIDES='1 0 0 1'
HEIGHT=$((12 * ${ZOOM}))
WIDTH=$((12 * ${ZOOM}))

DELAY=2

FUNC_NAME=gradient

cat ${TRACE_FILE} | ../util/build/HalideTraceViz -s $WIDTH $HEIGHT -t 1 -d 10000 -h 4 \
-f ${FUNC_NAME} ${MIN_VAL} ${MAX_VAL} ${COLOR_DIM} ${BLANK} ${ZOOM} ${COST_T} ${X} ${Y} ${STRIDES} | avconv -f rawvideo -pix_fmt bgr32 -s ${WIDTH}x${HEIGHT} -i /dev/stdin tmp/frames_%04d.tif
make_gif ${FUNC_NAME}.gif ${DELAY}

FUNC_NAME=gradient_col_major
cat ${TRACE_FILE} | ../util/build/HalideTraceViz -s $WIDTH $HEIGHT -t 1 -d 10000 -h 4 \
-f ${FUNC_NAME} ${MIN_VAL} ${MAX_VAL} ${COLOR_DIM} ${BLANK} ${ZOOM} ${COST_T} ${X} ${Y} ${STRIDES} | avconv -f rawvideo -pix_fmt bgr32 -s ${WIDTH}x${HEIGHT} -i /dev/stdin tmp/frames_%04d.tif
make_gif ${FUNC_NAME}.gif ${DELAY}

FUNC_NAME=gradient_in_vectors
MAX_VAL=11
cat ${TRACE_FILE} | ../util/build/HalideTraceViz -s $WIDTH $HEIGHT -t 1 -d 10000 -h 4 \
-f ${FUNC_NAME} ${MIN_VAL} ${MAX_VAL} ${COLOR_DIM} ${BLANK} ${ZOOM} ${COST_T} ${X} ${Y} ${STRIDES} | avconv -f rawvideo -pix_fmt bgr32 -s ${WIDTH}x${HEIGHT} -i /dev/stdin tmp/frames_%04d.tif
make_gif ${FUNC_NAME}.gif ${DELAY}

echo "DONE"