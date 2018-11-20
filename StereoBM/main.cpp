#include "Halide.h"
#include <stdio.h>
#include "halide_image_io.h"

#include <string>


#include "StereoBM.h"

using namespace Halide;
using namespace Halide::Tools;


int main()
{

    Image<uint8_t> left = load_image("/root/repositories/halide_sandbox/images/im0.png");
    Image<uint8_t> right = load_image("/root/repositories/halide_sandbox/images/im1.png");

    for (int numberOfDisparities : {256, 512})
    {
        for(int SADWindowSize : {8, 16, 32, 64})
        {
            int width = left.width(), height = left.height();
            int win2 = SADWindowSize/2;
            int maxDisparity = numberOfDisparities - 1;
            int xmin = maxDisparity + win2;
            int xmax = width - win2 - 1;
            int ymin = win2;
            int ymax = height - win2 - 1;

            Image<ushort> disp_image = stereoBM(left, right, SADWindowSize, 0, (numberOfDisparities-1)/16*16+16, xmin, xmax, ymin, ymax);

            Image<float> scaled_disp(disp_image.width(), disp_image.height());
            for (int y = 0; y < disp_image.height(); y++) {
                for (int x = 0; x < disp_image.width(); x++) {
                    scaled_disp(x, y) = std::min(1.f, std::max(0.f, disp_image(x,y) * 1.0f / maxDisparity));
                }
            };
            
            save_image(scaled_disp, "/root/repositories/halide_sandbox/images/disparity_out_" + std::to_string(numberOfDisparities) + "_" + std::to_string(SADWindowSize) + ".png");
        }
    }
    
    return 0;
}