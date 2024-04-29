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

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "digilivolo.h"

#include "git_version.h"
#include <argp.h>

// [argp] A description of the arguments we accept.
extern char args_doc[];

// [argp] The options we understand.
extern struct argp_option options[];

// [argp] Program documentation.
// const char* argp_program_version = GIT_VERSION;
extern const char prognamever[];
extern const char doc[];
extern const char* argp_program_bug_address;

// [argp] Command-line arguments.
typedef struct arguments {
    uint16_t remote_id;
    uint8_t key_id;
    bool verbose;
} arguments_t;

extern arguments_t arguments;

extern error_t parse_opt(int key, char* arg, struct argp_state* state);

#endif // __args_h__
