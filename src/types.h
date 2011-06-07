#ifndef _SIMPLE_TYPES_H
#define _SIMPLE_TYPES_H

#include <gdal/ogr_api.h>
#include <gdal/ogr_srs_api.h>
#include <cairo/cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

/* bounds and simple points */

typedef struct {
  double x;
  double y;
} simplet_point_t;

typedef struct {
  simplet_point_t *nw;
  simplet_point_t *se;
  double width;
  double height;
} simplet_bounds_t;


/* lists, nodes, and iterators */
typedef struct simplet_node_t {
  struct simplet_node_t *next;
  struct simplet_node_t *prev;
  void *value;
} simplet_node_t;

typedef struct simplet_list_t {
  simplet_node_t *head;
  simplet_node_t *tail;
  void (*free)(void *val);
  unsigned int length;
} simplet_list_t;

typedef struct simplet_listiter_t {
  simplet_node_t *next;
} simplet_listiter_t;


/* map structures */

typedef struct {
  simplet_bounds_t     *bounds;
  simplet_list_t       *layers;
  OGRSpatialReferenceH proj;
  cairo_t              *_ctx; /* ephemeral, not for outside usage */
  int valid;
  unsigned int width;
  unsigned int height;
} simplet_map_t;

typedef struct {
  OGRDataSourceH _source; /* ephemeral, not for outside usage */
  char           *source;
  simplet_list_t *filters;
} simplet_layer_t;

typedef struct {
  char *ogrsql;
  simplet_list_t   *styles;
  cairo_t          *_ctx;    /* ephemeral, not for outside usage */
  simplet_bounds_t *_bounds; /* ephemeral, not for outside usage */
} simplet_filter_t;

typedef struct {
  char *key;
  char *arg;
} simplet_style_t;

#ifndef M_PI
#define M_PI acos(-1.0)
#endif

#define SIMPLET_WGS84 "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"
#define SIMPLET_MERCATOR "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over"
#define SIMPLET_SLIPPY_SIZE 256

#ifdef __cplusplus
}
#endif

#endif