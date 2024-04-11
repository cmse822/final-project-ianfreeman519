// Initializer.cpp

/*
This is where the ACTUAL initial conditions are set. As of now, only one initial condition
is implemented, but this can be quickly changed by redefining initConditionFunc
*/

#include "Initializer.h"

void initializeArray(double* array, int Nx, int Ny, double* x, double*y, double (*initConditionFunc)(double, double)) {
    for (int i = 0; i < Nx; i++) {
        for (int j = 0; j < Ny; j++) {
            array[i * Ny + j] = initConditionFunc(x[i], y[j]);
        }
    }
}

// Initializing arrays as 0
double zeros_init(double x, double y) {
    return 0; // The initial condition function: x + y
}

// Short example to initially test pressure profiles
double x_plus_y_init(double x, double y) {
    return x + y; // The initial condition function: x + y
}