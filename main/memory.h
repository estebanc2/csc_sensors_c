/*
    ANEMOMETER_C - An app to manage force and magnetic sensors to
    get wind speed and direction

    Copyright (C) 2024  Esteban Castro  ecastro@miratucuadra.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "config.h"

void nvs_read_write(int, void *, size_t);
void nvs_read_write_init();