/* I have used the project1 reference Dr. Shaw provided on Piazza */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "utils.h"
#include "raytracer-project2.h"
/* some convenience constructors for objects, etc. */

surface surf_const(double r, double g, double b)
{
  surface s;
  s.tag = CONSTANT;
  s.c.k = color_new(r,g,b);
  return s;
}

surface surf_fn(color*(*f)(vector3*,vector3*))
{
  surface s;
  s.tag = FUNCTION;
  s.c.f = f;
  return s;
}

/* create a container object for a sphere */
object *obj_sph(sphere *s)
{
  if (!s) {
    fprintf(stderr,"obj_sph given NULL\n");
    exit(1);
  }
  object *o = (object*)malloc(sizeof(object));
  check_malloc("obj_sph",o);
  o->tag = SPHERE;
  o->o.s = s;
  return o;
}

/* create a container object for a rectangle */
object *obj_rect(rectangle *r)
{
  if (!r) {
    fprintf(stderr,"obj_rect given NULL\n");
    exit(1);
  }
  object *o = (object*)malloc(sizeof(object));
  check_malloc("obj_rect",o);
  o->tag = RECTANGLE;
  o->o.r = r;
  return o;
}

/* private internal sphere constructor that leaves color slot uninitialized */
sphere *sph(double cx, double cy, double cz, double r, double sr, 
  double sg, double sb)
{
  sphere *s = (sphere*)malloc(sizeof(sphere));
  check_malloc("sph",s);
  s->center = vector3_new(cx,cy,cz);
  if (r<0) {
    fprintf(stderr,"sph: r<0 (r=%lf)\n",r);
    exit(1);
  }
  s->radius = r;
  s->shine = color_new(sr,sg,sb);
  return s;
}

/* solid-color sphere constructor */
object *sphere_new(double cx, double cy, double cz,
                   double r,
                   double cr, double cg, double cb,
                   double sr, double sg, double sb)
{
  sphere *s = sph(cx,cy,cz,r,sr,sg,sb);
  s->surf   = surf_const(cr,cg,cb);
  return obj_sph(s);
}

/* private internal rectangle constructor that leaves color slot uninitialized */
rectangle *rect(double ulx, double uly, double ulz,
                double w, double h,
                double sr, double sg, double sb)
{
  rectangle *r = (rectangle*)malloc(sizeof(rectangle));
  check_malloc("rect",r);
  r->upper_left = vector3_new(ulx,uly,ulz);
  if (w<0) {
    fprintf(stderr,"rectangle_new: negative width (%lf)\n",w);
    exit(1);
  }
  r->w = w;
  if (h<0) {
    fprintf(stderr,"rectangle_new: negative height (%lf)\n",h);
    exit(1);
  }
  r->h = h;
  r->shine = color_new(sr,sg,sb);
  return r;
}

/* solid-color rectangle constructor */
object *rectangle_new(double ulx, double uly, double ulz,
                      double w, double h,
                      double cr, double cg, double cb,
                      double sr, double sg, double sb)
{
  rectangle *r = rect(ulx,uly,ulz,w,h,sr,sg,sb);
  r->surf = surf_const(cr,cg,cb);
  return obj_rect(r);
}

/* shallow-copy object list cons */
object_list *cons(object *o, object_list *os)
{
  object_list *l = (object_list*)malloc(sizeof(object_list));
  check_malloc("cons",l);
  l->first = *o;
  l->rest  = os;
  return l;
}

/* (mostly) shallow-copy scene constructor */
scene *scene_new(color *bg, color *amb, light *dl, object_list *objs)
{
  if (!bg || !amb || !dl) {
    fprintf(stderr,"scene_new: unexpected NULL\n");
    exit(1);
  }
  scene *sc = (scene*)malloc(sizeof(scene));
  check_malloc("scene_new",sc);
  sc->bg.tag = CONSTANT;
  sc->bg.c.k = color_new(bg->r,bg->g,bg->b);
  sc->amb_light = amb;
  sc->dir_light = dl;
  sc->objects = objs;
  return sc;
}

/* dl_new: new directional light */
/* note: direction vector need not be a unit vector, it is normalized here */
light *dl_new(double x, double y, double z, double r, double g, double b)
{
  light *dl = (light*)malloc(sizeof(light));
  check_malloc("dl_new",dl);
  dl->direction = vector3_new(x,y,z);  
  vector3_normify(dl->direction);
  dl->color = color_new(r,g,b);
  return dl;
}

/* shallow copy environment constructor */
environment *environment_new(double z, uint w, uint h, scene *sc)
{
  environment *e = (environment*)malloc(sizeof(environment));
  check_malloc("environment_new",e);
  e->camera_z = z;
  e->image_width = w;
  e->image_height = h;
  e->scene = sc;
  return e;					
}


void render_ppm(FILE *f, environment *e)
{
  printf("P3\n");
  uint h = e->image_height;
  uint w = e->image_width;
  printf("%u %u\n", w, h);
  vector3 *cam = vector3_new(0.0, 0.0, e->camera_z);
  printf("255\n");
  int i;
  int j;
  for (j = 0; j < h; j++){
    for (i = 0; i < w; i++){
      vector3 *lc = logical_coord(h, w, j, i);
      vector3 *D = vector3_sub(lc, cam);
      vector3 *d = vector3_normalize(D);
      ray3 *r = ray3_new(cam, d);
      color *c = trace_ray(r, e->scene);
      fprintf(f, "%d %d %d\n", 
        (int)(255 * c->r), (int)(255 * c->g), (int)(255 * c->b));
      free(lc);
      free(r);
    }
  }
  free(cam);
}

int is_pre (char* s, char* buf)
{
  int num = 1;
  int len = strlen(s);
  for (int i = 0; i < len; i++){
    if (s[i] != buf[i]){
      num = 0;
      break;
    }
  }
  return num;
}

environment *read_env()
{
  char buf[512];
  double r, g, b, h, w, x, y, z, l, m, n, p, q, c, ra;
  scene* s = (scene*)malloc(sizeof(scene));
  s->amb_light = (color*)malloc(sizeof(color));
  s->dir_light = (light*)malloc(sizeof(light));
  s->objects = (object_list*)malloc(sizeof(object_list));
  s->objects = NULL;
  while (fgets(buf, 512, stdin) != NULL) {
    if (is_pre("BG",buf)) {
      sscanf(buf,"BG %lf %lf %lf",&r,&g,&b);
      s->bg = surf_const(r, g, b);
    } else if (is_pre("AMB",buf)) {
      sscanf(buf,"AMB %lf %lf %lf",&r,&g,&b);
      s->amb_light = color_new(r, g, b);
    } else if (is_pre("ENV", buf)) {
      sscanf(buf, "ENV %lf %lf %lf", &c, &p, &q);
    } else if (is_pre("DL", buf)) {
      sscanf(buf, "DL %lf %lf %lf %lf %lf %lf", &x, &y, &z, &r, &g, &b);
      s->dir_light = dl_new(x,y,z,r,g,b);
    } else if (is_pre("SPHERE", buf)) {
      sscanf(buf, "SPHERE %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
        &x, &y, &z, &ra, &r, &g, &b, &l, &m, &n);
      object* sphere = sphere_new(x, y, z, ra, r, g, b, l, m, n);
      s->objects = cons(sphere, s->objects);
    } else if (is_pre("RECTANGLE", buf)) {
      sscanf(buf, "RECTANGLE %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
        &x, &y, &z, &w, &h, &r, &g, &b, &l, &m, &n);
      object* rectangle = rectangle_new(x,y,z,(uint)w,(uint)h,r,g,b,l,m,n);
      s->objects = cons(rectangle, s->objects);
    } else {
      fprintf(stderr,"skipping \"%s\"\n",buf);
    }
  }
  return environment_new(c, (uint)p, (uint)q, s);
}

void surf_free (surface *s)
{
  if (s == NULL){
    return;
  }
  if (s->tag == CONSTANT){
    if (s->c.k != NULL){
      free(s->c.k);
    } else if (s->tag == FUNCTION){
      if (s->c.f != NULL){
        free(s->c.f);
      }
    }
  }
}

void obj_free (object *o)
{
  if (o == NULL){
    return;
  }
  if (o->tag == SPHERE){
    if (o->o.s == NULL){
      return;
    } 
    if (o->o.s->center != NULL){
      free (o->o.s->center);
      surf_free(&(o->o.s->surf));
    } 
    if (o->o.s->shine != NULL){
      free(o->o.s->shine);
    }
  } else {
    if (o->o.r == NULL){
      return;
    }
    if (o->o.r->upper_left != NULL){
      free(o->o.r->upper_left);
      surf_free(&(o->o.r->surf));
    }
    if (o->o.r->shine != NULL){
      free(o->o.r->shine);
    }
  }
}

void env_free (environment *e)
{
  if (e == NULL){
    return;
  } else {
    if (e->scene == NULL){
      return;
    } else {
      surf_free(&e->scene->bg);
      if (e->scene->amb_light != NULL){
        free(e->scene->amb_light);
      }
      if (e->scene->dir_light != NULL){
        //free(e->scene->dir_light->direction);
        free(e->scene->dir_light->color);
        free(e->scene->dir_light);
      }
      object_list *ptr = e->scene->objects;
      while(ptr != NULL){
        obj_free(&ptr->first);
        object_list *tmp = ptr->rest;
        free(ptr);
        ptr = tmp;
      }
    }
    free(e->scene);
    free(e);
  }
}

scene* scene_fnct(color*(*f)(vector3*,vector3*), 
  color *am, light *dl, object_list* objs)
{
  scene* s      = (scene*)malloc(sizeof(scene));
  s->bg         = surf_fn(f);
  s->amb_light  = am;
  s->dir_light  = dl;
  s->objects    = objs;
  return s;
}

object* sph_fnct(double x, double y, double z, 
  double r, double l, double m, double n, color*(*f)(vector3*,vector3*))
{
  sphere *s = sph(x, y, z, r, l, m, n);
  s->surf   = surf_fn(f);
  return obj_sph(s); 
}

color *sphere_color_fn1(vector3 *c, vector3 *hp)
{
  double r = sin((hp->x+hp->y+hp->z)*16);
  double d = r/2.0+0.5;
  return color_new(d/2.0,d/1.5,d);
}

color *sphere_color_fn2(vector3 *c, vector3 *hp)
{
  double r = cos((hp->x+hp->y*hp->z)*2);
  double d = r/2.0+0.5;
  return color_new(1.0,d/1.5,d/1.1);
}

color *sunset(vector3 *ro, vector3 *vp)
{
  double grad = (1.0 - -vp->y)/2.0;
  return color_new((1.0-grad)/1.5,0.0,grad/2.0);
}

int main(int argc, char *argv[]) 
{
  if (argc==2 && !strcmp(argv[1],"1")) {
    object *sphere0    = sphere_new(1,0,3,0.6,1,0,0,1,1,1);
    object *rectangle0 = rectangle_new(1,1.3,4,1,2.5,0,0,1,0,0,0);
    object_list *objs0 = cons(sphere0,cons(rectangle0,NULL));
    scene *scene0      = scene_new(color_new(0.8,0.8,0.8),
       color_new(0.2,0.2,0.2),
       dl_new(-1,1,-1,1,1,1),
       objs0);
    environment *env0  = environment_new(-3.3,600,400,scene0);
    render_ppm(stdout,env0);
    env_free(env0);
  } else if (argc==2 && !strcmp(argv[1],"2")) {
    object* sphere1    = sph_fnct(-0.6,0.20,13,1.1,0.8,0.8,0.8,sphere_color_fn1);
    object* sphere2    = sph_fnct(1.4,-0.15,16,1.1,0.8,0.8,0.8,sphere_color_fn2);
    object_list* objs1 = cons(sphere1, cons(sphere2,NULL));
    scene* scene1      = scene_fnct (sunset, 
      color_new(0.2,0.2,0.2), 
      dl_new(-1,1,-1,1,1,1), 
      objs1);
    environment* env1  = environment_new(-3.3,800,240,scene1);
    render_ppm(stdout, env1);
    env_free(env1);
  } else if (argc==1) {
    environment *e = read_env();
    render_ppm(stdout,e);
    env_free(e);
  }
  return 0;
}
