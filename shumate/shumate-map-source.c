/*
 * Copyright (C) 2010-2013 Jiri Techet <techet@gmail.com>
 * Copyright (C) 2019 Marcus Lundblad <ml@update.uu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * SECTION:shumate-map-source
 * @short_description: A base class for map sources
 *
 * #ShumateTile objects come from map sources which are represented by
 * #ShumateMapSource.  This is should be considered an abstract
 * type as it does nothing of interest.
 *
 * When loading new tiles, #ShumateView calls shumate_map_source_fill_tile()
 * on the current #ShumateMapSource passing it a #ShumateTile to be filled
 * with the image.
 *
 * Apart from being a base class of all map sources, #ShumateMapSource
 * also supports cooperation of multiple map sources by arranging them into
 * chains. Every map source has the #ShumateMapSource:next-source property
 * that determines the next map source in the chain. When a function of
 * a #ShumateMapSource object is invoked, the map source may decide to
 * delegate the work to the next map source in the chain by invoking the
 * same function on it.

 * To understand the concept of chains, consider for instance a chain
 * consisting of #ShumateFileCache whose next source is
 * #ShumateNetworkTileSource whose next source is an error tile source
 * created with shumate_map_source_factory_create_error_source ().
 * When shumate_map_source_fill_tile() is called on the first object of the
 * chain, #ShumateFileCache, the cache checks whether it contains the
 * requested tile in its database. If it does, it returns the tile; otherwise,
 * it calls shumate_map_source_fill_tile() on the next source in the chain
 * (#ShumateNetworkTileSource). The network tile source loads the tile
 * from the network. When successful, it returns the tile; otherwise it requests
 * the tile from the next source in the chain (error tile source).
 * The error tile source always generates an error tile, no matter what
 * its next source is.
 */

#include "shumate-map-source.h"
#include "shumate-location.h"

#include <math.h>

typedef struct
{
  ShumateMapSource *next_source;
} ShumateMapSourcePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ShumateMapSource, shumate_map_source, G_TYPE_INITIALLY_UNOWNED);

enum
{
  PROP_0,
  PROP_NEXT_SOURCE,
};

static void
shumate_map_source_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  ShumateMapSource *map_source = SHUMATE_MAP_SOURCE (object);
  ShumateMapSourcePrivate *priv = shumate_map_source_get_instance_private (map_source);

  switch (prop_id)
    {
    case PROP_NEXT_SOURCE:
      g_value_set_object (value, priv->next_source);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
shumate_map_source_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ShumateMapSource *map_source = SHUMATE_MAP_SOURCE (object);

  switch (prop_id)
    {
    case PROP_NEXT_SOURCE:
      shumate_map_source_set_next_source (map_source,
          g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
shumate_map_source_dispose (GObject *object)
{
  ShumateMapSource *map_source = SHUMATE_MAP_SOURCE (object);
  ShumateMapSourcePrivate *priv = shumate_map_source_get_instance_private (map_source);

  g_clear_object (&priv->next_source);

  G_OBJECT_CLASS (shumate_map_source_parent_class)->dispose (object);
}


static void
shumate_map_source_class_init (ShumateMapSourceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  object_class->dispose = shumate_map_source_dispose;
  object_class->get_property = shumate_map_source_get_property;
  object_class->set_property = shumate_map_source_set_property;

  klass->get_id = NULL;
  klass->get_name = NULL;
  klass->get_license = NULL;
  klass->get_license_uri = NULL;
  klass->get_min_zoom_level = NULL;
  klass->get_max_zoom_level = NULL;
  klass->get_tile_size = NULL;
  klass->get_projection = NULL;

  klass->fill_tile = NULL;

  /**
   * ShumateMapSource:next-source:
   *
   * Next source in the loading chain.
   */
  pspec = g_param_spec_object ("next-source",
        "Next Source",
        "Next source in the loading chain",
        SHUMATE_TYPE_MAP_SOURCE,
        G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_NEXT_SOURCE, pspec);
}


static void
shumate_map_source_init (ShumateMapSource *map_source)
{
  ShumateMapSourcePrivate *priv = shumate_map_source_get_instance_private (map_source);

  priv->next_source = NULL;
}


/**
 * shumate_map_source_get_next_source:
 * @map_source: a #ShumateMapSource
 *
 * Get the next source in the chain.
 *
 * Returns: (transfer none): the next source in the chain.
 */
ShumateMapSource *
shumate_map_source_get_next_source (ShumateMapSource *map_source)
{
  ShumateMapSourcePrivate *priv = shumate_map_source_get_instance_private (map_source);

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), NULL);

  return priv->next_source;
}

/**
 * shumate_map_source_set_next_source:
 * @map_source: a #ShumateMapSource
 * @next_source: the next #ShumateMapSource in the chain
 *
 * Sets the next map source in the chain.
 */
void
shumate_map_source_set_next_source (ShumateMapSource *map_source,
    ShumateMapSource *next_source)
{
  ShumateMapSourcePrivate *priv = shumate_map_source_get_instance_private (map_source);

  g_return_if_fail (SHUMATE_IS_MAP_SOURCE (map_source));

  if (priv->next_source != NULL)
    g_object_unref (priv->next_source);

  if (next_source)
    {
      g_return_if_fail (SHUMATE_IS_MAP_SOURCE (next_source));

      g_object_ref_sink (next_source);
    }

  priv->next_source = next_source;

  g_object_notify (G_OBJECT (map_source), "next-source");
}

/**
 * shumate_map_source_get_id:
 * @map_source: a #ShumateMapSource
 *
 * Gets map source's id.
 *
 * Returns: the map source's id.
 */
const char *
shumate_map_source_get_id (ShumateMapSource *map_source)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), NULL);

  return SHUMATE_MAP_SOURCE_GET_CLASS (map_source)->get_id (map_source);
}


/**
 * shumate_map_source_get_name:
 * @map_source: a #ShumateMapSource
 *
 * Gets map source's name.
 *
 * Returns: the map source's name.
 */
const char *
shumate_map_source_get_name (ShumateMapSource *map_source)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), NULL);

  return SHUMATE_MAP_SOURCE_GET_CLASS (map_source)->get_name (map_source);
}


/**
 * shumate_map_source_get_license:
 * @map_source: a #ShumateMapSource
 *
 * Gets map source's license.
 *
 * Returns: the map source's license.
 */
const char *
shumate_map_source_get_license (ShumateMapSource *map_source)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), NULL);

  return SHUMATE_MAP_SOURCE_GET_CLASS (map_source)->get_license (map_source);
}


/**
 * shumate_map_source_get_license_uri:
 * @map_source: a #ShumateMapSource
 *
 * Gets map source's license URI.
 *
 * Returns: the map source's license URI.
 */
const char *
shumate_map_source_get_license_uri (ShumateMapSource *map_source)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), NULL);

  return SHUMATE_MAP_SOURCE_GET_CLASS (map_source)->get_license_uri (map_source);
}


/**
 * shumate_map_source_get_min_zoom_level:
 * @map_source: a #ShumateMapSource
 *
 * Gets map source's minimum zoom level.
 *
 * Returns: the miminum zoom level this map source supports
 */
guint
shumate_map_source_get_min_zoom_level (ShumateMapSource *map_source)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0);

  return SHUMATE_MAP_SOURCE_GET_CLASS (map_source)->get_min_zoom_level (map_source);
}


/**
 * shumate_map_source_get_max_zoom_level:
 * @map_source: a #ShumateMapSource
 *
 * Gets map source's maximum zoom level.
 *
 * Returns: the maximum zoom level this map source supports
 */
guint
shumate_map_source_get_max_zoom_level (ShumateMapSource *map_source)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0);

  return SHUMATE_MAP_SOURCE_GET_CLASS (map_source)->get_max_zoom_level (map_source);
}


/**
 * shumate_map_source_get_tile_size:
 * @map_source: a #ShumateMapSource
 *
 * Gets map source's tile size.
 *
 * Returns: the tile's size (width and height) in pixels for this map source
 */
guint
shumate_map_source_get_tile_size (ShumateMapSource *map_source)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0);

  return SHUMATE_MAP_SOURCE_GET_CLASS (map_source)->get_tile_size (map_source);
}


/**
 * shumate_map_source_get_projection:
 * @map_source: a #ShumateMapSource
 *
 * Gets map source's projection.
 *
 * Returns: the map source's projection.
 */
ShumateMapProjection
shumate_map_source_get_projection (ShumateMapSource *map_source)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), SHUMATE_MAP_PROJECTION_MERCATOR);

  return SHUMATE_MAP_SOURCE_GET_CLASS (map_source)->get_projection (map_source);
}


/**
 * shumate_map_source_get_x:
 * @map_source: a #ShumateMapSource
 * @zoom_level: the zoom level
 * @longitude: a longitude
 *
 * Gets the x position on the map using this map source's projection.
 * (0, 0) is located at the top left.
 *
 * Returns: the x position
 */
double
shumate_map_source_get_x (ShumateMapSource *map_source,
    guint zoom_level,
    double longitude)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0.0);

  longitude = CLAMP (longitude, SHUMATE_MIN_LONGITUDE, SHUMATE_MAX_LONGITUDE);

  /* FIXME: support other projections */
  return ((longitude + 180.0) / 360.0) * shumate_map_source_get_tile_size (map_source) * pow (2.0, zoom_level);
}


/**
 * shumate_map_source_get_y:
 * @map_source: a #ShumateMapSource
 * @zoom_level: the zoom level
 * @latitude: a latitude
 *
 * Gets the y position on the map using this map source's projection.
 * (0, 0) is located at the top left.
 *
 * Returns: the y position
 */
double
shumate_map_source_get_y (ShumateMapSource *map_source,
    guint zoom_level,
    double latitude)
{
  double sin_latitude;

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0.0);

  latitude = CLAMP (latitude, SHUMATE_MIN_LATITUDE, SHUMATE_MAX_LATITUDE);
  /* FIXME: support other projections */
  sin_latitude = sin (latitude * G_PI / 180.0);
  return (0.5 - log ((1.0 + sin_latitude) / (1.0 - sin_latitude)) / (4.0 * G_PI)) * shumate_map_source_get_tile_size (map_source) * pow (2.0, zoom_level);
}


/**
 * shumate_map_source_get_longitude:
 * @map_source: a #ShumateMapSource
 * @zoom_level: the zoom level
 * @x: a x position
 *
 * Gets the longitude corresponding to this x position in the map source's
 * projection.
 *
 * Returns: the longitude
 */
double
shumate_map_source_get_longitude (ShumateMapSource *map_source,
    guint zoom_level,
    double x)
{
  double longitude;

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0.0);
  /* FIXME: support other projections */
  double dx = x / (double) shumate_map_source_get_tile_size (map_source);
  longitude = dx / pow (2.0, zoom_level) * 360.0 - 180.0;

  return CLAMP (longitude, SHUMATE_MIN_LONGITUDE, SHUMATE_MAX_LONGITUDE);
}


/**
 * shumate_map_source_get_latitude:
 * @map_source: a #ShumateMapSource
 * @zoom_level: the zoom level
 * @y: a y position
 *
 * Gets the latitude corresponding to this y position in the map source's
 * projection.
 *
 * Returns: the latitude
 */
double
shumate_map_source_get_latitude (ShumateMapSource *map_source,
    guint zoom_level,
    double y)
{
  double latitude, map_size, dy;

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0.0);
  /* FIXME: support other projections */
  map_size = shumate_map_source_get_tile_size (map_source) * shumate_map_source_get_row_count (map_source, zoom_level);
  dy = 0.5 - y / map_size;
  latitude = 90.0 - 360.0 / G_PI * atan (exp (-dy * 2.0 * G_PI));
  
  return CLAMP (latitude, SHUMATE_MIN_LATITUDE, SHUMATE_MAX_LATITUDE);
}


/**
 * shumate_map_source_get_row_count:
 * @map_source: a #ShumateMapSource
 * @zoom_level: the zoom level
 *
 * Gets the number of tiles in a row at this zoom level for this map source.
 *
 * Returns: the number of tiles in a row
 */
guint
shumate_map_source_get_row_count (ShumateMapSource *map_source,
    guint zoom_level)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0);
  /* FIXME: support other projections */
  return (zoom_level != 0) ? 2 << (zoom_level - 1) : 1;
}


/**
 * shumate_map_source_get_column_count:
 * @map_source: a #ShumateMapSource
 * @zoom_level: the zoom level
 *
 * Gets the number of tiles in a column at this zoom level for this map
 * source.
 *
 * Returns: the number of tiles in a column
 */
guint
shumate_map_source_get_column_count (ShumateMapSource *map_source,
    guint zoom_level)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0);
  /* FIXME: support other projections */
  return (zoom_level != 0) ? 2 << (zoom_level - 1) : 1;
}


#define EARTH_RADIUS 6378137.0 /* meters, Equatorial radius */

/**
 * shumate_map_source_get_meters_per_pixel:
 * @map_source: a #ShumateMapSource
 * @zoom_level: the zoom level
 * @latitude: a latitude
 * @longitude: a longitude
 *
 * Gets meters per pixel at the position on the map using this map source's projection.
 *
 * Returns: the meters per pixel
 */
double
shumate_map_source_get_meters_per_pixel (ShumateMapSource *map_source,
    guint zoom_level,
    double latitude,
    G_GNUC_UNUSED double longitude)
{
  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE (map_source), 0.0);

  /*
   * Width is in pixels. (1 px)
   * m/px = radius_at_latitude / width_in_pixels
   * k = radius of earth = 6 378.1 km
   * radius_at_latitude = 2pi * k * sin (pi/2-theta)
   */

  double map_size = shumate_map_source_get_tile_size (map_source) * shumate_map_source_get_row_count (map_source, zoom_level);
  /* FIXME: support other projections */
  return 2.0 * G_PI * EARTH_RADIUS * sin (G_PI / 2.0 - G_PI / 180.0 * latitude) / map_size;
}


/**
 * shumate_map_source_fill_tile:
 * @map_source: a #ShumateMapSource
 * @tile: a #ShumateTile
 * @cancellable: a #GCancellable or %NULL
 *
 * Fills the tile with image data (either from cache, network or rendered
 * locally).
 */
void
shumate_map_source_fill_tile (ShumateMapSource *map_source,
                              ShumateTile      *tile,
                              GCancellable     *cancellable)
{
  g_return_if_fail (SHUMATE_IS_MAP_SOURCE (map_source));

  shumate_tile_set_state (tile, SHUMATE_STATE_LOADING);
  SHUMATE_MAP_SOURCE_GET_CLASS (map_source)->fill_tile (map_source, tile, cancellable);
}
