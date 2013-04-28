#pragma once

#ifdef G_BIOS
#include <types.h>
#define __GBIOS_FONT__ __USED__ __attribute__((section(".gbios_font")))
#endif

struct font_desc {
	const char *name;
	int width, height;
	short word_gap;
	short line_gap;
	const void *data;
#ifndef G_BIOS
	struct font_desc *next;
#endif
};

const struct font_desc *find_font(const char *name);

#ifndef G_BIOS
int font_init();
void font_add(const struct font_desc *);
#endif

