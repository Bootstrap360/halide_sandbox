#pragma once

#include "Halide.h"

#include <string>

namespace HalideCV
{


    Halide::Expr scale(Halide::Expr value, float factor)
    {
        return value * factor;
    }

    // Halide::Expr remap(Halide::Expr input, Halide::Expr map_x, Halide::Expr map_y)
    // {
    //     Func map_x_uint("map_x_uint"), map_y_uint("map_y_uint");
    //     map_x_uint(x, y, c) = cast<uint>(map_x(x, y, c));
    //     map_y_uint(x, y, c) = cast<uint>(map_y(x, y, c));

    //     Expr x_clamped = clamp(x, 0, input.width()-1);
    //     Expr y_clamped = clamp(y, 0, input.height()-1);
    //     clamped(x, y, c) = input(x_clamped, y_clamped, c);

    //     return clamped( map_x_uint(x, y), map_y_uint(x,y), c) ;
    // }


}