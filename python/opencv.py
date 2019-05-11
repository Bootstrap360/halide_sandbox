import halide as hl
import numpy as np

# conda install -c conda-forge halide-python
# from https://github.com/halide/Halide/blob/master/python_bindings/tutorial/lesson_01_basics.py


def generate_gradient(size):
    gradient = hl.Func("gradient")
    x, y = hl.Var("x"), hl.Var("y")
    e = x + y
    assert type(e) == hl.Expr
    gradient[x, y] = e
    output = gradient.realize(size)
    assert output.type() == hl.Int(32)
    for j in range(output.height()):
        for i in range(output.width()):
            # We can access a pixel of an hl.Buffer object using similar
            # syntax to defining and using functions.
            if (output[i, j] != i + j):
                print("Something went wrong!\n"
                       "Pixel %d, %d was supposed to be %d, but instead it's %d\n"
                        % (i, j, i+j, output[i, j]))
                return -1


    # Everything worked! We defined a hl.Func, then called 'realize' on
    # it to generate and run machine code that produced a hl.Buffer.
    print("Success!")

def main():
    grad = np.array(generate_gradient((10, 10)))



if __name__ == "__main__":
    main()