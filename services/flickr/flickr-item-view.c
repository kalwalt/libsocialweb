/*
 * libsocialweb - social data store
 * Copyright (C) 2008 - 2010 Intel Corporation.
 *
 * Author: Rob Bradford <rob@linux.intel.com>
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

#include <config.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <libsocialweb/sw-utils.h>

#include <rest/rest-proxy.h>
#include <rest/rest-xml-parser.h>
#include <libsoup/soup.h>

#include <libsocialweb/sw-debug.h>
#include <libsocialweb/sw-item.h>
#include <libsocialweb-keyfob/sw-keyfob.h>
#include <rest-extras/flickr-proxy.h>

#include "flickr-item-view.h"
#include "flickr.h"

G_DEFINE_TYPE (SwFlickrItemView, sw_flickr_item_view, SW_TYPE_ITEM_VIEW)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), SW_TYPE_FLICKR_ITEM_VIEW, SwFlickrItemViewPrivate))

typedef struct _SwFlickrItemViewPrivate SwFlickrItemViewPrivate;

struct _SwFlickrItemViewPrivate {
  RestProxy *proxy;
  guint timeout_id;
  GHashTable *params;
};

enum
{
  PROP_0,
  PROP_PROXY,
  PROP_PARAMS
};


#define UPDATE_TIMEOUT 5 * 60


static void
sw_flickr_item_view_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  SwFlickrItemViewPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_PROXY:
      g_value_set_object (value, priv->proxy);
      break;
    case PROP_PARAMS:
      g_value_set_boxed (value, priv->params);
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
sw_flickr_item_view_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  SwFlickrItemViewPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_PROXY:
      priv->proxy = g_value_get_object (value);
      break;
    case PROP_PARAMS:
      priv->params = g_value_dup_boxed (value);
      break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
sw_flickr_item_view_dispose (GObject *object)
{
  SwFlickrItemViewPrivate *priv = GET_PRIVATE (object);

  if (priv->proxy)
  {
    g_object_unref (priv->proxy);
    priv->proxy = NULL;
  }

  if (priv->params)
  {
    g_boxed_free (G_TYPE_HASH_TABLE,
                  priv->params);
    priv->params = NULL;
  }

  G_OBJECT_CLASS (sw_flickr_item_view_parent_class)->dispose (object);
}

static void
sw_flickr_item_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (sw_flickr_item_view_parent_class)->finalize (object);
}

static void
_photos_received_cb (RestProxyCall *call,
                     const GError  *error,
                     GObject       *weak_object,
                     gpointer       userdata)
{
  SwItemView *item_view = SW_ITEM_VIEW (weak_object);
  RestXmlParser *parser;
  RestXmlNode *root, *node;
  SwSet *set;

  if (error) {
    g_warning ("Cannot get Flickr photos: %s", error->message);
    return;
  }

  parser = rest_xml_parser_new ();
  root = rest_xml_parser_parse_from_data (parser,
                                          rest_proxy_call_get_payload (call),
                                          rest_proxy_call_get_payload_length (call));
  set = sw_set_new ();

  node = rest_xml_node_find (root, "photos");
  for (node = rest_xml_node_find (node, "photo"); node; node = node->next) {
    SwItem *item;
    SwService *service;

    service = sw_item_view_get_service (item_view);
    item = _flickr_item_from_from_photo_node (SW_SERVICE_FLICKR (service),
                                              node);
    sw_set_add (set, G_OBJECT (item));
    g_object_unref (item);
  }

  rest_xml_node_unref (root);
  g_object_unref (parser);

  sw_item_view_set_from_set (item_view, set);

  sw_set_unref (set);
}

static void
_make_request (SwFlickrItemView *item_view)
{
  SwFlickrItemViewPrivate *priv = GET_PRIVATE (item_view);
  RestProxyCall *call;
  GError *error = NULL;
  const gchar *request_type;

  call = rest_proxy_new_call (priv->proxy);

  request_type = g_hash_table_lookup (priv->params, "request_type");

  if (g_str_equal (request_type, "search"))
  {
    rest_proxy_call_set_function (call, "flickr.photos.search");

    if (g_hash_table_lookup (priv->params, "text"))
      rest_proxy_call_add_param (call, "text", g_hash_table_lookup (priv->params, "text"));

    if (g_hash_table_lookup (priv->params, "tags"))
      rest_proxy_call_add_param (call, "tags", g_hash_table_lookup (priv->params, "tags"));

    rest_proxy_call_add_param (call, "extras", "date_upload,icon_server,geo");
    rest_proxy_call_add_param (call, "count", "50");
  }

  if (!rest_proxy_call_async (call,
                              _photos_received_cb,
                              (GObject *)item_view,
                              NULL,
                              &error)) {
    g_warning ("Cannot get photos: %s", error->message);
    g_error_free (error);
  }
}

static void
_got_tokens_cb (RestProxy *proxy,
                gboolean   authorised,
                gpointer   userdata)
{
  SwFlickrItemView *item_view = SW_FLICKR_ITEM_VIEW (userdata);

  if (authorised) 
  {
    _make_request (item_view);
  }

  /* Drop reference we took for callback */
  g_object_unref (item_view);
}

static void
_get_status_updates (SwFlickrItemView *item_view)
{
  SwFlickrItemViewPrivate *priv = GET_PRIVATE (item_view);

  sw_keyfob_flickr ((FlickrProxy *)priv->proxy,
                    _got_tokens_cb,
                    g_object_ref (item_view)); /* ref gets dropped in cb */
}

static gboolean
_update_timeout_cb (gpointer data)
{
  SwFlickrItemView *item_view = SW_FLICKR_ITEM_VIEW (data);

  _get_status_updates (item_view);

  return TRUE;
}

static void
sw_flickr_item_view_start (SwItemView *item_view)
{
  SwFlickrItemViewPrivate *priv = GET_PRIVATE (item_view);

  if (priv->timeout_id)
  {
    g_warning (G_STRLOC ": View already started.");
  } else {
    SW_DEBUG (TWITTER, G_STRLOC ": Setting up the timeout");
    priv->timeout_id = g_timeout_add_seconds (UPDATE_TIMEOUT,
                                              (GSourceFunc)_update_timeout_cb,
                                              item_view);
    _get_status_updates ((SwFlickrItemView *)item_view);
  }
}

static void
sw_flickr_item_view_class_init (SwFlickrItemViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  SwItemViewClass *item_view_class = SW_ITEM_VIEW_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (SwFlickrItemViewPrivate));

  object_class->get_property = sw_flickr_item_view_get_property;
  object_class->set_property = sw_flickr_item_view_set_property;
  object_class->dispose = sw_flickr_item_view_dispose;
  object_class->finalize = sw_flickr_item_view_finalize;

  item_view_class->start = sw_flickr_item_view_start;

  pspec = g_param_spec_object ("proxy",
                               "proxy",
                               "proxy",
                               REST_TYPE_PROXY,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_PROXY, pspec);

  pspec = g_param_spec_boxed ("params",
                              "params",
                              "params",
                              G_TYPE_HASH_TABLE,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_PARAMS, pspec);
}

static void
sw_flickr_item_view_init (SwFlickrItemView *self)
{
}
