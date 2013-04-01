//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#include "CLGLDataLoader.hpp"

// ------------------ //
// Gen DATA Functions //
// ------------------ //

void genGalaxy(vector color, vector center, body * part, int begin, int end)
{
  for(int i = begin; i < end; i++)
  {
    part->mass[i] = 1.0f;

    //distribute the particles in a random circle around z axis
    float x = (i-begin)*sin((i-begin)) / (float) (2 * end-begin);
    float y = (i-begin)*cos((i-begin)) / (float) (2 * end-begin);
    float z = 0;
    part->pos[i].x = x + center.x;
    part->pos[i].y = y + center.y;
    part->pos[i].z = z + center.z;
    part->pos[i].w = 0.0f;

    //give some initial velocity 
    part->vel[i].x = -6 * y;
    part->vel[i].y =  6 * x;
    part->vel[i].z = 0;
    part->vel[i].w = 0;

    //just make them red and full alpha
    part->color[i].x = color.x;
    part->color[i].y = color.y;
    part->color[i].z = color.z;
    part->color[i].w = color.w;
  }
  return;
}

body * genData(int numPart)
{
  int num = numPart;
  body * part = new body[1];
  
  part->mass = new GLfloat[num];
  part->pos = * new std::vector<vector>(num);
  part->color = * new std::vector<vector>(num);
  part->vel = * new std::vector<vector>(num);
  
  // Generate Galaxy data
  vector color;
  color.x = color.y = color.z = color.w = 1.0f;
  vector center;
  center.x = center.y = center.z = -1.0f;
  genGalaxy(color, center, part, num/2, num);
  center.x = center.y = center.z =  1.0f;
  genGalaxy(color, center, part, 0, num/2);
  return part;
}

#define getAtributes(FIELD); \
  file >> str; \
  if(str.compare(FIELD))\
  std::cout << "Error reading data file" << std::endl; \
  file >> str; \
  if(str.compare("=")) \
  std::cout << "Error reading data file" << std::endl;

body * loadDataFromFile(std::string dataFileName, int * numPart)
{
  std::fstream file(dataFileName.c_str());
  std::string str;
  
  body * b = new body;
  *numPart = 0;

  // IF file exists
  if(!file){
    std::cout << "File: " << dataFileName.c_str() << " is not valid as data file!" << std::endl;
    std::cout << "CLGLSim will be closed" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Get the number of particles 
  while(!file.eof()){
    file >> str;
    if(!str.compare("r"))
      *numPart = *numPart + 1;
  }

  b->mass = new GLfloat[*numPart];
  b->radius = new GLfloat[*numPart];
  b->pos = * new std::vector<vector>(*numPart);
  b->vel = * new std::vector<vector>(*numPart);
  b->color = * new std::vector<vector>(*numPart);

  file.close();
  file.open(dataFileName.c_str());

  for(int i=0; i < *numPart && !file.eof(); i++){
    getAtributes("r");
    file >> b->radius[i];
    
    getAtributes("m");
    file >> b->mass[i];
    
    getAtributes("px");
    file >> b->pos[i].x;
    getAtributes("py");
    file >> b->pos[i].y;
    getAtributes("pz");
    file >> b->pos[i].z;
    b->pos[i].w = 0.0f;

    getAtributes("vx");
    file >> b->vel[i].x;
    getAtributes("vy");
    file >> b->vel[i].y;
    getAtributes("vz");
    file >> b->vel[i].z;
    b->vel[i].w = 0.0f;

    getAtributes("red");
    file >> b->color[i].x;
    getAtributes("green");
    file >> b->color[i].y;
    getAtributes("blue");
    file >> b->color[i].z;
    getAtributes("alpha");
    file >> b->color[i].w;
  }

  return b;
}
