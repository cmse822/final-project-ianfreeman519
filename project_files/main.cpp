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
#include <algorithm>
#include <random>

// Directory where we will pull data from csv
const std::string directory_name = "vortex";
const bool do_outputs = false;
const bool write_velocities = true;
const bool write_tracers = false;
const int outputFrequency = 50;

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

// Function to return the index of the nearest meshpoint in the grid
int findClosestIndex(const std::vector<double>& arr, double value) {
    auto lower = std::lower_bound(arr.begin(), arr.end(), value);
    if (lower == arr.end()) return arr.size() - 1;  // If value is greater than any in array, return last index
    else if (lower == arr.begin()) return 0;  // If value is less than any in array, return first index
    else {
        auto prev = lower - 1;
        if ((value - *prev) <= (*lower - value)) return prev - arr.begin();  // Closer to the previous element
        else return lower - arr.begin();  // Closer to the current element
    }
}


int main() {
    /**************************************************************************
     * STEP 1:
     * Read data from directory into arrays to use in program
     * This uses the 
    **************************************************************************/
    // First, seed the random number generator
    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne-Twister generator (?)

    if(do_outputs) std::cout << std::setprecision(2);
    if(do_outputs) std::cout << std::scientific;
    if(do_outputs) std::cout << "Starting Simulation" << std::endl << std::endl << std::endl << std::endl;

    // Read 2D data from CSV files
    auto u = readCsv(directory_name + "/u.csv");
    auto v = readCsv(directory_name + "/v.csv");
    auto u_new = readCsv(directory_name + "/u.csv");    // these will store the the updated velocity fields
    auto v_new = readCsv(directory_name + "/v.csv");
    auto p = readCsv(directory_name + "/p.csv");
    auto gx = readCsv(directory_name + "/gx.csv");
    auto gy = readCsv(directory_name + "/gy.csv");
    auto Re = readCsv(directory_name + "/u.csv");       // Initialize Re array to save reynold's numbers

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

    // Creating objects for the tracers, so fields are easier to visualize
    int N_tracer = Nx + Ny;     // Number of tracer particles
    // uniform distributors from min+3h to max-3h
    std::uniform_real_distribution <> dis_x(xmin + 3*dx, xmax - 3*dx); 
    std::uniform_real_distribution <> dis_y(ymin + 3*dy, ymax - 3*dy);
    // Tracer position tracker
    std::vector<std::vector<double>> tracer_position(2, std::vector<double>(N_tracer, 0.0));
    for (int j = 0; j < N_tracer; j++) {
        tracer_position[0][j] = dis_x(gen);
        tracer_position[1][j] = dis_y(gen);
    }
    // Output initial tracer positions
    if (write_tracers) writeToCsv(tracer_position, "output/tracer0.csv");


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
    int sx, sy;

    // Calculating the first reynolds number iteration
    for (int j=NG; j<Ny+NG; j++){
        for (int i=NG; i<Nx+NG; i++){
            Re[i][j] = ((rho*sqrt(u[i][j]*u[i][j] + v[i][j]*v[i][j])*L) + 0.1)/mu;
        }
    }

    // Writing the initial conditions so they are all in the same spot
    if (write_velocities) {
        writeToCsv(u, "output/u" + std::to_string(ti) + ".csv");
        writeToCsv(v, "output/v" + std::to_string(ti) + ".csv");
        writeToCsv(Re, "output/Re" + std::to_string(ti) + ".csv");
    }

    while (t <= tmax)
    {   
        if (ti % outputFrequency == 0) std::cout << "Simulating timestep " << ti << " at time " << t << std::endl;
        // First update the boundary conditions to fill in the ghost zones
        // along interior x points
        for (int i=NG; i<Nx+NG; i++){
            // y=0, along interior x points
            for (int j=0; j<NG; j++) {
                u[i][j] = u[i][NG];
                v[i][j] = v[i][NG];
                if(do_outputs) std::cout << "1 i=" << i << " | j=" << j << " | u=" << u[i][j] << " |      u[i][NG]=" << u[i][NG] << " | v=" << v[i][j] << " |      v[i][NG]=" << v[i][NG] << std::endl;
            }
            // y=ymax, along interior x points
            for (int j=Ny+NG; j<Ny+2*NG; j++) {
                u[i][j] = u[i][Ny+NG-1];
                v[i][j] = v[i][Ny+NG-1];
                if(do_outputs) std::cout << "1 i=" << i << " | j=" << j << " | u=" << u[i][j] << " | u[i][Ny+NG-1]=" << u[i][Ny+NG-1] << " | v=" << v[i][j] << " | v[i][Ny+NG-1]=" << v[i][Ny+NG-1] << std::endl;
            }
        }
        // along all y points - can handle
        for (int j=0; j<Ny+2*NG; j++) {
            // x = 0, along ALL y points
            for (int i=0; i<NG; i++) {
                u[i][j] = u[NG][j]; 
                v[i][j] = v[NG][j];
                if(do_outputs) std::cout << "1 i=" << i << " | j=" << j << " | u=" << u[i][j] << " |      u[NG][j]=" << u[NG][j] << " | v=" << v[i][j] << " |      v[NG][j]=" << v[NG][j] << std::endl;
            }
            // x = max, along ALL y points
            for (int i=Nx+NG; i<Nx+2*NG; i++) {
                u[i][j] = u[Nx+NG-1][j];
                v[i][j] = v[Nx+NG-1][j];
                if(do_outputs) std::cout << "1 i=" << i << " | j=" << j << " | u=" << u[i][j] << " | u[Nx+NG-1][j]=" << u[Nx+NG-1][j] << " | v=" << v[i][j] << " | v[Nx+NG-1][j]=" << v[Nx+NG-1][j] << std::endl;
            }
        }

        // Actual iteration
        for (int j=NG; j<Ny+NG; j++){
            for (int i=NG; i<Nx+NG; i++){
                if(do_outputs) std::cout << "i=" << i << " | j=" << j;

                // 1/Re - inverse reynolds number
                invRe = ((rho*sqrt(u[i][j]*u[i][j] + v[i][j]*v[i][j])*L) + 0.1)/mu;
                Re[i][j] = invRe;
                invRe = 1/invRe;
                if(do_outputs) std::cout << " | invRe=" << invRe;
                // if (!isinf(invRe) || invRe > 5000.0) invRe = 5000.0;        // Necessary to ensure finite Re
                // Actual stencil - using upwind scheme
                
                // Defining an 's' to represent the sign of velocity
                sx = u[i][j] >= 0 ? 1 : -1;
                sy = v[i][j] >= 0 ? 1 : -1;
                
                if(do_outputs) std::cout << " | us=" << sx;
                // dpdx
                ut1 = sx/(2*dx)*(3*p[i][j] - 4*p[i-1*sx][j] + p[i-2*sx][j]);
                if(do_outputs) std::cout << " | ut1=" << ut1;
                // d2udx2
                ut2 = 1/(dx*dx) * (u[i][j] - 2*u[i-1*sx][j] + u[i-2*sx][j]);
                if(do_outputs) std::cout << " | ut2=" << ut2;
                // d2udy2
                ut3 = 1/(dy*dy) * (u[i][j] - 2*u[i][j-1*sy] + u[i][j-2*sy]);
                if(do_outputs) std::cout << " | ut3=" << ut3;
                // d(u2)dx
                ut4 = sx/(2*dx)*(3*u[i][j]*u[i][j] - 4*u[i-1*sx][j]*u[i-1*sx][j] + u[i-2*sx][j]*u[i-2*sx][j]);
                if(do_outputs) std::cout << " | ut4=" << ut4;
                // d(uv)dy
                ut5 = sy/(2*dy)*(3*u[i][j]*v[i][j] - 4*u[i][j-1*sy]*v[i][j-1*sy] + u[i][j-2*sy]*v[i][j-2*sy]);
                if(do_outputs) std::cout << " | ut5=" << ut5;

                if(do_outputs) std::cout << " | vs=" << sy;
                // dpdy
                vt1 = sy/(2*dy)*(3*p[i][j] - 4*p[i][j-1*sy] + p[i][j-2*sy]);
                if(do_outputs) std::cout << " | vt1=" << vt1;
                // d2vdx2
                vt2 = 1/(dx*dx) * (v[i][j] - 2*v[i-1*sx][j] + v[i-2*sx][j]);
                if(do_outputs) std::cout << " | vt2=" << vt2;
                // d2vdy2
                vt3 = 1/(dy*dy) * (v[i][j] - 2*v[i][j-1*sy] + v[i][j-2*sy]);
                if(do_outputs) std::cout << " | vt3=" << vt3;
                // d(uv)dx
                vt4 = sx/(2*dx)*(3*u[i][j]*v[i][j] - 4*u[i-1*sx][j]*v[i-1*sx][j] + u[i-2*sx][j]*v[i-2*sx][j]);
                if(do_outputs) std::cout << " | vt4=" << vt4;
                // d(v2)dy
                vt5 = sy/(2*dy)*(3*v[i][j]*v[i][j] - 4*v[i][j-1*sy]*v[i][j-1*sy] + v[i][j-2*sy]*v[i][j-2*sy]);
                if(do_outputs) std::cout << " | vt5=" << vt5;

                // Updating u and v
                v_new[i][j] = v[i][j] + dt*(-vt1 + invRe*(vt2 + vt3) - vt4 - vt5 + gy[i][j]);
                u_new[i][j] = u[i][j] + dt*(-ut1 + invRe*(ut2 + ut3) - ut4 - ut5 + gx[i][j]);
                if(do_outputs) std::cout << " | u=" << u_new[i][j] << " | v=" << v_new[i][j] << std::endl;
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

        // Because the search algorithm to find the tracer's nearest velocity is slow, wrap the tracer update in an if statement
        if (write_tracers) {
            for (int tra=0; tra < N_tracer; tra++) {
                // Quick error check to make sure the tracer stays inside the boundaries:
                if (tracer_position[0][tra] < xmin || tracer_position[0][tra] > xmax || tracer_position[1][tra] < ymin || tracer_position[1][tra] > ymax) {
                    tracer_position[0][tra] = dis_x(gen);       // move the tracer back into the active region
                    tracer_position[1][tra] = dis_y(gen);       // move the tracer back into the active region
                }
                int i = findClosestIndex(x, tracer_position[0][tra]);
                int j = findClosestIndex(y, tracer_position[1][tra]);
                // update velocity using simple iteration, but 4x the actual velocity value so the movement is noticable
                tracer_position[0][tra] = tracer_position[0][tra] + dt * u[i][j] * 10;
                tracer_position[1][tra] = tracer_position[1][tra] + dt * v[i][j] * 10;
            }
            if (ti % outputFrequency == 0) writeToCsv(tracer_position, "output/tracer" + std::to_string(ti) + ".csv");
        }

        // Output to file
        if (ti % outputFrequency == 0 && write_velocities){
            writeToCsv(u, "output/u" + std::to_string(ti) + ".csv");
            writeToCsv(v, "output/v" + std::to_string(ti) + ".csv");
            writeToCsv(Re, "output/Re" + std::to_string(ti) + ".csv");
        }
    }

    return 0;
}