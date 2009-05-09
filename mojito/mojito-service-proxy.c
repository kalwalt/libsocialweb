/*
 * Mojito - social data store
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

#include "mojito-service-proxy.h"
#include "mojito-service-ginterface.h"
static void service_iface_init (gpointer g_iface, gpointer iface_data);
G_DEFINE_TYPE_WITH_CODE (MojitoServiceProxy, mojito_service_proxy, MOJITO_TYPE_SERVICE,
                         G_IMPLEMENT_INTERFACE (MOJITO_TYPE_SERVICE_IFACE, service_iface_init));


#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOJITO_TYPE_SERVICE_PROXY, MojitoServiceProxyPrivate))

typedef struct _MojitoServiceProxyPrivate MojitoServiceProxyPrivate;

struct _MojitoServiceProxyPrivate {
  GType type;
  MojitoService *instance;
};

enum {
  PROP_0,
  PROP_TYPE
};

static void
mojito_service_proxy_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_TYPE:
      priv->type = g_value_get_gtype (value);
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
mojito_service_proxy_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_TYPE:
      g_value_set_gtype (value, priv->type);
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
mojito_service_proxy_dispose (GObject *object)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (object);

  if (priv->instance)
  {
    g_object_unref (priv->instance);
    priv->instance = NULL;
  }

  G_OBJECT_CLASS (mojito_service_proxy_parent_class)->dispose (object);
}

static void
decode_caps (guint32 caps, gboolean *can_get_persona_icon, gboolean *can_update_status)
{
  g_assert (can_get_persona_icon);
  g_assert (can_update_status);

  *can_get_persona_icon = caps & SERVICE_CAN_GET_PERSONA_ICON;
  *can_update_status = caps & SERVICE_CAN_UPDATE_STATUS;

#if 0
  g_debug ("got caps: %spersona icons %supdate status",
           *can_get_persona_icon ? "+" : "-",
           *can_update_status ? "+" : "-");
#endif
}

static void
on_caps_changed (MojitoService *service, guint32 caps, gpointer user_data)
{
  MojitoServiceProxy *proxy = MOJITO_SERVICE_PROXY (user_data);
  gboolean can_get_persona_icon, can_update_status;

  decode_caps (caps, &can_get_persona_icon, &can_update_status);

  mojito_service_iface_emit_capabilities_changed
    (proxy, can_get_persona_icon, can_update_status);
}

static void
on_avatar_retrieved (MojitoService *service,
                              const gchar   *path,
                              gpointer       userdata)
{
  MojitoServiceProxy *proxy = MOJITO_SERVICE_PROXY (userdata);

  mojito_service_iface_emit_avatar_retrieved (proxy, path);
}

static void
construct_instance (MojitoService *service)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (service);

  if (!priv->instance) {
    priv->instance = g_object_new (priv->type, NULL);

    g_signal_connect (priv->instance,
                      "caps-changed",
                      G_CALLBACK (on_caps_changed),
                      service);

    g_signal_connect (priv->instance,
                      "avatar-retrieved-internal",
                      G_CALLBACK (on_avatar_retrieved),
                      service);
  }
}

static const gchar *
mojito_service_proxy_get_name (MojitoService *service)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (service);
  MojitoServiceClass *class;

  construct_instance (service);

  class = MOJITO_SERVICE_GET_CLASS (priv->instance);
  return class->get_name (priv->instance);
}

static void
mojito_service_proxy_start (MojitoService *service)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (service);
  MojitoServiceClass *class;

  construct_instance (service);

  class = MOJITO_SERVICE_GET_CLASS (priv->instance);

  class->start (priv->instance);
}

static void
mojito_service_proxy_refresh (MojitoService *service)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (service);
  MojitoServiceClass *class;

  construct_instance (service);

  class = MOJITO_SERVICE_GET_CLASS (priv->instance);

  class->refresh (priv->instance);
}

static void
mojito_service_proxy_class_init (MojitoServiceProxyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  MojitoServiceClass *service_class = MOJITO_SERVICE_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (MojitoServiceProxyPrivate));

  object_class->get_property = mojito_service_proxy_get_property;
  object_class->set_property = mojito_service_proxy_set_property;
  object_class->dispose = mojito_service_proxy_dispose;

  service_class->get_name = mojito_service_proxy_get_name;
  service_class->start = mojito_service_proxy_start;
  service_class->refresh = mojito_service_proxy_refresh;

  pspec = g_param_spec_gtype ("type",
                              "type",
                              "The type we are proxying",
                              MOJITO_TYPE_SERVICE,
                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_TYPE, pspec);
}

static void
mojito_service_proxy_init (MojitoServiceProxy *self)
{
}

MojitoServiceProxy *
mojito_service_proxy_new (GType       service_type)
{
  return (MojitoServiceProxy *)g_object_new (MOJITO_TYPE_SERVICE_PROXY,
                                             "type",
                                             service_type,
                                             NULL);
}


static void
service_get_persona_icon (MojitoServiceIface    *self,
                          DBusGMethodInvocation *context)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (self);
  MojitoServiceClass *service_class;
  gchar *persona_icon = NULL;

  construct_instance ((MojitoService*)self);

  service_class = MOJITO_SERVICE_GET_CLASS (priv->instance);

  if (service_class->get_persona_icon)
  {
    persona_icon = service_class->get_persona_icon (priv->instance);
  }

  mojito_service_iface_return_from_get_persona_icon (context, persona_icon);

  g_free (persona_icon);
}

static void
service_request_avatar (MojitoServiceIface    *self,
                        DBusGMethodInvocation *context)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (self);
  MojitoServiceClass *service_class;

  construct_instance ((MojitoService*)self);

  service_class = MOJITO_SERVICE_GET_CLASS (priv->instance);

  if (service_class->request_avatar)
  {
    service_class->request_avatar (priv->instance);
  }

  mojito_service_iface_return_from_request_avatar (context);
}

static void
service_update_status (MojitoServiceIface    *self,
                       const gchar           *status_message,
                       DBusGMethodInvocation *context)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (self);
  MojitoServiceClass *service_class;
  gboolean res = TRUE;

  construct_instance ((MojitoService*)self);

  service_class = MOJITO_SERVICE_GET_CLASS (priv->instance);

  if (service_class->update_status)
  {
    res = service_class->update_status (priv->instance, status_message);
    if (res == FALSE)
    {
      g_warning (G_STRLOC ": Error updating status message");
    }
  }

  mojito_service_iface_return_from_update_status (context, res);
}

static void
service_get_capabilities (MojitoServiceIface    *self,
                          DBusGMethodInvocation *context)
{
  MojitoServiceProxyPrivate *priv = GET_PRIVATE (self);
  MojitoServiceClass *service_class;
  guint32 caps = 0;
  gboolean can_get_persona_icon;
  gboolean can_update_status;

  construct_instance ((MojitoService*)self);

  service_class = MOJITO_SERVICE_GET_CLASS (priv->instance);

  if (service_class->get_capabilities)
    caps = service_class->get_capabilities (priv->instance);

  decode_caps (caps, &can_get_persona_icon, &can_update_status);

  mojito_service_iface_return_from_get_capabilities (context,
                                                     can_get_persona_icon,
                                                     can_update_status);
}

static void
service_iface_init (gpointer g_iface,
                    gpointer iface_data)
{
  MojitoServiceIfaceClass *klass = (MojitoServiceIfaceClass *)g_iface;

  mojito_service_iface_implement_get_persona_icon (klass,
                                                   service_get_persona_icon);
  mojito_service_iface_implement_update_status (klass,
                                                service_update_status);
  mojito_service_iface_implement_get_capabilities (klass,
                                                   service_get_capabilities);
  mojito_service_iface_implement_request_avatar (klass,
                                                 service_request_avatar);
}
