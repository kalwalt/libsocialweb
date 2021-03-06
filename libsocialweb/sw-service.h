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
#include <libsocialweb/sw-enum-types.h>

G_BEGIN_DECLS

#define SW_TYPE_SERVICE sw_service_get_type()

typedef enum {
  SW_SERVICE_ERROR_NO_KEYS, /*< nick=NoKeys >*/
  SW_SERVICE_ERROR_INVALID_QUERY, /*< nick=InvalidQuery >*/
  SW_SERVICE_ERROR_NOT_SUPPORTED, /*< nick=NotSupported >*/
  SW_SERVICE_ERROR_REMOTE_ERROR /*< nick=RemoteError >*/
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
  /* vfuncs */
  const char *(*get_name) (SwService *service);
  const gchar ** (*get_static_caps) (SwService *service);
  const gchar ** (*get_dynamic_caps) (SwService *service);
  void (*credentials_updated) (SwService *service);
};

/* Keep in sync with sw-client-service.h */
#define IS_CONFIGURED "is-configured"
#define CAN_VERIFY_CREDENTIALS "can-verify-credentials"
#define CREDENTIALS_VALID "credentials-valid"
#define CREDENTIALS_INVALID "credentials-invalid"
#define CAN_UPDATE_STATUS "can-update-status"
#define CAN_REQUEST_AVATAR "can-request-avatar"
#define CAN_GEOTAG "can-geotag"

#define HAS_UPDATE_STATUS_IFACE "has-update-status-iface"
#define HAS_AVATAR_IFACE "has-avatar-iface"
#define HAS_PHOTO_UPLOAD_IFACE "has-photo-upload-iface"
#define HAS_VIDEO_UPLOAD_IFACE "has-video-upload-iface"
#define HAS_BANISHABLE_IFACE "has-banishable-iface"
#define HAS_QUERY_IFACE "has-query-iface"
#define HAS_CONTACTS_QUERY_IFACE "has-contacts-query-iface"
#define HAS_COLLECTIONS_IFACE "has-collections-iface"

#define CAN_UPDATE_STATUS_WITH_GEOTAG "can-update-status-with-geotag"

GType sw_service_get_type (void);

const char *sw_service_get_name (SwService *service);

void sw_service_emit_capabilities_changed (SwService   *service,
                                           const char **caps);
void sw_service_emit_user_changed (SwService *service);

gboolean sw_service_is_uid_banned (SwService   *service,
                                   const gchar *uid);

gboolean sw_service_has_cap (const char **caps, const char *cap);

typedef struct {
  char *lsw_param;
  char *service_param;
} ParameterNameMapping;

/**
 * SwServiceSetParamFunc:
 * @object: A remote call or hash object that can store key/value pairs.
 * @param_name: The parameter name.
 * @param_value: The parameter value.
 *
 * This type signature reflects a function that sets a name/value association
 * on a given object. Functions such as g_hash_table_insert and
 * rest_proxy_call_add_param are good examples.
 */


typedef void (*SwServiceSetParamFunc) (gpointer object,
                                       const gchar *param_name,
                                       const gchar *param_value);

void sw_service_map_params (const ParameterNameMapping *mapping,
                            GHashTable                 *parameters,
                            SwServiceSetParamFunc       set_param_func,
                            gpointer                    remote_call_object);

G_END_DECLS

#endif /* _SW_SERVICE */

