#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "utils.h"
#include "raytracer-project2.h"

vector3 *logical_coord(uint ih, uint iw, uint pixel_row, uint pixel_col)
{
  double pixw, pixh, logic_x, logic_y;
  pixh = pixw = 2.0 / (ih>iw ? ih : iw);
  if (pixel_col > iw || pixel_row > ih) {
    fprintf(stderr, 
	    "logical_coord: out of bounds (row %u, col %u)\n",
	    pixel_row, pixel_col);
    exit(1);
  }
  logic_x = -1.0 + (pixw*pixel_col) + (pixw/2.0);
  logic_y =  1.0 - ((pixh*pixel_row) + (pixh/2.0));
  if (iw>ih)
    logic_y -= (iw-ih)/2.0*pixh;
  else
    logic_x += (ih-iw)/2.0*pixw;
  return vector3_new(logic_x,logic_y,0.0);
}
