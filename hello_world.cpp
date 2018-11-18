
#include <iostream>
#include <string>

#include "Halide.h"

#define SHOW(var) std::cout << #var << " = " << var << std::endl

int main(int argc, char **argv)
{
    std::string tmp = "hello world";
    SHOW(tmp);
}