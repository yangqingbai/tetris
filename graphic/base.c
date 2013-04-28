#include <string.h>
#include <graphic/draw.h>
#include <graphic/graphic.h>

static void fillrect(struct lcd_info *lcd, int y, int x, struct rectangle *rect)
{
	int i, j;
	char *base = lcd->lcd_base;

	base += y * lcd->line_length;

	for (i = 0; i < rect->h && y + i < lcd->height; i++) {
		base += x * lcd->bpp;

		for (j = 0; j < rect->w && x + j < lcd->width; j++) {
			if (i < rect->gap || i >= rect->h - rect->gap 
				|| j < rect->gap || j >= rect->w - rect->gap)
				memcpy(base, &rect->out_color, sizeof(rect->out_color));
			else
				memcpy(base, &rect->in_color, sizeof(rect->in_color));

			base += sizeof(rect->out_color);
		}

		base += (lcd->width - x - rect->w) * lcd->bpp;
	}
}

void draw_cell(struct position *start, struct cell *cell, struct lcd_info *lcd)
{
	int x, y;

	x = start->x + cell->pos.x * cell->rect->w;
	y = start->y + cell->pos.y * cell->rect->h;

	fillrect(lcd, y, x, cell->rect);
}

