#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "utils.h"
#include "raytracer-project2.h"

vector3 *vector3_new(double x, double y, double z)
{
  vector3 *v = (vector3*)malloc(sizeof(vector3));
  check_malloc("vector3_new", v);
  v->x = x;
  v->y = y;
  v->z = z;
  return v;
}

void vector3_free(vector3 *v)
{
  free(v);
}

char *vector_format = "<%lf,%lf,%lf>";

char *vector3_tos(vector3 *v)
{
  char buf[256]; /* way more than enough, but chars are cheap */  
  sprintf(buf,vector_format,v->x,v->y,v->z);
  return strdup(buf);
}

void vector3_show(FILE *f, vector3 *v)
{
  fprintf(f,vector_format,v->x,v->y,v->z);
}

vector3 *vector3_add(vector3 *v1, vector3 *v2)
{
  return vector3_new(v1->x+v2->x,v1->y+v2->y,v1->z+v2->z);
}

vector3 *vector3_sub(vector3 *v1, vector3 *v2)
{
  return vector3_new(v1->x-v2->x,v1->y-v2->y,v1->z-v2->z);
}

vector3 *vector3_negate(vector3 *v)
{
  return vector3_new(-v->x,-v->y,-v->z);
}

vector3 *vector3_scale(double s, vector3 *v)
{
  return vector3_new(s*v->x,s*v->y,s*v->z);
}

double vector3_dot(vector3 *v1, vector3 *v2)
{
  return v1->x*v2->x+v1->y*v2->y+v1->z*v2->z;
}

double vector3_magnitude(vector3 *v)
{
  return sqrt(v->x*v->x+v->y*v->y+v->z*v->z);
}

vector3 *vector3_normalize(vector3 *v)
{
  double m = vector3_magnitude(v);
  if (fabs(m)<0.0001)
    return v;
  return vector3_scale(1.0/m,v);
}

void vector3_normify(vector3 *v)
{
  double m = vector3_magnitude(v);
  if (fabs(m)<0.0001)
    return;
  v->x/=m;
  v->y/=m;
  v->z/=m;
}
