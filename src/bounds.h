#ifndef SIMPLET_BOUNDS_H
#define SIMPLET_BOUNDS_H

#include <gdal/ogr_api.h>
#include <gdal/ogr_srs_api.h>
#include "point.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  simplet_point_t *nw;
  simplet_point_t *se;
  double width;
  double height;
} simplet_bounds_t;

simplet_bounds_t*
simplet_bounds_new();

void
simplet_bounds_extend(simplet_bounds_t *bounds, double x, double y);

OGRGeometryH *
simplet_bounds_to_ogr(simplet_bounds_t *bounds, OGRSpatialReferenceH *proj);

void
simplet_bounds_free(simplet_bounds_t *bounds);

simplet_point_t*
simplet_bounds_project(simplet_bounds_t* source, double x, double y, simplet_bounds_t* other);

#ifdef __cplusplus
}
#endif

#endif