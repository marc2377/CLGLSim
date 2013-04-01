//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#include "CLGLSim.hpp"

/*
 * Initiate static attributes
 */
int CLGLSim::ParticlesNum = 0;
float CLGLSim::rungeStep = 0.1f;
CLGL * CLGLSim::clgl = NULL;
Grid * CLGLSim::dataStruct = NULL;
std::vector<int> * CLGLSim::vbo = NULL;
std::vector<cl::Buffer> * CLGLSim::buffer = NULL;

// Kernel
std::vector<cl::Kernel> * CLGLSim::rkx = NULL;
int CLGLSim::curKernel = 0;

void printParameters(void)
{
  int nGridCubesVar, sideSizeVar;
  int * gridIndexVar = new int[CLGLSim::ParticlesNum];
  vector * gridCoordVar = new vector[CLGLSim::ParticlesNum];
  int * nPartPerIndexVar;
  bool mVF, rTF;

  std::cout << "-------------------------------------" << std::endl;

  // nGridCubes
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + nGridCubes], CL_TRUE, sizeof(int), &nGridCubesVar);
  std::cout << "nGridCubes: " << nGridCubesVar << std::endl;
  // sideSize
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + sideSize], CL_TRUE, sizeof(int), &sideSizeVar);
  std::cout << "sideSize: " << sideSizeVar << std::endl;
  // rebuildTreeFlag
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + rebuildTreeFlag], CL_TRUE, sizeof(bool), &rTF);
  std::cout << "rebuildTreeFlag: " << rTF << std::endl;
  // modificationVectorFlag
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + modificationVectorFlag], CL_TRUE, sizeof(bool), &mVF);
  std::cout << "modificationVectorFlag: " << mVF << std::endl;
  // gridIndex
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + gridIndex], CL_TRUE, sizeof(int) * CLGLSim::ParticlesNum, gridIndexVar);
  std::cout << "gridIndex: ";
  for(int i=0; i < CLGLSim::ParticlesNum; i++)
    std::cout << gridIndexVar[i] << " | ";
  std::cout << std::endl;
  // gridCoord
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + gridCoord], CL_TRUE, sizeof(vector) * CLGLSim::ParticlesNum, gridCoordVar);
  std::cout << "gridCoord: ";
  for(int i=0; i < CLGLSim::ParticlesNum; i++)
    std::cout << "(" << gridCoordVar[i].x << "," << gridCoordVar[i].y << "," << gridCoordVar[i].z << "," << gridCoordVar[i].w << ") | ";
  std::cout << std::endl;
  // nPartPerIndexVar
  nPartPerIndexVar = new int[nGridCubesVar * nGridCubesVar * nGridCubesVar];
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + nPartPerIndex], CL_TRUE, sizeof(int) * nGridCubesVar * nGridCubesVar * nGridCubesVar, nPartPerIndexVar);
 std::cout << "nPartPerIndex: ";
  for(int i=0; i < nGridCubesVar * nGridCubesVar * nGridCubesVar; i++)
    std::cout << nPartPerIndexVar[i] << " | ";
  std::cout << std::endl;

  std::cout << "-------------------------------------" << std::endl;
}

/*
 * Run the kernel Function
 */
void CLGLSim::CLGLRunKernel()
{
  bool isTreeIncorrect = false;

  printParameters();

  // Runs the kernel
  CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[0], CLGLSim::ParticlesNum);

  // Get the isTreeIncorrect from device
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + rebuildTreeFlag], CL_TRUE, sizeof(bool), &isTreeIncorrect);
  if(isTreeIncorrect)
    CLGLSim::dataStruct->refreshGrid();

  return;
}

void CLGLSim::CLGLStartPhysics(CLGL * clgl, float rungeStep, int NUM_PART, body * hostData, CLGLParser * console)
{
  int nPhysicalKernels = 0;
 
  try{
    /*
     * Loads the physical simulation kernels
     */
    // Build the Source of the kernel
    clgl->CLGLBuildProgramSource(console->kernelFile, "-I./kernels");

    // Build the function console kernel in the kernel 
    clgl->CLGLBuildKernel(console->kernel);
    console->kernel = "Gravity_rk1";
    clgl->CLGLBuildKernel(console->kernel);
    console->kernel = "Gravity_rk2";
    clgl->CLGLBuildKernel(console->kernel);
    console->kernel = "Gravity_rk4";
    clgl->CLGLBuildKernel(console->kernel);
    //Set the number of physical kernels that were charged
    nPhysicalKernels = clgl->CLGLGetKernel()->size();

    /*
     * Pushing data to Device
     */
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Pushing Data to Device" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    // Physical data
    clgl->CLGLLoadVBODataToDevice(hostData->pos.size() * sizeof(vector), &(hostData->pos[0]), CL_MEM_READ_WRITE);
    clgl->CLGLLoadVBODataToDevice(hostData->color.size() * sizeof(vector), &(hostData->color[0]), CL_MEM_READ_WRITE);

    clgl->CLGLLoadDataToDevice(CL_TRUE, NUM_PART * sizeof(GLfloat), hostData->mass, CL_MEM_READ_ONLY);
    clgl->CLGLLoadDataToDevice(CL_TRUE, hostData->vel.size() * sizeof(vector), &(hostData->vel[0]), CL_MEM_READ_WRITE);

    // Local variables for speed
    std::vector<cl::Buffer> * buff = clgl->CLGLGetBuffer();
    std::vector<cl::Memory> * mem = clgl->CLGLGetBufferGL();
    std::vector<cl::Kernel> * ker = clgl->CLGLGetKernel();

    // Set CLGLSim static members
    CLGLSim::vbo = clgl->CLGLGetVBO();
    CLGLSim::ParticlesNum = NUM_PART;
    CLGLSim::rungeStep = rungeStep;
    CLGLSim::clgl = clgl;
    CLGLSim::rkx = ker;
    CLGLSim::buffer = buff;
    CLGLSim::dataStruct = new Grid(clgl, &((*mem)[0]), NUM_PART); //Add the data struct to the simulator

    /*
     * Set Arguments for the kernels
     */
    // Set Physical Kernel ARGS
    int beg = CLGLSim::dataStruct->getBuffBegin();
    for(int i=0; i < 1/*nPhysicalKernels*/; i++){
      clgl->CLGLSetArg(0, (*buff)[0], (*ker)[i]);
      clgl->CLGLSetArg(1, (*buff)[1], (*ker)[i]);
      clgl->CLGLSetArg(2, (*mem)[0], (*ker)[i]);
      clgl->CLGLSetArg(3, (*buff)[beg + gridIndex], (*ker)[i]);
      clgl->CLGLSetArg(4, (*buff)[beg + gridCoord], (*ker)[i]);
      clgl->CLGLSetArg(5, (*buff)[beg + nGridCubes], (*ker)[i]);
      clgl->CLGLSetArg(6, (*buff)[beg + nPartPerIndex], (*ker)[i]);
      clgl->CLGLSetArg(7, (*buff)[beg + rebuildTreeFlag], (*ker)[i]);
      clgl->CLGLSetArg(8, (*buff)[beg + sideSize], (*ker)[i]);
      clgl->CLGLSetArg(9, sizeof(float), &rungeStep, (*ker)[i]);
      clgl->CLGLSetArg(10, sizeof(int), &NUM_PART, (*ker)[i]);
    }
  }
  catch(cl::Error error)
  {
    std::cout << error.what() << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(EXIT_FAILURE);
  }
  return;
}
