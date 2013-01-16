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
float rand_float(float mn, float mx)
{
      float r = random() / (float) RAND_MAX;
          return mn + (mx-mn)*r;
}

body * loadData(int numPart)
{
  int num = numPart;
  body * part = new body[1];
  
  part->mass = new GLfloat[num];
  part->pos = * new std::vector<vector>(num);
  part->color = * new std::vector<vector>(num);
  part->vel = * new std::vector<vector>(num);
  //fill our vectors with initial data
  for(int i = 0; i < num; i++)
  {
    part->mass[i] = 1.0f;

    //distribute the particles in a random circle around z axis
    float rad = rand_float(.2, .5);
    float z = rad*tanh(2*3.14 * i/num);//*sin(2*i);
    float x = 2*rad*cos(2*3.14 * i/num)*cos(2*i);//0;// -.1 + .2f * i/num;
    float y = 2*rad*sin(2*3.14 * i/num)*sin(2*i);
    part->pos[i].x = x;
    part->pos[i].y = y;
    part->pos[i].z = z;
    part->pos[i].w = 0.0f;

    //give some initial velocity 
    part->vel[i].x = x;//rad*sin(5*i+1);
    part->vel[i].y = y;//rad*cos(6*i+i);
    part->vel[i].z = z;//rad*sin(i)*cos(i)*cos(i);
    part->vel[i].w = 0;

    //just make them red and full alpha
    part->color[i].x = 1.0;
    part->color[i].y = 1.0;
    part->color[i].z = 1.0;
    part->color[i].w = 1.0;
  }
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
    exit(1);
  }

  // Get the number of particles 
  while(!file.eof()){
    file >> str;
    if(!str.compare("r"))
      *numPart = *numPart + 1;
  }
  std::cout << *numPart << std::endl;

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
