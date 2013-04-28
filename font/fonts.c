#include <string.h>
#include <stdio.h>
#include "font/font.h"

#ifndef G_BIOS

static struct font_desc *first = NULL;

void font_add(const struct font_desc *font)
{
	struct font_desc **p;

	printf("%s() line: %d\n", __func__, __LINE__);
	printf("font->name = %s\n", font->name);

	p = &first;
	for (; *p != NULL; p = &(*p)->next)
		;

	*p = font;

	printf("first->name = %s\n", first->name);
	printf("%s() line %d\n", __func__, __LINE__);
}

int font_init()
{
	extern const struct font_desc font_vga_8x16;
	extern const struct font_desc font_sun_12x22;

	printf("%s() line %d\n", __func__, __LINE__);

	font_add(&font_vga_8x16);
	font_add(&font_sun_12x22);

	printf("%s() line %d\n", __func__, __LINE__);

	return 0;
}

const struct font_desc *find_font(const char *name)
{
	const struct font_desc *font;

	for (font = first; font != NULL; font = font->next) {
		if (!strcmp(name, font->name))
			return font;
	}

	return NULL;
}

#else

const struct font_desc *find_font(const char *name)
{
	const struct font_desc *font;
	extern const struct font_desc font_list_begin[], font_list_end[];

	for (font = font_list_begin; font < font_list_end; font++) {
		if (!strcmp(name, font->name))
			return font;
	}

	return NULL;
}

#endif
