// Wendy Zhu
// wzhu62@emory.edu
/* THIS  CODE  WAS MY OWN WORK , IT WAS  WRITTEN  WITHOUT  CONSULTING  ANY
SOURCES  OUTSIDE  OF  THOSE  APPROVED  BY THE  INSTRUCTOR. Wendy Zhu  */

#include "pbm.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

PPMImage *copy(char *in, char *out) {
  PPMImage *original = read_ppmfile(in);

  int w = original->width;
  int h = original->height;
  int m = original->max;

  PPMImage *new = new_ppmimage(w, h, m);

  for (int j = 0; j < w; j++) {
    for (int i = 0; i < h; i++) {
      new->pixmap[0][i][j] = original->pixmap[0][i][j];
      new->pixmap[1][i][j] = original->pixmap[1][i][j];
      new->pixmap[2][i][j] = original->pixmap[2][i][j];
    }
  }

  del_ppmimage(original);
  return new;
}

typedef struct {
  char mode[2];
  char color[10];
  int factor;
  char in[40];
  char out[40];
} options;

void bitmapConversion(char *in, char *out) {
  PPMImage *ppm = copy(in, out);

  int w = ppm->width;
  int h = ppm->height;
  int m = ppm->max;

  PBMImage *pbm = new_pbmimage(w, h);

  for (int j = 0; j < w; j++) {
    for (int i = 0; i < h; i++) {
      float sum = ppm->pixmap[0][i][j] + ppm->pixmap[1][i][j] + ppm->pixmap[2][i][j];
      float average = sum / 3;
      if (average >= (float)m / 2) {
        pbm->pixmap[i][j] = 0;
      } else {
        pbm->pixmap[i][j] = 1;
      }
    }
  }

  write_pbmfile(pbm, out);
  del_pbmimage(pbm);
  del_ppmimage(ppm);
}

void graymapConversion(char *in, char *out, options option) {
  PPMImage *ppm = copy(in, out);

  int w = ppm->width;
  int h = ppm->height;
  int m = ppm->max;

  int pgmMax = option.factor;
  PGMImage *pgm = new_pgmimage(w, h, pgmMax);

  if (option.factor < 1 || option.factor > 65535) {
    fprintf(stderr, "Error: Invalid max grayscale pixel value: %d; must be less than 65,536\n", option.factor);
    exit(0);
  }

  for (int j = 0; j < w; j++) {
    for (int i = 0; i < h; i++) {
      float sum = ppm->pixmap[0][i][j] + ppm->pixmap[1][i][j] + ppm->pixmap[2][i][j];
      float average = sum / 3;
      pgm->pixmap[i][j] = average / (float)m * (float)pgmMax;
    }
  }

  write_pgmfile(pgm, out);
  del_pgmimage(pgm);
  del_ppmimage(ppm);
}

void isolateRGB(char *in, char *out, options option) {
  PPMImage *new = copy(in, out);

  int w = new->width;
  int h = new->height;

  if (strcmp(option.color, "red") == 0) {
    for (int j = 0; j < w; j++) {
      for (int i = 0; i < h; i++) {
        new->pixmap[1][i][j] = 0;
        new->pixmap[2][i][j] = 0;
      }
    }
  } else if (strcmp(option.color, "blue") == 0) {
    for (int j = 0; j < w; j++) {
      for (int i = 0; i < h; i++) {
        new->pixmap[0][i][j] = 0;
        new->pixmap[1][i][j] = 0;
      }
    }
  } else if (strcmp(option.color, "green") == 0) {
    for (int j = 0; j < w; j++) {
      for (int i = 0; i < h; i++) {
        new->pixmap[0][i][j] = 0;
        new->pixmap[2][i][j] = 0;
      }
    }
  } else {
    fprintf(stderr, "Error: Invalid channel specification: (%s); should be ‘red’, ‘green’, or ‘blue’\n", option.color);
    exit(0);
  }

  write_ppmfile(new, out);
  del_ppmimage(new);
}

void removeRGB(char *in, char *out, options option) {
  PPMImage *new = copy(in, out);

  int w = new->width;
  int h = new->height;

  if (strcmp(option.color, "red") == 0) {
    for (int j = 0; j < w; j++) {
      for (int i = 0; i < h; i++) {
        new->pixmap[0][i][j] = 0;
      }
    }
  } else if (strcmp(option.color, "blue") == 0) {
    for (int j = 0; j < w; j++) {
      for (int i = 0; i < h; i++) {
        new->pixmap[2][i][j] = 0;
      }
    }
  } else if (strcmp(option.color, "green") == 0) {
    for (int j = 0; j < w; j++) {
      for (int i = 0; i < h; i++) {
        new->pixmap[1][i][j] = 0;
      }
    }
  }

  write_ppmfile(new, out);
  del_ppmimage(new);
}

void sepia(char *in, char *out) {
  PPMImage *new = copy(in, out);

  int w = new->width;
  int h = new->height;
  int m = new->max;

  for (int j = 0; j < w; j++) {
    for (int i = 0; i < h; i++) {
      float oldR = new->pixmap[0][i][j];
      float oldG = new->pixmap[1][i][j];
      float oldB = new->pixmap[2][i][j];

      float newR = 0.393*oldR + 0.769*oldG + 0.189*oldB;
      if (newR > m) {
        newR = m;
      }
      new->pixmap[0][i][j] = newR;

      float newG = 0.349*oldR + 0.686*oldG + 0.168*oldB;
      if (newG > m) {
        newG = m;
      }
      new->pixmap[1][i][j] = newG;

      float newB = 0.272*oldR + 0.534*oldG + 0.131*oldB;
      if (newB > m) {
        newB = m;
      }
      new->pixmap[2][i][j] = newB;
    }
  }

  write_ppmfile(new, out);
  del_ppmimage(new);
}

void mirror(char *in, char *out) {
  PPMImage *new = copy(in, out);

  int w = new->width;
  int h = new->height;

  for (int j = 0; j < h; j++) {
    for (int i = w/2; i < w; i++) {
      new->pixmap[0][j][i] = new->pixmap[0][j][w - i - 1];
      new->pixmap[1][j][i] = new->pixmap[1][j][w - i - 1];
      new->pixmap[2][j][i] = new->pixmap[2][j][w - i - 1];
    }
  }

  write_ppmfile(new, out);
  del_ppmimage(new);
}

void thumbnail(char *in, char *out, options option) {
  PPMImage *read = read_ppmfile(in);

  int newWidth = read->width / option.factor;
  int newHeight = read->height / option.factor;
  PPMImage *new = new_ppmimage(newWidth, newHeight, read->max);

  if (option.factor < 1 || option.factor > 8) {
    fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", option.factor);
    exit(0);
  }

  int w = 0;
  for (int j = 0; j < newWidth; j++) {
    int h = 0;
    for (int i = 0; i < newHeight; i++) {
      new->pixmap[0][i][j] = read->pixmap[0][h][w];
      new->pixmap[1][i][j] = read->pixmap[1][h][w];
      new->pixmap[2][i][j] = read->pixmap[2][h][w];
      h+=option.factor;
    }
    w+=option.factor;
  }

  write_ppmfile(new, out);
  del_ppmimage(read);
  del_ppmimage(new);
}

void thumbnailTiles(char *in, char *out, options option) {
  PPMImage *read = read_ppmfile(in);
  PPMImage *ppm = new_ppmimage(read->width, read->height, read->max);

  int newWidth = read->width / option.factor;
  int newHeight = read->height / option.factor;
  PPMImage *thumbnail = new_ppmimage(newWidth, newHeight, read->max);

  if (option.factor < 1 || option.factor > 8) {
    fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", option.factor);
    exit(0);
  }

  int w = 0;
  for (int j = 0; j < newWidth; j++) {
    int h = 0;
    for (int i = 0; i < newHeight; i++) {
      thumbnail->pixmap[0][i][j] = read->pixmap[0][h][w];
      thumbnail->pixmap[1][i][j] = read->pixmap[1][h][w];
      thumbnail->pixmap[2][i][j] = read->pixmap[2][h][w];
      h+=option.factor;
    }
    w+=option.factor;
  }

  int y = 0;
  for (int j = 0; j < ppm->width; j++) {
    int x = 0;
    for (int i = 0; i < ppm->height; i++) {
      if (y == newWidth) {
        y = 0;
      }
      if (x == newHeight) {
        x = 0;
      }
      ppm->pixmap[0][i][j] = thumbnail->pixmap[0][x][y];
      ppm->pixmap[1][i][j] = thumbnail->pixmap[1][x][y];
      ppm->pixmap[2][i][j] = thumbnail->pixmap[2][x][y];
      x++;
    }
    y++;
  }

  write_ppmfile(ppm, out);
  del_ppmimage(thumbnail);
  del_ppmimage(read);
  del_ppmimage(ppm);
}

int main( int argc, char *argv[] )
{
  if (argc > 6) {
    printf("Error: Mutiple transformation specified\n");
    exit(0);
  }

  int opt;
  options option;

  while ((opt = getopt(argc, argv, "bg:i:r:smt:n:o:")) != -1) {
    switch (opt) {
      case 'b':
        strcpy(option.mode, "b");
        break;
      case 'g':
        strcpy(option.mode, "g");
        option.factor = atoi(optarg);
        break;
      case 'i':
        strcpy(option.mode, "i");
        strcpy(option.color, optarg);
        break;
      case 'r':
        strcpy(option.mode, "r");
        strcpy(option.color, optarg);
        break;
      case 's':
        strcpy(option.mode, "s");
        break;
      case 'm':
        strcpy(option.mode, "m");
        break;
      case 't':
        strcpy(option.mode, "t");
        option.factor = atoi(optarg);
        break;
      case 'n':
        strcpy(option.mode, "n");
        option.factor = atoi(optarg);
        break;
      case 'o':
        if (strcmp(option.mode, "") == 0) {
          strcpy(option.mode, "b");
        }
        break;
      default:
        fprintf(stderr, "Useage: ppmcvt [-bgirsmtno] [FILE]\n");
        exit(0);
    }
  }

  strcpy(option.in, argv[argc-1]);
  strcpy(option.out, argv[argc-2]);

  if (strcmp(option.mode, "b") == 0) {
    bitmapConversion(option.in, option.out);
  } else if (strcmp(option.mode, "g") == 0) {
    graymapConversion(option.in, option.out, option);
  } else if (strcmp(option.mode, "i") == 0) {
    isolateRGB(option.in, option.out, option);
  } else if (strcmp(option.mode, "r") == 0) {
    removeRGB(option.in, option.out, option);
  } else if (strcmp(option.mode, "s") == 0) {
    sepia(option.in, option.out);
  } else if (strcmp(option.mode, "m") == 0) {
    mirror(option.in, option.out);
  } else if (strcmp(option.mode, "t") == 0) {
    thumbnail(option.in, option.out, option);
  } else if (strcmp(option.mode, "n") == 0) {
    thumbnailTiles(option.in, option.out, option);
  }

  return 0;
}
