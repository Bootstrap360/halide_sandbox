#pragma once
#include "Halide.h"
#include <stdio.h>
#include "timer.h"

// Halide tutorial lesson 5: Vectorize, parallelize, unroll and tile your code

// This lesson demonstrates how to manipulate the order in which you
// evaluate pixels in a Func, including vectorization,
// parallelization, unrolling, and tiling.

// http://halide-lang.org/tutorials/tutorial_lesson_05_scheduling_1.html

void tutorial_05()
{

    using namespace Halide;
    
    printf("tutorial_05 success!\n");

    // We're going to define and schedule our gradient function in
    // several different ways, and see what order pixels are computed
    // in.

    Var x("x"), y("y");
    Timer t;

    int width = 2058;
    int height = 1536;

    // First we observe the default ordering.
    {
        Func gradient("gradient");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        // By default we walk along the rows and then down the
        // columns. This means x varies quickly, and y varies
        // slowly. x is the column and y is the row, so this is a
        // row-major traversal.
        printf("Evaluating gradient row-major\n");
        Image<int> output = gradient.realize(4, 4);

        // See figures/lesson_05_row_major.gif for a visualization of
        // what this did.

        // The equivalent C is:
        printf("Equivalent C:\n");
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
            }
        }
        printf("\n\n");

        // Tracing is one useful way to understand what a schedule is
        // doing. You can also ask Halide to print out pseudocode
        // showing what loops Halide is generating:
        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");

        // Because we're using the default ordering, it should print:
        // compute gradient:
        //   for y:
        //     for x:
        //       gradient(...) = ...
    }


    // Reorder variables.
    {
        Func gradient("gradient_col_major");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        // If we reorder x and y, we can walk down the columns
        // instead. The reorder call takes the arguments of the func,
        // and sets a new nesting order for the for loops that are
        // generated. The arguments are specified from the innermost
        // loop out, so the following call puts y in the inner loop:
        gradient.reorder(y, x);

        // This means y (the row) will vary quickly, and x (the
        // column) will vary slowly, so this is a column-major
        // traversal.

        printf("Evaluating gradient column-major\n");
        Image<int> output = gradient.realize(4, 4);

        // See figures/lesson_05_col_major.gif for a visualization of
        // what this did.

        printf("Equivalent C:\n");
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
            }
        }
        printf("\n");

        // If we print pseudo-code for this schedule, we'll see that
        // the loop over y is now inside the loop over x.
        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");
    }


}