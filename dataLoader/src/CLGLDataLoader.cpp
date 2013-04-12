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

void genGalaxy(float4 color, float4 center, billard * part, int begin, int end)
{
  for(int i = begin; i < end; i++)
  {
    part->mass[i] = 10.0f;

    //distribute the particles in a random circle around z axis
    float x = (i-begin)*sin((i-begin)) / (float) (2 * end-begin);
    float y = (i-begin)*cos((i-begin)) / (float) (2 * end-begin);
    float z = 0;
    (*part->pos)[i].x = x + center.x;
    (*part->pos)[i].y = y + center.y;
    (*part->pos)[i].z = z + center.z;
    (*part->pos)[i].w = 0.0f;

    //give some initial velocity 
    (*part->vel)[i].x = 0;//-6 * y;
    (*part->vel)[i].y = 0;//6 * x;
    (*part->vel)[i].z = 0;
    (*part->vel)[i].w = 0;

    //just make them red and full alpha
    (*part->color)[i].x = color.x;
    (*part->color)[i].y = color.y;
    (*part->color)[i].z = color.z;
    (*part->color)[i].w = color.w;
  }
  return;
}

billard * genGalaxy(int numPart)
{
int num = numPart;
  if(num <= 0) 
    return NULL;
  billard * part = new billard[1];
  
  part->mass = new GLfloat[num];
  part->pos = new std::vector<float4>(num);
  part->color = new std::vector<float4>(num);
  part->vel = new std::vector<float4>(num);
  
  // Generate Galaxy data
  float4 color;
  color.x = color.y = color.z = color.w = 1.0f;
  float4 center;
  center.x = center.y = center.z = -1.0f;
  genGalaxy(color, center, part, num/2, num);
  center.x = center.y = center.z =  1.0f;
  genGalaxy(color, center, part, 0, num/2);
  return part;
}

fluid * genFluid(int numPart)
{
  int i;
  int num = numPart;
  if(num <= 0) 
    return NULL;
  fluid * p = new fluid[1];
  
  p->mass = new GLfloat[num];
  p->pos = new std::vector<float4>(num);
  p->color = new std::vector<float4>(num);
  p->vel = new std::vector<float4>(num);
  
 // set mass, color, velocity
  for( i = 0; i < numPart; i++){
    p->mass[i] = 1;
    (*p->color)[i].x = (*p->color)[i].y = (*p->color)[i].w = 1.0f;
    (*p->color)[i].z = 1.0f;
    (*p->vel)[i].x = (*p->vel)[i].y = (*p->vel)[i].z = 0.0f;
  }
  (*p->pos)[0].x = (*p->pos)[0].y = (*p->pos)[0].z = 0.0f;
  (*p->pos)[0].w = 0.0f;
  // set position and neighbors
  for (i = 1; i < numPart; i++) {
    (*p->pos)[i].x = (*p->pos)[i].y = (*p->pos)[i].z = (*p->pos)[i-1].x + 0.12;
    (*p->pos)[i].w = 0.0f;
  }
  return p;
}

float lenth(float4 u, float4 v)
{
  float x, y, z;

  x = u.x - v.x;
  y = u.y - v.y;
  z = u.z - v.z;

  return sqrt(x*x + y*y + z*z);
}

solid * genSolid(int numPart)
{
  int i;
  if(numPart <= 0)
    return NULL;
  solid * p = new solid[1];

  // instantiate
  p->mass = new GLfloat[numPart];
  p->radius = new GLfloat[numPart];
  p->color = new std::vector<float4>(numPart);
  p->pos = new std::vector<float4>(numPart);
  p->vel = new std::vector<float4>(numPart);
  p->accel = new std::vector<float4>(numPart);
  p->neighbors = new std::vector<int>();
  p->lZero = new std::vector<float>();

  // set mass, color, velocity
  for( i = 0; i < numPart; i++){
    p->mass[i] = 1;
    (*p->color)[i].x = (*p->color)[i].y = (*p->color)[i].w = 1.0f;
    (*p->color)[i].z = 0.0f;
    (*p->vel)[i].x = (*p->vel)[i].y = (*p->vel)[i].z = 0.0f;
    (*p->accel)[i].x = (*p->accel)[i].y = (*p->accel)[i].z = 0;
  }
  (*p->pos)[0].x = (*p->pos)[0].y = (*p->pos)[0].z = 0.0f;
  (*p->pos)[0].w = 0.0f;
  // set position and neighbors
  for (i = 1; i < numPart; i++) {
    (*p->pos)[i].x = (*p->pos)[i].y = (*p->pos)[i].z = (*p->pos)[i-1].x + 0.1;
    (*p->pos)[i].w = 0.0f;
    p->neighbors->push_back(i-1);
    p->neighbors->push_back(i);
    p->lZero->push_back(lenth((*p->pos)[i-1], (*p->pos)[i]));
  }

  return p;
}

#define getAtributes(FIELD); \
  file >> str; \
  if(str.compare(FIELD))\
  std::cout << "Error reading data file" << std::endl; \
  file >> str; \
  if(str.compare("=")) \
  std::cout << "Error reading data file" << std::endl;
/*
data * loadDataFromFile(std::string dataFileName, int * numPart)
{
  std::fstream file(dataFileName.c_str());
  std::string str;
  
  data * b = new data;
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
  b->pos = * new std::vector<float4>(*numPart);
  b->vel = * new std::vector<float4>(*numPart);
  b->color = * new std::vector<float4>(*numPart);

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
*/
data * genData(int NUM_PART_FLUID, int NUM_PART_SOLID)
{
  data * d = new data[1];

  d->f = NULL;
  d->s = NULL;

  d->f = genFluid(NUM_PART_FLUID);
  d->s = genSolid(NUM_PART_SOLID);

  d->f->size = NUM_PART_FLUID;
  d->s->size = NUM_PART_SOLID;
  
  return d;
}
