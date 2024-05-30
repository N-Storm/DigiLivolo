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

#ifdef _WIN32
#include <windows.h>
#define dl_sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define dl_sleep_ms(ms) usleep(ms * 1000)
#endif

#include "defs.h"
#include "args.h"

#include <hidapi.h>
#include "usb_func.h"

#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_darwin.h>
#endif

 // [argp] Our argp parser.
static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char* argv[])
{
	hid_device* handle = NULL;
	struct hid_device_info* devices, * dl_dev;
	dlusb_packet_t packet;
	int res;

	// [argp] Default values.
	arguments.remote_id = 0;
	arguments.btn_id = 0;
	arguments.verbose = false;
	arguments.old_alg = false;

	// Print program name & version
	printf("%s\n", PROG_NAME_VERSION);

	/* Parse our arguments; every option seen by parse_opt will
	 * be reflected in arguments. */
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if (arguments.verbose) {
		printf("Compiled with hidapi version %s, runtime version %s.\n", HID_API_VERSION_STR, hid_version_str());
		printf("Arguments: REMOTE_ID = %d, KEY_CODE = %d\n", arguments.remote_id, arguments.btn_id);
	}

	if (hid_init())
		return -1;

#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
	// To work properly needs to be called before hid_open/hid_open_path after hid_init.
	// Best/recommended option - call it right after hid_init.
	hid_darwin_set_open_exclusive(0);
#endif

	devices = hid_enumerate(DIGILIVOLO_VID, DIGILIVOLO_PID);
	dl_dev = find_digilivolo(devices);
	if (!dl_dev) {
		printf("ERROR: unable to find device\n");
		if (arguments.verbose) {
			if (devices) {
				printf("Devices with matching VID/PID (0x%04x:0x%04x), but wrong product or manufacturer string:\n", DIGILIVOLO_VID, DIGILIVOLO_PID);
				print_devices(devices);
			}
			else {
				hid_free_enumeration(devices);
				devices = hid_enumerate(0, 0);
				printf("All enumerated devices, but none of them match VID/PID (0x%04x:0x%04x):\n", DIGILIVOLO_VID, DIGILIVOLO_PID);
				print_devices(devices);
			}
		}
	}
	else {
		if (arguments.verbose) {
			printf("Device found: ");
			print_device(dl_dev);
			printf("Opening device path: %s\n", dl_dev->path);
		}
		handle = hid_open_path(dl_dev->path);
	}

	hid_free_enumeration(devices);

	// Check if devices was opened succesfully previously
	if (!handle) {
		printf("ERROR: unable to open device\n");
		hid_exit();
		return 1;
	}

	struct hid_device_info* info = hid_get_device_info(handle);
	if (info == NULL) {
		printf("ERROR: Unable to get device info\n");
		hid_close(handle);
		hid_exit();
		return 1;
	}

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);

	res = 1;
	while (res) {
		res = dlusb_read(&packet, handle);
		if (res < 0) {
			printf("WARN: (%d) Unable to get a feature report: %ls\n", res, hid_error(handle));
		}
		else if (res > 0 && arguments.verbose) {
#ifdef DEBUG
			// Print out the returned buffer.
			printf("Read Feature Report from DigiLivolo: ");
			for (uint32_t i = 0; i < res; i++)
				printf("%02x ", *(((uint8_t*)&packet) + i));
			printf("\n");
#endif // DEBUG
		}
	}

	if (dl_dev->release_number < 0x200) {
		arguments.old_alg = false;
		if (arguments.verbose) {
			printf("WARN: Device firmware version doesn't supports old-alg feature. Using default, which should be old algorithm anyways.\n");
		}
	}

	// Send a Feature Report to the device
	res = dlusb_send(arguments.remote_id, arguments.btn_id, arguments.old_alg, handle);
	if (res < 0) {
		printf("ERROR: Unable to send a feature report.\n");
		hid_close(handle);
		hid_exit();
		return 1;
	}
	else
		printf("Command sent to device. Waiting for a reply...\n");

	// Read a Feature Report from the device
	res = 0;

	while (res == 0) {
		dl_sleep_ms(300);
		res = dlusb_read(&packet, handle);
		if (res == -1) { // This error usually means that the hardware didn't finished processing yet, give it one more try after delay
			if (arguments.verbose) {
				printf("WARN: (%d) Unable to get ACK feature report: %ls.\n", res, hid_error(handle));
				dl_sleep_ms(10);
				printf(" Retrying...\n");
			}
			dl_sleep_ms(300);
			res = dlusb_read(&packet, handle);
		}
		if (res < 0) {
			printf("WARN: (%d) Unable to get ACK feature report: %ls\n", res, hid_error(handle));
			continue;
		}
		else if (res > 0) {
			if (packet.cmd_id == ((arguments.old_alg == true) ? CMD_SWITCH_OLD : CMD_SWITCH) && \
				packet.remote_id == arguments.remote_id && packet.btn_id == arguments.btn_id) {
				printf("Device acks codes correctly.\n");
			}
			else {
				printf("ERROR: Got wrong reply from device!\n");
				hid_close(handle);

				/* Free static HIDAPI objects. */
				hid_exit();

				return -1;
			}
#ifdef DEBUG
			// Print out the returned buffer.
			printf("Read Feature Report from DigiLivolo: ");
			for (uint32_t i = 0; i < res; i++)
				printf("%02x ", *(((uint8_t*)&packet) + i));
			printf("\n");
#endif // DEBUG
		}
	}

	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

	return 0;
}
