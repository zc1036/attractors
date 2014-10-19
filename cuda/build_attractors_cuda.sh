
/usr/local/cuda-5.5/bin/nvcc -DATTRACTOR_ATOMIC_OPS --compiler-options -Wall -arch=sm_20 -c -G attractor.cu -o attractor.cu.o
#/opt/gcc-4.9.0/bin/
g++ -std=c++0x -c -g -Wall attractor.cu.cpp -o attractor.cu.cpp.o
#/opt/gcc-4.9.0/bin/
g++ -std=c++0x -Wall attractor.cu.o -g attractor.cu.cpp.o -o attractor -L/usr/local/cuda/lib64 -lpng -lcuda -lcudart
