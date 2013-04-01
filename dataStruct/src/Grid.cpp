//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#include "Grid.hpp"

void Grid::printParameters(void)
{
  int nGridCubesVar, sideSizeVar;
  int * gridIndexVar = new int[this->NUM_PART];
  int4 * gridCoordVar = new int4[this->NUM_PART];
  int * nPartPerIndexVar;
  bool mVF, rTF;

  std::cout << "-------------------------------------" << std::endl;

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
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->getBuffBegin() + gridIndex], CL_TRUE, sizeof(int) * this->NUM_PART, gridIndexVar);
  std::cout << "gridIndex: ";
  for(int i=0; i < this->NUM_PART; i++)
    std::cout << gridIndexVar[i] << " | ";
  std::cout << std::endl;
  // gridCoord
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->getBuffBegin() + gridCoord], CL_TRUE, sizeof(int4) * this->NUM_PART, gridCoordVar);
  std::cout << "gridCoord: ";
  for(int i=0; i < this->NUM_PART; i++)
    std::cout << "(" << gridCoordVar[i].x << "," << gridCoordVar[i].y << "," << gridCoordVar[i].z << "," << gridCoordVar[i].w << ") | ";
  std::cout << std::endl;
  // nPartPerIndexVar
  nPartPerIndexVar = new int[nGridCubesVar * nGridCubesVar * nGridCubesVar];
  this->clgl->CLGLGetDataFromDevice(&(*this->buff)[this->getBuffBegin() + nPartPerIndex], CL_TRUE, sizeof(int) * nGridCubesVar * nGridCubesVar * nGridCubesVar, nPartPerIndexVar);
 std::cout << "nPartPerIndex: ";
  for(int i=0; i < nGridCubesVar * nGridCubesVar * nGridCubesVar; i++)
    std::cout << nPartPerIndexVar[i] << " | ";
  std::cout << std::endl;

  std::cout << "-------------------------------------" << std::endl;
}


/*
 * Constructor: builds grid's kernels, allocs 
 * grid memory, set rguments to grid's kernels
 */
Grid::Grid(CLGL * clgl, cl::Memory * pos, int NUM_PART)
{
  // Sets privates arguments
  this->clgl = clgl;
  this->NUM_PART = NUM_PART;
  this->kernel = this->clgl->CLGLGetKernel();
  this->buff = this->clgl->CLGLGetBuffer();
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
  this->setGridKernelArgs(pos);

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
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + getNGridCubes], this->NUM_PART);

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
    // Deletes the last nPartPerIndex vector
    //this->clgl->CLGLReleaseMemory(&(this->buff->back()));
    this->buff->pop_back();
    // Loads a vector of gridCubes size
    nGridCubesDevice = nGridCubesDevice * nGridCubesDevice * nGridCubesDevice; // nGridCubesDevice ^ 3
    //Creates a vector with zeros inside
    int * zero = new int[nGridCubesDevice];
    memset(zero, 0, sizeof(int) * nGridCubesDevice);
    // Loads to device the vector of zeros
    this->clgl->CLGLLoadDataToDevice(CL_TRUE, sizeof(int) * nGridCubesDevice, zero, CL_MEM_READ_WRITE); //nPartPerIndex
    // Sets nPartPerIndex as the argument for the kernels
    this->clgl->CLGLSetArg(4, (*this->buff)[this->buffBegin + nPartPerIndex], (*this->kernel)[this->kerBegin + setGridIndex]);
    this->clgl->CLGLSetArg(6, (*this->buff)[this->buffBegin + nPartPerIndex], (*this->kernel)[0]);
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
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + getGridSideSize], this->NUM_PART);

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
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + getNGridCubes], this->NUM_PART);

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

    // Loads a vector of gridCubes size
    nGridCubesDevice = nGridCubesDevice * nGridCubesDevice * nGridCubesDevice; // nGridCubesDevice ^ 3
    //Creates a vector with zeros inside
    int * zero = new int[nGridCubesDevice];
    memset(zero, 0, sizeof(int) * nGridCubesDevice);
    // Loads to device the vector of zeros
    this->clgl->CLGLLoadDataToDevice(CL_TRUE, sizeof(int) * nGridCubesDevice, zero, CL_MEM_READ_WRITE); //nPartPerIndex
    // Sets this vector as the argument 4 of setGridIndex
    this->clgl->CLGLSetArg(4, (*this->buff)[this->buffBegin + nPartPerIndex], (*this->kernel)[this->kerBegin + setGridIndex]);
    // Runs the kernel setGridIndex
    this->clgl->CLGLRunKernel((*this->kernel)[this->kerBegin + setGridIndex], this->NUM_PART);
    
    /*
     * Ordenate the Vector of the grid
     */
    this->ordenateVector();
    
    //this->printParameters();
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
  //build the kernels
  kernel = "getGridSideSize";
  this->clgl->CLGLBuildKernel(kernel);
  kernel = "getNGridCubes";
  this->clgl->CLGLBuildKernel(kernel);
  kernel = "setGridIndex";
  this->clgl->CLGLBuildKernel(kernel);
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
  this->clgl->CLGLLoadDataToDevice(CL_TRUE, this->NUM_PART * sizeof(int), NULL, CL_MEM_READ_WRITE); //grid Index
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
void Grid::setGridKernelArgs(cl::Memory * pos)
{
  // Setting args for kernel getGridSideSize
  this->clgl->CLGLSetArg(0, *pos, (*this->kernel)[this->kerBegin + getGridSideSize]);
  this->clgl->CLGLSetArg(1, (*this->buff)[this->buffBegin + sideSize], (*this->kernel)[this->kerBegin + getGridSideSize]);
  this->clgl->CLGLSetArg(2, sizeof(int), &this->NUM_PART, (*this->kernel)[this->kerBegin + getGridSideSize]);
  // Setting args to kernel getNGridCubes
  this->clgl->CLGLSetArg(0, *pos, (*this->kernel)[this->kerBegin + getNGridCubes]);
  this->clgl->CLGLSetArg(1, (*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + getNGridCubes]);
  this->clgl->CLGLSetArg(2, (*this->buff)[this->buffBegin + nGridCubes], (*this->kernel)[this->kerBegin + getNGridCubes]);
  this->clgl->CLGLSetArg(3, (*this->buff)[this->buffBegin + sideSize], (*this->kernel)[this->kerBegin + getNGridCubes]);
  this->clgl->CLGLSetArg(4, sizeof(int), &this->NUM_PART, (*this->kernel)[this->kerBegin + getNGridCubes]);
  // Setting args to kernel setGridIndex
  this->clgl->CLGLSetArg(0, *pos, (*this->kernel)[this->kerBegin + setGridIndex]);
  this->clgl->CLGLSetArg(1, (*this->buff)[this->buffBegin + gridIndex], (*this->kernel)[this->kerBegin + setGridIndex]);
  this->clgl->CLGLSetArg(2, (*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + setGridIndex]);
  this->clgl->CLGLSetArg(3, (*this->buff)[this->buffBegin + nGridCubes], (*this->kernel)[this->kerBegin + setGridIndex]);
  this->clgl->CLGLSetArg(5, sizeof(int), &this->NUM_PART, (*this->kernel)[this->kerBegin + setGridIndex]);
  // Setting args to kernel bubbleSort3D_even
  this->clgl->CLGLSetArg(0, (*this->buff)[this->buffBegin + gridIndex], (*this->kernel)[this->kerBegin + bubbleSort3D_even]);
  this->clgl->CLGLSetArg(1, (*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + bubbleSort3D_even]);
  this->clgl->CLGLSetArg(2, (*this->buff)[this->buffBegin + modificationVectorFlag], (*this->kernel)[this->kerBegin + bubbleSort3D_even]);
  this->clgl->CLGLSetArg(3, sizeof(int), &this->NUM_PART, (*this->kernel)[this->kerBegin + bubbleSort3D_even]);
  // Setting args to kernel bubbleSort3D_odd
  this->clgl->CLGLSetArg(0, (*this->buff)[this->buffBegin + gridIndex], (*this->kernel)[this->kerBegin + bubbleSort3D_odd]);
  this->clgl->CLGLSetArg(1, (*this->buff)[this->buffBegin + gridCoord], (*this->kernel)[this->kerBegin + bubbleSort3D_odd]);
  this->clgl->CLGLSetArg(2, (*this->buff)[this->buffBegin + modificationVectorFlag], (*this->kernel)[this->kerBegin + bubbleSort3D_odd]);
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
