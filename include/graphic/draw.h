#pragma once

#include <font/font.h>

struct lcd_info {
	char *lcd_base;
	int width;
	int height;
	int bpp;
	int line_length;
};

typedef unsigned int color_t;

static inline color_t get_color(int alpha, int red, int green, int blue)
{
	return (alpha & 0xff) << 24 | (red & 0xff) << 16 | (green & 0xff) << 8 | (blue & 0xff);
}

int draw_n_char(struct lcd_info *lcd, int x, int y,
	const struct font_desc * font, color_t str_color, color_t back_color, const char *str, int n);
