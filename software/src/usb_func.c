/* Part of the DigiLivolo control software.
 * https://github.com/N-Storm/DigiLivolo/
 * Copyright (c) 2024 GitHub user N-Storm.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "defs.h"

#include <hidapi.h>
#include "usb_func.h"

 // Fallback/example
#ifndef HID_API_MAKE_VERSION
#define HID_API_MAKE_VERSION(mj, mn, p) (((mj) << 24) | ((mn) << 8) | (p))
#endif
#ifndef HID_API_VERSION
#define HID_API_VERSION HID_API_MAKE_VERSION(HID_API_VERSION_MAJOR, HID_API_VERSION_MINOR, HID_API_VERSION_PATCH)
#endif

const char* hid_bus_name(hid_bus_type bus_type) {
	static const char* const HidBusTypeName[] = {
		"Unknown",
		"USB",
		"Bluetooth",
		"I2C",
		"SPI",
	};

	if ((int)bus_type < 0)
		bus_type = HID_API_BUS_UNKNOWN;
	if ((int)bus_type >= (int)(sizeof(HidBusTypeName) / sizeof(HidBusTypeName[0])))
		bus_type = HID_API_BUS_UNKNOWN;

	return HidBusTypeName[bus_type];
}

void print_device(struct hid_device_info* cur_dev) {
	printf("VID/PID: 0x%04hx:0x%04hx, Product: %ls, Manufacturer: %ls, FW Ver: %d.%02d.\n", \
		cur_dev->vendor_id, cur_dev->product_id, cur_dev->product_string, cur_dev->manufacturer_string, \
		cur_dev->release_number >> 8, cur_dev->release_number & 0xFF);
}

void print_device_details(struct hid_device_info* cur_dev) {
	printf("Device 0x%04hx:0x%04hx found:\n", cur_dev->vendor_id, cur_dev->product_id);
	printf("  Path:          %s\n", cur_dev->path);
	printf("  Manufacturer:  %ls\n", cur_dev->manufacturer_string);
	printf("  Product:       %ls\n", cur_dev->product_string);
	printf("  Serial Number: %ls\n", cur_dev->serial_number);
	printf("  Release:       %hx\n", cur_dev->release_number);
	printf("  Interface:     %d\n", cur_dev->interface_number);
	printf("  Usage (page):  0x%hx (0x%hx)\n", cur_dev->usage, cur_dev->usage_page);
	printf("  Bus type:      %d (%s)\n", cur_dev->bus_type, hid_bus_name(cur_dev->bus_type));
	printf("\n");
}

void print_devices(struct hid_device_info* cur_dev) {
	for (; cur_dev; cur_dev = cur_dev->next) {
		print_device(cur_dev);
	}
}

struct hid_device_info* find_digilivolo(struct hid_device_info* cur_dev) {
	for (; cur_dev; cur_dev = cur_dev->next) {
		if ((wcscmp(cur_dev->manufacturer_string, DIGILIVOLO_MANUFACTURER_STRING) == 0) && \
			(wcscmp(cur_dev->product_string, DIGILIVOLO_PRODUCT_STRING) == 0))
			return cur_dev;
	}

	return NULL;
}

error_t dlusb_send(uint16_t remote_id, uint8_t btn_id, bool use_old_alg, hid_device* handle) {
	int res;
	// Buffer to constuct packet. HID Report descriptor configured to work with 8 bytes.
	// But the actual packet struct a bit smaller, so we "fit" it inside buffer.
	unsigned char buf[8] = { 0 };
	dlusb_packet_t* packet = (dlusb_packet_t*)buf;

	packet->report_id = REPORT_ID;
	packet->cmd_id = use_old_alg ? CMD_SWITCH_OLD : CMD_SWITCH;
	packet->remote_id = remote_id;
	packet->btn_id = btn_id;

	/// Send a Feature Report to the device
	res = hid_send_feature_report(handle, buf, sizeof(buf));
	return res;
}

error_t dlusb_read(dlusb_packet_t* packet, hid_device* handle) {
	int res;

	// Buffer for incoming packet
	unsigned char buf[8] = { 0 };

	// Read a Feature Report from the device
	// ((dlusb_packet_t*)buf)->report_id = REPORT_ID;
	buf[0] = REPORT_ID;
	res = hid_get_feature_report(handle, buf, sizeof(buf));

	// Copy packet data to output pointer if request was successful
	if (res >= 0)
		memcpy(packet, buf, sizeof(*packet));

	return res;
}
