#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <gdal/cpl_port.h>
#include <gdal/cpl_error.h>
#include "map.h"
#include "style.h"
#include "util.h"

static void
rule_free(void *rule){
  simplet_rule_t *tmp = rule;
  simplet_list_free(tmp->styles);
  free(tmp->ogrsql);
  free(tmp);
}

// move to rules.c
int
process_rule(simplet_map_t *map, simplet_rule_t *rule, cairo_t *ctx){
  OGRGeometryH *bounds = simplet_bounds_to_ogr(map->bounds, map->proj);
  assert(bounds != NULL);
  OGRLayerH *layer = OGR_DS_ExecuteSQL(map->source, rule->ogrsql, bounds, "");
  OGR_G_DestroyGeometry(bounds);
  if(!layer)
    return 0;

  simplet_apply_styles(ctx, rule->styles);
  simplet_bounds_t *imagesize;
  if(!(imagesize = simplet_bounds_new()))
    return 0;
  simplet_bounds_extend(imagesize, 0, 0);
  simplet_bounds_extend(imagesize, map->width, map->height);

  OGRFeatureH *feature;
  while((feature = OGR_L_GetNextFeature(layer))){
    OGRGeometryH *geom = OGR_F_GetGeometryRef(feature);
    if(geom == NULL)
      continue;
    for(int i = 0; i < OGR_G_GetGeometryCount(geom); i++){
      OGRGeometryH *subgeom = OGR_G_GetGeometryRef(geom, i);

    }
    OGR_F_Destroy(feature);
  }
  OGR_DS_ReleaseResultSet(map->source, layer);
  return 1;
}


simplet_map_t*
simplet_map_new(){
  simplet_map_t *map;
  if(!(map = malloc(sizeof(*map))))
    return NULL;

  if(!(map->rules = simplet_list_new()))
    return NULL;
  map->rules->free = rule_free;
  map->source = NULL;
  map->bounds = NULL;
  map->proj   = NULL;
  map->height = 0;
  map->width  = 0;
  map->valid  = MAP_OK;
  return map;
}

void
simplet_map_free(simplet_map_t *map){
  if(map->bounds)
    simplet_bounds_free(map->bounds);
  if(map->source)
    OGR_DS_Destroy(map->source);
  if(map->proj)
    OSRRelease(map->proj);
  if(map->rules)
    simplet_list_free(map->rules);
  free(map);
}

int
simplet_map_set_srs(simplet_map_t *map, char *proj){
  assert(map->valid == MAP_OK);

  if(!(map->proj = OSRNewSpatialReference(NULL)))
    return (map->valid = MAP_ERR);

  if(OSRSetFromUserInput(map->proj, proj) != OGRERR_NONE)
    return (map->valid = MAP_ERR);

  return MAP_OK;
}

int
simplet_map_set_size(simplet_map_t *map, int width, int height){
  assert(map->valid == MAP_OK);

  map->height = height;
  map->width  = width;
  return MAP_OK;
}

int
simplet_map_set_bounds(simplet_map_t *map, double maxx, double maxy, double minx, double miny){
  assert(map->valid == MAP_OK);
  if(!(map->bounds = simplet_bounds_new()))
    return (map->valid = MAP_ERR);
  simplet_bounds_extend(map->bounds, maxx, maxy);
  simplet_bounds_extend(map->bounds, minx, miny);
  return MAP_OK;
}

int
simplet_map_add_layer(simplet_map_t *map, char *datastring){
  assert(map->valid == MAP_OK);

  OGRRegisterAll();
  if(!(map->source = OGROpen(datastring, 0, NULL)))
    return (map->valid = MAP_ERR);
  return MAP_OK;
}

int
simplet_map_add_rule(simplet_map_t *map, char *sqlquery){
  assert(map->valid == MAP_OK);

  // move to rules.c
  simplet_rule_t *rule;
  if(!(rule = malloc(sizeof(*rule))))
    return (map->valid = MAP_ERR);

  if(!(rule->styles = simplet_list_new()))
    return (map->valid = MAP_ERR);

  char *sql;
  if(!(sql = copy_string(sqlquery)))
    return (map->valid = MAP_ERR);

  rule->ogrsql = sql;

  if(!simplet_list_push(map->rules, rule))
    return (map->valid = MAP_ERR);

  assert(map->valid == MAP_OK);
  return MAP_OK;
}

int
simplet_map_add_style(simplet_map_t *map, char *key, char *arg){
  assert(map->valid == MAP_OK);

  if(!map->rules->tail)
    return (map->valid = MAP_ERR);
  simplet_rule_t *rule = map->rules->tail->value;

  simplet_style_t *style;
  if(!(style = simplet_lookup_style(key)))
    return (map->valid = MAP_ERR);

  style->arg = copy_string(arg);

  if(!(style->arg || style->call))
    return (map->valid = MAP_ERR);

  simplet_list_push(rule->styles, style);

  return MAP_OK;
}

int
simplet_map_isvalid(simplet_map_t *map){
  assert(map->valid == MAP_OK);

  if(map->valid == MAP_ERR)
    return MAP_ERR;

  if(!map->bounds)
    return MAP_ERR;

  if(!map->source)
    return MAP_ERR;

  if(!map->proj)
    return MAP_ERR;

  if(!map->height)
    return MAP_ERR;

  if(!map->width)
    return MAP_ERR;

  if(!map->rules->tail)
    return MAP_ERR;

  return MAP_OK;
}

int
simplet_map_render_to_png(simplet_map_t *map, char *path){
  if(simplet_map_isvalid(map) == MAP_ERR)
    return (map->valid = MAP_ERR);

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, map->width, map->height);
  if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    return (map->valid = MAP_ERR);
  cairo_t *ctx = cairo_create(surface);

  simplet_listiter_t *iter = simplet_get_list_iter(map->rules);
  simplet_rule_t *rule;
  while((rule = simplet_list_next(iter)))
    process_rule(map, rule, ctx);

  cairo_destroy(ctx);
}