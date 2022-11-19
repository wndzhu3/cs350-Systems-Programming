## Homework 1: PPM Image Transformations

This program implements `ppmcvt`, which manipulates input Portable Pixel Map (PPM) files and outputs a new image based on its given options.

### Usage
`ppmcvt [bg:i:r:smt:n:o:] file`

*Options:*

`-b`: convert input file to a Portable Bitmap (PBM) file (DEFAULT).

`-g`: convert input file to a Portable Gray Map (PGM) file using the specified max grayscale pixel value [1-65535].

`-i`: isolate the specified RGB channel (`red`, `green`, or `blue`).

`-r`: remove the specified RGB channel (`red`, `green`, or `blue`).

`-s`: apply a sepia transformation.

`-m`: vertically mirror the first half of the image to the second half.

`-t`: reduce the input image to a thumbnail based on the given scaling factor [1-8].

`-n`: tile thumbnaiils of the input image based on the given scaling factor [1-8].

`-o`: write output image to the specified file. Existent output files will be overwritten.


**Examples**

`ppmcvt -o out.pbm in.ppm`

  read in.ppm PPM file and write converted PBM file to out.pbm
  
`ppmcvt -g 16 -o out.pgm in.ppm`

  convert the PPM image in.ppm to a PGM image in out.pgm
  
`ppmcvt -s -o out.ppm in.ppm`

  apply a sepia transformation to the PPM image in in.ppm and output the new image to out.ppm
  
`ppmcvt -n 4 -o out.ppm in.ppm`

  tile 4 1:4 scaled (quarter sized) thumbnails of the image in in.ppm into a new PPM image in out.ppm
  
