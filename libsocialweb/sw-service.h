/*
 * libsocialweb - social data store
 * Copyright (C) 2008 - 2009 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef _SW_SERVICE
#define _SW_SERVICE

#include <glib-object.h>
#include <libsocialweb/sw-types.h>
#include <libsocialweb/sw-item.h>
#include <libsocialweb/sw-set.h>

G_BEGIN_DECLS

#define SW_TYPE_SERVICE sw_service_get_type()

typedef enum {
  SW_SERVICE_ERROR_NO_KEYS
} SwServiceError;

#define SW_SERVICE_ERROR sw_service_error_quark ()
GQuark sw_service_error_quark (void);

#define SW_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SW_TYPE_SERVICE, SwService))

#define SW_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), SW_TYPE_SERVICE, SwServiceClass))

#define SW_IS_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SW_TYPE_SERVICE))

#define SW_IS_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), SW_TYPE_SERVICE))

#define SW_SERVICE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SW_TYPE_SERVICE, SwServiceClass))

struct _SwService {
  GObject parent;
};

typedef struct _SwServiceClass SwServiceClass;
struct _SwServiceClass {
  GObjectClass parent_class;
  /* signals */
  void (*avatar_retrieved)(SwService *service, const gchar *path);
  void (*caps_changed) (SwService *service, char **caps);

  /* vfuncs */
  /* Call before construction to see if this service can be used */
  gboolean (*ready) (SwServiceClass *klass); /* TODO: rename */
  const char *(*get_name) (SwService *service);
  void (*start) (SwService *service);
  /* fires ::refreshed (SwSet *set) signal */
  void (*refresh) (SwService *service);
  SwItem *(*get_last_item) (SwService *service);

  const gchar ** (*get_static_caps) (SwService *service);
  const gchar ** (*get_dynamic_caps) (SwService *service);

  void (*request_avatar) (SwService *service);

  void (*credentials_updated) (SwService *service);
};

#define CAN_UPDATE_STATUS "can-update-status"
#define CAN_REQUEST_AVATAR "can-request-avatar"

GType sw_service_get_type (void);

gboolean sw_service_ready (SwServiceClass *klass);

const char *sw_service_get_name (SwService *service);

void sw_service_start (SwService *service);

void sw_service_refresh (SwService *service);

void sw_service_emit_refreshed (SwService *service,
                                SwSet     *set);

void sw_service_emit_capabilities_changed (SwService   *service,
                                           const char **caps);

void sw_service_emit_status_updated (SwService *service,
                                     gboolean   success);

void sw_service_emit_avatar_retrieved (SwService   *service,
                                       const gchar *path);

void sw_service_emit_user_changed (SwService *service);

const char * sw_service_get_param (SwService  *service,
                                   const char *key);

G_END_DECLS

#endif /* _SW_SERVICE */
