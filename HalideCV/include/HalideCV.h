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


}