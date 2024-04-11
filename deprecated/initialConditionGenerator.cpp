/******************************************************************************
 * \file initialConditionGenerator.cpp
 * 
 * Standalone program, which generates a '.h5' file storing all data from a set
 * of initial conditions, defined below in the main loop. To modify the initial
 * conditions, modify the "// MODIFY" lines
 * 
 * compile with
 * g++ initialConditionGenerator.cpp -lhdf5_cpp -lhdf5
*/

#include <iostream>
#include <cmath>
#include <iomanip> // For std::setw

#include "H5Cpp.h"

// NAME of HDF5 file
const H5std_string fileName("vortex.h5");       // MODIFY

// WILL be passed to HDF5 file
const int Nx = 10;      // MODIFY
const int Ny = 10;      // MODIFY
const int NG = 2;       // MODIFY
const double rho = 1.5; // MODIFY
const double mu = 0.5;  // MODIFY    // viscosity

// NOT passed to HDF5 file
const int totalNx = Nx + 2 * NG;    
const int totalNy = Ny + 2 * NG;
const double xmin = -0.5;       // MODIFY
const double xmax = 0.5;        // MODIFY
const double ymin = -0.5;       // MODIFY
const double ymax = 0.5;        // MODIFY
const double dx = (xmax - xmin) / (Nx - 1);
const double dy = (ymax - ymin) / (Ny - 1); 

int main() {
    // Arrays that will be stored in the HDF5 file
    double u[totalNx][totalNy] = {0};
    double v[totalNx][totalNy] = {0};
    double gx[totalNx][totalNy] = {0};
    double gy[totalNx][totalNy] = {0};
    double p[totalNx][totalNy] = {0};
    
    // creating x and y values
    double x[totalNx] = {0};
    double y[totalNy] = {0};
    

    // A is a constant to decide how fast the initial flow should be
    double A = 1.5;
    // Apparently some compilers don't define pi?
    #ifndef M_PI
        #define M_PI 3.14159265358979323846
    #endif

    for (int j = 0; j < totalNy; j++) {  // Adjusted loop to cover the entire range
        for (int i = 0; i < totalNx; i++) {  // Adjusted loop to cover the entire range
            // Compute x and y coordinates considering ghost cells
            x[i] = xmin + (i - NG) * dx;
            y[j] = ymin + (j - NG) * dy;

            // Initialize vortex only within the physical domain, excluding ghost cells
            if (i >= NG && i < Nx + NG && j >= NG && j < Ny + NG) {
                u[i][j] = -A * sin(M_PI * x[i]) * sin(M_PI * x[i]) * sin(2 * M_PI * y[j]);
                v[i][j] = A * sin(M_PI * y[j]) * sin(M_PI * y[j]) * sin(2 * M_PI * x[i]);
            }

            // Gravity always points down, initialized for the whole domain including ghost cells
            gx[i][j] = 0;
            gy[i][j] = -0.5;

            // Initially uniform pressure, initialized for the whole domain including ghost cells
            p[i][j] = 0.5;
        }
    }

    // Create a new HDF5 file
    H5::H5File file(fileName, H5F_ACC_TRUNC);

    // Data dimensions
    hsize_t dims[2] = {totalNx, totalNy};

    // Create data spaces for the arrays
    H5::DataSpace dataspace(2, dims);

    // Create the 2d datasets
    H5::DataSet dataset_u = file.createDataSet("u", H5::PredType::NATIVE_DOUBLE, dataspace);
    H5::DataSet dataset_v = file.createDataSet("v", H5::PredType::NATIVE_DOUBLE, dataspace);
    H5::DataSet dataset_gx = file.createDataSet("gx", H5::PredType::NATIVE_DOUBLE, dataspace);
    H5::DataSet dataset_gy = file.createDataSet("gy", H5::PredType::NATIVE_DOUBLE, dataspace);
    H5::DataSet dataset_p = file.createDataSet("p", H5::PredType::NATIVE_DOUBLE, dataspace);

    // Data dimensions for 1D datasets
    hsize_t dims_1D_x[1] = {Nx};
    H5::DataSpace dataspace_1D_x(1, dims_1D_x);
    H5::DataSet dataset_x = file.createDataSet("x", H5::PredType::NATIVE_DOUBLE, dataspace_1D_x);

    hsize_t dims_1D_y[1] = {Ny};
    H5::DataSpace dataspace_1D_y(1, dims_1D_y);
    H5::DataSet dataset_y = file.createDataSet("y", H5::PredType::NATIVE_DOUBLE, dataspace_1D_y);

    // Write the arrays to the file
    dataset_u.write(&u[0][0], H5::PredType::NATIVE_DOUBLE);
    dataset_v.write(&v[0][0], H5::PredType::NATIVE_DOUBLE);
    dataset_gx.write(&gx[0][0], H5::PredType::NATIVE_DOUBLE);
    dataset_gy.write(&gy[0][0], H5::PredType::NATIVE_DOUBLE);
    dataset_p.write(&p[0][0], H5::PredType::NATIVE_DOUBLE);
    dataset_x.write(&x[0], H5::PredType::NATIVE_DOUBLE);
    dataset_y.write(&y[0], H5::PredType::NATIVE_DOUBLE);

    // Data dimensions for scalars
    hsize_t dims_scalar[1] = {1};

    // Dataspace for the 1D scalar values
    H5::DataSpace dataspace_scalar(1, dims_scalar);

    // Create the 1D datasets for scalars
    H5::DataSet dataset_rho = file.createDataSet("rho", H5::PredType::NATIVE_DOUBLE, dataspace_scalar);
    H5::DataSet dataset_Nx = file.createDataSet("Nx", H5::PredType::NATIVE_DOUBLE, dataspace_scalar);
    H5::DataSet dataset_Ny = file.createDataSet("Ny", H5::PredType::NATIVE_DOUBLE, dataspace_scalar);
    H5::DataSet dataset_NG = file.createDataSet("NG", H5::PredType::NATIVE_DOUBLE, dataspace_scalar);
    H5::DataSet dataset_mu = file.createDataSet("mu", H5::PredType::NATIVE_DOUBLE, dataspace_scalar);

    // Write the scalar values to their respective datasets
    dataset_rho.write(&rho, H5::PredType::NATIVE_DOUBLE);
    dataset_Nx.write(&Nx, H5::PredType::NATIVE_DOUBLE);
    dataset_Ny.write(&Ny, H5::PredType::NATIVE_DOUBLE);
    dataset_NG.write(&NG, H5::PredType::NATIVE_DOUBLE);
    dataset_mu.write(&mu, H5::PredType::NATIVE_DOUBLE);

        for (int i = NG; i < Nx+NG; ++i) {
            for (int j = NG; j < Ny+NG; ++j) {
                std::cout << std::setw(5) << u[i][j] << " ";
            }
            std::cout << std::endl; // New line at the end of each row
        }

    // Close the datasets and file
    dataset_u.close();
    dataset_v.close();
    dataset_gx.close();
    dataset_gy.close();
    dataset_p.close();
    dataset_rho.close();
    dataset_Nx.close();
    dataset_Ny.close();
    dataset_NG.close();
    dataset_mu.close();
    dataset_x.close();
    dataset_y.close();
    file.close();

    return 0;
}