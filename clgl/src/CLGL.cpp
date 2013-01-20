//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#include "CLGL.hpp"

// ----------- //
// Constructor //
// ----------- //
CLGL::CLGL(void)
{
  // Initializate Vectors
  this->bufferGL = new std::vector<cl::Memory>();
  this->buffer = new std::vector<cl::Buffer>();
  this->kernel = new std::vector<cl::Kernel>();
}

/*
 * Start OpenCL OpenGL interop and initialize command queue
 */
void CLGL::CLGLStart(int argc, char * argv[], std::string windowName, int windowHeight, int windowWidth)
{
  // ----------------- //
  // Init Basic OpenGL //
  // ----------------- //
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 -windowWidth /2, 
      glutGet(GLUT_SCREEN_HEIGHT)/2 - windowHeight/2);

  this->windowID = glutCreateWindow(windowName.c_str());

  glewInit();

  try{
    // ------------------------------- //
    // Init OpenCL Device and Platform //
    // ------------------------------- //
    
    // Device Type
    this->dev = CL_DEVICE_TYPE_DEFAULT;

    // Get the Platforms
    cl::Platform::get(&(this->platform));

    // Get the Devices
    this->platform[0].getDevices(this->dev, &(this->device));
  
    // --------------------------------- // 
    // Init OpenCL OpenGL Shared Context //
    // --------------------------------- //
    this->CLGLCreateContext();

    // ------------------------- //
    // Init OpenCL Command Queue //
    // ------------------------- //
    this->CLGLCreateCommandQueue();
  }
  catch(cl::Error error){
    std::cout << error.what() << ' ' << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(1);
  }
}

/*
 * Get the info from device dev
 */
void CLGL::CLGLGetDevicesInfo(cl::Device dev, cl_device_info name, std::string * info)
{
  try{
    dev.getInfo(name, info);
  }
  catch(cl::Error error){
    std::cout << error.what() << " " << CLGLError::errToStr(error.err()) << std::endl;
  }
  
  return;
}


/*
 * Macro for print devices info
 */
#define PRINTDEVICEINFO(INFO_NAME, STR_INFO_NAME, INFO_TYPE, SPACE) \
  std::cout << "---------------------------------------------------------------" << std::endl; \
  std::cout << STR_INFO_NAME << std::endl; \
  try{ \
    for(unsigned int i=0; i < this->device.size(); i++){ \
      this->device[i].getInfo(INFO_NAME, &INFO_TYPE); \
      std::cout << "Device " << i << ": "; \
      for(unsigned int j=0; j < INFO_TYPE.size(); j++){ \
        std::cout << INFO_TYPE[j] << SPACE; \
      } \
      std::cout << std::endl; \
    } \
  } \
  catch(cl::Error error){ \
    std::cout << error.what() << " " << CLGLError::errToStr(error.err()) << std::endl; \
  } \
 
/*
 * Macro for print platform info
 */
#define PRINTPLATFORMINFO(INFO_NAME, STR_INFO_NAME) \
  std::cout << "---------------------------------------------------------------" << std::endl; \
  this->platform[0].getInfo(INFO_NAME, &info); \
  std::cout << STR_INFO_NAME << std::endl; \
  std::cout << info << std::endl;
  

/*
 * Print All information about the current platform 
 * and all Devices
 */
void CLGL::CLGLPrintAllInfo(void)
{
  std::string info;
  std::vector<size_t> v;
  std::vector<cl_uint> c;

  std::cout << std::endl << "---------------------------------------------------------------" << std::endl;
  std::cout << "PLATFORM INFO" << std::endl;
  std::cout << "---------------------------------------------------------------" << std::endl << std::endl;
  

  PRINTPLATFORMINFO(CL_PLATFORM_PROFILE, "--->Plaftorm Profile") 
  PRINTPLATFORMINFO(CL_PLATFORM_VERSION, "--->Plaftorm Version") 
  PRINTPLATFORMINFO(CL_PLATFORM_NAME, "--->Plaftorm Name") 
  PRINTPLATFORMINFO(CL_PLATFORM_VENDOR, "--->Plaftorm Vendor") 
  PRINTPLATFORMINFO(CL_PLATFORM_EXTENSIONS, "--->Plaftorm Extensions") 
  std::cout << "---------------------------------------------------------------" << std::endl;

  std::cout << std::endl << "---------------------------------------------------------------" << std::endl;
  std::cout << "DEVICES INFO  " << std::endl;
  std::cout << "---------------------------------------------------------------" << std::endl << std::endl;
  
  PRINTDEVICEINFO(CL_DEVICE_NAME, "--->Devices Names:", info, "")
  PRINTDEVICEINFO(CL_DEVICE_VENDOR, "--->Devices Vendors:", info, "")
  PRINTDEVICEINFO(CL_DEVICE_PROFILE, "--->Devices Profiles:", info, "")
  PRINTDEVICEINFO(CL_DEVICE_VERSION, "--->Devices Versions:", info, "")
  PRINTDEVICEINFO(CL_DRIVER_VERSION, "--->Devices Versions:", info, "")
  PRINTDEVICEINFO(CL_DEVICE_OPENCL_C_VERSION, "--->Devices OpenCL C Versions", info, "")
  PRINTDEVICEINFO(CL_DEVICE_EXTENSIONS, "--->Devices Extensions", info, "")
  PRINTDEVICEINFO(CL_DEVICE_MAX_WORK_ITEM_SIZES, "--->Devices Max Work Itens Sizes", v, " ")
  PRINTDEVICEINFO(CL_DEVICE_MAX_WORK_GROUP_SIZE, "--->Devices Max Work Group Sizes", v, " ")
  PRINTDEVICEINFO(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, "--->Devices Max Work Item Dimensions", c, " ")
  PRINTDEVICEINFO(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, "--->Devices Global Mem Cache Size", v, " ")
  PRINTDEVICEINFO(CL_DEVICE_GLOBAL_MEM_SIZE, "--->Devices Global Mem Size", v, " ")
  PRINTDEVICEINFO(CL_DEVICE_MAX_CLOCK_FREQUENCY, "--->Devices Max Clock Frequency", c, " ")
  PRINTDEVICEINFO(CL_DEVICE_MAX_COMPUTE_UNITS, "--->Devices Max Compute Units", c, " ")
  PRINTDEVICEINFO(CL_DEVICE_MAX_MEM_ALLOC_SIZE, "--->Devices Max Mem Alloc Size", v, " ")
  PRINTDEVICEINFO(CL_DEVICE_ADDRESS_BITS, "--->Devices Address Bits", c, " ")
  std::cout << "---------------------------------------------------------------" << std::endl;
 
  std::cout << std::endl;
}

/*
 * Build the Program Source
 */
void CLGL::CLGLBuildProgramSource(std::string programName, std::string compilerFlags)
{
  int begin = 0, end = 0, lenth;
  char * buffer = NULL;
  std::fstream programFile;
  std::string programSource;

  // Opens the file  
  programFile.open(programName.c_str());
  
  // Check if file is valid
  if(!programFile){
    std::cout << "File: " << programName.c_str() << " is not valid as data file!" << std::endl;
    std::cout << "CLGLSim will be closed" << std::endl;
    exit(1);
  }   

  // Get the size of the file
  begin = programFile.tellg();
  programFile.seekg(0, std::ios::end);
  end = programFile.tellg();
  programFile.seekg(0, std::ios::beg);
  lenth = end - begin;
  buffer = new char[lenth];
  
  // Stores the content of the file in buffer
  programFile.read(buffer, lenth);
  programSource = buffer;
  
  try{
    // Builds the Program Source
    cl::Program::Sources source(1, std::make_pair(programSource.c_str(), programSource.size()));
    this->program = cl::Program(this->context, source);
    this->program.build(this->device, compilerFlags.data());
  }
  catch(cl::Error error){
    std::cout << error.what() << ' ' << CLGLError::errToStr(error.err())->c_str() << std::endl;
    if(!strcmp(error.what(), "clBuildProgram")){ // If Compilation Errors print then
      cl::STRING_CLASS log;
      this->program.getBuildInfo(this->device[0], CL_PROGRAM_BUILD_LOG, &log);
      std::cout << log.data() << std::endl;
    }
    exit(1);
  }
  
  return;
}

/*
 * Build the Program Source without any compiler flag
 */
void CLGL::CLGLBuildProgramSource(std::string programName)
{
  std::string str = "";

  CLGLBuildProgramSource(programName, str);
  return;
}

/*
 * Put the kernel in the last position in the kernel array
 */
cl::Kernel* CLGL::CLGLBuildKernel(std::string kernelFunctionName)
{
  std::vector<cl::Kernel>::iterator pos;

  try{
    pos = this->kernel->end();
    pos = this->kernel->insert(pos, cl::Kernel(this->program, kernelFunctionName.data()));
  }
  catch(cl::Error error){
    std::cout << error.what() << ' ' << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(1);
  }

  return &(*pos);
}

/*
 * Load the data in hostMemory to OpenCL Device
 */
cl::Buffer* CLGL::CLGLLoadDataToDevice(cl_bool blocking, size_t bufferBytesSize, const void * hostMemory, cl_mem_flags flag)
{
  cl::Buffer buff;

  try{
    this->commandQueue.finish();
    buff = cl::Buffer(this->context, flag, bufferBytesSize);
    
    this->buffer->push_back(buff);
    
    this->commandQueue.enqueueWriteBuffer(buff, blocking, 0, bufferBytesSize, hostMemory);
  }
  catch(cl::Error error){
    std::cout << error.what() << ' ' << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(1);
  }

  return &(this->buffer->back());  
}

/*
 * Load host data hostMemory to Device in VBO
 */
cl::Memory* CLGL::CLGLLoadVBODataToDevice(size_t bufferBytesSize, void * hostMemory, cl_mem_flags flag)
{
  GLuint id = 0;  // 0 is reserved, glGenBuffersARB() will return non-zero id if success

  glGenBuffers(1, &id);                          // create a vbo
  glBindBuffer(GL_ARRAY_BUFFER, id);                      // activate vbo id to use
  glBufferData(GL_ARRAY_BUFFER, bufferBytesSize, hostMemory, GL_DYNAMIC_DRAW); // create buffer

  // check data size in VBO is same as input array, if not return 0 and delete VBO
  int bufferSize = 0;
  glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
  if((int)bufferBytesSize != bufferSize){
    glDeleteBuffers(1, &id);
    id = 0;
    std::cout << "[createVBO()] Data size is mismatch with input array\n";
    exit(1);
  }

  //Adds the vbo id
  this->vbo.push_back(id);
  
  //this was important for working inside blender!
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glFinish();  // Wait for OpenGL to finish

  // Create buffer from OpenGL VBO
  try{
    this->bufferGL->push_back(cl::BufferGL(this->context, flag, id));
  }
  catch(cl::Error error){
    std::cout << error.what() << ' ' << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(1);
  }
  return &(this->bufferGL->back());
}

/*
 * Set Arguments to kernel
 */
void CLGL::CLGLSetArg(int argNum, cl::Memory buffer, cl::Kernel kernel)
{
  kernel.setArg(argNum, buffer);
  return;
}

/*
 * Set Arguments to kernel
 */
void CLGL::CLGLSetArg(int argNum, size_t bytesSize, void* buffer, cl::Kernel kernel)
{
  kernel.setArg(argNum, bytesSize, buffer);
  return;
}

/*
 * Run the kernel
 */
void CLGL::CLGLRunKernel(std::vector<cl::Memory>* bufferGL, cl::Kernel kernel, int numThreads)
{
  try{
    // Wait for OpenGL finish what it is doing
    glFinish();

    // Get the GL Objects
    this->commandQueue.enqueueAcquireGLObjects(bufferGL, NULL, NULL);

    //Wait for OpenCL to finish
    this->commandQueue.finish();

    // Enqueue the kernel
    this->commandQueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(numThreads), cl::NullRange);

    // Wait for OpenCL to finish
    this->commandQueue.finish();

    // Release the OpenGL objects
    this->commandQueue.enqueueReleaseGLObjects(bufferGL, NULL, NULL);

    //Wait for OpenCL to finish
    this->commandQueue.finish();
  }
  catch(cl::Error error){
    std::cout << error.what() << ' ' << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(1);
  }
  return;
}

/*
 * Run the kernel
 */
void CLGL::CLGLRunKernel(std::vector<cl::Memory> bufferGL, cl::Kernel kernel, int numThreads)
{
  CLGLRunKernel(&bufferGL, kernel, numThreads);
  
  return;
}

/*
 * Run the kernel
 */
void CLGL::CLGLRunKernel(cl::Kernel kernel, int numThreads)
{
  CLGLRunKernel(this->bufferGL, kernel, numThreads);

  return;
}

/*
 * Get data from device
 */
void CLGL::CLGLGetDataFromDevice(cl::Buffer *buffer, cl_bool blocking, int bytes_size, void * data)
{
  // Wait until OpenGL finish it's tasks
  glFinish();
  // Wait until OpenCL finish it's tasks
  this->commandQueue.finish();
  // Read the data from the buffer
  this->commandQueue.enqueueReadBuffer(*buffer, blocking, 0, bytes_size, data);
  
  return;
}

/*
 * Get OpenGL VBO vector
 */
std::vector<cl::Memory>* CLGL::CLGLGetBufferGL(void)
{
  return this->bufferGL;
}

/*
 * Get OpenCL buffer vector
 */
std::vector<cl::Buffer>* CLGL::CLGLGetBuffer(void)
{
  return this->buffer;
}

/*
 * Get Kernels
 */
std::vector<cl::Kernel>* CLGL::CLGLGetKernel(void)
{
  return this->kernel;
}

/*
 * Get the VBO ID
 */
std::vector<int>* CLGL::CLGLGetVBO(void)
{
  return &(this->vbo);
}

/*
 * Get the Window ID
 */
int CLGL::CLGLGetWindowID(void)
{
  return this->windowID;
}
