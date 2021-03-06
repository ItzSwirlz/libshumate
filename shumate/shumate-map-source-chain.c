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
 * SECTION:shumate-map-source-chain
 * @short_description: A map source simplifying creation of source chains
 *
 * This map source simplifies creation of map chains by providing two
 * functions for their creation and modification in a stack-like manner:
 * shumate_map_source_chain_push() and shumate_map_source_chain_pop().
 * For instance, to create a chain consisting of #ShumateMemoryCache,
 * #ShumateFileCache and #ShumateNetworkTileSource, the map
 * sources have to be pushed into the chain in the reverse order starting
 * from #ShumateNetworkTileSource. After its creation, #ShumateMapSourceChain
 * behaves as a chain of map sources it contains.
 */

#include "shumate-map-source-chain.h"
#include "shumate-tile-cache.h"
#include "shumate-tile-source.h"

typedef struct
{
  ShumateMapSource *stack_top;
  ShumateMapSource *stack_bottom;
} ShumateMapSourceChainPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (ShumateMapSourceChain, shumate_map_source_chain, SHUMATE_TYPE_MAP_SOURCE);

static const char *get_id (ShumateMapSource *map_source);
static const char *get_name (ShumateMapSource *map_source);
static const char *get_license (ShumateMapSource *map_source);
static const char *get_license_uri (ShumateMapSource *map_source);
static guint get_min_zoom_level (ShumateMapSource *map_source);
static guint get_max_zoom_level (ShumateMapSource *map_source);
static guint get_tile_size (ShumateMapSource *map_source);

static void fill_tile (ShumateMapSource *map_source,
                       ShumateTile      *tile,
                       GCancellable     *cancellable);
static void on_set_next_source_cb (ShumateMapSourceChain *source_chain,
    G_GNUC_UNUSED gpointer user_data);


static void
shumate_map_source_chain_dispose (GObject *object)
{
  ShumateMapSourceChain *source_chain = SHUMATE_MAP_SOURCE_CHAIN (object);
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  while (priv->stack_top)
    shumate_map_source_chain_pop (source_chain);

  G_OBJECT_CLASS (shumate_map_source_chain_parent_class)->dispose (object);
}

static void
shumate_map_source_chain_class_init (ShumateMapSourceChainClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ShumateMapSourceClass *map_source_class = SHUMATE_MAP_SOURCE_CLASS (klass);

  object_class->dispose = shumate_map_source_chain_dispose;

  map_source_class->get_id = get_id;
  map_source_class->get_name = get_name;
  map_source_class->get_license = get_license;
  map_source_class->get_license_uri = get_license_uri;
  map_source_class->get_min_zoom_level = get_min_zoom_level;
  map_source_class->get_max_zoom_level = get_max_zoom_level;
  map_source_class->get_tile_size = get_tile_size;

  map_source_class->fill_tile = fill_tile;
}


static void
shumate_map_source_chain_init (ShumateMapSourceChain *source_chain)
{
  g_signal_connect (source_chain, "notify::next-source",
      G_CALLBACK (on_set_next_source_cb), NULL);
}


/**
 * shumate_map_source_chain_new:
 *
 * Constructor of #ShumateMapSourceChain.
 *
 * Returns: a new empty #ShumateMapSourceChain.
 */
ShumateMapSourceChain *
shumate_map_source_chain_new (void)
{
  return g_object_new (SHUMATE_TYPE_MAP_SOURCE_CHAIN, NULL);
}


static const char *
get_id (ShumateMapSource *map_source)
{
  ShumateMapSourceChain *source_chain = SHUMATE_MAP_SOURCE_CHAIN (map_source);
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE_CHAIN (map_source), NULL);
  g_return_val_if_fail (priv->stack_top, NULL);

  return shumate_map_source_get_id (priv->stack_top);
}


static const char *
get_name (ShumateMapSource *map_source)
{
  ShumateMapSourceChain *source_chain = SHUMATE_MAP_SOURCE_CHAIN (map_source);
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE_CHAIN (map_source), NULL);
  g_return_val_if_fail (priv->stack_top, NULL);

  return shumate_map_source_get_name (priv->stack_top);
}


static const char *
get_license (ShumateMapSource *map_source)
{
  ShumateMapSourceChain *source_chain = SHUMATE_MAP_SOURCE_CHAIN (map_source);
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE_CHAIN (map_source), NULL);
  g_return_val_if_fail (priv->stack_top, NULL);

  return shumate_map_source_get_license (priv->stack_top);
}


static const char *
get_license_uri (ShumateMapSource *map_source)
{
  ShumateMapSourceChain *source_chain = SHUMATE_MAP_SOURCE_CHAIN (map_source);
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE_CHAIN (map_source), NULL);
  g_return_val_if_fail (priv->stack_top, NULL);

  return shumate_map_source_get_license_uri (priv->stack_top);
}


static guint
get_min_zoom_level (ShumateMapSource *map_source)
{
  ShumateMapSourceChain *source_chain = SHUMATE_MAP_SOURCE_CHAIN (map_source);
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE_CHAIN (map_source), 0);
  g_return_val_if_fail (priv->stack_top, 0);

  return shumate_map_source_get_min_zoom_level (priv->stack_top);
}


static guint
get_max_zoom_level (ShumateMapSource *map_source)
{
  ShumateMapSourceChain *source_chain = SHUMATE_MAP_SOURCE_CHAIN (map_source);
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE_CHAIN (map_source), 0);
  g_return_val_if_fail (priv->stack_top, 0);

  return shumate_map_source_get_max_zoom_level (priv->stack_top);
}


static guint
get_tile_size (ShumateMapSource *map_source)
{
  ShumateMapSourceChain *source_chain = SHUMATE_MAP_SOURCE_CHAIN (map_source);
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  g_return_val_if_fail (SHUMATE_IS_MAP_SOURCE_CHAIN (map_source), 0);
  g_return_val_if_fail (priv->stack_top, 0);

  return shumate_map_source_get_tile_size (priv->stack_top);
}


static void
fill_tile (ShumateMapSource *map_source,
           ShumateTile      *tile,
           GCancellable     *cancellable)
{
  ShumateMapSourceChain *source_chain = SHUMATE_MAP_SOURCE_CHAIN (map_source);
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  g_return_if_fail (SHUMATE_IS_MAP_SOURCE_CHAIN (map_source));
  g_return_if_fail (priv->stack_top);

  shumate_map_source_fill_tile (priv->stack_top, tile, cancellable);
}


static void
on_set_next_source_cb (ShumateMapSourceChain *source_chain,
    G_GNUC_UNUSED gpointer user_data)
{
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);

  g_return_if_fail (source_chain);

  ShumateMapSource *map_source = SHUMATE_MAP_SOURCE (source_chain);
  ShumateMapSource *next_source;

  next_source = shumate_map_source_get_next_source (map_source);

  if (priv->stack_bottom)
    shumate_map_source_set_next_source (priv->stack_bottom, next_source);
}


static void
assign_cache_of_next_source_sequence (ShumateMapSourceChain *source_chain,
    ShumateMapSource *start_map_source,
    ShumateTileCache *tile_cache)
{
  ShumateMapSource *map_source = start_map_source;
  ShumateMapSource *chain_next_source = shumate_map_source_get_next_source (SHUMATE_MAP_SOURCE (source_chain));

  do
    {
      map_source = shumate_map_source_get_next_source (map_source);
    }
  while (SHUMATE_IS_TILE_CACHE (map_source));

  while (SHUMATE_IS_TILE_SOURCE (map_source) && map_source != chain_next_source)
    {
      shumate_tile_source_set_cache (SHUMATE_TILE_SOURCE (map_source), tile_cache);
      map_source = shumate_map_source_get_next_source (map_source);
    }
}


/**
 * shumate_map_source_chain_push:
 * @source_chain: a #ShumateMapSourceChain
 * @map_source: the #ShumateMapSource to be pushed into the chain
 *
 * Pushes a map source into the chain.
 */
void
shumate_map_source_chain_push (ShumateMapSourceChain *source_chain,
    ShumateMapSource *map_source)
{
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);
  gboolean is_cache = FALSE;

  if (SHUMATE_IS_TILE_CACHE (map_source))
    is_cache = TRUE;
  else
    g_return_if_fail (SHUMATE_IS_TILE_SOURCE (map_source));

  g_object_ref_sink (map_source);

  if (!priv->stack_top)
    {
      ShumateMapSource *chain_next_source = shumate_map_source_get_next_source (SHUMATE_MAP_SOURCE (source_chain));

      /* tile source has to be last */
      g_return_if_fail (!is_cache);

      priv->stack_top = map_source;
      priv->stack_bottom = map_source;
      if (chain_next_source)
        shumate_map_source_set_next_source (priv->stack_bottom, chain_next_source);
    }
  else
    {
      shumate_map_source_set_next_source (map_source, priv->stack_top);
      priv->stack_top = map_source;

      if (is_cache)
        {
          ShumateTileCache *tile_cache = SHUMATE_TILE_CACHE (map_source);
          assign_cache_of_next_source_sequence (source_chain, priv->stack_top, tile_cache);
        }
    }
}


/**
 * shumate_map_source_chain_pop:
 * @source_chain: a #ShumateMapSourceChain
 *
 * Pops a map source from the top of the stack from the chain.
 */
void
shumate_map_source_chain_pop (ShumateMapSourceChain *source_chain)
{
  ShumateMapSourceChainPrivate *priv = shumate_map_source_chain_get_instance_private (source_chain);
  ShumateMapSource *old_stack_top = priv->stack_top;
  ShumateMapSource *next_source = shumate_map_source_get_next_source (priv->stack_top);

  g_return_if_fail (priv->stack_top);

  if (SHUMATE_IS_TILE_CACHE (priv->stack_top))
    {
      ShumateTileCache *tile_cache = NULL;

      if (SHUMATE_IS_TILE_CACHE (next_source))
        tile_cache = SHUMATE_TILE_CACHE (next_source);

      /* _push() guarantees that the last source is tile_source so we can be
         sure that the next map source is still within the chain */
      assign_cache_of_next_source_sequence (source_chain, priv->stack_top, tile_cache);
    }

  if (next_source == shumate_map_source_get_next_source (SHUMATE_MAP_SOURCE (source_chain)))
    {
      priv->stack_top = NULL;
      priv->stack_bottom = NULL;
    }
  else
    priv->stack_top = next_source;

  g_object_unref (old_stack_top);
}
