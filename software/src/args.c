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

#include "git_version.h"
#include "args.h"

const char* argp_program_version = GIT_VERSION;
const char doc[] = "\nSoftware to control DigiLivolo devices.\n";

const char* argp_program_bug_address = "https://github.com/N-Storm/DigiLivolo/\n\
Copyright (c) 2024 GitHub user N-Storm.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>";

char args_doc[] = "REMOTE_ID KEY_CODE";

struct argp_option options[] = {
  {"old-alg",   'o',   0,                            0, "Use deperecated original transmit algorithm" },
  {0,             0,   0,                            0, "Positional arguments:"                       },
  {"REMOTE_ID",   0,   0, OPTION_DOC | OPTION_NO_USAGE, "Livilo Remote ID (1-65535)"                  },
  {"KEY_CODE",    0,   0, OPTION_DOC | OPTION_NO_USAGE, "Livilo Key ID (1-255)"                       },
  {0,             0,   0,                            0, "Options:"                                    },
  {"verbose",   'v',   0,                            0, "Produce verbose output"                      },
  { 0 }
};

arguments_t arguments;

error_t parse_opt(int key, char* arg, struct argp_state* state)
{
	/* Get the input argument from argp_parse, which we
	 * know is a pointer to our arguments structure. */
	struct arguments* arguments = state->input;

	switch (key) {
	case 'o':
		arguments->old_alg = true;
		break;
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
					arguments->btn_id = (uint8_t)value;
				break;

			default:
				return ARGP_ERR_UNKNOWN;
			}
		}
		else
			// REMOTE_ID or KEY_CODE not an unsigned integer
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
