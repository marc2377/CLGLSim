//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#include "CLGLWindow.hpp"

/*
 * Static Members definitions
 */

// GLUT Window ID
int CLGLWindow::glutWindowHandle = 0;

// Window Properties
int CLGLWindow::window_height = 600;
int CLGLWindow::window_width = 800;

// Mouse and orientation stuff
int CLGLWindow::mouse_buttons = 0;
float  CLGLWindow::translate_z = -1.f;
float  CLGLWindow::rotate_x = 0.0;
float  CLGLWindow::rotate_y = 0.0;
int CLGLWindow::mouse_old_x = 0;
int CLGLWindow::mouse_old_y = 0;
float CLGLWindow::scale[3] = {0.3, 0.3, 0.3};

// Frame Per Second Variables
float CLGLWindow::fps = 0.0f;

// Play Pause Variable
bool CLGLWindow::play = ON;

// String variables
bool CLGLWindow::showInfo = ON;
void * CLGLWindow::font = GLUT_BITMAP_TIMES_ROMAN_24; //GLUT_BITMAP_8_BY_13;
int CLGLWindow::fontSize = 30;
float CLGLWindow::stringColor[4] = {1.0, 1.0, 1.0, 1.0};

// Number of Particles
int CLGLWindow::NumParticles = 0;

/*
 * Constructor. Initate the Window
 */
CLGLWindow::CLGLWindow()
{
  // GLUT Functions
  glutDisplayFunc(CLGLWindowRender); //Main Rendering Function
  glutTimerFunc(30, CLGLWindowTimerCB, 30); //Determinate a Minimum Time Between Frames
  glutKeyboardFunc(CLGLWindowKeyboard);
  glutSpecialFunc(CLGLWindowSpecialKeys);
  glutMouseFunc(CLGLWindowMouse);
  glutMotionFunc(CLGLWindowMotion);
  glutIdleFunc(CLGLWindowCalculateFPS);
  CLGLWindowCreateMenus(); //Creates Menus

  glShadeModel(GL_SMOOTH);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glDisable(GL_DEPTH_TEST);

  // viewport
  glViewport(0, 0, CLGLWindow::window_width, CLGLWindow::window_height);

  // projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(90.0, (GLfloat)window_width / (GLfloat)window_height, 0.01, 1000.0);

  // set view matrix
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, translate_z);
  glScalef(CLGLWindow::scale[0], CLGLWindow::scale[1], CLGLWindow::scale[2]);
}

/*
 * Render each frame of the window
 */
void CLGLWindowRender(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // ------------------------------------ //
  // Updates The Positions and Velocities //
  // ------------------------------------ //
  // Updates the particle system by calling the kernel
  if(CLGLWindow::play == ON)
    CLGLSim::CLGLRunKernel();

  // -------------------------- //
  // Draw information on screen //
  // -------------------------- //
  // Draw info on screen as FPS etc
  CLGLWindowDrawInfo();

  // ------------------------------ //
  // Render the particles from VBOs //
  // ------------------------------ //
  //render the particles from VBOs
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_POINT_SMOOTH);
  glPointSize(0.1);

  //VBO Color must be inserted in last place
  glBindBuffer(GL_ARRAY_BUFFER, CLGLSim::vbo->back());
  glColorPointer(4, GL_FLOAT, 0, 0);

  // Binds the vertex VBO's
  glBindBuffer(GL_ARRAY_BUFFER, (*CLGLSim::vbo)[0]);
  glVertexPointer(3, GL_FLOAT, 16, 0);

  // Enable Cient State
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  //Need to disable these for blender
  glDisableClientState(GL_NORMAL_ARRAY);

  // Draw the simulation points
  glDrawArrays(GL_POINTS, 0, CLGLSim::ParticlesNum);
  glFlush();

  // Disable Client State
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  // Bind the buffers to zero
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_COLOR_ARRAY, 0);

  // Swap the buffers
  glutSwapBuffers();

}

/*
 * Draw info strings on screen, like FPS
 */
void CLGLWindowDrawInfo(void)
{
  if(CLGLWindow::showInfo == true){
    std::stringstream fps(std::stringstream::in | std::stringstream::out);
    std::stringstream simTime(std::stringstream::in | std::stringstream::out);
    static std::stringstream pause(std::stringstream::in | std::stringstream::out);
    static std::stringstream kernel(std::stringstream::in | std::stringstream::out);
    static float simulationTime = 0;

    pause.seekp(std::ios::beg);
    kernel.seekp(std::ios::beg);

    // Draw FPS
    fps << "FPS: " << CLGLWindow::fps;
    CLGLWindowDrawString(fps.str().c_str(), 1, CLGLWindow::window_height-CLGLWindow::fontSize, CLGLWindow::stringColor, CLGLWindow::font);

    // Draw How many particles are beeing simulated and if it is paused
    if(CLGLWindow::play == ON){
      pause << "Simulating " << CLGLWindow::NumParticles << " Particles";
      CLGLWindowDrawString(pause.str().c_str(), 1, 33, CLGLWindow::stringColor, CLGLWindow::font);
    }
    else{
      CLGLWindowDrawString("Paused", 1, 33, CLGLWindow::stringColor, CLGLWindow::font);
    }

    // Draw Simulaion time
    if(CLGLWindow::play == ON)
      simulationTime += CLGLSim::rungeStep;
    simTime << "Simulation Time: " << simulationTime;
    CLGLWindowDrawString(simTime.str().c_str(), CLGLWindow::window_width - 245, 7, CLGLWindow::stringColor, CLGLWindow::font);

    // Draw Current Kernel in Use
    kernel << "Kernel: Gravity with Runge Kutta " << CLGLSim::curKernel << std::endl;
    CLGLWindowDrawString(kernel.str().c_str(), 1, 7, CLGLWindow::stringColor, CLGLWindow::font);
  }
}

/*
 * Write 2d string using GLUT
 */
void CLGLWindowDrawString(const char *str, int x, int y, float color[4], void *font)
{
  //backup current model-view matrix
  glPushMatrix();                     // save current modelview matrix
  glLoadIdentity();                   // reset modelview matrix

  //set to 2D orthogonal projection
  glMatrixMode(GL_PROJECTION);     // switch to projection matrix
  glPushMatrix();                  // save current projection matrix
  glLoadIdentity();                // reset projection matrix
  gluOrtho2D(0, CLGLWindow::window_width, 0, CLGLWindow::window_height);  // set to orthogonal projection

  glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask

  glColor4fv(color);          // set text color

  glRasterPos2i(x, y);        // place text position

  // loop all characters in the string
  while(*str){
    glutBitmapCharacter(font, *str);
    ++str;
  }

  glPopAttrib();

  // restore projection matrix
  glPopMatrix();                   // restore to previous projection matrix

  // restore modelview matrix
  glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
  glPopMatrix();                   // restore to previous modelview matrix  
}

/*
 * Calculates the frames per second
 */
void CLGLWindowCalculateFPS(void)
{
  static int frameCount = 0;
  static int previousTime = 0;
  int currentTime = 0;

  //  Increase frame count
  frameCount++;

  //  Get the number of milliseconds since glutInit called
  //  (or first call to glutGet(GLUT ELAPSED TIME)).
  currentTime = glutGet(GLUT_ELAPSED_TIME);

  //  Calculate time passed
  int timeInterval = currentTime - previousTime;

  if(timeInterval > 1000)
  {
    //  calculate the number of frames per second
    CLGLWindow::fps = frameCount / (timeInterval / 1000.0f);

    //  Set time
    previousTime = currentTime;

    //  Reset frame count
    frameCount = 0;
  }
}

/*
 * Setup And Creates Menus
 */
void CLGLWindowCreateMenus(void){
  int kernelMenu;

  // Create Sub Menu
  kernelMenu = glutCreateMenu(CLGLWindowMenus);
  glutAddMenuEntry("Runge Kutta 1", RK1);
  glutAddMenuEntry("Runge Kutta 2", RK2);
  glutAddMenuEntry("Runge Kutta 4", RK4);

  // Create Menu
  glutCreateMenu(CLGLWindowMenus);
  glutAddMenuEntry("Pause", PLAY);
  glutAddMenuEntry("Hide Info", INFO);
  glutAddSubMenu("Change Kernel", kernelMenu);
  glutAddMenuEntry("Quit", QUIT);

  // Attach Menu to Mouse Button
  glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

/*
 * Menus Handler Function
 */
void CLGLWindowMenus(int value)
{
  switch(value){
    case PLAY: 
      if(CLGLWindow::play == ON){
        CLGLWindow::play = OFF;   //Play Pause Button
        glutChangeToMenuEntry(1, "Play", PLAY);
      }
      else{
        CLGLWindow::play = ON;
        glutChangeToMenuEntry(1, "Pause", PLAY);
      }
      break;
    case INFO:
      if(CLGLWindow::showInfo == ON){
        CLGLWindow::showInfo = OFF;
        glutChangeToMenuEntry(2, "Show Info", INFO);
      }
      else{
        CLGLWindow::showInfo = ON;
        glutChangeToMenuEntry(2, "Hide Info", INFO);
      }
      break;
    case RK1:
      (*CLGLSim::rkx)[0] = (*CLGLSim::rkx)[1];
      CLGLSim::curKernel = 1;
      break;
    case RK2:
      (*CLGLSim::rkx)[0] = (*CLGLSim::rkx)[2];
      CLGLSim::curKernel = 2;
      break;
    case RK4:
      (*CLGLSim::rkx)[0] = (*CLGLSim::rkx)[3];
      CLGLSim::curKernel = 4;
      break;
    case QUIT:
      CLGLWindowDestroy();
      break;
  }
}

/*
 * Delet OpenGL and OpenCL context
 */
void CLGLWindowDestroy(void)
{
  //this makes sure we properly cleanup our OpenCL context
  //delete example;
  if(CLGLWindow::glutWindowHandle)
    glutDestroyWindow(CLGLWindow::glutWindowHandle);

  exit(0);
}

void CLGLWindowTimerCB(int ms)
{ 
  //this makes sure the appRender function is called every ms miliseconds
  glutTimerFunc(ms, CLGLWindowTimerCB, ms);
  glutPostRedisplay();
}

/*
 * Defines what each key does
 */
void CLGLWindowKeyboard(unsigned char key, int x, int y)
{
  //this way we can exit the program cleanly
  switch(key)
  {
    case '\033': // escape quits
    case '\015': // Enter quits    
    case 'Q':    // Q quits
    case 'q':    // q (or escape) quits
      // Cleanup up and quit
      CLGLWindowDestroy();
      break;
    case ' ':
      if(CLGLWindow::play == ON){
        CLGLWindow::play = OFF;   //Play Pause Button
        glutChangeToMenuEntry(1, "Play", PLAY);
      }
      else{
        CLGLWindow::play = ON;
        glutChangeToMenuEntry(1, "Pause", PLAY);
      }
      break;
    case 'i':
    case 'I':
      if(CLGLWindow::showInfo == ON){
        CLGLWindow::showInfo = OFF;
        glutChangeToMenuEntry(2, "Show Info", INFO);
      }
      else{
        CLGLWindow::showInfo = ON;
        glutChangeToMenuEntry(2, "Hide Info", INFO);
      }
      break;
  }
}

/*
 * Defines what each special key does
 */
void CLGLWindowSpecialKeys(int key, int x, int y)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  switch(key)
  {
    case GLUT_KEY_LEFT:
      glRotatef(-3, 0.0, 1.0, 0.0);
      break;
    case GLUT_KEY_RIGHT:
      glRotatef(3, 0.0, 1.0, 0.0);
      break;
    case GLUT_KEY_UP:
      glRotatef(3, 1.0, 0.0, 0.0);
      break;
    case GLUT_KEY_DOWN:
      glRotatef(-3, 1.0, 0.0, 0.0);
      break;
    case GLUT_KEY_PAGE_UP:
      glTranslatef(0.0, 0.0, 0.3);
      break;
    case GLUT_KEY_PAGE_DOWN:
      glTranslatef(0.0, 0.0, -0.3);
      break;
  }
  return;
}

/*
 * Define Mouse interaction
 */
void CLGLWindowMouse(int button, int state, int x, int y)
{
  //handle mouse interaction for rotating/zooming the view
  if (state == GLUT_DOWN) {
    CLGLWindow::mouse_buttons |= 1<<button;
  } else if (state == GLUT_UP) {
    CLGLWindow::mouse_buttons = 0;
  }

  CLGLWindow::mouse_old_x = x;
  CLGLWindow::mouse_old_y = y;
}

/*
 * Define motion of the coordinate system
 */
void CLGLWindowMotion(int x, int y)
{
  //hanlde the mouse motion for zooming and rotating the view
  float dx, dy;
  dx = x - CLGLWindow::mouse_old_x;
  dy = y - CLGLWindow::mouse_old_y;

  if (CLGLWindow::mouse_buttons & 1) {
    CLGLWindow::rotate_x += dy * 0.2;
    CLGLWindow::rotate_y += dx * 0.2;
  } else if (CLGLWindow::mouse_buttons & 4) {
    CLGLWindow::translate_z += dy * 0.1;
  }

  CLGLWindow::mouse_old_x = x;
  CLGLWindow::mouse_old_y = y;

  // set view matrix
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, CLGLWindow::translate_z);
  glScalef(CLGLWindow::scale[0], CLGLWindow::scale[1], CLGLWindow::scale[2]);
  glRotatef(CLGLWindow::rotate_x, 1.0, 0.0, 0.0);
  glRotatef(CLGLWindow::rotate_y, 0.0, 1.0, 0.0);
}

/*
 * Run the Main Loop of GLUT
 */
void CLGLWindow::CLGLRunWindow(void)
{
  // Main loop
  glutMainLoop();

  return;
}
