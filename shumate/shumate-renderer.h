/*
 * Copyright (C) 2010-2013 Jiri Techet <techet@gmail.com>
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

#ifndef __SHUMATE_RENDERER_H__
#define __SHUMATE_RENDERER_H__

#include <shumate/shumate-tile.h>

G_BEGIN_DECLS

#define SHUMATE_TYPE_RENDERER shumate_renderer_get_type ()

#define SHUMATE_RENDERER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SHUMATE_TYPE_RENDERER, ShumateRenderer))

#define SHUMATE_RENDERER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), SHUMATE_TYPE_RENDERER, ShumateRendererClass))

#define SHUMATE_IS_RENDERER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SHUMATE_TYPE_RENDERER))

#define SHUMATE_IS_RENDERER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), SHUMATE_TYPE_RENDERER))

#define SHUMATE_RENDERER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SHUMATE_TYPE_RENDERER, ShumateRendererClass))

typedef struct _ShumateRenderer ShumateRenderer;
typedef struct _ShumateRendererClass ShumateRendererClass;


/**
 * ShumateRenderer:
 *
 * The #ShumateRenderer structure contains only private data
 * and should be accessed using the provided API
 *
 * Since: 0.8
 */
struct _ShumateRenderer
{
  GInitiallyUnowned parent;
};

struct _ShumateRendererClass
{
  GInitiallyUnownedClass parent_class;

  void (*set_data)(ShumateRenderer *renderer,
      const gchar *data,
      guint size);
  void (*render)(ShumateRenderer *renderer,
      ShumateTile *tile);
};

GType shumate_renderer_get_type (void);

void shumate_renderer_set_data (ShumateRenderer *renderer,
    const gchar *data,
    guint size);
void shumate_renderer_render (ShumateRenderer *renderer,
    ShumateTile *tile);

G_END_DECLS

#endif /* __SHUMATE_RENDERER_H__ */
