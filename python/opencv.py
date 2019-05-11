import halide as hl
import numpy as np
import scipy.misc
import time

# conda install -c conda-forge halide-python
# from https://github.com/halide/Halide/blob/master/python_bindings/tutorial/lesson_01_basics.py


def runAndMeasure(myFunc, shape, nTimes=5):
    L=[]
    output=None
    myFunc.compile_jit()    
    for i in range(nTimes):
        t=time.time()
        output = myFunc.realize(*shape)
        L.append (time.time()-t)
    #print 'running times :', L
    hIm=np.array(output)
    
    mpix=np.size(hIm)/1e6
    print ('best: ', np.min(L), 'average: ', np.mean(L))
    print  ('%.5f ms per megapixel (%.7f ms for %d megapixels)' % (np.mean(L)/mpix*1e3, np.mean(L)*1e3, mpix))
    return np.min(L) 

def imsave(name, func):
    if isinstance(func, hl.Buffer):
        scipy.misc.imsave(name, np.array(func))
    else :
        scipy.misc.imsave(name, func)

def mult(input, scale):

    brighter = hl.Func("mult")
    x, y, c = hl.Var("x"), hl.Var("y"), hl.Var("c")

    value = input[x, y, c]
    value = hl.cast(hl.Float(32), value)
    value = value * scale
    value = hl.min(value, 255.0)
    value = hl.cast(hl.UInt(8), value)

    brighter[x, y, c] = value
    return brighter

def resize_scale(input, fx, fy):
    shr = hl.Func('resize')
    x, y, c = hl.Var("x"), hl.Var("y"), hl.Var("c")
    
    index_x = hl.Func("index_x")
    index_y = hl.Func("index_y")
    index_x.trace_stores()
    index_y.trace_stores()

    index_x[x] =  hl.cast(hl.Int(32), x / fx)
    index_y[y] =  hl.cast(hl.Int(32), y / fy)

    final = hl.Func("final")
    final[x, y, c] = input[index_x[x], index_y[y], c]
    return final

def resize(input, shape = None, fx = None, fy = None):
    if shape is None:
        return resize_scale(input, fx = fx, fy = fy)
    return None


def main():
    # grad = generate_gradient((100, 100))
    # imsave("grad.jpg", grad)

    img_in = scipy.misc.imread('images/kingfisher.jpg')
    print("img_in.shape", img_in.shape)
    input = hl.Buffer(img_in)

    brighten_factor = np.random.uniform(0.1, 2)
    fx = 2
    fy = 2
    
    bright = mult(input, brighten_factor)
    final = resize(bright, fx = fx, fy = fy)
    shape = (int(input.width() * fx), int(input.height() * fy), input.channels())
    output_image = final.realize(*shape)
    final.print_loop_nest()
    print("np.array(output_image).shape", np.array(output_image).shape)
    imsave("output.jpg", output_image)

    runAndMeasure(final, shape, nTimes=5)

if __name__ == "__main__":
    main()