#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"
#include "string"

using namespace Halide;
using namespace Halide::Tools;

#include "HalideCV.h"

#include <sys/time.h>

const std::string image_dir = "/home/glen/repositories/halide_sandbox/images/rgb.png";


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
    Buffer<uint8_t> input(8, 8, "histogram_input");
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

void remap_floor()
{
    // This method truncates the indexes of mapx and mapy to int32s.

    // Create read input from disk
    Buffer<uint8_t> input = load_image(image_dir);
    Buffer<int32_t> mapx(input.width(), input.height(), "mapx");
    Buffer<int32_t> mapy(input.width(), input.height(), "mapy");

    // create a map that flips vertically and horizontally
    int noise = 16;
    for (int i = 0; i < input.height(); ++i) {
        for (int j = 0; j < input.width(); ++j) {
            mapx(j, i) = (input.width() - j + (rand() % noise) - noise / 2 );
            mapy(j, i) = (input.height() - i + (rand() % noise) - noise / 2 );
        }
    }

    Var x("x"), y("y"), c("c"), i("i"), u("u"), v("v");

    Func rmap_out("rmap_out");

    // First, add a boundary condition to the input.
    Func clamped;
    Expr x_clamped = clamp(x, 0, input.width()-1);
    Expr y_clamped = clamp(y, 0, input.height()-1);
    clamped(x, y, c) = input(x_clamped, y_clamped, c);

    rmap_out(x, y, c) = clamped( mapx(x, y), clamp( mapy(x, y), 0, input.height() -1 ), c);
    rmap_out.realize( input.width(), input.height(), input.channels() );

}

void remap_1D_1()
{
    // This method truncates the indexes of mapx and mapy to int32s.
    std::cout << "remap_1D" << std::endl;
    // Create read input from disk
    Buffer<uint8_t> input(2, "input");
    Buffer<uint8_t> output(input.width() - 1, "output");
    Buffer<float> map_x(output.width(), "mapx");

    input(0) = 0;
    input(1) = 1;

    map_x(0) = 0.5;

    Var x("x");

    Func rmap_out("rmap_out");

    // First, add a boundary condition to the input.
    Func clamped("float_clamped");
    Func map_x_int;
    map_x_int(x) = cast<int>(map_x(x));

    map_x_int(x) = print(map_x_int(x), " map_xint(", x,")", std::to_string(__LINE__));
    Func ta("ta"), tb("tb"), b("b"), m("m");

    Expr x_clamped = clamp(x, 0, input.width()-1);
    clamped(x) = input(x_clamped);

    ta(x) = clamped( map_x_int(x) );
    tb(x) = clamped( map_x_int(x) + 1 );

    ta(x) = print(ta(x), " ta(", x, ")", std::to_string(__LINE__));
    tb(x) = print(tb(x), " tb(", x, ")", std::to_string(__LINE__));

    m(x) = tb(x) - ta(x);
    m(x) = print(m(x), " m(", x, ")", std::to_string(__LINE__));

    b(x) = ta(x) - m(x) * map_x_int(x);

    b(x) = print(b(x), " b(", x, ")", std::to_string(__LINE__));
    Func tmp;
    tmp(x) = print(m(x) * map_x(x), "m(", x, ") * map_x(", x,")");
    Func tmp2;
    tmp2(x) = print(tmp(x) + b(x), "tmp(", x, ") + b(", x, ")");
    rmap_out(x) = cast<int>(tmp2(x));

    // Func tmp3;
    // tmp3 = m(x) * map_x_int(x);
    // tmp3.realize( 1 );

    // for(auto o : output)
    // {
    //     std::cout << o << std::endl;
    // }

    std::cout << "Done " << __LINE__ << std::endl;

}

void remap_1D()
{
    // This method truncates the indexes of mapx and mapy to int32s.
    // This method truncates the indexes of mapx and mapy to int32s.
    std::cout << "remap_1D" << std::endl;
    // Create read input from disk
    Var x("x");

    Buffer<uint8_t> input(2, "input");
    Buffer<uint8_t> output(input.width() - 1, "output");
    Buffer<float> map_x(output.width(), "mapx");

    input(0) = 0;
    input(1) = 1;

    map_x(0) = 0.5;



    // First, add a boundary condition to the input.
    Func clamped("float_clamped");
    Func rmap_out("rmap_out");
    Func map_x_int;
    map_x_int(x) = cast<int>( map_x(x) );

    map_x_int(x) = print(map_x_int(x), " map_xint(", x,")", std::to_string(__LINE__));
    map_x(x) = print(map_x(x), " map_x(", x,")", std::to_string(__LINE__));
    Func alpha("alpha"), tmp("tmp"), lhs("lhs"), rhs("rhs");

    Expr x_clamped = clamp(x, 0, 2);
    clamped(x) = input(x_clamped);
    clamped(x) = print(clamped(x), " clamped(", x,")", std::to_string(__LINE__));

    alpha(x) = map_x(x) - cast<float>(map_x_int(x));
    alpha(x) = print(alpha(x), " alpha(", x,")", std::to_string(__LINE__));
    

    lhs(x) = clamped( clamp(map_x_int(x), 0, 1) ) * alpha(x);
    lhs(x) = print(lhs(x), " lhs(", x,")", std::to_string(__LINE__));

    rhs(x) = clamped( clamp(map_x_int(x) + 1, 0, 1) ) * (Expr(1.0) - alpha(x));
    rhs(x) = print(rhs(x), " rhs(", x,")", std::to_string(__LINE__));

    tmp(x) = lhs(x) + rhs(x);
    tmp(x) = print(tmp(x), " tmp(", x,")", std::to_string(__LINE__));
    // rmap_out(x) = cast<uint8_t>(tmp(x));
    output = tmp.realize(1);

    // for(int i = 0; i < )
    // {
    //     std::cout << "o = " << o << std::endl;
    // }

    std::cout << "output[0] = " << output(0) << std::endl;

    std::cout << "Done " << __LINE__ << std::endl;

}

void remap_rgb()
{
    // This method truncates the indexes of mapx and mapy to int32s.
    std::cout << "remap" << std::endl;
    // Create read input from disk
    Buffer<uint8_t> input(2, 2, 3, "input");
    Buffer<uint8_t> output(2, 2, 3, "output");
    Buffer<float> mapx(input.width(), input.height(), "mapx");
    Buffer<float> mapy(input.width(), input.height(), "mapy");

    for( int c = 0; c < 3; ++c)
    {
        input(0, 0, c) = 0;
        input(0, 1, c) = 1;
        input(1, 0, c) = 0;
        input(1, 1, c) = 0;
    }

    for (int i = 0; i < input.height(); ++i) {
        for (int j = 0; j < input.width(); ++j) {
            mapx(j, i) = 0;
            mapy(j, i) = 0;
        }
    }

    mapx(0, 1) = 1;
    
    // create a map that flips vertically and horizontally
    // float noiseAmplitude = 0.1;
    // for (int i = 0; i < input.height(); ++i) {
    //     for (int j = 0; j < input.width(); ++j) {
    //         input(i, j, 0) = i;
    //         input(i, j, 1) = i;
    //         input(i, j, 2) = i;
    //         // mapx(j, i) = (j + (((rand() % 255)/255.0 - 0.5) * noiseAmplitude) );
    //         // mapy(j, i) = (i + (((rand() % 255)/255.0 - 0.5) * noiseAmplitude) );
    //         mapx(j, i) = 0;
    //         mapy(j, i) = 0;
    //     }
    // }

    Var x("x"), y("y"), c("c"), i("i"), u("u"), v("v");

    Func rmap_out("rmap_out");

    // First, add a boundary condition to the input.
    Func clamped("float_clamped");
    Func mapx_int, mapy_int;
    mapx_int(x, y) = cast<int>(mapx(x, y));
    mapy_int(x, y) = cast<int>(mapy(x, y));

    mapx_int(x, y) = print(mapx_int(x, y), " map_xint(", x,",", y,")", std::to_string(__LINE__));
    mapy_int(x, y) = print(mapy_int(x, y), " map_yint(", x,",", y,")", std::to_string(__LINE__));
    Func ta("ta"), tb("tb"), b("b"), m("m");

    Expr x_clamped = clamp(x, 0, input.width()-1);
    Expr y_clamped = clamp(y, 0, input.height()-1);
    clamped(x, y, c) = input(x_clamped, y_clamped, c);

    ta(x, y, c) = clamped(mapx_int(x, y), mapy_int(x, y), c);
    tb(x, y, c) = clamped(mapx_int(x, y) + 1, mapy_int(x, y) + 1, c);

    ta(x, y, c) = print(ta(x,y,c), " ta(", x, ",", y, ",", c, ")", std::to_string(__LINE__));
    tb(x, y, c) = print(tb(x,y,c), " tb(", x, ",", y, ",", c, ")", std::to_string(__LINE__));

    m(x, y, c) = tb(x, y, c) - ta(x, y, c);
    m(x, y, c) = print(m(x,y,c), " m(", x, ",", y, ",", c, ")", std::to_string(__LINE__));
    b(x, y, c) = ta(x, y, c) - m(x, y, c) * mapx_int(x, y);
    b(x, y, c) = print(b(x, y, c), " b(", x, ",", y, ",", c, ")", std::to_string(__LINE__));
    rmap_out(x, y, c) = m(x, y, c) * mapx_int(x, y) + b(x, y, c);
    rmap_out.compute_root();
    output = rmap_out.realize( input.width() - 1, input.height() -1, input.channels() );

    // for(auto o : output)
    // {
    //     std::cout << o << std::endl;
    // }

    std::cout << "Done " << __LINE__ << std::endl;

}

Expr myExpr(Expr a, Expr b)
{
    return a + b;
}

void function_sandbox()
{
    Func gradient("gradient");
    Func gradientx2("gradientx2");
    Var x("x"), y("y"), c("c");
    gradient(x, y, c ) = x + y;
    print(gradient(x, y, c ));
    gradientx2(x, y, c) = myExpr(gradient(x, y, c), gradient(x, y, c));

    Buffer<int> output = gradient.realize(8, 8, 3);

    for( auto p: output)
    {
        std::cout << p << std::endl;
    }


}

void blur_kernel()
{
    //int W = 64*3, H = 64*3;
    const int W = 128, H = 48;

    Buffer<uint8_t> in(W, H);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if(x > W / 2)
            {
                in(x, y) = (255);
            }
            else
            {
                in(x, y) = 0;
            }
        }
    }

    Var x("x"), y("y"), c("c");

    Buffer<float> tent(3, 3);
    tent(0, 0) = 1;
    tent(0, 1) = 2;
    tent(0, 2) = 1;
    tent(1, 0) = 2;
    tent(1, 1) = 4;
    tent(1, 2) = 2;
    tent(2, 0) = 1;
    tent(2, 1) = 2;
    tent(2, 2) = 1;

    Func input("input");
    input(x, y) = in(clamp(x, 0, in.width()-1), clamp(y, 0, in.height()-1));
    input.compute_root();

    RDom r(tent);

    Func blur1("blur1");
    blur1(x, y) += tent(r.x, r.y) * input(x + r.x - 1, y + r.y - 1);
    blur1.realize(in.width(), in.height());

}

void blur_kernel2()
{
    //int W = 64*3, H = 64*3;
    const int W = 128, H = 48;

    Buffer<uint8_t> in(W, H);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if(x > W / 2)
            {
                in(x, y) = (255);
            }
            else
            {
                in(x, y) = 0;
            }
        }
    }

    Var x("x"), y("y"), c("c");
    

    // Func input("input");
    // input(x, y) = in(clamp(x, 0, in.width()-1), clamp(y, 0, in.height()-1));
    // input.compute_root();

    // RDom r(5,5);
    // Func kern("kern");

    // kern(r.x, r.y) = exp2( -( pow( r.x, 2 ) + pow(r.y, 2.0) )  )

    // Func blur1("blur1");
    // blur1(x, y) += tent(r.x, r.y) * input(x + r.x - 1, y + r.y - 1);
    // blur1.realize(in.width(), in.height());

}

void derivative()
{

    // Create an input with random values.
    Buffer<uint8_t> input = load_image(image_dir);

    Var x("x"), y("y"), c("c"), i("i"), u("u"), v("v");
    Func derivative("derivative");
    
    Func clamped;
    Expr x_clamped = clamp(x, 0, input.width()-1);
    Expr y_clamped = clamp(y, 0, input.height()-1);
    clamped(x, y, c) = input(x_clamped, y_clamped, c);

    derivative(x, y, c) = clamped( x + 1, y , c ) - clamped(x - 1, y , c );
    derivative.realize( 256, 256, input.channels() );

}

int main()
{

    // indexing_histogram();
    // remap_floor();
    // derivative();
    // blur_kernel();
    // function_sandbox();
    remap_1D();

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