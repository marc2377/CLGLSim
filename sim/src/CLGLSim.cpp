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
int CLGLSim::NUM_PART_FLUID = 0;
int CLGLSim::NUM_PART_SOLID = 0;
int CLGLSim::indexBufferSize = 0;
float CLGLSim::rungeStep = 0.1f;
CLGL * CLGLSim::clgl = NULL;
Grid * CLGLSim::dataStruct = NULL;
std::vector<int> * CLGLSim::vbo = NULL;
std::vector<cl::Buffer> * CLGLSim::buffer = NULL;

// Kernel
std::vector<cl::Kernel> * CLGLSim::rkx = NULL;
int CLGLSim::curKernel = 0;

/*
 * Run the kernel Function
 */
void CLGLSim::CLGLRunKernel()
{
  bool isTreeIncorrect = false;
 
  // Runs the kernel
  CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[Fluid_Density], CLGLSim::ParticlesNum);
  CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[Fluid_rk1], CLGLSim::ParticlesNum);
 
  // Debug Fuction 
  //static int a = 0;
  //std::cout << "CLGLRunKernel: " << a << std::endl;
  //CLGLSim::dataStruct->printParameters();
  //a++;

  CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[Memset], CLGLSim::NUM_PART_SOLID);
  CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[Solid_accel], CLGLSim::indexBufferSize);
  CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[Solid_rk1], CLGLSim::NUM_PART_SOLID);

  // Get the isTreeIncorrect from device
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + rebuildTreeFlag], CL_TRUE, sizeof(bool), &isTreeIncorrect);
  if(isTreeIncorrect){
   // std::cout << "Rebuilding Tree" << std::endl;
    CLGLSim::dataStruct->refreshGrid();
  }

  return;
}

#define setKernel(KERNEL) str = KERNEL; \
    clgl->CLGLBuildKernel(str);

void BuildKernels(CLGL * clgl, std::string str)
{
  setKernel(str.data())
  setKernel("Gravity_rk1")
  setKernel("Gravity_rk2")
  setKernel("Gravity_rk4")
  setKernel("memset")
  setKernel("Solid_accel")
  setKernel("Solid_rk1")
  setKernel("Fluid_Density")
  setKernel("Fluid_rk1")
  setKernel("copyVBO")
}

void PushDataToDevice(CLGL * clgl, data * hostData)
{
  // Load Fluid data
  if(hostData->f != NULL){
      clgl->CLGLLoadVBODataToDevice(hostData->f->pos->size() * sizeof(float4), &((*hostData->f->pos)[0]), CL_MEM_READ_WRITE);
      clgl->CLGLLoadVBODataToDevice(hostData->f->color->size() * sizeof(float4), &((*hostData->f->color)[0]), CL_MEM_READ_WRITE);

      clgl->CLGLLoadDataToDevice(CL_TRUE, hostData->f->size * sizeof(GLfloat), hostData->f->mass, CL_MEM_READ_ONLY);
      clgl->CLGLLoadDataToDevice(CL_TRUE, hostData->f->vel->size() * sizeof(float4), &((*hostData->f->vel)[0]), CL_MEM_READ_WRITE);
      clgl->CLGLLoadDataToDevice(CL_TRUE, hostData->f->vel->size() * sizeof(float), NULL, CL_MEM_READ_WRITE);
    }
    // Load solid data
    if(hostData->s != NULL){
      clgl->CLGLLoadVBODataToDevice(hostData->s->pos->size() * sizeof(float4), &((*hostData->s->pos)[0]), CL_MEM_READ_WRITE);
      clgl->CLGLLoadVBODataToDevice(hostData->s->color->size() * sizeof(float4), &((*hostData->s->color)[0]), CL_MEM_READ_WRITE);

      clgl->CLGLLoadDataToDevice(CL_TRUE, hostData->s->size * sizeof(GLfloat), hostData->s->mass, CL_MEM_READ_ONLY);
      clgl->CLGLLoadDataToDevice(CL_TRUE, hostData->s->vel->size() * sizeof(float4), &((*hostData->s->vel)[0]), CL_MEM_READ_WRITE);
      clgl->CLGLLoadDataToDevice(CL_TRUE, hostData->s->accel->size() * sizeof(float4), &((*hostData->s->accel)[0]), CL_MEM_READ_WRITE);

      clgl->CLGLLoadVBODataToDevice(hostData->s->neighbors->size() * sizeof(int), &((*hostData->s->neighbors)[0]), CL_MEM_READ_WRITE, GL_ELEMENT_ARRAY_BUFFER);
      clgl->CLGLLoadDataToDevice(CL_TRUE, hostData->s->lZero->size() * sizeof(float), &((*hostData->s->lZero)[0]), CL_MEM_READ_WRITE);

      // DEBUG
      clgl->CLGLLoadDataToDevice(CL_TRUE, (hostData->s->pos->size() + hostData->f->pos->size()) * sizeof(float4), NULL, CL_MEM_READ_WRITE);
    }
}

void CLGLSim::CLGLStartPhysics(CLGL * clgl, float rungeStep, data * hostData, CLGLParser * console)
{
  int nPhysicalKernels = 0;
 
  try{
    /*
     * Loads the physical simulation kernels
     */
    // Build the Source of the kernel
    clgl->CLGLBuildProgramSource(console->kernelFile, "-I./kernels");
    // Build the function console kernel in the kernel 
    BuildKernels(clgl, console->kernel);
    //Set the number of physical kernels that were charged
    nPhysicalKernels = clgl->CLGLGetKernel()->size();

    /*
     * Pushing data to Device
     */
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Pushing Data to Device" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    // Load Physical data
    PushDataToDevice(clgl, hostData);

    // Local variables for speed
    std::vector<cl::Buffer> * buff = clgl->CLGLGetBuffer();
    std::vector<cl::Memory> * mem = clgl->CLGLGetBufferGL();
    std::vector<cl::Kernel> * ker = clgl->CLGLGetKernel();

    // Set CLGLSim static members
    CLGLSim::vbo = clgl->CLGLGetVBO();
    CLGLSim::ParticlesNum = hostData->f->size + hostData->s->size;
    CLGLSim::NUM_PART_FLUID = hostData->f->size;
    CLGLSim::NUM_PART_SOLID = hostData->s->size;
    CLGLSim::rungeStep = rungeStep;
    CLGLSim::clgl = clgl;
    CLGLSim::rkx = ker;
    CLGLSim::buffer = buff;
    CLGLSim::indexBufferSize = hostData->s->neighbors->size();
    int num[3] = {console->particlesNum, console->NUM_PART_FLUID, console->NUM_PART_SOLID};
    CLGLSim::dataStruct = new Grid(clgl, num); //Add the data struct to the simulator

    /*
     * Set Arguments for the kernels
     */
    // Set Gravity_rk1 Arguments
    int beg = CLGLSim::dataStruct->getBuffBegin();
    for(int i=0; i < 1/*nPhysicalKernels*/; i++){
      clgl->CLGLSetArg(0, &(*buff)[0], (*ker)[i]);
      clgl->CLGLSetArg(1, &(*buff)[1], (*ker)[i]);
      clgl->CLGLSetArg(2, &(*mem)[0], (*ker)[i]);
      clgl->CLGLSetArg(3, &(*buff)[beg + gridIndex], (*ker)[i]);
      clgl->CLGLSetArg(4, &(*buff)[beg + gridCoord], (*ker)[i]);
      clgl->CLGLSetArg(5, &(*buff)[beg + nGridCubes], (*ker)[i]);
      clgl->CLGLSetArg(6, &(*buff)[beg + rebuildTreeFlag], (*ker)[i]);
      clgl->CLGLSetArg(7, &(*buff)[beg + sideSize], (*ker)[i]);
      clgl->CLGLSetArg(8, sizeof(float), &rungeStep, (*ker)[i]);
      clgl->CLGLSetArg(9, sizeof(int), &(CLGLSim::ParticlesNum), (*ker)[i]);
    }

    // Set Args to MemSet
    float4 aux;
    aux.x = aux.y = aux.z = aux.w = 0;
    clgl->CLGLSetArg(0, sizeof(float4), &aux, (*ker)[Memset]);
    clgl->CLGLSetArg(1, &(*buff)[solidAccel], (*ker)[Memset]);
    clgl->CLGLSetArg(2, sizeof(int), &(CLGLSim::NUM_PART_SOLID), (*ker)[Memset]);
    // Set Args to Solid_accel
    clgl->CLGLSetArg(0, &(*buff)[solidMass], (*ker)[Solid_accel]);
    clgl->CLGLSetArg(1, &(*mem)[solidPos], (*ker)[Solid_accel]);
    clgl->CLGLSetArg(2, &(*buff)[solidAccel], (*ker)[Solid_accel]);
    clgl->CLGLSetArg(3, &(*buff)[lZero], (*ker)[Solid_accel]);
    clgl->CLGLSetArg(4, &(*mem)[indexBuffer], (*ker)[Solid_accel]);
    clgl->CLGLSetArg(5, sizeof(float), &rungeStep, (*ker)[Solid_accel]);
    int n = hostData->s->neighbors->size();
    clgl->CLGLSetArg(6, sizeof(int), &n, (*ker)[Solid_accel]);
    // Set Args to Solid_rk1
    clgl->CLGLSetArg(0, &(*mem)[solidPos], (*ker)[Solid_rk1]);
    clgl->CLGLSetArg(1, &(*buff)[solidVel], (*ker)[Solid_rk1]);
    clgl->CLGLSetArg(2, &(*buff)[solidAccel], (*ker)[Solid_rk1]);
    clgl->CLGLSetArg(3, sizeof(float), &rungeStep, (*ker)[Solid_rk1]);
    clgl->CLGLSetArg(4, sizeof(int), &(CLGLSim::NUM_PART_SOLID), (*ker)[Solid_rk1]);

    // FLUID
    // Fluid_Density
    clgl->CLGLSetArg(0, &(*buff)[fluidMass], (*ker)[Fluid_Density]);
    clgl->CLGLSetArg(1, &(*buff)[fluidDensity], (*ker)[Fluid_Density]);
    clgl->CLGLSetArg(2, &(*mem)[fluidPos], (*ker)[Fluid_Density]);
    clgl->CLGLSetArg(3, &(*buff)[beg + gridCoord], (*ker)[Fluid_Density]);
    clgl->CLGLSetArg(4, &(*buff)[beg + nGridCubes], (*ker)[Fluid_Density]);
    clgl->CLGLSetArg(5, &(*buff)[beg + sideSize], (*ker)[Fluid_Density]);
    clgl->CLGLSetArg(6, &(*buff)[beg + gridIndex], (*ker)[Fluid_Density]);
    clgl->CLGLSetArg(7, sizeof(int), &(CLGLSim::ParticlesNum), (*ker)[Fluid_Density]);
    // Fluid_rk1
    clgl->CLGLSetArg(0, &(*buff)[fluidMass], (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(1, &(*buff)[fluidVel], (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(2, &(*buff)[fluidDensity], (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(3, &(*mem)[fluidPos], (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(4, &(*buff)[beg + gridCoord], (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(5, &(*buff)[beg + nGridCubes], (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(6, &(*buff)[beg + sideSize], (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(7, &(*buff)[beg + gridIndex], (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(8, &(*buff)[beg + rebuildTreeFlag], (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(9, sizeof(float), &(CLGLSim::rungeStep), (*ker)[Fluid_rk1]);
    clgl->CLGLSetArg(10, sizeof(int), &(CLGLSim::ParticlesNum), (*ker)[Fluid_rk1]);
  }
  catch(cl::Error error)
  {
    std::cout << error.what() << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(EXIT_FAILURE);
  }
  return;
}
