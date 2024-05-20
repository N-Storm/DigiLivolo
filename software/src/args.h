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

#ifndef __args_h__
#define __args_h__

#include "git_version.h"
#include <argp.h>

/// @brief String definition with program name & version number
#define PROG_NAME_VERSION "digilivolo " GIT_VERSION "\n"

/// @brief [argp] A description of the command line arguments we accept.
extern char args_doc[];

/// @brief [argp] Command line options we understand.
extern struct argp_option options[];

/// @brief [argp] Program documentation.
extern const char doc[];

/// @brief [argp] This string are printed in the footer of the help text.
extern const char* argp_program_bug_address;

/// @brief [argp] Command-line arguments.
typedef struct arguments {
    uint16_t remote_id;
    uint8_t btn_id;
    bool verbose, old_alg;
} arguments_t;

extern arguments_t arguments;


/// @brief [argp] Parse a single option.
/// @param key[in] option key
/// @param arg[in] pointer to argument value
/// @param state pointer to argp_state
/// @return error_t error code
extern error_t parse_opt(int key, char* arg, struct argp_state* state);

#endif // __args_h__
