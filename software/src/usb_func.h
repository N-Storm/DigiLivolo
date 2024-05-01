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

#ifndef __error_t_defined
typedef int error_t;
#define __error_t_defined
#endif

extern const char* hid_bus_name(hid_bus_type bus_type);

/// @brief Prints brief info on one device supplied by the cur_dev.
/// @param cur_dev pointer to device which info we want to print.
extern void print_device(struct hid_device_info* cur_dev);

/// @brief Similar to print_device() but prints more details on multiple lines.
/// @param cur_dev pointer to device which info we want to print.
/// @see print_device
extern void print_device_details(struct hid_device_info* cur_dev);

/// @brief Prints all devices in the supplied linked list.
/// @param cur_dev device list to print.
extern void print_devices(struct hid_device_info* cur_dev);

/// @brief Iterates through enumerated device linked list until it finds
///        device with a matching manufacturer & product string. Returns
///        pointer to this device struct on first match.
/// @param cur_dev[in] pointer to a linked device list
/// @return Pointer to matched device or NULL if no matches were found.
extern struct hid_device_info* find_digilivolo(struct hid_device_info* cur_dev);

/// @brief Sends Livolo remote key press event
/// @param remote_id[in] Livolo Remote ID to send
/// @param btn_id[in] Livolo Keycode to send
/// @param handle[in] pointer to DigiLivolo device
/// @return Passes return code from hid_send_feature_report()
/// @see hid_send_feature_report
extern error_t dlusb_send(uint16_t remote_id, uint8_t btn_id, hid_device* handle);

/// @brief Read a Feature Report from the device
/// @param packet[out] pointer to a dlusb_packet_t
/// @param handle[in] pointer to DigiLivolo device
/// @return Passes return code from hid_get_feature_report()
/// @see hid_get_feature_report
extern error_t dlusb_read(dlusb_packet_t* packet, hid_device* handle);

#endif // __usb_func_h__
