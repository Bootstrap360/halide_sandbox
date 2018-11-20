#pragma once

#include "Halide.h"

namespace Halide
{

Image<ushort> stereoBM(Image<uint8_t> left_image
                , Image<uint8_t> right_image
                , int SADWindowSize
                , int minDisparity
                , int numDisparities
                , int xmin
                , int xmax
                , int ymin
                , int ymax);
                
}