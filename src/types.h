#ifndef _SIMPLE_TYPES_H
#define _SIMPLE_TYPES_H

#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#ifdef __cplusplus
extern "C" {
#endif

/* bounds and simple points */
typedef struct {
  double x;
  double y;
} simplet_point_t;

typedef struct {
  simplet_point_t nw;
  simplet_point_t se;
  double width;
  double height;
} simplet_bounds_t;

typedef void (*simplet_user_data_free)(void *val);
#define SIMPLET_USER_DATA \
  void *user_data;
#define SIMPLET_FREEFUNC \
  simplet_user_data_free free;

/* lists, nodes, and iterators */
typedef struct simplet_node_t {
  struct simplet_node_t *next;
  struct simplet_node_t *prev;
  SIMPLET_USER_DATA
} simplet_node_t;


typedef struct simplet_list_t {
  simplet_node_t *head;
  simplet_node_t *tail;
  SIMPLET_FREEFUNC
  unsigned int length;
} simplet_list_t;

typedef struct simplet_listiter_t {
  simplet_node_t *next;
} simplet_listiter_t;

/* errors */
typedef enum {
  SIMPLET_ERR = 0,   // Generic error
  SIMPLET_OOM,       // Out of memory for allocation
  SIMPLET_CAIRO_ERR, // Cairo error
  SIMPLET_OGR_ERR,   // OGR Error
  SIMPLET_OK         // OK
} simplet_status_t;

#define SIMPLET_ERROR_FIELDS \
  simplet_error_t error;

#define SIMPLET_MAX_ERROR 1024
typedef struct {
  simplet_status_t status;
  char msg[SIMPLET_MAX_ERROR];
} simplet_error_t;

/* map structures */
typedef struct {
  SIMPLET_ERROR_FIELDS
} simplet_errorable_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
} simplet_with_user_data_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  simplet_bounds_t     *bounds;
  simplet_list_t       *layers;
  OGRSpatialReferenceH proj;
  double buffer; // pixel coords
  unsigned int width;
  unsigned int height;
  char *bgcolor;
} simplet_map_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  char           *source;
  simplet_list_t *filters;
} simplet_layer_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  char *ogrsql;
  simplet_list_t *styles;
} simplet_filter_t;

typedef struct {
  SIMPLET_ERROR_FIELDS
  SIMPLET_USER_DATA
  char *key;
  char *arg;
} simplet_style_t;


#ifndef M_PI
#define SIMPLET_PI acos(-1.0)
#else
#define SIMPLET_PI M_PI
#endif

#define SIMPLET_MERCATOR "epsg:3785"
#define SIMPLET_WGS84    "epsg:4326"


#ifdef __cplusplus
}
#endif

#endif
