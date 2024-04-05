#ifndef INPUTREADER_H
#define INPUTREADER_H

#include <string>

// structure to hold the input parameters
struct InputReader {
    int Nx, Ny, block_size;
    double xmin, xmax, ymin, ymax, viscosity, dt, t_max;
};

// Declare the function that reads the configuration from a file
InputReader readInput(const std::string& filename);

#endif