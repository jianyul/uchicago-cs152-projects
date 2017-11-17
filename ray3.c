#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "raytracer-project2.h"

/* note: shallow constructor */
ray3 *ray3_new(vector3 *origin, vector3 *direction)
{
  ray3 *r = (ray3*)malloc(sizeof(ray3));
  check_malloc("ray3_new", r);
  r->origin = origin;
  r->direction = direction;
  return r;
}

char *ray3_format = "(o=%s,d=%s)";

char *ray3_tos(ray3 *r)
{
  todo("ray3_tos");
  return 0;
}

void ray3_show(FILE *f, ray3 *r)
{
  todo("ray3_show");
}

vector3 *ray3_position(ray3 *r, double t)
{
  vector3 *t1 = vector3_scale(t,r->direction);
  vector3 *t2 = vector3_add(r->origin,t1);
  free(t1);
  return t2;
}
