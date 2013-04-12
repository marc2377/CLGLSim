//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

//#include "CLGLSim.hpp"

#ifndef CLGLDATALOADER_HPP
#define CLGLDATALOADER_HPP

#include <fstream>
#include <vector>
#include <iostream>
#include <cstdlib>

#include <math.h>
#include <GL/gl.h>

#include "definitions.h"

// Load data from file dataFileName and returns also
// the number of particles

data * loadDataFromFile(std::string dataFileName, int * numPart);

data * genData(int NUM_PART_FLUID, int NUM_PART_SOLID);

#endif
