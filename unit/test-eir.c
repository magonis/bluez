/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Intel Corporation
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/sdp.h"

#include <src/shared/tester.h>

#include "src/eir.h"

struct test_data {
	const void *eir_data;
	size_t eir_size;
	unsigned int flags;
	const char *name;
	bool name_complete;
	int8_t tx_power;
	const char **uuid;
};

static const unsigned char macbookair_data[] = {
		0x17, 0x09, 0x4d, 0x61, 0x72, 0x63, 0x65, 0x6c,
		0xe2, 0x80, 0x99, 0x73, 0x20, 0x4d, 0x61, 0x63,
		0x42, 0x6f, 0x6f, 0x6b, 0x20, 0x41, 0x69, 0x72,
		0x11, 0x03, 0x12, 0x11, 0x0c, 0x11, 0x0a, 0x11,
		0x1f, 0x11, 0x01, 0x11, 0x00, 0x10, 0x0a, 0x11,
		0x17, 0x11, 0x11, 0xff, 0x4c, 0x00, 0x01, 0x4d,
		0x61, 0x63, 0x42, 0x6f, 0x6f, 0x6b, 0x41, 0x69,
		0x72, 0x33, 0x2c, 0x31, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const char *macbookair_uuid[] = {
		"00001112-0000-1000-8000-00805f9b34fb",
		"0000110c-0000-1000-8000-00805f9b34fb",
		"0000110a-0000-1000-8000-00805f9b34fb",
		"0000111f-0000-1000-8000-00805f9b34fb",
		"00001101-0000-1000-8000-00805f9b34fb",
		"00001000-0000-1000-8000-00805f9b34fb",
		"0000110a-0000-1000-8000-00805f9b34fb",
		"00001117-0000-1000-8000-00805f9b34fb",
		NULL
};

static const struct test_data macbookair_test = {
	.eir_data = macbookair_data,
	.eir_size = sizeof(macbookair_data),
	.name = "Marcel’s MacBook Air",
	.name_complete = true,
	.tx_power = 127,
	.uuid = macbookair_uuid,
};

static const unsigned char iphone5_data[] = {
		0x14, 0x09, 0x4d, 0x61, 0x72, 0x63, 0x65, 0x6c,
		0xe2, 0x80, 0x99, 0x73, 0x20, 0x69, 0x50, 0x68,
		0x6f, 0x6e, 0x65, 0x20, 0x35, 0x0f, 0x03, 0x00,
		0x12, 0x1f, 0x11, 0x2f, 0x11, 0x0a, 0x11, 0x0c,
		0x11, 0x16, 0x11, 0x32, 0x11, 0x01, 0x05, 0x11,
		0x07, 0xfe, 0xca, 0xca, 0xde, 0xaf, 0xde, 0xca,
		0xde, 0xde, 0xfa, 0xca, 0xde, 0x00, 0x00, 0x00,
		0x00, 0x27, 0xff, 0x00, 0x4c, 0x02, 0x24, 0x02,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const char *iphone5_uuid[] = {
		"00001200-0000-1000-8000-00805f9b34fb",
		"0000111f-0000-1000-8000-00805f9b34fb",
		"0000112f-0000-1000-8000-00805f9b34fb",
		"0000110a-0000-1000-8000-00805f9b34fb",
		"0000110c-0000-1000-8000-00805f9b34fb",
		"00001116-0000-1000-8000-00805f9b34fb",
		"00001132-0000-1000-8000-00805f9b34fb",
		"00000000-deca-fade-deca-deafdecacafe",
		NULL
};

static const struct test_data iphone5_test = {
	.eir_data = iphone5_data,
	.eir_size = sizeof(iphone5_data),
	.name = "Marcel’s iPhone 5",
	.name_complete = true,
	.tx_power = 127,
	.uuid = iphone5_uuid,
};

static const unsigned char ipadmini_data[] = {
		0x13, 0x09, 0x4d, 0x61, 0x72, 0x63, 0x65, 0x6c,
		0x27, 0x73, 0x20, 0x69, 0x50, 0x61, 0x64, 0x20,
		0x6d, 0x69, 0x6e, 0x69, 0x0b, 0x03, 0x00, 0x12,
		0x1f, 0x11, 0x0a, 0x11, 0x0c, 0x11, 0x32, 0x11,
		0x01, 0x05, 0x11, 0x07, 0xfe, 0xca, 0xca, 0xde,
		0xaf, 0xde, 0xca, 0xde, 0xde, 0xfa, 0xca, 0xde,
		0x00, 0x00, 0x00, 0x00, 0x27, 0xff, 0x00, 0x4c,
		0x02, 0x24, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const char *ipadmini_uuid[] = {
		"00001200-0000-1000-8000-00805f9b34fb",
		"0000111f-0000-1000-8000-00805f9b34fb",
		"0000110a-0000-1000-8000-00805f9b34fb",
		"0000110c-0000-1000-8000-00805f9b34fb",
		"00001132-0000-1000-8000-00805f9b34fb",
		"00000000-deca-fade-deca-deafdecacafe",
		NULL
};

static const struct test_data ipadmini_test = {
	.eir_data = ipadmini_data,
	.eir_size = sizeof(ipadmini_data),
	.name = "Marcel's iPad mini",
	.name_complete = true,
	.tx_power = 127,
	.uuid = ipadmini_uuid,
};

static const unsigned char gigaset_sl400h_data[] = {
		0x0b, 0x03, 0x01, 0x11, 0x05, 0x11, 0x12, 0x11,
		0x03, 0x12, 0x1f, 0x11, 0x10, 0x09, 0x4d, 0x61,
		0x72, 0x63, 0x65, 0x6c, 0x27, 0x73, 0x20, 0x53,
		0x4c, 0x34, 0x30, 0x30, 0x48, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const char *gigaset_sl400h_uuid[] = {
		"00001101-0000-1000-8000-00805f9b34fb",
		"00001105-0000-1000-8000-00805f9b34fb",
		"00001112-0000-1000-8000-00805f9b34fb",
		"00001203-0000-1000-8000-00805f9b34fb",
		"0000111f-0000-1000-8000-00805f9b34fb",
		NULL
};

static const struct test_data gigaset_sl400h_test = {
	.eir_data = gigaset_sl400h_data,
	.eir_size = sizeof(gigaset_sl400h_data),
	.name = "Marcel's SL400H",
	.name_complete = true,
	.tx_power = 127,
	.uuid = gigaset_sl400h_uuid,
};

static const unsigned char gigaset_sl910_data[] = {
		0x0b, 0x03, 0x01, 0x11, 0x05, 0x11, 0x12, 0x11,
		0x03, 0x12, 0x1f, 0x11, 0x0f, 0x09, 0x4d, 0x61,
		0x72, 0x63, 0x65, 0x6c, 0x27, 0x73, 0x20, 0x53,
		0x4c, 0x39, 0x31, 0x30, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const char *gigaset_sl910_uuid[] = {
		"00001101-0000-1000-8000-00805f9b34fb",
		"00001105-0000-1000-8000-00805f9b34fb",
		"00001112-0000-1000-8000-00805f9b34fb",
		"00001203-0000-1000-8000-00805f9b34fb",
		"0000111f-0000-1000-8000-00805f9b34fb",
		NULL
};

static const struct test_data gigaset_sl910_test = {
	.eir_data = gigaset_sl910_data,
	.eir_size = sizeof(gigaset_sl910_data),
	.name = "Marcel's SL910",
	.name_complete = true,
	.tx_power = 127,
	.uuid = gigaset_sl910_uuid,
};

static const unsigned char nokia_bh907_data[] = {
		0x16, 0x09, 0x4e, 0x6f, 0x6b, 0x69, 0x61, 0x20,
		0x52, 0x65, 0x61, 0x63, 0x74, 0x69, 0x6f, 0x6e,
		0x20, 0x42, 0x48, 0x2d, 0x39, 0x30, 0x37, 0x02,
		0x0a, 0x04, 0x0f, 0x02, 0x0d, 0x11, 0x0b, 0x11,
		0x0e, 0x11, 0x0f, 0x11, 0x1e, 0x11, 0x08, 0x11,
		0x31, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const char *nokia_bh907_uuid[] = {
		"0000110d-0000-1000-8000-00805f9b34fb",
		"0000110b-0000-1000-8000-00805f9b34fb",
		"0000110e-0000-1000-8000-00805f9b34fb",
		"0000110f-0000-1000-8000-00805f9b34fb",
		"0000111e-0000-1000-8000-00805f9b34fb",
		"00001108-0000-1000-8000-00805f9b34fb",
		"00001131-0000-1000-8000-00805f9b34fb",
		NULL
};

static const struct test_data nokia_bh907_test = {
	.eir_data = nokia_bh907_data,
	.eir_size = sizeof(nokia_bh907_data),
	.name = "Nokia Reaction BH-907",
	.name_complete = true,
	.tx_power = 4,
	.uuid = nokia_bh907_uuid,
};

static const unsigned char fuelband_data[] = {
		0x0f, 0x09, 0x4e, 0x69, 0x6b, 0x65, 0x2b, 0x20,
		0x46, 0x75, 0x65, 0x6c, 0x42, 0x61, 0x6e, 0x64,
		0x11, 0x07, 0x00, 0x00, 0x00, 0x00, 0xde, 0xca,
		0xfa, 0xde, 0xde, 0xca, 0xde, 0xaf, 0xde, 0xca,
		0xca, 0xff, 0x02, 0x0a, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const char *fuelband_uuid[] = {
		"ffcacade-afde-cade-defa-cade00000000",
		NULL
};

static const struct test_data fuelband_test = {
	.eir_data = fuelband_data,
	.eir_size = sizeof(fuelband_data),
	.name = "Nike+ FuelBand",
	.name_complete = true,
	.tx_power = 0,
	.uuid = fuelband_uuid,
};

static const unsigned char bluesc_data[] = {
		0x02, 0x01, 0x06, 0x03, 0x02, 0x16, 0x18, 0x12,
		0x09, 0x57, 0x61, 0x68, 0x6f, 0x6f, 0x20, 0x42,
		0x6c, 0x75, 0x65, 0x53, 0x43, 0x20, 0x76, 0x31,
		0x2e, 0x34,
};

static const char *bluesc_uuid[] = {
		"00001816-0000-1000-8000-00805f9b34fb",
		NULL
};

static const struct test_data bluesc_test = {
	.eir_data = bluesc_data,
	.eir_size = sizeof(bluesc_data),
	.flags = 0x06,
	.name = "Wahoo BlueSC v1.4",
	.name_complete = true,
	.tx_power = 127,
	.uuid = bluesc_uuid,
};

static const unsigned char wahoo_scale_data[] = {
		0x02, 0x01, 0x06, 0x03, 0x02, 0x01, 0x19, 0x11,
		0x09, 0x57, 0x61, 0x68, 0x6f, 0x6f, 0x20, 0x53,
		0x63, 0x61, 0x6c, 0x65, 0x20, 0x76, 0x31, 0x2e,
		0x33, 0x05, 0xff, 0x00, 0x00, 0x00, 0x9c,
};

static const char *wahoo_scale_uuid[] = {
		"00001901-0000-1000-8000-00805f9b34fb",
		NULL
};

static const struct test_data wahoo_scale_test = {
	.eir_data = wahoo_scale_data,
	.eir_size = sizeof(wahoo_scale_data),
	.flags = 0x06,
	.name = "Wahoo Scale v1.3",
	.name_complete = true,
	.tx_power = 127,
	.uuid = wahoo_scale_uuid,
};

static const unsigned char mio_alpha_data[] = {
		0x02, 0x01, 0x06, 0x03, 0x02, 0x0d, 0x18, 0x06,
		0x09, 0x41, 0x4c, 0x50, 0x48, 0x41,
};

static const char *mio_alpha_uuid[] = {
		"0000180d-0000-1000-8000-00805f9b34fb",
		NULL
};

static const struct test_data mio_alpha_test = {
	.eir_data = mio_alpha_data,
	.eir_size = sizeof(mio_alpha_data),
	.flags = 0x06,
	.name = "ALPHA",
	.name_complete = true,
	.tx_power = 127,
	.uuid = mio_alpha_uuid,
};

static const unsigned char cookoo_data[] = {
		0x02, 0x01, 0x05, 0x05, 0x02, 0x02, 0x18, 0x0a,
		0x18, 0x0d, 0x09, 0x43, 0x4f, 0x4f, 0x4b, 0x4f,
		0x4f, 0x20, 0x77, 0x61, 0x74, 0x63, 0x68,
};

static const char *cookoo_uuid[] = {
		"00001802-0000-1000-8000-00805f9b34fb",
		"0000180a-0000-1000-8000-00805f9b34fb",
		NULL
};

static const struct test_data cookoo_test = {
	.eir_data = cookoo_data,
	.eir_size = sizeof(cookoo_data),
	.flags = 0x05,
	.name = "COOKOO watch",
	.name_complete = true,
	.tx_power = 127,
	.uuid = cookoo_uuid,
};

static const unsigned char citizen_adv_data[] = {
		0x02, 0x01, 0x05, 0x05, 0x12, 0x7f, 0x01, 0x8f,
		0x01, 0x14, 0x09, 0x45, 0x63, 0x6f, 0x2d, 0x44,
		0x72, 0x69, 0x76, 0x65, 0x20, 0x50, 0x72, 0x6f,
		0x78, 0x69, 0x6d, 0x69, 0x74, 0x79,
};

static const struct test_data citizen_adv_test = {
	.eir_data = citizen_adv_data,
	.eir_size = sizeof(citizen_adv_data),
	.flags = 0x05,
	.name = "Eco-Drive Proximity",
	.name_complete = true,
	.tx_power = 127,
};

static const unsigned char citizen_scan_data[] = {
		0x02, 0x0a, 0x00, 0x11, 0x07, 0x1b, 0xc5, 0xd5,
		0xa5, 0x02, 0x00, 0x46, 0x9a, 0xe1, 0x11, 0xb7,
		0x8d, 0x60, 0xb4, 0x45, 0x2d,
};

static const char *citizen_scan_uuid[] = {
		"2d45b460-8db7-11e1-9a46-0002a5d5c51b",
		NULL
};

static const struct test_data citizen_scan_test = {
	.eir_data = citizen_scan_data,
	.eir_size = sizeof(citizen_scan_data),
	.tx_power = 0,
	.uuid = citizen_scan_uuid,
};

static void test_basic(const void *data)
{
	struct eir_data eir;
	unsigned char buf[HCI_MAX_EIR_LENGTH];

	memset(buf, 0, sizeof(buf));
	memset(&eir, 0, sizeof(eir));

	eir_parse(&eir, buf, HCI_MAX_EIR_LENGTH);
	g_assert(eir.services == NULL);
	g_assert(eir.name == NULL);

	eir_data_free(&eir);

	tester_test_passed();
}

static void test_parsing(gconstpointer data)
{
	const struct test_data *test = data;
	struct eir_data eir;
	GSList *list;

	memset(&eir, 0, sizeof(eir));

	eir_parse(&eir, test->eir_data, test->eir_size);

	tester_debug("Flags: %d", eir.flags);
	tester_debug("Name: %s", eir.name);
	tester_debug("TX power: %d", eir.tx_power);

	for (list = eir.services; list; list = list->next) {
		char *uuid_str = list->data;

		tester_debug("UUID: %s", uuid_str);
	}

	g_assert(eir.flags == test->flags);

	if (test->name) {
		g_assert_cmpstr(eir.name, ==, test->name);
		g_assert(eir.name_complete == test->name_complete);
	} else {
		g_assert(eir.name == NULL);
	}

	g_assert(eir.tx_power == test->tx_power);

	if (test->uuid) {
		GSList *list;
		int n = 0;

		for (list = eir.services; list; list = list->next, n++) {
			char *uuid_str = list->data;
			g_assert(test->uuid[n]);
			g_assert_cmpstr(test->uuid[n], ==, uuid_str);
		}
	} else {
		g_assert(eir.services == NULL);
	}

	eir_data_free(&eir);

	tester_test_passed();
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	tester_add("/eir/basic", NULL, NULL, test_basic, NULL);

	tester_add("/eir/macbookair", &macbookair_test, NULL, test_parsing,
									NULL);
	tester_add("/eir/iphone5", &iphone5_test, NULL, test_parsing, NULL);
	tester_add("/eir/ipadmini", &ipadmini_test, NULL, test_parsing, NULL);
	tester_add("/eir/sl400h", &gigaset_sl400h_test, NULL, test_parsing,
									NULL);
	tester_add("/eir/sl910", &gigaset_sl910_test, NULL, test_parsing, NULL);
	tester_add("/eir/bh907", &nokia_bh907_test, NULL, test_parsing, NULL);
	tester_add("/eir/fuelband", &fuelband_test, NULL, test_parsing, NULL);
	tester_add("/ad/bluesc", &bluesc_test, NULL, test_parsing, NULL);
	tester_add("/ad/wahooscale", &wahoo_scale_test, NULL, test_parsing,
									NULL);
	tester_add("/ad/mioalpha", &mio_alpha_test, NULL, test_parsing, NULL);
	tester_add("/ad/cookoo", &cookoo_test, NULL, test_parsing, NULL);
	tester_add("/ad/citizen1", &citizen_adv_test, NULL, test_parsing, NULL);
	tester_add("/ad/citizen2", &citizen_scan_test, NULL, test_parsing,
									NULL);

	return tester_run();
}
