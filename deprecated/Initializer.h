// Initializer.h

/*
Header file for the initial conditions file
*/
#ifndef INITIALIZER_H
#define INITIALIZER_H

void initializeArray(double* array, int Nx, int Ny, double (*initConditionFunc)(double, double));

#endif // INITIALIZER_H