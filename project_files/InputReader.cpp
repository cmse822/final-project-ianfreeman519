#include "InputReader.h"
#include <fstream>
#include <sstream>
#include <cstdio>

InputReader readInput(const std::string& filename) {
    std::ifstream inputFile(filename);
    InputReader input{}; // Initializing the config input

    if (!inputFile.is_open()) {
        throw std::runtime_error("Failed to open input file: " + filename);
    }

    std::string line;
    while (std::getline(inputFile, line)) {     // keep reading the file, storing each line in 'line'
        std::istringstream lineStream(line);    // Grabbing the next line in lineStream object
        std::string key;                        // LHS of 'x=y'
        if (std::getline(lineStream, key, '=')) {   // reads line of input until '=' is hit, stores it into 'key'
            double value;
            lineStream >> value;        // Remainder of line is stored in 'value'

            if (key == "Nx") input.Nx = static_cast<int>(value);
            else if (key == "Ny") input.Ny = static_cast<int>(value);
            else if (key == "xmin") input.xmin = value;
            else if (key == "xmax") input.xmax = value;
            else if (key == "ymin") input.ymin = value;
            else if (key == "ymax") input.ymax = value;
            else if (key == "block_size") input.block_size = static_cast<int>(value);
            else if (key == "dt") input.dt = value;
            else if (key == "t_max") input.t_max = value;
            else if (key == "viscosity") input.viscosity = value;
        }
    }

    // Error handling so I don't have to ever debug this again
    if (input.Nx % input.block_size != 0 && input.Ny % input.block_size != 0) {
        throw std::runtime_error("Block_size must be divisible into the domain mesh in file " + filename);
    }
    if (input.xmin >= input.xmax || input.ymin > input.ymax != 0) {
        throw std::runtime_error("xmin and ymin must be < xmax and ymax in file " + filename);
    }
    if (input.dt >= 0.5 * input.t_max) {
        throw std::runtime_error("dt must be at most half of t_max in file " + filename);
    }

    // Printing out input values for user verification
    printf("\nNx = %i | Ny = %i | block_size = %i", input.Nx, input.Ny, input.block_size);
    printf("\nxmin:xmax = %f:%f | ymin:ymax = %f:%f", input.xmin, input.xmax, input.ymin, input.ymax);
    printf("\ndt = %f | t_max = %f", input.dt, input.t_max);
    printf("\nviscosity = %f", input.viscosity);
    printf("\n");

    return input; // Return the populated input structure
}