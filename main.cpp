//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#ifdef WIN32
  #include "CLGLWindows.hpp"
#else
  #include "CLGLLinux.hpp"
#endif

#include "CLGLWindow.hpp"
#include "CLGLError.hpp"

#include <string>
#include <fstream>
#include <CL/cl.hpp>

int NUM_PART = 2000;

typedef struct t_vector4
{
  float x,y,z,w;
} vector;

typedef struct t_body
{
  GLfloat* mass;
  std::vector<vector> pos;
  std::vector<vector> vel;
  std::vector<vector> color;
} body;

// Load data from file dataFileName and returns also the 
// number of particles
body * loadData(std::string dataFileName, int *numPart);

int main(int argc, char * argv[])
{
  float rungeStep = 0.001;
  int numPart = 0;
  std::string windowTitle = "CLGLSim 1.0";
  std::string kernelFileName = "rk4.cl";
  std::string dataFileName = "data.sim";
  std::string str_kernel = "Gravity_rk2";

  // ----------------- //
  // Arguments in argv //
  // ----------------- //
  for(int i=0; i < argc; i++){
    if(!strcmp(argv[i], "--kernel")){
      str_kernel = argv[i+1];
      if(argv[i+1][2] == '1')
        CLGLSim::curKernel = 1;
      else if(argv[i+1][2] == '2')
        CLGLSim::curKernel = 2;
      else
        CLGLSim::curKernel = 4;
    }
    else if(!strcmp(argv[i], "--num"))
      sscanf(argv[i+1], "%d", &NUM_PART);
  }

  std::cout << "-----------------------------------" << std::endl;
  std::cout << "Using " << str_kernel << " Kernel !" << std::endl;
  std::cout << "-----------------------------------" << std::endl;

  body * hostData;

  // Define Window Height and Width
  CLGLWindow::window_height = 768;
  CLGLWindow::window_width = 1366;
 
  try{
    // Start CLGL
#ifdef WIN32
    CLGLWindows clgl = CLGLWindows();
#else
    CLGLLinux clgl = CLGLLinux();
#endif
    clgl.CLGLStart(argc, argv, windowTitle, CLGLWindow::window_height, CLGLWindow::window_width);
    
    // Get Window ID
    CLGLWindow::glutWindowHandle = clgl.CLGLGetWindowID();
    CLGLWindow window = CLGLWindow();

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Loading Data" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    hostData = loadData(dataFileName, &numPart);
   
    // Set the Number of Particles beeing simulated
    CLGLWindow::NumParticles = numPart;

    // Print info About Current Platform and devices
    clgl.CLGLPrintAllInfo();

    // Build the Source of the kernel
    clgl.CLGLBuildProgramSource(kernelFileName);

    // Build the function str_kernel in the kernel 
    clgl.CLGLBuildKernel(str_kernel);
    str_kernel = "Gravity_rk1";
    clgl.CLGLBuildKernel(str_kernel);
    str_kernel = "Gravity_rk2";
    clgl.CLGLBuildKernel(str_kernel);
    str_kernel = "Gravity_rk4";
    clgl.CLGLBuildKernel(str_kernel);

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Pushing Data to Device" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    clgl.CLGLLoadVBODataToDevice((hostData->pos.size()+1) * sizeof(vector), &(hostData->pos[0]), CL_MEM_READ_WRITE);
    clgl.CLGLLoadVBODataToDevice(hostData->color.size() * sizeof(vector), &(hostData->color[0]), CL_MEM_READ_WRITE);

    clgl.CLGLLoadDataToDevice(CL_TRUE, numPart * sizeof(GLfloat), hostData->mass, CL_MEM_READ_WRITE);
    clgl.CLGLLoadDataToDevice(CL_TRUE, hostData->vel.size() * sizeof(vector), &(hostData->vel[0]), CL_MEM_READ_WRITE);

    std::vector<cl::Buffer> buff = *clgl.CLGLGetBuffer();
    std::vector<cl::Memory> mem = *clgl.CLGLGetBufferGL();
    std::vector<cl::Kernel> ker = *clgl.CLGLGetKernel();

    // Set the Kernel ARGS
    for(unsigned int i=0; i < ker.size(); i++){
      clgl.CLGLSetArg(0, buff[0], ker[i]);
      clgl.CLGLSetArg(1, buff[1], ker[i]);
      clgl.CLGLSetArg(2, mem[0], ker[i]);
      clgl.CLGLSetArg(3, sizeof(float), &rungeStep, ker[i]);
      clgl.CLGLSetArg(4, sizeof(int), &numPart, ker[i]);
    }

    // Set CLGLSim static members
    CLGLSim::vbo = clgl.CLGLGetVBO();
    CLGLSim::ParticlesNum = numPart;
    CLGLSim::rungeStep = rungeStep;
    CLGLSim::clgl = &(clgl);
    CLGLSim::rkx = &ker;
    CLGLSim::curKernel = 2;

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Runing the Window" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    window.CLGLRunWindow();
  }
  catch(cl::Error error)
  {
    std::cout << error.what() << CLGLError::errToStr(error.err())->c_str() << std::endl;
  }

  return 0;
}

// ------------------ //
// Gen DATA Functions //
// ------------------ //

float rand_float(float mn, float mx)
{
      float r = random() / (float) RAND_MAX;
          return mn + (mx-mn)*r;
}

body * loadData(std::string dataFileName, int *numPart)
{
  int num = *numPart = NUM_PART;
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
    float z = 0;//rad*tanh(2*3.14 * i/num);//*sin(2*i);
    float x = 2*rad*cos(2*3.14 * i/num)*cos(2*i);//0;// -.1 + .2f * i/num;
    float y = 2*rad*sin(2*3.14 * i/num)*sin(2*i);
    part->pos[i].x = x;
    part->pos[i].y = y;
    part->pos[i].z = z;
    part->pos[i].w = 0.0f;

    //give some initial velocity 
    part->vel[i].x = 0;//rad*sin(5*i+1);
    part->vel[i].y = 0;//rad*cos(6*i+i);
    part->vel[i].z = 0;//rad*sin(i)*cos(i)*cos(i);
    part->vel[i].w = 0;

    //just make them red and full alpha
    part->color[i].x = 1.0;
    part->color[i].y = 1.0;
    part->color[i].z = 1.0;
    part->color[i].w = 1.0;
  }
  return part;
}

