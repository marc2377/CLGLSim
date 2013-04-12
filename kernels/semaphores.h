//------------------------------------//
//                                    //
//  Author : Tiago Lobato Gimenes     //
//  email : tlgimenes at gmail.com    //
//                                    //
//------------------------------------//

inline void getSemaphor(__global int * semaphor) {
  int occupied = atom_xchg(semaphor, 1);
  while(occupied > 0)
  {
    occupied = atom_xchg(semaphor, 1);
  }
  return;
}


inline void releaseSemaphor(__global int * semaphor)
{
  int prevVal = atom_xchg(semaphor, 0);

  return;
}
