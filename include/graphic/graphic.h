#ifndef __GRAPHIC_H__
#define __GRAPHIC_H__

#include <graphic/draw.h>

struct position {
	int x;
	int y;
};

struct rectangle {
	int w;
	int h;
	int gap;
	color_t out_color;
	color_t in_color;
};

struct cell {
	struct position pos;
	struct rectangle *rect;
	void *param;
};

struct move_operations {
	int (*left_move)(void *base, void *dest, int step);
	int (*right_move)(void *base, void *dest, int step);
	int (*up_move)(void *base, void *dest, int step);
	int (*down_move)(void *base, void *dest, int step);
};

struct component {
	char *name;
	struct component *next;
	struct component *parent;

	int (*release)(struct component *comp);
	int (*set_name)(struct component *comp, const char *name);
	int (*draw_component)(struct component *parent, struct component *dest, struct lcd_info *lcd);
};

struct label {
	int width;
	int height;
	char text[21];
	char *minor_text;
	int text_len;
	struct position pix_pos;
	struct component comp_node;
	const struct font_desc *font;
	color_t back_color;
	color_t text_color;

	void (*set_text)(struct label *label, const char *text, int text_len);
	char * (*get_text)(struct label *label);
	void (*set_font)(struct label *label, struct font_desc *font);
};

#define MAJOR_COLUMNS 14
#define MAJOR_LINES 24
#define MINOR_COLUMNS 5
#define MINOR_LINES 5

struct panel {
	int columns;
	int lines;
	struct rectangle *rects;
	int rect_num;
	char *val;   			//每一个单元格的所填充的矩形
	int back_rect;
	struct position pix_pos;
	struct component comp_node;

	void (*set_rectangles)(struct panel *panel, struct rectangle *rect, int rect_num);
	int (*set_back_rect)(struct panel *panel, int back_rect);
	int (*set_val)(struct panel *panel, int x, int y, int val);
};

struct frame {
	int width;
	int height;
	color_t back_color;

	struct position pix_pos;
	struct component comp_head;

	int (*add_component)(struct frame *frame, struct component *comp);
	int (*del_component)(struct frame *frame, const char *name);
	int (*set_position)(struct frame *frame, int x, int y, struct lcd_info *lcd);
	int (*set_size)(struct frame *frame, int width, int height, struct lcd_info *lcd);
	struct component (*get_component)(struct frame *frame, const char *name);
};


void draw_cell(struct position *start, struct cell *cell, struct lcd_info *lcd);

int init_label(struct label *label, int x, int y, int width, int height,
	color_t back_color, color_t text_color, const struct font_desc *font, struct lcd_info *lcd);

int init_panel(struct panel *panel, int x, int y, int columns, int lines,
	struct rectangle *rects, int rect_num, int back_rect, struct lcd_info *lcd);

int init_frame(struct frame *frame, int x, int y, int width, int height,
	color_t back_color, struct lcd_info *lcd);

#endif
