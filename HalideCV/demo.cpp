#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"
#include "string"

using namespace Halide;
using namespace Halide::Tools;

#include "HalideCV.h"

// Target find_gpu_target() {
//     // Start with a target suitable for the machine you're running this on.
//     Target target = get_host_target();

//     // Uncomment the following lines to try CUDA instead:
//     // target.set_feature(Target::CUDA);
//     // return target;

// #ifdef _WIN32
//     if (LoadLibrary("d3d12.dll") != NULL) {
//         target.set_feature(Target::D3D12Compute);
//     } else if (LoadLibrary("OpenCL.dll") != NULL) {
//         target.set_feature(Target::OpenCL);
//     }
// #elif __APPLE__
//     // OS X doesn't update its OpenCL drivers, so they tend to be broken.
//     // CUDA would also be a fine choice on machines with NVidia GPUs.
//     if (dlopen("/System/Library/Frameworks/Metal.framework/Versions/Current/Metal", RTLD_LAZY) != NULL) {
//         target.set_feature(Target::Metal);
//     }
// #else
//     if (dlopen("libOpenCL.so", RTLD_LAZY) != NULL) {
//         target.set_feature(Target::OpenCL);
//     }
// #endif

//     return target;
// }

int main()
{

    Image<uint8_t> input = load_image("/home/glen/repositories/halide_sandbox/images/rgb.png");
    Halide::Buffer<uint8_t> output;
    
    Halide::Var x, y, c, i, ii, xo, yo, xi, yi, tile_index;

    
    Halide::Func inputFunc("inputFunc");
    inputFunc(x, y, c) = input(x, y, c);
    inputFunc.trace_loads();

    Halide::Expr value = inputFunc(x, y, c);
    value = Halide::cast<float>(value);
    value = value * 1.5f;
    value = Halide::min(value, 255.0f);
    value = Halide::cast<uint8_t>(value);

    uint width = input.width();
    uint height = input.height();

    // {
    //     Halide::Func scale("simple");
    //     scale(x, y, c) = value;
    //     scale.trace_stores();
    //     scale.reorder(y, x, c);
    //     Halide::Buffer<uint8_t> output = scale.realize(width, height, input.channels());
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

    //     Halide::Buffer<uint8_t> output = scale.realize(width, height, input.channels());
    // }

    {
        Halide::Func scale("my_gpu");
        scale(x, y, c) = value;
        // scale.trace_stores();
        scale.gpu_tile(x, y, 128, 128);

        // Construct a target that uses the GPU.
        Halide::Target target = Halide::get_host_target();
        // Halide::Target target = find_gpu_target();
        printf("Target: %s\n", target.to_string().c_str());

        // Enable OpenCL as the GPU backend.
        target.set_feature(Halide::Target::CUDA);
        // target.set_feature(Halide::Target::OpenCL);

        // Enable debugging so that you can see what OpenCL API calls we do.
        target.set_feature(Halide::Target::Debug);
        printf("Target: %s\n", target.to_string().c_str());

        // JIT-compile the pipeline.
        scale.compile_jit(target);

        Halide::Buffer<uint8_t> output = scale.realize(width, height, input.channels());
    }

    // save_image(output, "output.png");

}