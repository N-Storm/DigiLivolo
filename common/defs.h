/* Part of the DigiLivolo project.
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

#ifndef __defs_h__
#define __defs_h__

#define DIGILIVOLO_VID 0x16c0
#define DIGILIVOLO_PID 0x05df
#define DIGILIVOLO_MANUFACTURER_STRING L"digilivolo@yandex.com"
#define DIGILIVOLO_PRODUCT_STRING L"DigiLivolo"

#define REPORT_ID 0x4c

#define CMD_SWITCH 0x01 // IN,OUT send Livolo keycode command or send ACK to the host
#define CMD_SWITCH_OLD 0x02 // IN,OUT send Livolo keycode command or send ACK to the host, but use original Livolo lib method
#define CMD_ERR_UNKNOWN 0xFF // OUT ERROR unknown CMD code
#define CMD_RDY 0x10 // OUT, device ready command
#define CMD_FAIL_BIT (uint8_t)(1 << 7) // Not used

typedef struct dlusb_packet {
  uint8_t report_id;
  uint8_t cmd_id;
  uint16_t remote_id;
  uint8_t btn_id;
} dlusb_packet_t;

#endif // __defs_h__
