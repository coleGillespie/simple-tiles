#include "layer.h"
#include "filter.h"
#include "util.h"
#include "error.h"
#include <cpl_error.h>

// Set up user data.
SIMPLET_HAS_USER_DATA(layer)

// Create and return a new layer instance.
simplet_layer_t*
simplet_layer_new(const char *datastring){
  simplet_layer_t *layer;
  if(!(layer = malloc(sizeof(*layer))))
    return NULL;

  memset(layer, 0, sizeof(*layer));

  layer->source = simplet_copy_string(datastring);
  layer->error.status = SIMPLET_OK;

  if(!(layer->filters = simplet_list_new())){
    free(layer);
    return NULL;
  }

  return layer;
}

// Add in an error function.
SIMPLET_ERROR_FUNC(layer_t)

// Free a void pointer pointing to a layer instance.
void
simplet_layer_vfree(void *layer){
  simplet_layer_free(layer);
}

// Free a layer object, and associated layers.
void
simplet_layer_free(simplet_layer_t *layer){
  simplet_list_set_item_free(layer->filters, simplet_filter_vfree);
  simplet_list_free(layer->filters);
  free(layer->source);
  free(layer);
}

// Creat and append a filter to the layer's filters.
simplet_filter_t*
simplet_layer_add_filter(simplet_layer_t *layer, const char *ogrsql){
  simplet_filter_t* filter;
  if(!(filter = simplet_filter_new(ogrsql)))
    return NULL;

  if(!simplet_list_push(layer->filters, filter)){
    simplet_filter_free(filter);
    return NULL;
  }

  return filter;
}

// Add a previously initialized filter to the layer.
simplet_filter_t*
simplet_layer_add_filter_directly(simplet_layer_t *layer, simplet_filter_t *filter){
  if(!simplet_list_push(layer->filters, filter)) return NULL;
  return filter;
}

// Process a layer and add labels.
simplet_status_t
simplet_layer_process(simplet_layer_t *layer, simplet_map_t *map, simplet_lithograph_t *litho, cairo_t *ctx){
  simplet_listiter_t *iter; OGRDataSourceH source;
  if(!(source = OGROpenShared(layer->source, 0, NULL)))
    return set_error(layer, SIMPLET_OGR_ERR, "error opening layer source");

  // Retain the datasource because we want to cache open connections to a
  // data source like postgres.
  if(OGR_DS_GetRefCount(source) == 1) OGR_DS_Reference(source);
  if(!(iter = simplet_get_list_iter(layer->filters))){
    OGRReleaseDataSource(source);
    return set_error(layer, SIMPLET_OOM, "out of memory getting list iterator");
  }

  // Loop through the layer's filters and process them.
  simplet_filter_t *filter;
  simplet_status_t status = SIMPLET_OK;
  while((filter = simplet_list_next(iter))) {
    status = simplet_filter_process(filter, map, source, litho, ctx);

    if(status != SIMPLET_OK){
      simplet_list_iter_free(iter);
      OGRReleaseDataSource(source);
      return status;
    }

    simplet_lithograph_apply(litho, filter->styles);
  }
  OGRReleaseDataSource(source);
  return SIMPLET_OK;
}

// Get the datasource string for this layer.
void
simplet_layer_get_source(simplet_layer_t *layer, char **source){
  *source = simplet_copy_string(layer->source);
}

// Set a copy of this source as this layers datasource string.
void
simplet_layer_set_source(simplet_layer_t *layer, char *source){
  char *src = simplet_copy_string(source);
  if(!src) set_error(layer, SIMPLET_OOM, "out of memory setting source");
  layer->source = src;
}

