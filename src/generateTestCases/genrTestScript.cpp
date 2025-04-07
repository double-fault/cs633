
//genrates 2 files
//one with the data and other with the corrosponding output




#include <iostream>
#include <fstream>
#include <cstdlib>
#include <limits>
#include <ctime>
#include <bits/stdc++.h>

using namespace std;

float generateRandomFloat() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f; // generates floats in the range [-1 to 1]
}

vector<vector<vector<float>>> cube;
float global_max = numeric_limits<float>::lowest();
float global_min = numeric_limits<float>::max();
int local_min_count = 0;
int local_max_count = 0;


void generateCube(int size){
    
    cube.resize(size, vector<vector<float>>(size, vector<float>(size)));
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            for (int k = 0; k < size; ++k) {
                cube[i][j][k] = generateRandomFloat();
                global_max = max(global_max, cube[i][j][k]);
                global_min = min(global_min, cube[i][j][k]);
            }
        }
    }

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            for (int k = 0; k < size; ++k) {
                bool isLocalMin = true, isLocalMax = true;

                int directions[6][3] = {
                    {-1, 0, 0}, {1, 0, 0}, 
                    {0, -1, 0}, {0, 1, 0}, 
                    {0, 0, -1}, {0, 0, 1} 
                };

                for (int d = 0; d < 6; ++d) {
                    int ni = i + directions[d][0];
                    int nj = j + directions[d][1];
                    int nk = k + directions[d][2];

                    if (ni >= 0 && ni < size && nj >= 0 && nj < size && nk >= 0 && nk < size) {
                        if (cube[ni][nj][nk] <= cube[i][j][k]) isLocalMin = false;
                        if (cube[ni][nj][nk] >= cube[i][j][k]) isLocalMax = false;
                    }
                }

                if (isLocalMin) ++local_min_count;
                if (isLocalMax) ++local_max_count;
            }
        }
    }
}

int main() {
   
    int size = 64;  // do not make size more than 2^7 range, will require too much ram to store the data :(
    int time_steps = 3; // Number of time steps

    ofstream dataFile("test1_data.txt"); // use diffrent names to make new test files with their output;
    ofstream outputFile("test1_output.txt");

    // a 3D vector to store all time step data for writing
    vector<vector<vector<vector<float>>>> allData(time_steps, vector<vector<vector<float>>>(size, vector<vector<float>>(size, vector<float>(size))));

    for (int t = 0; t < time_steps; ++t) {
        srand(static_cast<unsigned>(time(0)) + t);
        generateCube(size);

        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                for (int k = 0; k < size; ++k) {
                    allData[t][i][j][k] = cube[i][j][k];
                }
            }
        }

        outputFile << "Time Step " << t + 1 << ":\n";
        outputFile << "Global Max: " << global_max << "\n";
        outputFile << "Global Min: " << global_min << "\n";
        outputFile << "Local Max Count: " << local_max_count << "\n";
        outputFile << "Local Min Count: " << local_min_count << "\n";
        outputFile << "--------------------------\n";

        global_max = numeric_limits<float>::lowest();
        global_min = numeric_limits<float>::max();
        local_min_count = 0;
        local_max_count = 0;
    }

    // Filling dataFile
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            for (int k = 0; k < size; ++k) {
                for (int t = 0; t < time_steps; ++t) {
                    dataFile << allData[t][i][j][k] << " ";
                }
                dataFile << "\n"; 
            }
        }
    }
    dataFile.close();
    outputFile.close();
    return 0;
}