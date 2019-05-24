/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <ell/ell.h>

#include "mesh/mesh-defs.h"
#include "mesh/dbus.h"
#include "mesh/error.h"
#include "mesh/mesh.h"
#include "mesh/node.h"
#include "mesh/keyring.h"
#include "mesh/manager.h"

static struct l_dbus_message *add_node_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct l_dbus_message_iter iter_uuid;
	uint8_t *uuid;
	uint32_t n;

	l_debug("Add node request");

	if (!l_dbus_message_get_arguments(msg, "ay", &iter_uuid))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!l_dbus_message_iter_get_fixed_array(&iter_uuid, &uuid, &n)
								|| n != 16)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Bad device UUID");

	/* TODO */
	return dbus_error(msg, MESH_ERROR_NOT_IMPLEMENTED, NULL);
}

static struct l_dbus_message *delete_node_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	uint16_t primary;
	uint8_t num_ele;

	if (!l_dbus_message_get_arguments(msg, "qy", &primary, &num_ele))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	/* TODO */
	return dbus_error(msg, MESH_ERROR_NOT_IMPLEMENTED, NULL);
}

static struct l_dbus_message *start_scan_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	uint16_t duration;

	if (!l_dbus_message_get_arguments(msg, "q", &duration))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	/* TODO */
	return dbus_error(msg, MESH_ERROR_NOT_IMPLEMENTED, NULL);
}

static struct l_dbus_message *cancel_scan_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	/* TODO */
	return dbus_error(msg, MESH_ERROR_NOT_IMPLEMENTED, NULL);
}

static struct l_dbus_message *store_new_subnet(struct mesh_node *node,
					struct l_dbus_message *msg,
					uint16_t net_idx, uint8_t *new_key)
{
	struct keyring_net_key key;

	if (net_idx > MAX_KEY_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (keyring_get_net_key(node, net_idx, &key)) {
		/* Allow redundant calls only if key values match */
		if (!memcmp(key.old_key, new_key, 16))
			return l_dbus_message_new_method_return(msg);

		return dbus_error(msg, MESH_ERROR_ALREADY_EXISTS, NULL);
	}

	memcpy(key.old_key, new_key, 16);
	key.net_idx = net_idx;
	key.phase = KEY_REFRESH_PHASE_NONE;

	if (!keyring_put_net_key(node, net_idx, &key))
		return dbus_error(msg, MESH_ERROR_FAILED, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *create_subnet_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	uint8_t key[16];
	uint16_t net_idx;

	if (!l_dbus_message_get_arguments(msg, "q", &net_idx) ||
						net_idx == PRIMARY_NET_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	/* Generate key and store */
	l_getrandom(key, sizeof(key));

	return store_new_subnet(node, msg, net_idx, key);
}

static struct l_dbus_message *update_subnet_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct keyring_net_key key;
	uint16_t net_idx;

	if (!l_dbus_message_get_arguments(msg, "q", &net_idx) ||
						net_idx > MAX_KEY_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!keyring_get_net_key(node, net_idx, &key))
		return dbus_error(msg, MESH_ERROR_DOES_NOT_EXIST, NULL);

	switch (key.phase) {
	case KEY_REFRESH_PHASE_NONE:
		/* Generate Key and update phase */
		l_getrandom(key.new_key, sizeof(key.new_key));
		key.phase = KEY_REFRESH_PHASE_ONE;

		if (!keyring_put_net_key(node, net_idx, &key))
			return dbus_error(msg, MESH_ERROR_FAILED, NULL);

		/* Fall Through */

	case KEY_REFRESH_PHASE_ONE:
		/* Allow redundant calls to start Key Refresh */
		return l_dbus_message_new_method_return(msg);

	default:
		break;
	}

	/* All other phases mean KR already in progress over-the-air */
	return dbus_error(msg, MESH_ERROR_BUSY, "Key Refresh in progress");
}

static struct l_dbus_message *delete_subnet_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	uint16_t net_idx;

	if (!l_dbus_message_get_arguments(msg, "q", &net_idx) ||
						net_idx > MAX_KEY_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	keyring_del_net_key(node, net_idx);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *import_subnet_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct l_dbus_message_iter iter_key;
	uint16_t net_idx;
	uint8_t *key;
	uint32_t n;

	if (!l_dbus_message_get_arguments(msg, "qay", &net_idx, &iter_key))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!l_dbus_message_iter_get_fixed_array(&iter_key, &key, &n)
								|| n != 16)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Bad network key");

	return store_new_subnet(node, msg, net_idx, key);
}

static struct l_dbus_message *create_appkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	uint16_t net_idx, app_idx;

	if (!l_dbus_message_get_arguments(msg, "qq", &net_idx, &app_idx))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	/* TODO */
	return dbus_error(msg, MESH_ERROR_NOT_IMPLEMENTED, NULL);
}

static struct l_dbus_message *update_appkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	uint16_t net_idx, app_idx;

	if (!l_dbus_message_get_arguments(msg, "qq", &net_idx, &app_idx))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	/* TODO */
	return dbus_error(msg, MESH_ERROR_NOT_IMPLEMENTED, NULL);
}

static struct l_dbus_message *delete_appkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	uint16_t net_idx, app_idx;

	if (!l_dbus_message_get_arguments(msg, "qq", &net_idx, &app_idx))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	/* TODO */
	return dbus_error(msg, MESH_ERROR_NOT_IMPLEMENTED, NULL);
}

static struct l_dbus_message *import_appkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	uint16_t net_idx, app_idx;
	struct l_dbus_message_iter iter_key;
	uint8_t *key;
	uint32_t n;

	if (!l_dbus_message_get_arguments(msg, "qqay", &net_idx, &app_idx,
								&iter_key))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!l_dbus_message_iter_get_fixed_array(&iter_key, &key, &n)
								|| n != 16)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Bad application key");

	/* TODO */
	return dbus_error(msg, MESH_ERROR_NOT_IMPLEMENTED, NULL);
}

static struct l_dbus_message *set_key_phase_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	uint16_t net_idx;
	uint8_t phase;

	if (!l_dbus_message_get_arguments(msg, "qy", &net_idx, &phase))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	/* TODO */
	return dbus_error(msg, MESH_ERROR_NOT_IMPLEMENTED, NULL);
}

static void setup_management_interface(struct l_dbus_interface *iface)
{
	l_dbus_interface_method(iface, "AddNode", 0, add_node_call, "", "ay",
								"", "uuid");
	l_dbus_interface_method(iface, "DeleteRemodeNode", 0, delete_node_call,
					"", "qy", "", "primary", "count");
	l_dbus_interface_method(iface, "UnprovisionedScan", 0, start_scan_call,
							"", "q", "", "seconds");
	l_dbus_interface_method(iface, "UnprovisionedScanCancel", 0,
						cancel_scan_call, "", "");
	l_dbus_interface_method(iface, "CreateSubnet", 0, create_subnet_call,
						"", "q", "", "net_index");
	l_dbus_interface_method(iface, "UpdateSubnet", 0, update_subnet_call,
						"", "q", "", "net_index");
	l_dbus_interface_method(iface, "DeleteSubnet", 0, delete_subnet_call,
						"", "q", "", "net_index");
	l_dbus_interface_method(iface, "ImportSubnet", 0, import_subnet_call,
					"", "qay", "", "net_index", "net_key");
	l_dbus_interface_method(iface, "CreateAppKey", 0, create_appkey_call,
					"", "qq", "", "net_index", "app_index");
	l_dbus_interface_method(iface, "UpdateAppKey", 0, update_appkey_call,
					"", "qq", "", "net_index", "app_index");
	l_dbus_interface_method(iface, "DeleteAppKey", 0, delete_appkey_call,
					"", "qq", "", "net_index", "app_index");
	l_dbus_interface_method(iface, "ImportAppKey", 0, import_appkey_call,
				"", "qqay", "", "net_index", "app_index",
								"app_key");
	l_dbus_interface_method(iface, "SetKeyPhase", 0, set_key_phase_call,
					"", "qy", "", "net_index", "phase");
}

bool manager_dbus_init(struct l_dbus *bus)
{
	if (!l_dbus_register_interface(bus, MESH_MANAGEMENT_INTERFACE,
						setup_management_interface,
						NULL, false)) {
		l_info("Unable to register %s interface",
						MESH_MANAGEMENT_INTERFACE);
		return false;
	}

	return true;
}
