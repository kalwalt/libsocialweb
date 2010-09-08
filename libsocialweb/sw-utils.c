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

#include <config.h>
#include "sw-utils.h"
#include <string.h>
#include <libsoup/soup.h>

time_t
sw_time_t_from_string (const char *s)
{
  SoupDate *date;
  time_t t;

  g_return_val_if_fail (s, 0);

  date = soup_date_new_from_string (s);
  t = soup_date_to_time_t (date);
  soup_date_free (date);

  return t;
}

char *
sw_time_t_to_string (time_t t)
{
  SoupDate *date;
  char *s;

  date = soup_date_new_from_time_t (t);
  s = soup_date_to_string (date, SOUP_DATE_HTTP);
  soup_date_free (date);

  return s;
}

/*
 * Return a hash of the contents of a string->string hash table.
 */
char *
sw_hash_string_dict (GHashTable *hash)
{
  GList *keys;
  GChecksum *sum;
  char *md5;

  g_return_val_if_fail (hash, NULL);

  sum = g_checksum_new (G_CHECKSUM_MD5);

  keys = g_hash_table_get_keys (hash);
  keys = g_list_sort (keys, (GCompareFunc)strcmp);

  while (keys) {
    g_checksum_update (sum, keys->data, -1);
    g_checksum_update (sum, g_hash_table_lookup (hash, keys->data), -1);
    keys = g_list_delete_link (keys, keys);
  }

  md5 = g_strdup (g_checksum_get_string (sum));
  g_checksum_free (sum);

  return md5;
}

/**
 * sw_next_opid:
 *
 * Get the next operation ID.  Operation IDs are globally unique for a
 * libsocialweb instance.  In the current implementation, they are simply
 * incrementing integers.
 */
int
sw_next_opid (void)
{
  static volatile gint opid = 1;

  return g_atomic_int_exchange_and_add (&opid, 1);
}
