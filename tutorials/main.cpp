
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#define SHOW(var) std::cout << #var << " = " << var << std::endl


#include "01.h"
#include "02.h"
#include "03.h"
#include "04.h"
#include "05.h"


int main(int argc, char **argv)
{
    // lessons can be found here http://halide-lang.org/tutorials/tutorial_introduction.html

    // tutorial_01();
    
    // for( auto gain : {0.2, 0.5, 1.5, 10.0} )
    // {
    //     tutorial_02(gain);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(300));
    // }
    // tutorial_03();

    // tutorial_04();
    tutorial_05();


    return 0;

}