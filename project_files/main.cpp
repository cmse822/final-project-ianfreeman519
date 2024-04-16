/******************************************************************************
 * \file initialConditionGenerator.cpp
 * 
 * Main program for navier stokes FDM solver.
 * 
 * compile with
 * g++ main.cpp -o simulateNS
*/

#include <iostream>
#include <iomanip> // For std::setw
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cmath>

// Directory where we will pull data from csv
const std::string directory_name = "laminar";

// Function to read array from the csv files generated by initialConditionGenerator.ipynb
// Function to read a CSV file into a 2D vector
std::vector<std::vector<double>> readCsv(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::vector<std::vector<double>> matrix;
    bool isOneDimensional = true;

    // First pass: check if the file contains more than one line
    while (std::getline(file, line)) {
        if (!line.empty()) {  // Ensure the line is not empty
            std::vector<double> row;
            std::stringstream ss(line);
            std::string value;

            while (std::getline(ss, value, ',')) {
                row.push_back(std::stod(value));
            }

            matrix.push_back(row);

            // If we've read more than one line, it's not one-dimensional
            if (matrix.size() > 1) {
                isOneDimensional = false;
                break;
            }
        }
    }
    // If the file is one-dimensional, wrap the vector in another vector to maintain consistency
    if (isOneDimensional && !matrix.empty()) {
        std::vector<double> row = matrix[0];
        matrix.clear();  // Clear the existing content
        matrix.push_back(row);  // Push the 1D vector as a single row of the 2D vector
    }
    // If it's not one-dimensional, continue reading the rest of the file
    if (!isOneDimensional) {
        while (std::getline(file, line)) {
            if (!line.empty()) {  // Ensure the line is not empty
                std::vector<double> row;
                std::stringstream ss(line);
                std::string value;
                while (std::getline(ss, value, ',')) {
                    row.push_back(std::stod(value));
                }
                matrix.push_back(row);
            }
        }
    }
    return matrix;
}

// Function to read scalars from a file into a map
std::map<std::string, double> readScalars(const std::string& filename) {
    std::map<std::string, double> scalars;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        size_t delimiterPos = line.find('=');
        std::string key = line.substr(0, delimiterPos);
        double value = std::stod(line.substr(delimiterPos + 1));
        scalars[key] = value;
    }

    return scalars;
}

// Function to write vectors to a csv file
void writeToCsv(const std::vector<std::vector<double>>& u, const std::string& filePath) {
    
    std::ofstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
        return;
    }

    for (const auto& row : u) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}


int main() {
    /**************************************************************************
     * STEP 1:
     * Read data from directory into arrays to use in program
     * This uses the 
    **************************************************************************/
    std::cout << std::setprecision(2);
    std::cout << std::scientific;
    std::cout << "Starting Simulation" << std::endl << std::endl << std::endl << std::endl;

    // Read 2D data from CSV files
    auto u = readCsv(directory_name + "/u.csv");
    auto v = readCsv(directory_name + "/v.csv");
    auto u_new = readCsv(directory_name + "/u.csv");
    auto v_new = readCsv(directory_name + "/v.csv");
    auto p = readCsv(directory_name + "/p.csv");
    auto gx = readCsv(directory_name + "/gx.csv");
    auto gy = readCsv(directory_name + "/gy.csv");

    // Read scalar data from a file
    auto scalars = readScalars(directory_name + "/scalars.dat");

    // Access scalar values
    double rho = scalars["rho"];        double mu = scalars["mu"];
    double xmin = scalars["xmin"];      double ymin = scalars["ymin"];
    double xmax = scalars["xmax"];      double ymax = scalars["ymax"];
    double dx = scalars["dx"];          double dy = scalars["dy"];
    double tmax = scalars["tmax"];      double dt = scalars["dt"];
    int Nx = static_cast<int>(scalars["Nx"]);
    int Ny = static_cast<int>(scalars["Ny"]);
    int NG = static_cast<int>(scalars["NG"]);
    // Geometric mean of active sim space for calculating Re
    double L = sqrt((xmax - xmin)*(ymax-ymin)); 

    // This is hack-ish, but I'm just trying to get it to work
    // Read x and y as 2d vectors
    auto x_temp = readCsv(directory_name + "/x.csv");
    auto y_temp = readCsv(directory_name + "/y.csv");
    // Cast them into 1d
    std::vector<double> x (Nx + 2*NG);
    std::vector<double> y (Ny + 2*NG);
    for (int i=0; i<Nx + 2*NG; i++) x[i]=x_temp[0][i];
    for (int j=0; j<Ny + 2*NG; j++) y[j]=y_temp[0][j];

    /**************************************************************************
     * STEP 2:
     * Fields have been initialized, so now we get to start the iteration! :(
    **************************************************************************/        
    // Naively, I need to store different terms of the updating scheme in different variables
    double ut1, ut2, ut3, ut4, ut5;
    double vt1, vt2, vt3, vt4, vt5;
    double invRe;

    // time - to be iterated
    double t = 0; int ti = 0;
    // Useful sign variable for upwind scheme
    int s;

    // Writing the initial conditions so they are all in the same spot
    writeToCsv(u, "output/u" + std::to_string(ti) + ".csv");
    writeToCsv(v, "output/v" + std::to_string(ti) + ".csv");

    while (t <= tmax)
    {   
        std::cout << "Simulating timestep " << ti << " at time " << t << std::endl;
        // First update the boundary conditions to fill in the ghost zones
        // along interior x points
        for (int i=NG; i<Nx+NG; i++){
            // y=0, along interior x points
            for (int j=0; j<NG; j++) {
                u[i][j] = u[i][NG];
                v[i][j] = v[i][NG];
                std::cout << "1 i=" << i << " | j=" << j << " | u=" << u[i][j] << " |      u[i][NG]=" << u[i][NG] << " | v=" << v[i][j] << " |      v[i][NG]=" << v[i][NG] << std::endl;
            }
            // y=ymax, along interior x points
            for (int j=Ny+NG; j<Ny+2*NG; j++) {
                u[i][j] = u[i][Ny+NG-1];
                v[i][j] = v[i][Ny+NG-1];
                std::cout << "1 i=" << i << " | j=" << j << " | u=" << u[i][j] << " | u[i][Ny+NG-1]=" << u[i][Ny+NG-1] << " | v=" << v[i][j] << " | v[i][Ny+NG-1]=" << v[i][Ny+NG-1] << std::endl;
            }
        }
        // along all y points - can handle
        for (int j=0; j<Ny+2*NG; j++) {
            // x = 0, along ALL y points
            for (int i=0; i<NG; i++) {
                u[i][j] = u[NG][j]; 
                v[i][j] = v[NG][j];
                std::cout << "1 i=" << i << " | j=" << j << " | u=" << u[i][j] << " |      u[NG][j]=" << u[NG][j] << " | v=" << v[i][j] << " |      v[NG][j]=" << v[NG][j] << std::endl;
            }
            // x = max, along ALL y points
            for (int i=Nx+NG; i<Nx+2*NG; i++) {
                u[i][j] = u[Nx+NG-1][j];
                v[i][j] = v[Nx+NG-1][j];
                std::cout << "1 i=" << i << " | j=" << j << " | u=" << u[i][j] << " | u[Nx+NG-1][j]=" << u[Nx+NG-1][j] << " | v=" << v[i][j] << " | v[Nx+NG-1][j]=" << v[Nx+NG-1][j] << std::endl;
            }
        }

        // Actual iteration
        for (int j=NG; j<Ny+NG; j++){
            for (int i=NG; i<Nx+NG; i++){
                std::cout << "i=" << i << " | j=" << j;

                // 1/Re - inverse reynolds number
                invRe = mu/(rho*sqrt(u[i][j]*u[i][j] + v[i][j]*v[i][j])*L) + 0.1;
                std::cout << " | invRe=" << invRe;
                if (!isinf(invRe) || invRe > 5000.0) invRe = 5000.0;        // Necessary to ensure finite Re
                // Actual stencil - using upwind scheme
                
                // Defining an 's' to represent the sign of velocity
                s = u[i][j] >= 0 ? 1 : -1;
                std::cout << " | us=" << s;
                // dpdx
                ut1 = s/(2*dx)*(3*p[i][j] - 4*p[i-1*s][j] + p[i-2*s][j]);
                std::cout << " | ut1=" << ut1;
                // d2udx2
                ut2 = 1/(dx*dx) * (u[i][j] - 2*u[i-1*s][j] + u[i-2*s][j]);
                std::cout << " | ut2=" << ut2;
                // d2udy2
                ut3 = 1/(dy*dy) * (u[i][j] - 2*u[i][j-1*s] + u[i][j-2*s]);
                std::cout << " | ut3=" << ut3;
                // d(u2)dx
                ut4 = s/(2*dx)*(3*u[i][j]*u[i][j] - 4*u[i-1*s][j]*u[i-1*s][j] + u[i-2*s][j]*u[i-2*s][j]);
                std::cout << " | ut4=" << ut4;
                // d(uv)dy
                ut5 = s/(2*dy)*(3*u[i][j]*v[i][j] - 4*u[i][j-1*s]*v[i][j-1*s] + u[i][j-2*s]*v[i][j-2*s]);
                std::cout << " | ut5=" << ut5;


                // Defining an 's' to represent the sign of velocity
                s = v[i][j] >= 0 ? 1 : -1;
                std::cout << " | vs=" << s;
                // dpdy
                vt1 = s/(2*dy)*(3*p[i][j] - 4*p[i][j-1*s] + p[i][j-2*s]);
                std::cout << " | vt1=" << vt1;
                // d2vdx2
                vt2 = 1/(dx*dx) * (v[i][j] - 2*v[i-1*s][j] + v[i-2*s][j]);
                std::cout << " | vt2=" << vt2;
                // d2vdy2
                vt3 = 1/(dy*dy) * (v[i][j] - 2*v[i][j-1*s] + v[i][j-2*s]);
                std::cout << " | vt3=" << vt3;
                // d(uv)dx
                vt4 = s/(2*dx)*(3*u[i][j]*v[i][j] - 4*u[i-1*s][j]*v[i-1*s][j] + u[i-2*s][j]*v[i-2*s][j]);
                std::cout << " | vt4=" << vt4;
                // d(v2)dy
                vt5 = s/(2*dy)*(3*v[i][j]*v[i][j] - 4*v[i][j-1*s]*v[i][j-1*s] + v[i][j-2*s]*v[i][j-2*s]);
                std::cout << " | vt5=" << vt5;

                // Updating u and v
                v_new[i][j] = v[i][j] + dt*(-vt1 + invRe*(vt2 + vt3) - vt4 - vt5) + gy[i][j];
                u_new[i][j] = u[i][j] + dt*(-ut1 + invRe*(ut2 + ut3) - ut4 - ut5) + gx[i][j];
                std::cout << " | u=" << u_new[i][j] << " | v=" << v_new[i][j] << std::endl; // TODOREMOVE
            }
        }

    // Update time
    t += dt; ti++;

    // Updating the next time step's u and v before writing to file
    for(int j=0; j<Ny+2*NG; j++) {
        for(int i=0; i<Nx+2*NG; i++) {
            u[i][j] = u_new[i][j];
            v[i][j] = v_new[i][j];
        }
    }

    // Output to file
    writeToCsv(u, "output/u" + std::to_string(ti) + ".csv");
    writeToCsv(v, "output/v" + std::to_string(ti) + ".csv");
    }

    return 0;
}