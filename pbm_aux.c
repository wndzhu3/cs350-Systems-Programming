// Wendy Zhu
// wzhu62@emory.edu
/* THIS  CODE  WAS MY OWN WORK , IT WAS  WRITTEN  WITHOUT  CONSULTING  ANY
SOURCES  OUTSIDE  OF  THOSE  APPROVED  BY THE  INSTRUCTOR. Wendy Zhu  */

#include "pbm.h"
#include <stdio.h>
#include <stdlib.h>

PPMImage * new_ppmimage( unsigned int w, unsigned int h, unsigned int m )
{
  PPMImage *new_ppm = malloc(sizeof(PPMImage));

  new_ppm->width = w;
  new_ppm->height = h;
  new_ppm->max = m;

  new_ppm->pixmap[0] = malloc(h*sizeof(unsigned int *));
  new_ppm->pixmap[1] = malloc(h*sizeof(unsigned int *));
  new_ppm->pixmap[2] = malloc(h*sizeof(unsigned int *));

  for (int i = 0; i < h; i++) {
    new_ppm->pixmap[0][i] = malloc(w*sizeof(unsigned int));
    new_ppm->pixmap[1][i] = malloc(w*sizeof(unsigned int));
    new_ppm->pixmap[2][i] = malloc(w*sizeof(unsigned int));
  }

  return new_ppm;
}

PBMImage * new_pbmimage( unsigned int w, unsigned int h )
{
  PBMImage *new_pbm = malloc(sizeof(PBMImage));

  new_pbm->width = w;
  new_pbm->height = h;

  new_pbm->pixmap = malloc(h*sizeof(unsigned int *));

  for (int i = 0; i < h; i++) {
    new_pbm->pixmap[i] = malloc(w*sizeof(unsigned int));
  }

  return new_pbm;
}

PGMImage * new_pgmimage( unsigned int w, unsigned int h, unsigned int m )
{
  PGMImage *new_pgm = malloc(sizeof(PGMImage));

  new_pgm->width = w;
  new_pgm->height = h;
  new_pgm->max = m;

  new_pgm ->pixmap = malloc(sizeof(unsigned int *)*h);

  for (int i = 0; i < h; i++) {
    new_pgm->pixmap[i] = malloc(sizeof(unsigned int)*w);
  }

  return new_pgm;
}

void del_ppmimage( PPMImage * p )
{
  for (int i = p->height - 1; i >= 0; i--) {
    free(p->pixmap[2][i]);
    free(p->pixmap[1][i]);
    free(p->pixmap[0][i]);
  }

  free(p->pixmap[2]);
  free(p->pixmap[1]);
  free(p->pixmap[0]);

  free(p);
}

void del_pgmimage( PGMImage * p )
{
  for (int i = p->height - 1; i >= 0; i--) {
    free(p->pixmap[i]);
  }

  free(p->pixmap);

  free(p);
}

void del_pbmimage( PBMImage * p )
{
  for (int i = p->height - 1; i >= 0; i--) {
    free(p->pixmap[i]);
  }

  free(p->pixmap);
  
  free(p);
}
