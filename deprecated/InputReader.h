// InputReader.h
/*
Header file for the input files. By default the input text file is input.txt,
but this can be changed in main.cpp.

Notably, this is NOT the initial conditions, instead it reads information from
input.txt relevant to the entire simulation, like Nx, xmin, and viscocity.
Initial conditions should be coded in the Initializer.cpp file.

Units are always going to be in SI
*/

#ifndef INPUTREADER_H
#define INPUTREADER_H

#include <string>

// structure to hold the input parameters
struct InputReader {
    int Nx, Ny, block_size;
    double xmin, xmax, ymin, ymax, viscosity, dt, t_max, density;
};

// Declare the function that reads the configuration from a file
InputReader readInput(const std::string& filename);

#endif