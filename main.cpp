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
#include "CLGLParser.hpp"
#include "CLGLDataLoader.hpp"

#include <string>
#include <fstream>
#include <CL/cl.hpp>

/*int main(int argc, char * argv[])
{
  float rungeStep;
  int NUM_PART;
  std::string windowTitle = "CLGLSim 1.0";
  CLGLParser console = CLGLParser(argc, argv);

  CLGLSim::curKernel = console.curKernel;
  NUM_PART = console.particlesNum;
  rungeStep = console.rungeStep;
  std::string kernelFileName = console.kernelFile;

  std::cout << "-----------------------------------" << std::endl;
  std::cout << "Using " << console.kernel << " Kernel !" << std::endl;
  std::cout << "-----------------------------------" << std::endl;

  body * hostData;

  // Define Window Height and Width
  CLGLWindow::window_height = 768;
  CLGLWindow::window_width = 1366;
 
  try{
    // Start CLGL
#ifndef WIN32
    CLGLLinux clgl = CLGLLinux();
#else 
    CLGLWindows clgl = CLGLWindows();
#endif
    clgl.CLGLStart(argc, argv, windowTitle, CLGLWindow::window_height, CLGLWindow::window_width);
    
    // Get Window ID
    CLGLWindow::glutWindowHandle = clgl.CLGLGetWindowID();
    CLGLWindow window = CLGLWindow();

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Loading Data" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    if(console.isDataFileSet())
      hostData = loadDataFromFile(console.dataFile, &NUM_PART);
    else
        hostData = genData(NUM_PART);
   
    // Set the Number of Particles beeing simulated
    CLGLWindow::NumParticles = NUM_PART;

    // Print info About Current Platform and devices
    clgl.CLGLPrintAllInfo();

    // Build the Source of the kernel
    clgl.CLGLBuildProgramSource(kernelFileName);

    // Build the function console.kernel in the kernel 
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "Gravity_rk1";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "Gravity_rk2";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "Gravity_rk4";
    clgl.CLGLBuildKernel(console.kernel);

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Pushing Data to Device" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    clgl.CLGLLoadVBODataToDevice(hostData->pos.size() * sizeof(vector), &(hostData->pos[0]), CL_MEM_READ_WRITE);
    clgl.CLGLLoadVBODataToDevice(hostData->color.size() * sizeof(vector), &(hostData->color[0]), CL_MEM_READ_WRITE);

    clgl.CLGLLoadDataToDevice(CL_TRUE, NUM_PART * sizeof(GLfloat), hostData->mass, CL_MEM_READ_ONLY);
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
      clgl.CLGLSetArg(4, sizeof(int), &NUM_PART, ker[i]);
    }

    // Set CLGLSim static members
    CLGLSim::vbo = clgl.CLGLGetVBO();
    CLGLSim::ParticlesNum = NUM_PART;
    CLGLSim::rungeStep = rungeStep;
    CLGLSim::clgl = &(clgl);
    CLGLSim::rkx = &ker;

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
}*/

int main(int argc, char * argv[])
{
  float rungeStep;
  int NUM_PART;
  int nPhysicalKernels = 0;
  std::string windowTitle = "CLGLSim 1.0";
  CLGLParser console = CLGLParser(argc, argv);

  CLGLSim::curKernel = console.curKernel;
  NUM_PART = console.particlesNum;
  rungeStep = console.rungeStep;
  std::string kernelFileName = console.kernelFile;

  std::cout << "-----------------------------------" << std::endl;
  std::cout << "Using " << console.kernel << " Kernel !" << std::endl;
  std::cout << "-----------------------------------" << std::endl;

  body * hostData;

  // Define Window Height and Width
  CLGLWindow::window_height = 768;
  CLGLWindow::window_width = 1366;
 
  try{
    // Start CLGL
#ifndef WIN32
    CLGLLinux clgl = CLGLLinux();
#else 
    CLGLWindows clgl = CLGLWindows();
#endif
    clgl.CLGLStart(argc, argv, windowTitle, CLGLWindow::window_height, CLGLWindow::window_width);
    
    // Print info About Current Platform and devices
    clgl.CLGLPrintAllInfo();
    
    // Get Window ID
    CLGLWindow::glutWindowHandle = clgl.CLGLGetWindowID();
    CLGLWindow window = CLGLWindow();

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Loading Data" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    if(console.isDataFileSet())
      hostData = loadDataFromFile(console.dataFile, &NUM_PART);
    else
        hostData = genData(NUM_PART);
   
    // Set the Number of Particles beeing simulated
    CLGLWindow::NumParticles = NUM_PART;

    /*
     * Loads the physical simulation kernels
     */
    // Build the Source of the kernel
    clgl.CLGLBuildProgramSource(console.kernelFile);

    // Build the function console.kernel in the kernel 
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "Gravity_rk1";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "Gravity_rk2";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "Gravity_rk4";
    clgl.CLGLBuildKernel(console.kernel);
    //Set the number of physical kernels that were charged
    nPhysicalKernels = clgl.CLGLGetKernel()->size();

    /*
     * Loads the Data Structures kernels
     */
    // Sort Grid implementations 
    clgl.CLGLBuildProgramSource("kernels/grid.cl");
    console.kernel = "getGridSideSize";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "getNGridCubes";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "setGridIndex";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "bubbleSort3D_even";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "bubbleSort3D_odd";
    clgl.CLGLBuildKernel(console.kernel);

    /*
     * Pushing data to Device
     */
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Pushing Data to Device" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    
    // Physical data
    clgl.CLGLLoadVBODataToDevice(hostData->pos.size() * sizeof(vector), &(hostData->pos[0]), CL_MEM_READ_WRITE);
    clgl.CLGLLoadVBODataToDevice(hostData->color.size() * sizeof(vector), &(hostData->color[0]), CL_MEM_READ_WRITE);

    clgl.CLGLLoadDataToDevice(CL_TRUE, NUM_PART * sizeof(GLfloat), hostData->mass, CL_MEM_READ_ONLY);
    clgl.CLGLLoadDataToDevice(CL_TRUE, hostData->vel.size() * sizeof(vector), &(hostData->vel[0]), CL_MEM_READ_WRITE);

    // Data Structure data
    clgl.CLGLLoadDataToDevice(CL_TRUE, NUM_PART * sizeof(int), NULL, CL_MEM_READ_WRITE); //grid Index
    clgl.CLGLLoadDataToDevice(CL_TRUE, NUM_PART * sizeof(vector), NULL, CL_MEM_READ_WRITE); //grid Coord
    clgl.CLGLLoadDataToDevice(CL_TRUE, sizeof(int), NULL, CL_MEM_READ_WRITE); //nGridCubes
    bool boolean = true;
    clgl.CLGLLoadDataToDevice(CL_TRUE, sizeof(bool), &boolean, CL_MEM_READ_WRITE); //modification
    int sideSize = 0;
    clgl.CLGLLoadDataToDevice(CL_TRUE, sizeof(int), &sideSize, CL_MEM_READ_WRITE); //sideSize

    // Local variables for speed
    std::vector<cl::Buffer> buff = *clgl.CLGLGetBuffer();
    std::vector<cl::Memory> mem = *clgl.CLGLGetBufferGL();
    std::vector<cl::Kernel> ker = *clgl.CLGLGetKernel();

    /*
     * Set Arguments for the kernels
     */
    // Set Physical Kernel ARGS.
    for(unsigned int i=0; i < nPhysicalKernels; i++){
      clgl.CLGLSetArg(0, buff[0], ker[i]);
      clgl.CLGLSetArg(1, buff[1], ker[i]);
      clgl.CLGLSetArg(2, mem[0], ker[i]);
      clgl.CLGLSetArg(3, sizeof(float), &rungeStep, ker[i]);
      clgl.CLGLSetArg(4, sizeof(int), &NUM_PART, ker[i]);
    }
    
    // Set argument to data structure kernels
    // Set arguments to kernel getGridSize
    clgl.CLGLSetArg(0, mem[0] , ker[4]);
    clgl.CLGLSetArg(1, buff[6], ker[4]);
    clgl.CLGLSetArg(2, sizeof(int), &NUM_PART, ker[4]);
    // Set arguments to kernel getNGridCubes
    clgl.CLGLSetArg(0, mem[0] , ker[5]);
    clgl.CLGLSetArg(1, 
    clgl.CLGLSetArg(2, buff[4], ker[5]);
    clgl.CLGLSetArg(3, buff[6], ker[5]);
    clgl.CLGLSetArg(4, sizeof(int), &NUM_PART, ker[5]);
    // Set arguments to kernel bubbleSort3D_even
    clgl.CLGLSetArg(0, buff[2], ker[7]);
    clgl.CLGLSetArg(1, buff[3], ker[7]);
    clgl.CLGLSetArg(2, buff[4], ker[7]);
    clgl.CLGLSetArg(3, sizeof(int), NUM_PART, ker[7]);
    // Set arguments to kernel bubbleSort3D_odd
    clgl.CLGLSetArg(0, buff[2], ker[8]);
    clgl.CLGLSetArg(1, buff[3], ker[8]);
    clgl.CLGLSetArg(2, buff[4], ker[8]);
    clgl.CLGLSetArg(3, sizeof(int), NUM_PART, ker[8]);

    // Set CLGLSim static members
    CLGLSim::vbo = clgl.CLGLGetVBO();
    CLGLSim::ParticlesNum = NUM_PART;
    CLGLSim::rungeStep = rungeStep;
    CLGLSim::clgl = &(clgl);
    CLGLSim::rkx = &ker;
    CLGLSim::buffer = &buff;

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
