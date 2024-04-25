#include <iostream>

// other scripts
#include "InputReader.h"

/*
To compile this for serial execution, it may be hack-ish and poor form, 
but I run the following in the terminal:
    g++ *.cpp -o run.exe; ./run.exe
When I do NOT do this, the compilation does not include functions outside 
of the main.cpp file.
*/

int main() {
    /*
        STEP 1: Read input file from input.txt
    */
    // Before error handling, initialize values
    int Nx, Ny, block_size;
    double xmin, xmax, ymin, ymax, nu, density, dt, t_max;

    // Always plan for 2 ghost zones, because I don't plan on doing any crazy stencils
    // Number of ghost zones
    const int NG = 2;   

    try {
        // Communicating to user
        std::cout << "Attempting to read input from input.txt" << std::endl;
        
        // Specify the path to your input file
        InputReader input = readInput("input.txt"); 
        
        // Storing the values
        Nx = input.Nx;
        Ny = input.Nx;
        xmin = input.Nx;
        xmax = input.Nx;
        ymin = input.Nx;
        ymax = input.Nx;
        nu = input.viscosity;
        density = input.density;
        block_size = input.block_size;
        dt = input.dt;
        t_max = input.t_max;

    // Basic error check from internet
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // dx and dy are based off of input.txt
    double dx = (xmax - xmin) / (Nx - 1);
    double dy = (ymax - ymin) / (Ny - 1);

    // Creating the x, y, ux, and uy mesh spaces, linearly spaced over min and max values, with two ghost zones
    double* x = new double[Nx + 2*NG];
    double* y = new double[Ny + 2*NG];
    double* ux = new double[Nx + 2*NG];
    double* uy = new double[Ny + 2*NG];

    for (int i = 0; i < Nx ; ++i)
        x[i] = xmin + i * dx;

    for (int j = 0; j < Ny; ++j) 
        y[j] = ymin + j * dy;

    /*
        STEP 2: Initializing velocity fields and pressure fields - since this
        is an implementation of INCOMPRESSIBLE NS equations, density is constant
    */
    double* ux = new double[Nx];
        ux
    return 0;
}