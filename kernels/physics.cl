//------------------------------------//
//                                    //
//  Author : Tiago Lobato Gimenes     //
//  email : tlgimenes at gmail.com    //
//                                    //
//------------------------------------//

#include "grid.h"

#define FLUID 0
#define SOLID 1

#define ZERO 0.000001

#define X 0
#define Y 1
#define Z 2
#define MIN_DISTANCE 0.02
#define MAX_DISTANCE 100000.00


// ------------------------
//  SOLID IMPLEMENTATIONS  |
// ------------------------

#define K 1.000000

__kernel void memset(
    float4 operand,
    __global float4 * mem,
    int size)
{
  __private uint i = get_global_id(0);

  if(i < size)
  {
    mem[i] = operand;
  }
  return;
}

__kernel void Solid_accel(
    __constant float * mass,
    __global float4 * pos,
    __global float4 * accel,
    __global float * lZero,
    __global int * index,
    float rungeStep,
    int n)
{
  __private uint i = 2 * get_global_id(0);
  __private uint p1;
  __private uint p2;
  __private float4 r1, r2, l0, l1;
  __private float4 a1, a2;

  if(i < n){
    // Get the index
    p1 = index[i];
    p2 = index[i+1];
    // Calculate acceleration
    r1 = pos[p1];
    r2 = pos[p2];
    l1 = r2 - r1;
    l0 = normalize(l1) * lZero[i/2];
    a1 = K * (l1 - l0);
    a2 = -K * (l1 - l0);
    a1 /= mass[p1];
    a2 /= mass[p2];
    // Add to total acceleration for 1
    atomicAdd((__global float*)(accel + p1) + X * sizeof(float), a1.x);
    atomicAdd((__global float*)(accel + p1) + Y * sizeof(float), a1.y);
    atomicAdd((__global float*)(accel + p1) + Z * sizeof(float), a1.z);
    // Add to total acceleration for 2
    atomicAdd((__global float*)(accel + p2) + X * sizeof(float), a2.x);
    atomicAdd((__global float*)(accel + p2) + Y * sizeof(float), a2.y);
    atomicAdd((__global float*)(accel + p2) + Z * sizeof(float), a2.z);
  }
    return;
}

__kernel void Solid_rk1(
    __global float4 * pos,
    __global float4 * vel,
    __global float4 * accel,
    float rungeStep,
    int n)
{
  __private uint i = get_global_id(0);

  __private float4 v;

  if(i < n)
  {
    v = vel[i];
    vel[i] += accel[i] * rungeStep;
    pos[i] += v * rungeStep;
  }
 
  return;
}

// ------------------------
//  FLUID IMPLEMENTATIONS  |
// ------------------------

#define REST_DENSITY 1
#define k_cte 1;
#define ETA 0.5;
#define THRESHOLD 1.0;
#define SIGMA -0.5;

inline float W_POLY6(float r, float h)
{
  return 315 * ( pow(h*h - r*r, 3) ) / (64 * pow(h, 9) * M_PI_F);
}

// Calculates de density of the fluid
__kernel void Fluid_Density(
    __constant float * mass,
    __global float * density,
    __global float4 * pos,
    __global int4 * gridCoord,
    __global int * nGridCubes,
    __global int * sideSize,
    __global int2 * gridIndex,
    int n)
{
  __private uint i = get_global_id(0);

  // Physical constant to improove speed
  __private float4 p;
  __private float4 lenth;
  __private float dist;
  __private float dens;
  __private float m;
  __private float h = (float)*sideSize;
 // __private float h2 = h * h;
 // __private float h9 = h2 * h2; h9 *= h9;
 // __private float C = 1.566681/*4710608448*/ / (h9*h);
  // Variables of the grid
  __private int4 gCoord;
  __private int gridCubes  = *nGridCubes;
  __private int gridCubes2 = gridCubes  * gridCubes;
  __private int shift = gridCubes / 2;
  __private int2 index;

  if(i < n){
    gCoord = gridCoord[i];
    if(gridIndex[i].y == SOLID)
      return;
    dens = m = mass[gCoord.w];
    p = pos[gCoord.w];

    for(int j = gCoord.x-1; j <= gCoord.x+1; j++){
      for(int k = gCoord.y-1; k <= gCoord.y+1; k++){
        for(int l = gCoord.z-1; l <= gCoord.z+1; l++){
          index = getNeighbors(gridIndex, (j+shift) * gridCubes2 + (k+shift) * gridCubes + (l+shift), n);
          for(int in = index.x; in < index.y; in++){
            if(gridIndex[in].y == SOLID || in == i)
              continue;
            lenth = p - pos[gridCoord[in].w];
            dist = fast_length(lenth);
            // Implemented W_poly6 kernel
   //         if(dist < h2){
              if(dist < h){
   //           h9 = h2 - dist;
 //             h9 = h9 * h9 * h9;
              dens += m * W_POLY6(dist, h);
            }
          }
        }
      }
    }
    density[gCoord.w] = dens;
  }
  return;
}

inline float4 Gradient_W_SPIKY(float4 r, float h, float r_mod)
{
  return -r * 45 * (h-r_mod) * (h-r_mod) / (M_PI_F * (r_mod + ZERO)* h * h * h * h * h * h);
}

inline float4 Gradient_W_POLY6(float4 r, float h, float r_mod)
{
  return -r * 945 * ( h*h - r_mod*r_mod ) * ( h*h - r_mod*r_mod ) / (32 * M_PI_F * pow(h, 9));
}

inline float Laplacian_W_VISCOSITY(float r, float h)
{
  return 45 * (1 - r / h) / (M_PI_F * h * h * h * h * h);
}

inline float Laplacian_W_POLY6(float r, float h)
{
  return 945 * (h*h - r*r) * (r*r - 3 * (h*h-r*r) / 4) / (8 * M_PI_F * pow(h,9));
}

__kernel void Fluid_rk1(
    __constant float * mass,
    __global float4 * vel,
    __global float * density,
    __global float4 * pos,
    __global int4 * gridCoord,
    __global int * nGridCubes,
    __global int * sideSize,
    __global int2 * gridIndex,
    __global bool * rebuildTreeFlag,
    float rungeStep,
    int n)
{
  __private uint i = get_global_id(0);

  // Variables of the physics
  __private float4 p;
  __private float4 v;
  __private float m;
  __private float4 r;
  __private float h = (float)*sideSize;
  __private float h2 = h * h;
  __private float len;
  __private float dens_p;
  __private float dens_n;
  __private float pressure_p;
  __private float pressure_n;
  __private float4 forceRes = 0;
  __private float4 GCF = 0;
  __private float4 LCF = 0;
  __private float threshold = 1.0;
  __private float4 g;
 // g.x = g.y = g.w = 1.0f;
  g.w = 0.0;

  // Variables of the grid
  __private int4 gCoord;
  __private int gridCubes  = *nGridCubes;
  __private int gridCubes2 = gridCubes  * gridCubes;
  __private int shift = gridCubes / 2;
  __private int2 index;
  __private int4 newCoord;

  if(i < n){
    if(gridIndex[i].y == FLUID){
      gCoord = gridCoord[i];
      p = pos[gCoord.w];
      v = vel[gCoord.w];
      m = mass[gCoord.w];
      dens_p = density[gCoord.w];

      for(int j = gCoord.x-1; j <= gCoord.x+1; j++){
        for(int k = gCoord.y-1; k <= gCoord.y+1; k++){
          for(int l = gCoord.z-1; l <= gCoord.z+1; l++){
            index = getNeighbors(gridIndex, (j+shift) * gridCubes2 + (k+shift) * gridCubes + (l+shift), n);
            for(int in = index.x; in < index.y; in++){
              if(gridIndex[in].y == SOLID || in == i)
                continue;
              r = p - pos[gridCoord[in].w];
              len = length(r);
              // Implemented W_poly6 kernel
              if(len < h){
                // Calculates density and pressure
                dens_n = density[gridCoord[in].w];
                pressure_p = (dens_p - REST_DENSITY) * k_cte; 
                pressure_n = (dens_n - REST_DENSITY) * k_cte;
                // Pressure force
                forceRes += Gradient_W_SPIKY(r, h, len) * m * (pressure_p + pressure_n) / (2 * dens_n);
                // Viscosity force
                forceRes += ((vel[gridCoord[in].w] - v) * m * Laplacian_W_VISCOSITY(len, h) / dens_n) * ETA;
                // Color Field Gradient
                GCF += Gradient_W_POLY6(r, h, len) * m / dens_n;
                // Color Field Laplacian
                LCF += m * Laplacian_W_POLY6(len, h) / dens_n;
              }
            }
          }
        }
      }
      // Surface Tension
      len = length(GCF);
      if(len > threshold){
        forceRes += (GCF * LCF / len) * SIGMA;
      }
      pos[gCoord.w] += v * rungeStep;
      vel[gCoord.w] += (forceRes / dens_p + g * m) * rungeStep;

      newCoord = convert_int4_rtz(pos[gCoord.w]) / (*sideSize);
      if(newCoord.x != gCoord.x || newCoord.y != gCoord.y || newCoord.z != gCoord.z){
        *rebuildTreeFlag = true;
      }
    }
  }
  return;
}
 
// --------------------------
//  GRAVITY IMPLEMENTATIONS  |
// --------------------------

#define G 0.004302

__kernel void  Gravity_rk1(
    __constant float * mass, 
    __global float4 * vel,
    __global float4 * pos,
    __global int2 * gridIndex,
    __global int4 * gridCoord,
    __global int * nGridCubes,
    __global bool * rebuildTreeFlag,
    __global int * sideSize,
    float rungeStep, 
    int n)
{
  __private unsigned int i = get_global_id(0);

  // Private variables improves the kernel speed
  __private float4 posCur, velCur;
  __private float4 aRes = {0.0, 0.0, 0.0, 0.0};
  __private float r;
  // Private variables for the grid
  __private int4 gCoord;
  __private int4 newCoord;
  __private int2 index;
  __private int gridCubes  = *nGridCubes;
  __private int gridCubes2 = gridCubes  * gridCubes;
  __private int shift = gridCubes / 2;

  __private float4 aux;

  if(i < n){
    gCoord = gridCoord[i];
    if(gridIndex[i].y == SOLID)
      return;
    posCur = pos[gCoord.w];
    velCur = vel[gCoord.w];
    for(int j = gCoord.x-1; j <= gCoord.x+1; j++){
      for(int k = gCoord.y-1; k <= gCoord.y+1; k++){
        for(int l = gCoord.z-1; l <= gCoord.z+1; l++){
          index = getNeighbors(gridIndex, (j+shift) * gridCubes2 + (k+shift) * gridCubes + (l+shift), n);
          for(int in = index.x; in < index.y; in++){
            if(gridIndex[in].y == SOLID)
              continue;
            aux = pos[gridCoord[in].w] - posCur;
            r = dot(aux, aux);
            if(r <= MIN_DISTANCE || r >= MAX_DISTANCE) 
              continue;
            aRes += G * mass[gridCoord[in].w] * (aux) * rsqrt(r) / r;
          }
        }
      }
    }
    vel[gCoord.w] = velCur + aRes * rungeStep;
    posCur = posCur + velCur * rungeStep;
    pos[gCoord.w] = posCur + velCur * rungeStep;
    
    newCoord = convert_int4_rtz(posCur) / *sideSize;
    if(newCoord.x != gCoord.x || newCoord.y != gCoord.y || newCoord.z != gCoord.z){
      *rebuildTreeFlag = true;
    }
  }
}

// kernel Runge-Kutta 2
__kernel void Gravity_rk2(
    __constant float * mass, 
    __global float4 * vel,
    __global float4 * pos, 
    float rungeStep, 
    int n)
{
  __private unsigned int cur = get_global_id(0);

  // Private variables improves the kernel speed
  __private float4 velCur, posCur, posAux;
  __private float4 aRes[2];
  __private float4 dist[2];

  __private float4 reset = {0.0, 0.0, 0.0, 0.0};

  //Runge Kutta 2 variables
  __private float4 kPos[2];
  __private float4 kVel[2];

  __private float r[2];

  if(cur < n){
    //Iniciate variables
    aRes[0] = aRes[1] = reset;

    posCur = pos[cur];
    velCur = vel[cur];

    posAux = rungeStep * velCur;
    for(int i=0; i < n; i++){
      dist[0] = pos[i] - posCur;
      dist[1] = dist[0] + posAux;
      r[0] = dot(dist[0], dist[0]);
      r[1] = dot(dist[1], dist[1]);
      if(r[0] <= MIN_DISTANCE || r[0] >= MAX_DISTANCE)
        continue;
      aRes[0] += G * mass[i] * dist[0] * rsqrt(r[0]) / r[0];
      aRes[1] += G * mass[i] * dist[1] * rsqrt(r[1]) / r[1];
    }
    kPos[0] = 2.0f * velCur;
    kVel[0] = aRes[0];
    kPos[1] = rungeStep * aRes[0];
    kVel[1] = aRes[1];

    pos[cur] += (kPos[0] + kPos[1]) * rungeStep / 2.0f;
    vel[cur] += (kVel[0] + kVel[1]) * rungeStep / 2.0f; 
  }
}

// kernel for Runge-Kutta 4 method
__kernel void Gravity_rk4(
    __constant float * mass, 
    __global float4 * vel,
    __global float4 * pos,
    float rungeStep, 
    int n)
{
  // Get Proccess Global ID
  __private unsigned int cur = get_global_id(0);

  // Runge Kutta 4 variables;
  __private float4 kPos[4];
  __private float4 kVel[4];

  // Total acceleration pull
  __private float4 aRes = {0.0, 0.0, 0.0, 0.0};

  // Distance
  __private float r;
  __private float4 dist;

  // Reset to 0 float4 variable
  __private float4 reset = {0.0, 0.0, 0.0, 0.0};

  // Put current particle data in
  // local variables for speed !!
  __private float4 posCur;
  __private float4 posInit;
  __private float4 velCur;
  __private float4 velInit;

  if(cur < n){
    posCur = posInit = pos[cur];
    velCur = velInit = vel[cur];

    // Get k_v[0] and k_r[0]
    for(int i = 0; i < n; i++){
      dist = pos[i] - posCur;
      r = dot(dist, dist);
      // If it is good to iterate with the particle i
      if(r <= MIN_DISTANCE || r >= MAX_DISTANCE)
        continue;
      aRes += G * mass[i] * dist * rsqrt(r) / r;
    }
    kVel[0] = aRes * rungeStep;
    kPos[0] = velCur * rungeStep;

    // Get k_v[1] and k_r[1]
    posCur = posInit + kPos[0] / 2.0f;
    velCur = velInit + kVel[0] / 2.0f;
    aRes = reset;
    for(int i = 0; i < n; i++){
      dist = pos[i] - posCur;
      r = dot(dist, dist);
      // If it is good to iterate with the particle i
      if(r <= MIN_DISTANCE || r >= MAX_DISTANCE)
        continue;
      aRes += G * mass[i] * dist * rsqrt(r) / r;
    }
    kVel[1] = aRes * rungeStep;
    kPos[1] = velCur * rungeStep;

    // Get k_v[2] and k_r[2]
    posCur = posInit + kPos[1] / 2.0f;
    velCur = velInit + kVel[1] / 2.0f;
    aRes = reset;
    for(int i = 0; i < n; i++){
      dist = pos[i] - posCur;
      r = dot(dist, dist);
      // If it is good to iterate with the particle i
      if(r <= MIN_DISTANCE || r >= MAX_DISTANCE)
        continue;
      aRes += G * mass[i] * dist * rsqrt(r) / r;
    }
    kVel[2] = aRes * rungeStep;
    kPos[2] = velCur * rungeStep;

    // Get k_v[3] and k_r[3]
    posCur = posInit + kPos[2];
    velCur = velInit + kVel[2];
    aRes = reset;
    for(int i = 0; i < n; i++){
      dist = pos[i] - posCur;
      r = dot(dist, dist);
      // If it is good to iterate with the particle i
      if(r <= MIN_DISTANCE || r >= MAX_DISTANCE)
        continue;
      aRes += G * mass[i] * dist * rsqrt(r) / r;
    }
    kVel[3] = aRes * rungeStep;
    kPos[3] = velCur * rungeStep;

    // Calculate the final Position
    vel[cur] = velInit + (kVel[0] + 2*(kVel[1] + kVel[2]) + kVel[3]) / 6.0f;
    pos[cur] = posInit + (kPos[0] + 2*(kPos[1] + kPos[2]) + kPos[3]) / 6.0f;
  }
}

__kernel void copyVBO(
    __global float4 * src,
    __global float4 * dest)
{
  __private uint i = get_global_id(0);

  dest[i] = src[i];

  return;
}
