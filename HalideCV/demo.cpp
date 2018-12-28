#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"
#include "string"

using namespace Halide;
using namespace Halide::Tools;

#include "HalideCV.h"

#include <sys/time.h>

const std::string image_dir = "/home/glen/repositories/halide_sandbox/images/rgb.png";


#define PRINT(v, a ) print( v(a), std::string(#v) + std::string("("), a, ")", std::string( "[") + std::to_string(__LINE__) + std::string("]") ) ;
#define SHOW(a) std::cout << "[" << __LINE__ <<"] " << #a << " = " << a << std::endl;


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

void remap_1D()
{
    std::cout << "remap_1D" << std::endl;
    // Create read input from disk
    Var x("x");
    Buffer<int> input(2, "input");
    Buffer<float> map_x(2, "map_x");


    input(0) = 10;
    input(1) = 20;

    map_x(0) = 0;
    map_x(1) = 0.3;

    Buffer<float> output(1, "output");

    Func f_input("f_input"), f_input_bounded("f_input_bounded");
    Func f_map_x("f_map_x"), f_map_x_bounded("f_map_x_bounded"), f_map_x_int_bounded("f_map_x_int_bounded");
    Func alpha("alpha");
    Func out("out");

    f_input(x) = input(x);
    f_map_x(x) = map_x(x);

    std::vector< std::pair< Expr, Expr> > boundaryCondition = { { Expr(0), input.dim(0).extent() } };

    // Do some bounds checking
    f_input_bounded  = BoundaryConditions::constant_exterior(f_input, 0, { { Expr(0), input.dim(0).extent() } } );
    // f_map_x_bounded  = BoundaryConditions::constant_exterior(f_map_x, -1.0f, { { Expr(0), map_x.dim(0).extent() } } );
    f_map_x_bounded(x) = f_map_x(x);
    
    f_map_x_int_bounded(x) = cast<int>(f_map_x_bounded(x));

    alpha(x) = select( x < 0, 0.0f
                    , x >= map_x.dim(0).extent(), 0.0f
                    , f_map_x_bounded(x) - cast<float>(f_map_x_int_bounded(x)));

    // alpha(x) = select( x < 0, 0.0f, f_map_x_bounded(x) - cast<float>(f_map_x_int_bounded(x)));

    alpha(x) = PRINT(alpha, x );

    Func beta("beta");
    beta(x) = select( x < 0, 0.0f
                    , x >= map_x.dim(0).extent(), 0.0f
                    , 1.0f - alpha(x) );

    beta(x) = PRINT(beta, x );
    // out(x) = f_input_bounded( f_map_x_int_bounded(x) ) * alpha(x) + f_input_bounded( f_map_x_int_bounded(x) + 1 ) * ( Expr(1.0) - alpha(x));
    out(x) = f_input_bounded( f_map_x_int_bounded(x) ) * alpha(x) + f_input_bounded( f_map_x_int_bounded(x) + 1 ) * beta(x);
    // out(x) = f_input_bounded( f_map_x_int_bounded(x) + 1); //+ (1.0f - alpha(x));

    output = out.realize(5);
    
    for(int i = 0; i < output.width(); ++i)
    {
        SHOW(i);
        SHOW(output(i));
    }

    std::cout << "Done " << __LINE__ << std::endl;

}

void remap_2D()
{

    std::cout << "remap_2D" << std::endl;
    // Create read input from disk
    Var x("x"), y("y");
    Buffer<int> input(2, 2, "input");
    Buffer<float> map_x(2, 2, "map_x");
    Buffer<float> map_y(2, 2, "map_y");

    Func f_input("f_input"), f_input_bounded("f_input_bounded");
    Func f_map_x("f_map_x"), f_map_x_bounded("f_map_x_bounded"), f_map_x_int_bounded("f_map_x_int_bounded");
    Func f_map_y("f_map_y"), f_map_y_bounded("f_map_y_bounded"), f_map_y_int_bounded("f_map_y_int_bounded");

    f_input_bounded  = BoundaryConditions::constant_exterior(f_input, {0, 0}, { { Expr(0), input.dim(0).extent() }, { Expr(0), input.dim(1).extent() } } );

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