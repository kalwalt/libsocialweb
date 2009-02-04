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

#ifndef _MOJITO_CACHE
#define _MOJITO_CACHE

#include <mojito/mojito-service.h>
#include <mojito/mojito-set.h>

G_BEGIN_DECLS

void mojito_cache_save (MojitoService *service, MojitoSet *set);

MojitoSet * mojito_cache_load (MojitoService *service);

G_END_DECLS

#endif /* _MOJITO_CACHE */