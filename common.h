#ifndef COMMON_H
#define COMMON_H

enum SPRITEACTION  {SA_NONE      = 0,
                    SA_KILL      = 1,
                    SA_ADDSPRITE = 2};

enum BOUNDSACTION {  BA_STOP   = 0,
                     BA_WRAP   = 1,
                     BA_BOUNCE = 2, // отскок
                     BA_DIE    = 3};

#endif // COMMON_H
