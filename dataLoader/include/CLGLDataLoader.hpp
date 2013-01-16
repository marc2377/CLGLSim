//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#include <fstream>
#include <vector>
#include <iostream>
#include <cstdlib>

#include <math.h>
#include <GL/gl.h>

typedef struct t_vector4
{
  float x,y,z,w;
} vector;

typedef struct t_body
{
  GLfloat* mass;
  GLfloat* radius;
  std::vector<vector> pos;
  std::vector<vector> vel;
  std::vector<vector> color;
} body;

// Load data from file dataFileName and returns also
// the number of particles
body * loadData(int numPart);

body * loadDataFromFile(std::string dataFileName, int * numPart);
