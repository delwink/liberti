/*
 *  LiberTI - TI-like calculator designed for LibreCalc
 *  Copyright (C) 2016-2017 Delwink, LLC
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, version 3 only.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <SDL_image.h>
#include <stdbool.h>
#include <string.h>

#include "log.h"
#include "tiberr.h"

extern unsigned char FONT_PNG_DATA[];
#define FONT_PNG_SIZE 861

#define NUM_TILES 256

static bool tiles_init = false;
static SDL_Surface *tiles[NUM_TILES];

int
font_init()
{
	SDL_Surface *tileset;

	if (tiles_init)
		return 0;

	memset(tiles, 0, sizeof tiles);

	tileset = IMG_Load_RW(SDL_RWFromMem(FONT_PNG_DATA, FONT_PNG_SIZE), 1);
	if (!tileset)
	{
		critical("Failed to load tileset: %s", SDL_GetError());
		return TIB_EALLOC;
	}

	for (unsigned int i = 0; i < NUM_TILES; ++i)
	{
		tiles[i] = SDL_CreateRGBSurface(0, 6, 8, 32, 0, 0, 0, 0);
		if (!tiles[i])
		{
			critical("Failed to alloc tile %u: %s", i,
				SDL_GetError());

			for (unsigned int j = 0; j < i; ++j)
				SDL_FreeSurface(tiles[j]);

			return TIB_EALLOC;
		}

		SDL_Rect src = { i * 6, 0, 6, 8 };
		if (SDL_BlitSurface(tileset, &src, tiles[i], NULL))
			error("Failed to blit tile %u: %s", i, SDL_GetError());
	}

	SDL_FreeSurface(tileset);
	tiles_init = true;
	return 0;
}

void
font_free()
{
	if (tiles_init)
		for (unsigned int i = 0; i < NUM_TILES; ++i)
			SDL_FreeSurface(tiles[i]);

	tiles_init = false;
}

SDL_Surface *
get_font_char(int c)
{
	if (!tiles_init || c < 0)
		return NULL;

	return tiles[c];
}

// generated from font.png

unsigned char FONT_PNG_DATA[] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
	0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x08,
	0x08, 0x03, 0x00, 0x00, 0x00, 0x08, 0xb3, 0xc4, 0x0e, 0x00, 0x00, 0x00,
	0x06, 0x50, 0x4c, 0x54, 0x45, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xa5,
	0xd9, 0x9f, 0xdd, 0x00, 0x00, 0x03, 0x12, 0x49, 0x44, 0x41, 0x54, 0x68,
	0xde, 0xed, 0x5a, 0x81, 0x76, 0x9c, 0x30, 0x0c, 0x93, 0xff, 0xff, 0xa7,
	0xf7, 0xda, 0x83, 0x20, 0xc9, 0x0e, 0x81, 0x5b, 0xdf, 0x8d, 0x6d, 0xf6,
	0xbb, 0xa6, 0x94, 0xa3, 0xc1, 0x71, 0x82, 0x64, 0x2b, 0x20, 0xbe, 0x0d,
	0x5f, 0x56, 0xb4, 0xbb, 0x21, 0xb5, 0x4f, 0x33, 0xbc, 0x1a, 0x8c, 0xf6,
	0x75, 0x28, 0x2e, 0xd3, 0xaf, 0xeb, 0x83, 0xd8, 0xc2, 0x80, 0x71, 0x3c,
	0x22, 0x43, 0xc7, 0x74, 0x7e, 0x3f, 0x81, 0xda, 0x45, 0x0e, 0x24, 0x5d,
	0x39, 0x7a, 0xd6, 0x63, 0x99, 0x8b, 0x57, 0x33, 0x6e, 0xbc, 0x77, 0x41,
	0x43, 0xa6, 0x63, 0x4c, 0xfb, 0x0c, 0x9e, 0x65, 0x0b, 0x9a, 0xb5, 0x34,
	0xba, 0x11, 0x8d, 0xb4, 0x06, 0x50, 0x1c, 0xa5, 0x6f, 0xe0, 0xe7, 0xd2,
	0x5c, 0x00, 0xef, 0xcc, 0xf8, 0xd5, 0x39, 0xd4, 0xf5, 0x90, 0xbe, 0xe3,
	0x5e, 0xd5, 0x55, 0x70, 0x2b, 0x0b, 0x02, 0xd5, 0x14, 0xfe, 0xdc, 0x7a,
	0x7e, 0xe6, 0x63, 0xd6, 0xf6, 0x8f, 0x19, 0x94, 0x00, 0xfc, 0x23, 0x20,
	0x88, 0x27, 0x33, 0x40, 0x22, 0x80, 0x7d, 0x44, 0xdf, 0x7f, 0x62, 0x42,
	0x63, 0xab, 0x01, 0x11, 0x98, 0x32, 0x5a, 0x0d, 0x04, 0x80, 0x61, 0x11,
	0x1c, 0x82, 0x4f, 0x40, 0x73, 0x01, 0xbe, 0x29, 0xe0, 0xd2, 0x33, 0xdd,
	0x79, 0x7c, 0x5e, 0xb7, 0xc2, 0xb5, 0x3e, 0xaf, 0xf8, 0xa0, 0x57, 0xa5,
	0x38, 0x5f, 0x27, 0x00, 0x0c, 0xb6, 0xac, 0x62, 0x8d, 0x78, 0x6b, 0x59,
	0xe1, 0xe6, 0x95, 0x60, 0xff, 0x43, 0x5d, 0x53, 0xa6, 0x50, 0x46, 0xaa,
	0xa2, 0x1f, 0x16, 0x45, 0x78, 0x50, 0x7e, 0x7f, 0x25, 0x47, 0x33, 0x40,
	0xdb, 0xa7, 0x09, 0x20, 0x76, 0xb8, 0x44, 0xf0, 0xc3, 0xba, 0x41, 0x28,
	0x04, 0xcf, 0x1e, 0x4b, 0x00, 0xfb, 0x58, 0x18, 0x22, 0x8b, 0x87, 0x9f,
	0xa8, 0x01, 0x27, 0x9d, 0x32, 0xf8, 0x87, 0x92, 0xc0, 0x91, 0xa7, 0x1b,
	0x9a, 0x94, 0x04, 0x80, 0x23, 0xce, 0x0c, 0x7a, 0x20, 0x2f, 0xea, 0x5c,
	0x3e, 0x0a, 0x1a, 0x88, 0x34, 0x3a, 0x70, 0x7f, 0x78, 0x9f, 0x00, 0xb6,
	0x71, 0x29, 0xd5, 0x15, 0x29, 0xbb, 0x31, 0xc0, 0x16, 0x07, 0x49, 0x88,
	0x8f, 0x35, 0x03, 0x1e, 0x51, 0x02, 0x59, 0xba, 0xdf, 0xf6, 0x4f, 0x51,
	0x54, 0x2d, 0xfc, 0xbf, 0x74, 0xc7, 0x4b, 0x15, 0x4c, 0x01, 0xe4, 0xb0,
	0x29, 0x03, 0x67, 0x3c, 0xbb, 0x1b, 0xf6, 0xd9, 0xd6, 0x15, 0x1d, 0xeb,
	0x79, 0xbb, 0xe3, 0xbb, 0xc5, 0x03, 0xee, 0x16, 0xa9, 0x6d, 0x6d, 0x3f,
	0x42, 0x00, 0xfc, 0xf0, 0x0b, 0xb0, 0x15, 0x02, 0xcb, 0x43, 0x07, 0x12,
	0x5c, 0x01, 0x54, 0x19, 0x9f, 0xe2, 0x9f, 0x48, 0x3a, 0x11, 0x85, 0xf0,
	0x65, 0x24, 0x01, 0xcd, 0xbb, 0x63, 0x00, 0x8d, 0xca, 0x44, 0x01, 0x24,
	0x69, 0x61, 0x96, 0xe7, 0xa9, 0xec, 0x66, 0x40, 0xc5, 0x72, 0x50, 0x86,
	0x18, 0x66, 0x19, 0x23, 0x14, 0x68, 0x0f, 0xec, 0x27, 0x79, 0xab, 0xa0,
	0x09, 0xa3, 0xb7, 0xdb, 0xd5, 0x92, 0x92, 0x2d, 0xc8, 0x87, 0xf0, 0x75,
	0x05, 0x4f, 0xa0, 0xd9, 0x8b, 0x8a, 0xc0, 0xa0, 0xfd, 0xdf, 0x20, 0x30,
	0x14, 0x85, 0x86, 0xab, 0x53, 0xf0, 0x0a, 0x00, 0x46, 0xae, 0x28, 0x62,
	0x55, 0x44, 0x0e, 0xa2, 0x34, 0xdd, 0x2c, 0x1e, 0x66, 0x02, 0x6c, 0x5b,
	0xdb, 0x27, 0x25, 0x20, 0x59, 0xde, 0xb9, 0xf0, 0xc7, 0xc3, 0x09, 0xe0,
	0x80, 0x48, 0x90, 0xe3, 0x95, 0x08, 0xad, 0x90, 0x87, 0xba, 0xa8, 0x10,
	0xc0, 0xa2, 0x9a, 0x68, 0x3c, 0xa2, 0xf4, 0xf0, 0x87, 0x5d, 0xbb, 0xa8,
	0x00, 0x5c, 0xd4, 0x59, 0x57, 0x00, 0x58, 0x56, 0x00, 0x04, 0x97, 0xd2,
	0x03, 0x3c, 0xef, 0xe5, 0x1f, 0x50, 0x34, 0x50, 0x67, 0xfd, 0x15, 0x97,
	0x8a, 0x64, 0x5d, 0xef, 0x46, 0x08, 0xa7, 0x1d, 0xd7, 0x28, 0x01, 0x78,
	0x6e, 0xee, 0x59, 0x7c, 0xa8, 0x7c, 0x74, 0x89, 0xc0, 0xea, 0xdc, 0xff,
	0xcc, 0x7f, 0xce, 0x1e, 0xea, 0xac, 0x3f, 0xc2, 0x2b, 0x80, 0x83, 0xe0,
	0xf6, 0xea, 0x64, 0x42, 0x9c, 0xbc, 0x99, 0x52, 0xa6, 0x0b, 0x5d, 0x01,
	0xb4, 0x3d, 0x8b, 0x00, 0xfe, 0x5e, 0x15, 0x52, 0xc5, 0x19, 0xc0, 0x75,
	0xde, 0x58, 0xe6, 0xb6, 0x98, 0x68, 0xe0, 0x30, 0x11, 0xc8, 0x73, 0x4f,
	0xbf, 0x16, 0x33, 0xe8, 0xa9, 0xb6, 0xd7, 0xef, 0xef, 0x01, 0x20, 0xef,
	0x01, 0xcc, 0x7b, 0xb0, 0xaa, 0x65, 0x46, 0x00, 0x88, 0x89, 0xee, 0x1f,
	0x67, 0xb0, 0xb4, 0x90, 0x9b, 0xd2, 0x1e, 0x09, 0xfb, 0x7f, 0x80, 0xe6,
	0x99, 0x60, 0x15, 0x29, 0x21, 0x19, 0xc2, 0x64, 0x49, 0x60, 0x3e, 0xda,
	0x15, 0x01, 0xd4, 0x27, 0xee, 0xb6, 0x15, 0x7d, 0x9e, 0x4c, 0x62, 0xb9,
	0xaf, 0x86, 0xde, 0x02, 0x68, 0xfb, 0xa3, 0x04, 0x30, 0x79, 0x23, 0xe8,
	0xd9, 0xc5, 0x69, 0x92, 0x80, 0xb8, 0xaa, 0x07, 0x8a, 0x27, 0xec, 0x04,
	0x02, 0x00, 0xcf, 0x0d, 0x91, 0xde, 0xf3, 0x21, 0x11, 0x29, 0x24, 0xdd,
	0xf4, 0x17, 0x67, 0x12, 0x8e, 0x5a, 0x4a, 0x0d, 0xa0, 0x90, 0x68, 0xe4,
	0x2d, 0xa0, 0x94, 0x65, 0xf3, 0x5d, 0x38, 0xd7, 0x2e, 0xde, 0x02, 0x22,
	0x55, 0x1b, 0x33, 0xc1, 0x8a, 0x77, 0x35, 0x18, 0xa0, 0xa7, 0x6f, 0x01,
	0x25, 0xb9, 0x8c, 0x24, 0x2c, 0x11, 0xc1, 0x52, 0x64, 0xb2, 0xff, 0xcc,
	0x9d, 0x53, 0xff, 0x19, 0x2b, 0x35, 0xfe, 0xbe, 0x2b, 0x13, 0xfa, 0x06,
	0x14, 0x84, 0xd0, 0xa7, 0x4b, 0xb7, 0x5a, 0xe7, 0xa5, 0x50, 0xbf, 0x68,
	0x39, 0x79, 0xd0, 0xe0, 0x79, 0x05, 0x30, 0x5b, 0x2d, 0x79, 0x05, 0xb6,
	0xb5, 0x7d, 0x02, 0x37, 0xdb, 0xda, 0xda, 0xda, 0xda, 0xfe, 0x27, 0xfb,
	0x05, 0x36, 0xf6, 0x29, 0xcf, 0xfc, 0x99, 0x25, 0xd1, 0x00, 0x00, 0x00,
	0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
