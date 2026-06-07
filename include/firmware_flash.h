/*
    firmware_flash.h
    Part of lumia-dloadtool
    Copyright (C) 2026 Emma / InvoxiPlayGames

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, see <https://www.gnu.org/licenses/>.
*/

#ifndef FIRMWARE_FLASH_H
#define FIRMWARE_FLASH_H

#include <stdio.h>
#include "rm_cert.h"

int flash_firmware(rm_cert_t *cert, FILE *fp, long offset);

#endif // FIRMWARE_VERIFY_H
