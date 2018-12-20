#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"
#include "string"

using namespace Halide;
using namespace Halide::Tools;

#include "HalideCV.h"

#include <sys/time.h>
double current_time() {
    static bool first_call = true;
    static timeval reference_time;
    if (first_call) {
        first_call = false;
        gettimeofday(&reference_time, NULL);
        return 0.0;
    } else {
        timeval t;
        gettimeofday(&t, NULL);
        return ((t.tv_sec - reference_time.tv_sec)*1000.0 +
                (t.tv_usec - reference_time.tv_usec)/1000.0);
    }
}

void timeit( Halide::Func &func, Halide::Buffer<uint8_t> &buff)
{
    double best_time = 0.0;
    double total_time = 0.0;
    int num_outer_loops = 3;
    int num_inner_loops = 100;

    std::cout << "Timing " << func.name() << std::endl;
    for (int i = 0; i < num_outer_loops; i++) {

        double t1 = current_time();

        // Run the filter 100 times.
        for (int j = 0; j < num_inner_loops; j++) {
            func.realize(buff);
        }

        // Force any GPU code to finish by copying the buffer back to the CPU.
        buff.copy_to_host();

        double t2 = current_time();

        double elapsed = (t2 - t1)/num_inner_loops;
        if (i == 0 || elapsed < best_time) {
            best_time = elapsed;
        }
        total_time += elapsed;
    }

    printf("%s, best_time = %1.4f milliseconds, average_time = %1.4f\n", func.name().c_str(), best_time, total_time / double(num_outer_loops) );
}

void populateMaps( Halide::Func& map_x, Halide::Func& map_y)
{
    
}

void simple()
{
    Var x("x"), y("y");
    Func f("f"), g("g");
    f(x, y) = x + y;
    // f.realize(4, 4);
    f.trace_loads();
    f.trace_stores();

    g(x, y) = f(x, y) * 2;

    g.realize(4, 4);

}

void indexing_histogram()
{
    Var x("x"), y("y"), i("i"), u("u"), v("v");

    // Create an input with random values.
    Buffer<uint8_t> input(8, 8, "input");
    for (int y = 0; y < input.height(); ++y) {
        for (int x = 0; x < input.width(); ++x) {
            input(x, y) = (rand() % 256);
        }
    }

    Func histogram("hist_serial");
    histogram(i) = 0;
    RDom r(0, input.width(), 0, input.height());
    histogram(input(r.x, r.y) / 32) += 1;

    histogram.vectorize(i, 8);
    histogram.realize(8);

}

void indexing_map()
{

    // Create an input with random values.
    Buffer<uint8_t> input(256, 256, "input");
    Buffer<uint8_t> output(input.width(), input.height(), "output");
    Buffer<int32_t> mapx(input.width(), input.height(), "mapx");
    Buffer<int32_t> mapy(input.width(), input.height(), "mapy");

    for (int i = 0; i < input.height(); ++i) {
        for (int j = 0; j < input.width(); ++j) {
            input(j, i) = double(j + i) / double(input.width() + input.height()) * 255.0;
            mapx(j, i) = (input.width() - j);
            mapy(j, i) = (input.height() - i);
        }
    }

    Var x("x"), y("y"), i("i"), u("u"), v("v");

    Func rmap_out("rmap_out");
    Func rmap_in("rmap_in");

    rmap_in(x, y) = input(x,y);
    RDom r(0, mapx.width(), 0, mapx.height());
    // rmap_out(r.x, r.y) = input(print(select(mapx(r.x,r.y) >= 8, 0, 0)), 0);
    // rmap_out(x, y) = rmap_in( clamp(mapx(x, y), 0, input.width() -1 ), clamp(mapy(x, y), 0, input.height() -1 ));
    HalideCV::remap(rmap_out, rmap_in, mapx, mapy, x, y, input.width(), input.height());

    Var x_outer, y_outer, x_inner, y_inner, tile_index;
    rmap_out.tile(x, y, x_outer, y_outer, x_inner, y_inner, 64, 64);
    rmap_out.fuse(x_outer, y_outer, tile_index);
    rmap_out.parallel(tile_index);
    rmap_out.realize(input.width(), input.height());
    rmap_out.compile_to_lowered_stmt("rmap_out.html", {}, HTML);


}

int main()
{

    indexing_histogram();
    indexing_map();

    // Halide::Buffer<uint8_t> input = load_image("/home/glen/repositories/halide_sandbox/images/rgb.png");
    // Halide::Buffer<uint8_t> output;
    
    // Halide::Var x, y, c, i, ii, xo, yo, xi, yi, tile_index;
    // Halide::Func inputFunc("input");
    // inputFunc(x, y, c) = input(x, y, c);
    // inputFunc.trace_loads();


    // Halide::Expr value = inputFunc(x, y, c);
    // value = Halide::cast<float>(value);
    // value = HalideCV::scale(value, 1.5f);
    // value = Halide::min(value, 255.0f);
    // value = Halide::cast<uint8_t>(value);

    // uint width = input.width();
    // uint height = input.height();

    // {
    //     Halide::Func scale("simple_xyc");
    //     scale(x, y, c) = value;
    //     scale.trace_stores();
    //     Halide::Buffer<uint8_t> output = scale.realize(width, height, input.channels());
    //     // timeit(scale, output);

    // }

    // {
    //     Halide::Func scale("reorder_yxc");
    //     scale(x, y, c) = value;
    //     scale.trace_stores();
    //     scale.reorder(y, x, c);
    //     Halide::Buffer<uint8_t> output = scale.realize(width, height, input.channels());
    // }

    // {
    //     Halide::Func scale("reorder_cxy");
    //     scale(x, y, c) = value;
    //     scale.trace_stores();
    //     scale.reorder(y, x, c);
    //     Halide::Buffer<uint8_t> output = scale.realize(width, height, input.channels());
    // }

    // {
    //     Halide::Func scale("vectorize_split");
    //     scale(x, y, c) = value;
    //     scale.trace_stores();
    //     scale.vectorize(x, 4);
    //     Halide::Buffer<uint8_t> output = scale.realize(width, height, input.channels());
    // }

    // {
    //     Halide::Func scale("tile_fuse_parallel");
    //     scale(x, y, c) = value;
    //     scale.trace_stores();
    //     Var x_outer, y_outer, x_inner, y_inner, tile_index;
    //     scale.tile(x, y, x_outer, y_outer, x_inner, y_inner, 128, 128);
    //     scale.fuse(x_outer, y_outer, tile_index);
    //     scale.parallel(tile_index);

    //     output = scale.realize(width, height, input.channels());
    //     // timeit(scale, output);

    // }

    // {
    //     Halide::Func scale("my_gpu");
    //     scale(x, y, c) = value;
    //     // scale.trace_stores();
    //     scale.gpu_tile(x, y, xo, yo, xi, yi, 8, 8);

    //     // Construct a target that uses the GPU.
    //     Halide::Target target = Halide::get_host_target();
    //     // Halide::Target target = find_gpu_target();
    //     printf("Target: %s\n", target.to_string().c_str());

    //     // Enable OpenCL as the GPU backend.
    //     target.set_feature(Halide::Target::CUDA);
    //     // target.set_feature(Halide::Target::OpenCL);

    //     // Enable debugging so that you can see what OpenCL API calls we do.
    //     target.set_feature(Halide::Target::Debug);
    //     printf("Target: %s\n", target.to_string().c_str());

    //     // JIT-compile the pipeline.
    //     scale.compile_jit(target);

    //     Halide::Buffer<uint8_t> output = scale.realize(width, height, input.channels());

    //     // timeit(scale, output, "GPU_128x_128");


    //     // save_image(output, "output_gpu.png");
    // }


}