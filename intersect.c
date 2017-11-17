#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "utils.h"
#include "raytracer-project2.h"

color *color_dup(color *c)
{
  return color_new(c->r,c->g,c->b);
}

/* hit_new: deep copy constructor */
hit *hit_new(double t, color *surf, color *shine, vector3 *surf_norm)
{
  hit *h = (hit*)malloc(sizeof(hit));
  check_malloc("hit_new", h);
  h->t = t;
  h->surface_color = color_new(surf->r,surf->g,surf->b);
  h->shine = color_new(shine->r, shine->g, shine->b);
  h->surface_normal = vector3_new(surf_norm->x,surf_norm->y,surf_norm->z);
  return h;
}

/* hit_free: deep free */
void hit_free(hit *h)
{
  if (h) {
    free(h->surface_color);
    free(h->surface_normal);
    free(h);
  }
}

color *sphere_hit_color (sphere *s, vector3 *hp)
{
  if (s->surf.tag == CONSTANT){
    return color_dup(s->surf.c.k);
  } else {
    return color_dup(s->surf.c.f(s->center, hp));
  }
}

color *rectangle_hit_color (rectangle *r, vector3 *hp)
{
  if (r->surf.tag == CONSTANT){
    return color_dup(r->surf.c.k);
  } else {
    return color_dup(r->surf.c.f(r->upper_left, hp));
  }
}

hit *sphere_intersect(ray3 *r, sphere *s)
{
  vector3 *ro = r->origin;
  vector3 *rd = r->direction;
  vector3 *sc = s->center;
  double   sr = s->radius;
  vector3 *a  = vector3_sub(ro,sc);
  double   b  = vector3_dot(a,rd);
  double   c  = vector3_dot(a,a)-(sr*sr);
  double   d  = (b*b)-c;
  double   t  = -b-sqrt(d);
  free(a);
  if (d<=0 || t<=0)
    return NULL;
  vector3 *hit_point = ray3_position(r,t);
  vector3 *surf_norm = vector3_sub(hit_point,sc);
  vector3_normify(surf_norm);
  hit* h;
  if (s->surf.tag == CONSTANT){
    h = hit_new(t,sphere_hit_color(s,hit_point),s->shine,surf_norm);
  } else {
    color* c = (s->surf.c.f)(s->center, hit_point);
    h = hit_new(t, c, s->shine, surf_norm);
  }
  free(surf_norm);
  free(hit_point);
  return h;
}


int between(double lo, double x, double hi)
{
  return (lo<=x) && (x<=hi);
}

int within_rectangle(rectangle *t, vector3 *p)
{
  vector3 *ul = t->upper_left;
  return between(ul->x,p->x,ul->x+t->w) &&
         between(ul->y-t->h,p->y,ul->y);
}

hit *rectangle_intersect(ray3 *r, rectangle *rec)
{
  vector3 *ro = r->origin;
  vector3 *rd = r->direction;
  vector3  *n = vector3_new(0,0,-1);
  double    d = rec->upper_left->z;
  double    t = -(vector3_dot(ro,n)+d)/vector3_dot(rd,n);
  hit      *h = NULL;
  if (t>0) {
    vector3 *hit_point = ray3_position(r,t);
    if (within_rectangle(rec,hit_point)){
      if(rec->surf.tag == CONSTANT){
        h = hit_new(t,rectangle_hit_color(rec,hit_point),rec->shine,n);
      } else {
        color * c = (rec->surf.c.f)(rec->upper_left, hit_point);
        h = hit_new(t, c, rec->shine, n);
      }
      
    }
    free(hit_point);
  }
  free(n);
  return h;
}

hit *intersect(ray3 *r, object *obj)
{
  switch (obj->tag) {
  case SPHERE:
    return sphere_intersect(r,obj->o.s);
  case RECTANGLE:
    return rectangle_intersect(r,obj->o.r);
  default:
    fprintf(stderr,"? object tag %d\n", obj->tag);
    exit(1);
  }
}

int      in_shadow(vector3 *loc, light *dl, object_list *objs)
{
  if (loc == NULL || dl == NULL || objs == NULL){
    fprintf(stderr, "invalid input\n");
    exit(1);
  }
  vector3 *nudge = vector3_scale(0.0001, dl->direction); 
  vector3 *lifted = vector3_add(loc, nudge);
  int num = 0;
  ray3 * r = (ray3*)malloc(sizeof(ray3));
  r->origin = lifted;
  r->direction = dl->direction;
  object_list * ptr = objs;
  while (ptr != NULL){
    hit* tmp = intersect(r, &(ptr->first));
    if (tmp != NULL){
      hit_free(tmp);
      num = 1;
      break;
    }
    hit_free(tmp);
    ptr = ptr->rest;
  }
  free(nudge);
  //ray3_free(r);
  free(lifted);
  free(r);
  return num;
}

vector3 *background_loc (ray3 *r)
{
  vector3 * o = r->origin;
  vector3 * d = r->direction;
  double scalar = (- 1) * o->z / d->z;
  vector3 *v = vector3_scale(scalar, d);
  vector3 *p = vector3_add(v, o);
  free(v);
  return p;
}

color *background_color (ray3 *r, scene *s, vector3 *loc)
{
  if (s->bg.tag == CONSTANT){
    return color_dup(s->bg.c.k);
  } else {
    return color_dup(s->bg.c.f(r->origin, loc));
  }
}

color *light_color(scene *s, ray3 *r, hit *h)
{
  if (s == NULL || r == NULL){
    fprintf(stderr, "invalid input\n");
    exit(1);
  }
  if (h == NULL){
    vector3* bg_loc  = background_loc(r);
    color* c = background_color(r, s, bg_loc);
    free(bg_loc);
    return c;
  } else {
    color * k = NULL;
    vector3 *p = vector3_scale(h->t, r->direction);
    vector3 *hit_point = vector3_add(p, r->origin);
    free(p);
    int shadow = in_shadow(hit_point, s->dir_light, s->objects);
    if (shadow){
      return color_modulate(s->amb_light, h->surface_color);
    } else {
      double nd = vector3_dot(h->surface_normal, s->dir_light->direction);
      double m = 0 > nd ? 0: nd;
      color * c = color_scale(m, s->dir_light->color);
      color * sum = color_add(c, s->amb_light);
      k = color_modulate(sum, h->surface_color);
      free(c);
      free(sum);
    }
    vector3 *n = h->surface_normal;
    vector3 *l = s->dir_light->direction;
    color *s = h->shine;
    double scalar = 2 * vector3_dot(n, l);
    vector3 *nl = vector3_scale(scalar, n);
    vector3 *r0 = vector3_sub(nl, l);
    free(nl);
    vector3 *v = vector3_negate(r->direction);
    color *d;
    if (scalar <= 0){
      d = color_new(0, 0, 0);
    } else {
      double rv = vector3_dot(r0, v);
      double m0 = 0 > rv ? 0: rv;
      d = color_scale(pow(m0, 6), s);
      free(v);
    }
    return color_add(d, k);
  }
}

color *trace_ray(ray3 *r, scene *s)
{
  hit *closest_hit = NULL;
  for (object_list *objs = s->objects; objs; objs=objs->rest) {
    hit *hp = intersect(r,&(objs->first));
    if (hp) {
      if (!closest_hit)
  closest_hit = hp;
      else if (closest_hit->t > hp->t) {
  hit_free(closest_hit);
  closest_hit = hp;
      } else {
  hit_free(hp);
      }
    }
  }
  color *c = light_color(s, r, closest_hit);
  return c;
}
