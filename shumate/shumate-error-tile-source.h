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

#if !defined (__SHUMATE_SHUMATE_H_INSIDE__) && !defined (SHUMATE_COMPILATION)
#error "Only <shumate/shumate.h> can be included directly."
#endif

#ifndef __SHUMATE_ERROR_TILE_SOURCE__
#define __SHUMATE_ERROR_TILE_SOURCE__
#include <glib-object.h>

#include <shumate/shumate-tile-source.h>

G_BEGIN_DECLS

#define SHUMATE_TYPE_ERROR_TILE_SOURCE shumate_error_tile_source_get_type ()
G_DECLARE_DERIVABLE_TYPE (ShumateErrorTileSource, shumate_error_tile_source, SHUMATE, ERROR_TILE_SOURCE, ShumateTileSource)

/**
 * ShumateErrorTileSource:
 *
 * The #ShumateErrorTileSource structure contains only private data
 * and should be accessed using the provided API
 */
struct _ShumateErrorTileSourceClass
{
  ShumateTileSourceClass parent_class;
};

ShumateErrorTileSource *shumate_error_tile_source_new_full (void);


G_END_DECLS

#endif /* __SHUMATE_ERROR_TILE_SOURCE__ */
