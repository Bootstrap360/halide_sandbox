#pragma once

#include "Halide.h"

#include <string>

namespace HalideCV
{


    Halide::Func scale(const Halide::Buffer<uint8_t>& input, float factor, std::string name = "scale")
    {
        Halide::Func scale_func(name);
        Halide::Var x, y, c;
        Halide::Expr value = input(x, y, c);
        value = Halide::cast<float>(value);
        value = value * factor;
        value = Halide::min(value, 255.0f);
        value = Halide::cast<uint8_t>(value);
        scale_func(x, y, c) = value;
        return scale_func;
    }

    Halide::Expr scale(Halide::Expr& value, float factor)
    {
        return value * factor;
    }

    // Halide::Func remap( Halide::Func& src, Halide::Func& dest, Halide::Func& map_x, Halide::Func& map_y , Var& x, Var& y, Var& c)
    // {
    //     dest(x, y, c) = src( map_x(x,y), map_y(x, y), c);
    //     return dest;
    // }

    Halide::Func remap( const Halide::Func& src, Halide::Func& dest, const Halide::Func& map_x, const Halide::Func& map_y , Var& x, Var& y, Var& c)
    {
        dest(x, y, c) = src( map_x(x,y), map_y(x, y), c);
        return dest;
    }

    void remap( Halide::Func& rmap_in, Halide::Func& rmap_out, Halide::Buffer<int>& map_x, Halide::Buffer<int>& map_y , Var& x, Var& y, Var& width, Var& height)
    {
        rmap_out(x, y) = rmap_in( clamp(map_x(x, y), 0, width -1 ), clamp(map_y(x, y), 0, height -1 ));
    }

    // Halide::Func remap( Halide::Func& src, Halide::Func& dest, const Halide::Buffer<uint8_t>& map_x, const Halide::Buffer<uint8_t>& map_y , Var& x, Var& y, Var& c)
    // {
    //     dest(x, y, c) = src( map_x(x,y), map_y(x, y), c);
    //     return dest;
    // }


}