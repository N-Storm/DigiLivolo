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

#ifndef __usb_func_h__
#define __usb_func_h__

#include <wchar.h>
#include <hidapi.h>

extern const char* hid_bus_name(hid_bus_type bus_type);
extern void print_device_details(struct hid_device_info* cur_dev);
extern void print_device(struct hid_device_info* cur_dev);
extern void print_devices(struct hid_device_info* cur_dev);
extern struct hid_device_info* find_digilivolo(struct hid_device_info* cur_dev);

#endif // __usb_func_h__
