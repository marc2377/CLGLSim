
                                                CLGLSim 1.0                             
******
******

CLGLSim is a General Purpose Physical Simulator. CLGLSim uses the CLGL library for using the OpenCL OpenGL shared context.
CLGLSim has the folowing simulation implementated

  + N-Body Gravity Simulation

---------------------------------------------------------------------------------------------------------------- 

  + How to Use

CLGLSim can receive comands through the terminal with these options

   -> OPTIONS                                                         
    
    --kernel : Defines what kernel to start with, it was implemented 
            Gravity_rk1 Gravity_rk2 and Gravity_rk4 kernels           
     
    --num : Defines how many stars to start with                     
    
    --precision : Set the precision for simulation                    
    
    --data-file : Set data file to load for simulation                 

Example:

 For running CLGLSim with Gravity_rk4 kernel just run the program:

  `./CLGLSim --kernel Gravity_rk4`

For running CLGLSim with more or less particles than 2000, 4000 for example, type the following:

  `./CLGLSim --num 4000`

---------------------------------------------------------------------------------------------------------------- 

  + TODO

    - Fix the constants units
    - Implement better Data Loader
    - Implement systems loader like a galaxy loader
    - Implement Other Physical Forces




