/* Part of the DigiLivolo control software.
 * https://github.com/N-Storm/DigiLivolo/ */

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <hidapi.h>

#include "git_version.h"
#include <argp.h>

// [argp] Program documentation.
// const char* argp_program_version = GIT_VERSION;
const char prognamever[] = "digilivolo " GIT_VERSION;
const char doc[] = "\nSoftware to control DigiLivolo devices.\n";
const char* argp_program_bug_address = "https://github.com/N-Storm/DigiLivolo/";

// [argp] A description of the arguments we accept.
static char args_doc[] = "REMOTE_ID KEY_ID";

// [argp] The options we understand.
static struct argp_option options[] = {
  {0,             0,   0,                            0, "Positional arguments:"      },
  {"REMOTE_ID",   0,   0, OPTION_DOC | OPTION_NO_USAGE, "Livilo Remote ID (1-65535)" },
  {"KEY_ID",      0,   0, OPTION_DOC | OPTION_NO_USAGE, "Livilo Key ID (1-255)"      },
  {0,             0,   0,                            0, "Options:"                   },
  {"verbose",   'v',   0,                            0, "Produce verbose output"     },
  { 0 }
};

// [argp] Command-line arguments.
struct arguments {
	uint16_t remote_id;
	uint8_t key_id;
	bool verbose;
} arguments;

#define DIGILIVOLO_VID 0x16c0
#define DIGILIVOLO_PID 0x05df
#define DIGILIVOLO_MANUFACTURER_STRING L"digilivolo@yandex.com"
#define DIGILIVOLO_PRODUCT_STRING L"DigiLivolo"

#define REPORT_ID 0x4c

#define CMD_SWITCH 0x01 // IN,OUT send Livolo keycode command or send ACK to the host
#define CMD_RDY 0x10 // OUT, device ready command

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

void print_device(struct hid_device_info* cur_dev) {
	printf("VID/PID: 0x%04hx:0x%04hx, Product: %ls, Manufacturer: %ls, FW Ver: %d.%02d.\n", \
		cur_dev->vendor_id, cur_dev->product_id, cur_dev->product_string, cur_dev->manufacturer_string, \
		cur_dev->release_number >> 8, cur_dev->release_number & 0xFF);
}

void print_devices(struct hid_device_info* cur_dev) {
	for (; cur_dev; cur_dev = cur_dev->next) {
		print_device(cur_dev);
	}
}

struct hid_device_info* find_digilivolo(struct hid_device_info* cur_dev) {
	for (; cur_dev; cur_dev = cur_dev->next) {
		if ( (wcscmp(cur_dev->manufacturer_string, DIGILIVOLO_MANUFACTURER_STRING) == 0) && \
		     (wcscmp(cur_dev->product_string, DIGILIVOLO_PRODUCT_STRING) == 0) )
				return cur_dev;
	}

    return NULL;
}

// [argp] Parse a single option.
static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
	/* Get the input argument from argp_parse, which we
	 * know is a pointer to our arguments structure. */
	struct arguments* arguments = state->input;

	switch (key) {
	case 'v':
		arguments->verbose = true;
		break;

	case ARGP_KEY_ARG:
		if (state->arg_num >= 2)
			// Too many arguments.
			argp_usage(state);

		char* endptr;
		// Convert argument to long
		long value = strtol(arg, &endptr, 0);
		// Check if it was valid long value
		if (*endptr == '\0') {
			switch (state->arg_num) {
			case 0:
				// Out of range
				if (value > 65535 || value <= 0)
					argp_usage(state);
				else
					arguments->remote_id = (uint16_t)value;
				break;

			case 1:
				// Out of range
				if (value > 255 || value <= 0)
					argp_usage(state);
				else
					arguments->key_id = (uint8_t)value;
				break;

			default:
				return ARGP_ERR_UNKNOWN;
			}
		}
		else
			// REMOTE_ID or KEY_ID not an unsigned integer
			argp_usage(state);

		break;

	case ARGP_KEY_END:
		if (state->arg_num < 2)
			// Not enough arguments.
			argp_usage(state);
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

// [argp] Our argp parser.
static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[9];
	hid_device* handle = NULL;
	int i;

	struct hid_device_info *devs, *dev;

	// [argp] Default values.
	arguments.remote_id = 0;
	arguments.key_id = 0;
	arguments.verbose = false;

	// Print program name & version
	printf("%s\n", prognamever);

	/* Parse our arguments; every option seen by parse_opt will
	 * be reflected in arguments. */
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if (arguments.verbose) {
		printf("Compiled with hidapi version %s, runtime version %s.\n", HID_API_VERSION_STR, hid_version_str());
		printf("Arguments: REMOTE_ID = %d, KEY_ID = %d\n", arguments.remote_id, arguments.key_id);
	}

	if (hid_init())
		return -1;

	#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
	// To work properly needs to be called before hid_open/hid_open_path after hid_init.
	// Best/recommended option - call it right after hid_init.
	hid_darwin_set_open_exclusive(0);
	#endif

	devs = hid_enumerate(DIGILIVOLO_VID, DIGILIVOLO_PID);
	dev = find_digilivolo(devs);
	if (!dev) {
		printf("ERROR: unable to find device\n");
    	if (arguments.verbose) {
			printf("All devices with matching VID & PID:\n");
			print_devices(devs);
		}
	}
	else {
		if (arguments.verbose) {
			printf("Device found: ");
			print_device(dev);
			printf("Opening device path: %s\n", dev->path);
		}
		handle = hid_open_path(dev->path);
	}


	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf, 0x00, sizeof(buf));

	// Open the device using VID & PID
	// handle = hid_open(DIGILIVOLO_VID, DIGILIVOLO_PID, NULL);
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
	else {
		// print_device(info);
	}

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);

	// Send a Feature Report to the device
	buf[0] = REPORT_ID;
	buf[1] = CMD_SWITCH;
	buf[2] = arguments.remote_id & 0xFF;
	buf[3] = (arguments.remote_id >> 8) & 0xFF;
	buf[4] = arguments.key_id;
	res = hid_send_feature_report(handle, buf, 8);
	if (res < 0) {
		printf("ERROR: Unable to send a feature report.\n");
		hid_close(handle);
		hid_exit();
		return 1;
	}
	else
		printf("Codes sent to device.\n");

	memset(buf, 0, sizeof(buf));

	// Read a Feature Report from the device
	buf[0] = REPORT_ID;
	res = hid_get_feature_report(handle, buf, 8);
	if (res < 0) {
		printf("WARN: Unable to get a feature report: %ls\n", hid_error(handle));
	}
	else if (arguments.verbose) {
		// Print out the returned buffer.
		printf("Feature Report: ");
		for (i = 0; i < res; i++)
			printf("%02x ", (unsigned int)buf[i]);
		printf("\n");
	}

	memset(buf, 0, sizeof(buf));

	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

	return 0;
}
