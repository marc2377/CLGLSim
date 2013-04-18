//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#include "Grid.hpp"

void Grid::printParameters(void)
{
  static int a = 0;
  int nGridCubesVar, sideSizeVar;
  int2 * gridIndexVar = new int2[this->NUM_PART];
  int4 * gridCoordVar = new int4[this->NUM_PART];
  bool mVF, rTF;

  std::cout << "-------------------------------------" << std::endl;

  std::cout << "Time: " << a << std::endl;
  a++;
  // nGridCubes
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->getBuffBegin() + nGridCubes], CL_TRUE, sizeof(int), &nGridCubesVar);
  std::cout << "nGridCubes: " << nGridCubesVar << std::endl;
  // sideSize
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->getBuffBegin() + sideSize], CL_TRUE, sizeof(int), &sideSizeVar);
  std::cout << "sideSize: " << sideSizeVar << std::endl;
  // rebuildTreeFlag
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->getBuffBegin() + rebuildTreeFlag], CL_TRUE, sizeof(bool), &rTF);
  std::cout << "rebuildTreeFlag: " << rTF << std::endl;
  // modificationVectorFlag
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->getBuffBegin() + modificationVectorFlag], CL_TRUE, sizeof(bool), &mVF);
  std::cout << "modificationVectorFlag: " << mVF << std::endl;
  // gridIndex
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->getBuffBegin() + gridIndex], CL_TRUE, sizeof(int2) * this->NUM_PART, gridIndexVar);
  std::cout << "gridIndex: ";
  for(int i=0; i < this->NUM_PART; i++)
    std::cout << " (" << gridIndexVar[i].x << " , " << gridIndexVar[i].y << ") ";
  std::cout << std::endl;
  // gridCoord
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->getBuffBegin() + gridCoord], CL_TRUE, sizeof(int4) * this->NUM_PART, gridCoordVar);
  std::cout << "gridCoord: ";
  for(int i=0; i < this->NUM_PART; i++)
    std::cout << "(" << gridCoordVar[i].x << "," << gridCoordVar[i].y << "," << gridCoordVar[i].z << "," << gridCoordVar[i].w << ") | ";
  std::cout << std::endl;
  // Position
  std::cout << "Solid Pos" << std::endl;
  this->clgl->CLGLSetArg(0, &(*this->buffGL)[solidPos], (*this->kernel)[copyVBO]);
  this->clgl->CLGLSetArg(1, &(*this->buff)[debug], (*this->kernel)[copyVBO]);
  this->clgl->CLGLRunKernel((*this->kernel)[copyVBO], this->NUM_PART_SOLID);
  float4 * v = new float4[this->NUM_PART];
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[debug], CL_TRUE, this->NUM_PART_SOLID * sizeof(float4), v);
  for (int i = 0; i < this->NUM_PART_SOLID; i++) {
    std::cout << "(" << v[i].x << "," << v[i].y << "," << v[i].z << ")";
  }
  std::cout << std::endl;
  std::cout << "Fluid Pos" << std::endl;
  this->clgl->CLGLSetArg(0, &(*this->buffGL)[fluidPos], (*this->kernel)[copyVBO]);
  this->clgl->CLGLSetArg(1, &(*this->buff)[debug], (*this->kernel)[copyVBO]);
  this->clgl->CLGLRunKernel((*this->kernel)[copyVBO], this->NUM_PART_FLUID);
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[debug], CL_TRUE, this->NUM_PART_FLUID * sizeof(float4), v);
  for (int i = 0; i < this->NUM_PART_FLUID; i++) {
    std::cout << "(" << v[i].x << "," << v[i].y << "," << v[i].z << ")";
  }
  std::cout << std::endl;
  std::cout << "Density" << std::endl;
  float * d = new float[this->NUM_PART_FLUID];
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[fluidDensity], CL_TRUE, this->NUM_PART_FLUID * sizeof(float), d);
  for (int i = 0; i < this->NUM_PART_FLUID; i++) {
    std::cout << d[i] << " | ";
  }
  std::cout << std::endl;

  std::cout << "-------------------------------------" << std::endl;
}


/*
 * Constructor: builds grid's kernels, allocs 
 * grid memory, set rguments to grid's kernels
 */
Grid::Grid(CLGL * clgl, int * NUM_PART)
{
  // Sets privates arguments
  this->clgl = clgl;
  this->NUM_PART = NUM_PART[0];
  this->NUM_PART_FLUID = NUM_PART[1];
  this->NUM_PART_SOLID = NUM_PART[2];
  this->kernel = this->clgl->CLGLGetKernel();
  this->buff = this->clgl->CLGLGetBuffer();
  this->buffGL = this->clgl->CLGLGetBufferGL();
  this->kerBegin = this->kernel->size(); //firt index of grid kernels
  this->buffBegin = this->buff->size();
  
  /*
   * Loads the Data Structures kernels
   */
  this->buildGridKernels();
 
  /*
   * Push grid data to device 
   */
  this->pushGridDataToDevice();

  /*
   * Set Arguments to grid kernels
   */
  this->setGridKernelArgs();

  /*
   * Starts the grid
   */
  this->startGrid();

  return;
}

/*
 * This function creates a new grid at sets it up
 */
void Grid::refreshGrid(void)
{
  try{
    /*
     * Gets the number of grid cubes to be 
     * used in the simulation
     */
    int z = 0;
    this->clgl->CLGLModifyBufferOfDevice(&(*this->buff)[this->buffBegin + nGridCubes], CL_TRUE, 0, sizeof(int), &z);
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + getNGridCubesRefresh], this->NUM_PART);

    /*
     * Set the corresponding index of the grid 
     * to each particle
     */
    int nGridCubesDevice=0;
    // Gets the number of grid Cubes we have
    this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->buffBegin + nGridCubes], CL_TRUE, sizeof(int), &nGridCubesDevice);
    if(nGridCubesDevice <= 0){
      std::cout << "PROBLEMS WITH THE GRID" << std::endl;
      exit(EXIT_FAILURE);
    }
    // Runs the kernel setGridIndex
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + setGridIndex], this->NUM_PART);

    /*
     * Ordenate the Vector of the grid
     */
    this->ordenateVector();

    /*
     * Sets the flag to rebuild tree as false
     */
    bool flag = false;
    this->clgl->CLGLModifyBufferOfDevice(&(*this->buff)[this->buffBegin + rebuildTreeFlag], CL_TRUE, 0, sizeof(bool), &flag);
  }
  catch(cl::Error error){
     std::cout << error.what() << ' ' << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(EXIT_FAILURE);
  } 

  return;
}

/*
 * Builds from zero a new grid
 */
void Grid::startGrid(void)
{
  int sideSizeDevice = 0;

  try{
    /*
     * Gets n * GridSize, divide by n and stores the average gridSize
     */
    // Gets n * GridSize
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + getGridSideSizeFluid], this->NUM_PART_FLUID);
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + getGridSideSizeSolid], this->NUM_PART_SOLID);

    this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->buffBegin + sideSize], CL_TRUE, sizeof(int), &sideSizeDevice);
    
    // Gets the average of distances
    sideSizeDevice /= this->NUM_PART;
    if(sideSizeDevice <= 0){
      std::cout << "The grid Found an anomalous ";
      std::cout << "phenomena happening, the ";
      std::cout << "program will go on, but maybe ";
      std::cout << "with problems" << std::endl;
      sideSizeDevice = 1;
    }
    // Stores in the device the average of the distances
    this->clgl->CLGLModifyBufferOfDevice(&(*this->buff)[this->buffBegin + sideSize], CL_TRUE, 0, sizeof(int), (void*)&sideSizeDevice);

    /*
     * Builds the grid
     */

    /*
     * Gets the number of grid cubes to be 
     * used in the simulation
     */
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + getNGridCubesFluid], this->NUM_PART_FLUID);
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + getNGridCubesSolid], this->NUM_PART_SOLID);

    /*
     * Set the corresponding index of the grid 
     * to each particle
     */
    int nGridCubesDevice=0;
    // Gets the number of grid Cubes we have
    this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->buffBegin + nGridCubes], CL_TRUE, sizeof(int), &nGridCubesDevice);
    if(nGridCubesDevice <= 0){
      std::cout << "PROBLEMS WITH THE GRID" << std::endl;
      exit(EXIT_FAILURE);
    }
   // Runs the kernel setGridIndex
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + setGridIndex], this->NUM_PART);
    
    /*
     * Ordenate the Vector of the grid
     */
    this->ordenateVector();
 }
  catch(cl::Error error){
    std::cout << error.what() << ' ' << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(EXIT_FAILURE);
  }
}

/*
 * Ordenate the grid Index vector in the GPU
 */
void Grid::ordenateVector(void)
{
  bool boolean = true;

  while(boolean == true){
    // Reset Algorithm
    boolean = false;
    this->clgl->CLGLModifyBufferOfDevice(&(*this->buff)[this->buffBegin + modificationVectorFlag], CL_TRUE, 0, sizeof(bool), (void*)&boolean);

    // Run two times the algorithm
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + bubbleSort3D_even], this->NUM_PART/2);
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + bubbleSort3D_odd], this->NUM_PART/2);

    // Get back the value of boolean
    this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->buffBegin + modificationVectorFlag], CL_TRUE, sizeof(bool), (void*)&boolean);
  }
}

/*
 * Builds the grid's kernels
 */
void Grid::buildGridKernels(void)
{
  std::string kernel;

  this->clgl->CLGLBuildProgramSource("kernels/grid.cl", "-I./kernels/"); //build the kernel file
  //build the kernels for the fluid particles
  kernel = "getGridSideSize";
  this->clgl->CLGLBuildKernel(kernel);
  kernel = "getNGridCubes";
  this->clgl->CLGLBuildKernel(kernel);

  //build the kernels for the solid particles
  kernel = "getGridSideSize";
  this->clgl->CLGLBuildKernel(kernel);
  kernel = "getNGridCubes";
  this->clgl->CLGLBuildKernel(kernel);
 
  // This function is commun for both
  kernel = "getNGridCubesRefresh";
  this->clgl->CLGLBuildKernel(kernel);
  kernel = "setGridIndex";
  this->clgl->CLGLBuildKernel(kernel);

  // sort kernel
  kernel = "bubbleSort3D_even";
  this->clgl->CLGLBuildKernel(kernel);
  kernel = "bubbleSort3D_odd";
  this->clgl->CLGLBuildKernel(kernel);

  return;
}

/*
 * Push the data that will be used by the grid 
 * to the GPU
 */
void Grid::pushGridDataToDevice(void)
{
  int2 * gridIndex = new int2[this->NUM_PART];
  
  for (int i = 0; i < this->NUM_PART_FLUID; i++) {
    gridIndex[i].x = 42;
    gridIndex[i].y = FLUID;
  }
  for (int i = this->NUM_PART_FLUID; i < this->NUM_PART; i++) {
    gridIndex[i].x = 51;
    gridIndex[i].y = SOLID;
  }
  this->clgl->CLGLLoadDataToDevice(CL_TRUE, this->NUM_PART * sizeof(int2), gridIndex, CL_MEM_READ_WRITE); //grid Index
  this->clgl->CLGLLoadDataToDevice(CL_TRUE, this->NUM_PART * sizeof(int4), NULL, CL_MEM_READ_WRITE); //grid Coord
  int nGridCubes = 3;
  this->clgl->CLGLLoadDataToDevice(CL_TRUE, sizeof(int), &nGridCubes, CL_MEM_READ_WRITE); //nGridCubes
  bool boolean = true;
  this->clgl->CLGLLoadDataToDevice(CL_TRUE, sizeof(bool), &boolean, CL_MEM_READ_WRITE); //modificationVectorFlag
  int sideSize = 0;
  this->clgl->CLGLLoadDataToDevice(CL_TRUE, sizeof(int), &sideSize, CL_MEM_READ_WRITE); //sideSize
  boolean = false;
  this->clgl->CLGLLoadDataToDevice(CL_TRUE, sizeof(bool), &boolean, CL_MEM_READ_WRITE); //rebuildTreeFlag
}

/*
 * Set the grid's kernels arguments
 */
void Grid::setGridKernelArgs()
{
  int offset = 0;

  // Set the fluids grid kernels arguments
  // Setting args for kernel getGridSideSize
  this->clgl->CLGLSetArg(0, &(*this->buffGL)[fluidPos], (*this->kernel)[this->kerBegin + getGridSideSizeFluid]);
  this->clgl->CLGLSetArg(1, &(*this->buff)[this->buffBegin + sideSize], (*this->kernel)[this->kerBegin + getGridSideSizeFluid]);
  this->clgl->CLGLSetArg(2, sizeof(int), &this->NUM_PART_FLUID, (*this->kernel)[this->kerBegin + getGridSideSizeFluid]);
  // Setting args to kernel getNGridCubes
  this->clgl->CLGLSetArg(0, &(*this->buffGL)[fluidPos], (*this->kernel)[this->kerBegin + getNGridCubesFluid]);
  this->clgl->CLGLSetArg(1, &(*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + getNGridCubesFluid]);
  this->clgl->CLGLSetArg(2, &(*this->buff)[this->buffBegin + nGridCubes], (*this->kernel)[this->kerBegin + getNGridCubesFluid]);
  this->clgl->CLGLSetArg(3, &(*this->buff)[this->buffBegin + sideSize], (*this->kernel)[this->kerBegin + getNGridCubesFluid]);
  this->clgl->CLGLSetArg(4, sizeof(int), &offset, (*this->kernel)[this->kerBegin + getNGridCubesFluid]);
  this->clgl->CLGLSetArg(5, sizeof(int), &this->NUM_PART_FLUID, (*this->kernel)[this->kerBegin + getNGridCubesFluid]);
  
  // Offset must be the number of fluid particles
  offset = this->NUM_PART_FLUID;

  // Sets the solids grids kernels arguments
  // Setting args for kernel getGridSideSize
  this->clgl->CLGLSetArg(0,&(*this->buffGL)[solidPos], (*this->kernel)[this->kerBegin + getGridSideSizeSolid]);
  this->clgl->CLGLSetArg(1, &(*this->buff)[this->buffBegin + sideSize], (*this->kernel)[this->kerBegin + getGridSideSizeSolid]);
  this->clgl->CLGLSetArg(2, sizeof(int), &this->NUM_PART_SOLID, (*this->kernel)[this->kerBegin + getGridSideSizeSolid]);
  // Setting args to kernel getNGridCubes
  this->clgl->CLGLSetArg(0, &(*this->buffGL)[solidPos], (*this->kernel)[this->kerBegin + getNGridCubesSolid]);
  this->clgl->CLGLSetArg(1, &(*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + getNGridCubesSolid]);
  this->clgl->CLGLSetArg(2, &(*this->buff)[this->buffBegin + nGridCubes], (*this->kernel)[this->kerBegin + getNGridCubesSolid]);
  this->clgl->CLGLSetArg(3, &(*this->buff)[this->buffBegin + sideSize], (*this->kernel)[this->kerBegin + getNGridCubesSolid]);
  this->clgl->CLGLSetArg(4, sizeof(int), &offset, (*this->kernel)[this->kerBegin + getNGridCubesSolid]);
  this->clgl->CLGLSetArg(5, sizeof(int), &(this->NUM_PART_SOLID), (*this->kernel)[this->kerBegin + getNGridCubesSolid]);

  // Sets args to commun kernel
  // Setting args to kernel getNGridCubesRefresh
  this->clgl->CLGLSetArg(0, &(*this->buffGL)[solidPos], (*this->kernel)[this->kerBegin + getNGridCubesRefresh]);
  this->clgl->CLGLSetArg(1, &(*this->buffGL)[fluidPos], (*this->kernel)[this->kerBegin + getNGridCubesRefresh]);
  this->clgl->CLGLSetArg(2, &(*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + getNGridCubesRefresh]);
  this->clgl->CLGLSetArg(3, &(*this->buff)[this->buffBegin + gridIndex], (*this->kernel)[this->kerBegin + getNGridCubesRefresh]);
  this->clgl->CLGLSetArg(4, &(*this->buff)[this->buffBegin + nGridCubes], (*this->kernel)[this->kerBegin + getNGridCubesRefresh]);
  this->clgl->CLGLSetArg(5, &(*this->buff)[this->buffBegin + sideSize], (*this->kernel)[this->kerBegin + getNGridCubesRefresh]);
  this->clgl->CLGLSetArg(6, sizeof(int), &this->NUM_PART, (*this->kernel)[this->kerBegin + getNGridCubesRefresh]);
  // Setting args to kernel setGridIndex
  this->clgl->CLGLSetArg(0, &(*this->buff)[this->buffBegin + gridIndex], (*this->kernel)[this->kerBegin + setGridIndex]);
  this->clgl->CLGLSetArg(1, &(*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + setGridIndex]);
  this->clgl->CLGLSetArg(2, &(*this->buff)[this->buffBegin + nGridCubes], (*this->kernel)[this->kerBegin + setGridIndex]);
  this->clgl->CLGLSetArg(3, sizeof(int), &this->NUM_PART, (*this->kernel)[this->kerBegin + setGridIndex]);


  // Setting args to kernel bubbleSort3D_even
  this->clgl->CLGLSetArg(0, &(*this->buff)[this->buffBegin + gridIndex], (*this->kernel)[this->kerBegin + bubbleSort3D_even]);
  this->clgl->CLGLSetArg(1, &(*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + bubbleSort3D_even]);
  this->clgl->CLGLSetArg(2, &(*this->buff)[this->buffBegin + modificationVectorFlag], (*this->kernel)[this->kerBegin + bubbleSort3D_even]);
  this->clgl->CLGLSetArg(3, sizeof(int), &this->NUM_PART, (*this->kernel)[this->kerBegin + bubbleSort3D_even]);
  // Setting args to kernel bubbleSort3D_odd
  this->clgl->CLGLSetArg(0, &(*this->buff)[this->buffBegin + gridIndex], (*this->kernel)[this->kerBegin + bubbleSort3D_odd]);
  this->clgl->CLGLSetArg(1, &(*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + bubbleSort3D_odd]);
  this->clgl->CLGLSetArg(2, &(*this->buff)[this->buffBegin + modificationVectorFlag], (*this->kernel)[this->kerBegin + bubbleSort3D_odd]);
  this->clgl->CLGLSetArg(3, sizeof(int), &this->NUM_PART, (*this->kernel)[this->kerBegin + bubbleSort3D_odd]);
}

int Grid::getBuffBegin(void)
{
  return this->buffBegin;
}

int Grid::getKerBegin(void)
{
  return this->kerBegin;
}
