#include <iostream>

// other scripts
#include "InputReader.h"

/*
To compile this for serial execution, it may be hack-ish and poor form, 
but I run the following in the terminal:
    g++ *.cpp -o run.exe
When I do NOT do this, the compilation does not include functions outside 
of the main.cpp file.
*/

int main() {
    /*
        STEP 1: Read input file from input.txt
    */
    try {
        // Communicating to user
        std::cout << "Attempting to read input from input.txt" << std::endl;
        
        // Specify the path to your input file
        InputReader input = readInput("input.txt"); 
        
        // Storing the values
        const int Nx = input.Nx;
        const int Ny = input.Nx;
        const double xmin = input.Nx;
        const double xmax = input.Nx;
        const double ymin = input.Nx;
        const double ymax = input.Nx;
        const double nu = input.viscosity;
        const int block_size = input.block_size;
        const double dt = input.dt;
        const double t_max = input.t_max;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}