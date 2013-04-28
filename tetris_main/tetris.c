#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#include <font/font.h>

#ifdef G_BIOS
#include <uart/uart.h>
#include <random.h>
#else
#include <time.h>
#include <sys/times.h>
#include <errno.h>
#include <graphic/draw.h>
#include <graphic/graphic.h>
#include <tty/tty_control.h>

#include "diamond.h"
#include "tetris.h"
#endif

#ifndef __DATA_FILE__
#define SCORE_FILE "score.txt"
#else
#define SCORE_FILE __DATA_FILE__
#endif

#if 0
typedef unsigned int color_t;

static inline color_t get_color(int alpha, int red, int green, int blue)
{
	return (alpha & 0xff) << 24 | (red & 0xff) << 16 | (green & 0xff) << 8 | (blue & 0xff);
}
#endif

#if 0
#define CELL_NUM 4
#define DIAM_NUM 7

#define HIT 1
#endif
#define container_of(ptr, type, member) \
	(type *)((char *)ptr - (long)(&((type *)0)->member))

#if 0
struct position {
	int x;
	int y;
};

struct lcd_info {
	char *lcd_base;
	int width;
	int height;
	int bpp;
	int line_length;
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

struct frame;

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

static int draw_char(struct lcd_info *lcd, int x, int y,
	const struct font_desc * font, color_t str_color, color_t back_color, char c)
{
	int i, j;
	unsigned char val = 0;   //字体点阵中每个字节的值
	int column;   //  所画字体的行的列
	int line;   //所画字体的行
	int width = (font->width + 7) / 8 * 8;   //字体每行所占像素点实际的个数
	int addr = c * (width / 8) * font->height;  //字符对应字体坐在位置
	char *base = (char *)lcd->lcd_base + y * lcd->line_length;
	int tmp_height = font->height + font->line_gap * 2;
	int tmp_width = font->width + font->word_gap * 2;

	for (i = 0; i < tmp_height; i++) {
		line = i - font->line_gap;
		for (j = 0; j < tmp_width; j++) {
			column = j - font->word_gap;
			if (line < 0 || line >= font->height || column < 0 || column >= font->width)
				memcpy(base + (x + j) * lcd->bpp, &back_color, sizeof(back_color));
			else {
				val = ((unsigned char *)font->data)[addr	+ line * (width / 8) + column / 8];

				if (val & (1 << (7 - column % 8))) {
					memcpy(base + (x + j) * lcd->bpp, &str_color, sizeof(str_color));
				} else
					memcpy(base + (x + j) * lcd->bpp, &back_color, sizeof(back_color));
			}
		}

		base += lcd->line_length;
	}

	return 0;
}

int draw_n_char(struct lcd_info *lcd, int x, int y,
	const struct font_desc * font, color_t str_color, color_t back_color, const char *str, int n)
{
	int i;

	for (i = 0; i < n; i++, str++) {
		draw_char(lcd, x, y, font, str_color, back_color, *str);

		x += font->width + font->word_gap * 2;
	}

	return 0;
}
#endif

#if 0
static inline void draw_boder(void *base, struct lcd_info *lcd, int min_x, int max_x, 
	int min_y, int max_y, color_t color)
{
	int x, y;
	for (y = min_y; y < lcd->height && y < max_y; y++) {
		for (x = min_x; x < lcd->width && x < max_x; x++)
			memcpy(base + x * lcd->bpp, &color, sizeof(color));
		base += lcd->line_length;
	}
}

int draw_label(struct component *parent, struct component *dest, struct lcd_info *lcd);

static int label_release(struct component *comp)
{
	struct label *label = container_of(comp, struct label, comp_node);

	if (label->minor_text)
		free(label->minor_text);

	return 0;
}

static void label_set_font(struct label *label, struct font_desc *font)
{
	if (!font)
		return;
	label->font = font;
}

static void label_set_text(struct label *label, const char *text, int text_len)
{
	int max_len = sizeof(label->text) - 1;

	if (text_len <= max_len) {
		memset(label->text, 0, sizeof(label->text));
		strncpy(label->text, text, text_len);
		label->minor_text = NULL;
	} else {
		label->minor_text = malloc(text_len + 1);
		strncpy(label->minor_text, text, text_len);
	}

	label->text[text_len] = '\0';
	label->text_len = text_len;
}

static char *label_get_text(struct label *label)
{
	return label->text;
}

int init_label(struct label *label, int x, int y, int width, int height,
	color_t back_color, color_t text_color, const struct font_desc *font, struct lcd_info *lcd)
{
	if (x < 0 || y < 0 || width < 0 || height < 0
		|| x + width >= lcd->width || y + height >= lcd->height)
		return -1;

	label->pix_pos.x = x;
	label->pix_pos.y = y;
	label->width = width;
	label->height = height;
	label->back_color = back_color;
	label->text_color = text_color;
	label->font = font;

	if (!font) {
		printf("---------font == NULL\n");
		label->font = find_font("VGA8x16");
	}

	memset(label->text, 0, sizeof(label->text));

	label->minor_text = NULL;
	label->text_len = 0;
	label->comp_node.name = NULL;
	label->comp_node.next = NULL;
	label->comp_node.parent = NULL;
	label->comp_node.release = label_release;
	label->comp_node.draw_component = draw_label;
	label->get_text = label_get_text;
	label->set_font = label_set_font;
	label->set_text = label_set_text;

	return 0;
}
#endif

#if 0
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

int panel_set_val(struct panel *panel, int x, int y, int val)
{
	printf("%s() line %d: ---------panel->val[%d][%d] = %d\n", __FUNCTION__, __LINE__, y, x, val);
	printf("%s() line %d: width = %d, height = %d, rect_num = %d\n", 
		__FUNCTION__, __LINE__, panel->columns, panel->lines, panel->rect_num);

	if (x < 0 || x >= panel->columns ||y < 0 || y >= panel->lines || val >= panel->rect_num)
		return -1;

	panel->val[x + y * panel->columns] = val;
//	printf("----------address xiang cha: %d\n", x + y * panel->columns);

	return 0;
}

int panel_set_back_rect(struct panel *panel, int back_rect)
{
	if (back_rect >= panel->rect_num)
		return -1;

	panel->back_rect = back_rect;
	return 0;
}

void panel_set_rectangles(struct panel *panel, struct rectangle *rects, int rect_num)
{
	panel->rects = rects;
	panel->rect_num = rect_num;
}

int panel_release(struct component *comp)
{
//	struct panel *panel = container_of(comp, struct panel, comp_node);

//	free(panel->val);
	return 0;
}

#define MAJOR_COLUMNS 14
#define MAJOR_LINES 24
#define MINOR_COLUMNS 5
#define MINOR_LINES 5

char val1[MAJOR_COLUMNS * MAJOR_LINES];
char val2[MINOR_COLUMNS * MINOR_LINES];

int draw_panel(struct component *, struct component *, struct lcd_info *);

int init_panel(struct panel *panel, int x, int y, int columns, int lines,
	struct rectangle *rects, int rect_num, int back_rect, struct lcd_info *lcd)
{
	int i, j;
	int w = rects[0].w + rects[0].gap * 2;
	int h = rects[0].h + rects[0].gap * 2;

	printf("%s() line %d: before init panel->val, x = %d, y = %d, lines = %d, columns = %d, size = %d\n",
		__FUNCTION__, __LINE__, x, y, lines, columns, sizeof(panel->val[0]) * columns * lines);

	if (x < 0 || y < 0 || columns < 0 || lines < 0  || x + columns * w > lcd->width
		|| y + lines * h > lcd->height)
		return -1;

	if (back_rect >= rect_num ||back_rect < 0)
		return -1;

	if (columns > MINOR_COLUMNS)
		panel->val = val1;
	else
		panel->val = val2;
/*
	panel->val = malloc(sizeof(panel->val[0]) * width * height);
	if (!panel->val) {
		perror("malloc panel->val");
		return -1;
	}
	printf("%s() line %d: after malloc panel->val\n", __FUNCTION__, __LINE__);
*/

	panel->rects = rects;
	panel->rect_num = rect_num;
	panel->back_rect = back_rect;

	for (i = 0; i < lines; i++)
		for (j = 0; j < columns; j++)
			panel->val[i * columns + j] = back_rect;

	panel->columns = columns;
	panel->lines = lines;
	panel->pix_pos.x = x;
	panel->pix_pos.y = y;
	panel->comp_node.next = NULL;
	panel->comp_node.parent= NULL;
	panel->comp_node.release = panel_release;
	panel->comp_node.draw_component = draw_panel;
	panel->set_rectangles = panel_set_rectangles;
	panel->set_back_rect = panel_set_back_rect;
	panel->set_val = panel_set_val;

	return 0;
}
#endif

#if 0
struct frame {
	int width;
	int height;static int draw_frame(struct component *parent, struct component *dest, struct lcd_info *lcd)
{
	int i, j;
	char *base = lcd->lcd_base;
	struct frame *frame = container_of(dest, struct frame, comp_head);

	base += frame->pix_pos.y * lcd->line_length;
	for (i = 0; i < frame->height; i++) {
		for (j = frame->pix_pos.x; j < frame->width + frame->pix_pos.x; j++)
			memcpy(base + j * lcd->bpp, &frame->back_color, sizeof(frame->back_color));

		base += lcd->line_length;
	}
	
	return 0;
}

static int frame_add_component(struct frame *frame, struct component *comp)
{
	struct component *p;

	for (p = &frame->comp_head; p->next; p = p->next)
		;

	p->next = comp;
	comp->next = NULL;
	comp->parent = &frame->comp_head;

	return 0;
}

static int frame_release(struct component *comp)
{
	struct frame *frame = container_of(comp, struct frame, comp_head);
	struct component *p, *next;

	next = frame->comp_head.next;
	while (next) {
		p = next;
		next = p->next;

		if (p->release)
			p->release(p);
	}

	frame->comp_head.next = NULL;

	return 0;
}

static int frame_set_position(struct frame *frame, int x, int y, struct lcd_info *lcd)
{
	if (x < 0 || y < 0 || x >= lcd->width || y >= lcd->height)
		return -1;

	frame->pix_pos.x = x;
	frame->pix_pos.y = y;

	return 0;
}

static int frame_set_size(struct frame *frame, int width, int height, struct lcd_info *lcd)
{
	if (width <= 0 || height <= 0 || frame->pix_pos.x + width > lcd->width
		|| frame->pix_pos.y + height > lcd->height)
		return -1;

	frame->width = width;
	frame->height = height;

	return 0;
}


int init_frame(struct frame *frame, int x, int y, int width, int height, color_t back_color, struct lcd_info *lcd)
{
	int ret;
	printf("%s() line %d\n", __FUNCTION__, __LINE__);
	if (x < 0 || y < 0 || width < 0 || height < 0
		|| x + width > lcd->width || y + height > lcd->height)
		return -1;
	printf("%s() line %d\n", __FUNCTION__, __LINE__);

	frame->set_position = frame_set_position;
	frame->set_size = frame_set_size;
	frame->add_component = frame_add_component;
	frame->comp_head.release = frame_release;
	frame->comp_head.draw_component = draw_frame;
	frame->comp_head.next = NULL;
	frame->comp_head.parent = NULL;
	frame->back_color = back_color;

	ret = frame->set_position(frame, x, y, lcd);
	if (ret < 0) {
		perror("set frame position");
		return -1;
	}

	printf("%s() line %d\n", __FUNCTION__, __LINE__);

	ret = frame->set_size(frame, width, height, lcd);
	if (ret < 0) {
		perror("set frame size");
		return -1;
	}

	return 0;
}
	color_t back_color;

	struct position pix_pos;
	struct component comp_head;

	int (*add_component)(struct frame *frame, struct component *comp);
	int (*del_component)(struct frame *frame, const char *name);
	int (*set_position)(struct frame *frame, int x, int y, struct lcd_info *lcd);
	int (*set_size)(struct frame *frame, int width, int height, struct lcd_info *lcd);
	struct component (*get_component)(struct frame *frame, const char *name);
};

static int draw_frame(struct component *parent, struct component *dest, struct lcd_info *lcd)
{
	int i, j;
	char *base = lcd->lcd_base;
	struct frame *frame = container_of(dest, struct frame, comp_head);

	base += frame->pix_pos.y * lcd->line_length;
	for (i = 0; i < frame->height; i++) {
		for (j = frame->pix_pos.x; j < frame->width + frame->pix_pos.x; j++)
			memcpy(base + j * lcd->bpp, &frame->back_color, sizeof(frame->back_color));

		base += lcd->line_length;
	}
	
	return 0;
}

static int frame_add_component(struct frame *frame, struct component *comp)
{
	struct component *p;

	for (p = &frame->comp_head; p->next; p = p->next)
		;

	p->next = comp;
	comp->next = NULL;
	comp->parent = &frame->comp_head;

	return 0;
}

static int frame_release(struct component *comp)
{
	struct frame *frame = container_of(comp, struct frame, comp_head);
	struct component *p, *next;

	next = frame->comp_head.next;
	while (next) {
		p = next;
		next = p->next;

		if (p->release)
			p->release(p);
	}

	frame->comp_head.next = NULL;

	return 0;
}

static int frame_set_position(struct frame *frame, int x, int y, struct lcd_info *lcd)
{
	if (x < 0 || y < 0 || x >= lcd->width || y >= lcd->height)
		return -1;

	frame->pix_pos.x = x;
	frame->pix_pos.y = y;

	return 0;
}

static int frame_set_size(struct frame *frame, int width, int height, struct lcd_info *lcd)
{
	if (width <= 0 || height <= 0 || frame->pix_pos.x + width > lcd->width
		|| frame->pix_pos.y + height > lcd->height)
		return -1;

	frame->width = width;
	frame->height = height;

	return 0;
}


int init_frame(struct frame *frame, int x, int y, int width, int height, color_t back_color, struct lcd_info *lcd)
{
	int ret;
	printf("%s() line %d\n", __FUNCTION__, __LINE__);
	if (x < 0 || y < 0 || width < 0 || height < 0
		|| x + width > lcd->width || y + height > lcd->height)
		return -1;
	printf("%s() line %d\n", __FUNCTION__, __LINE__);

	frame->set_position = frame_set_position;
	frame->set_size = frame_set_size;
	frame->add_component = frame_add_component;
	frame->comp_head.release = frame_release;
	frame->comp_head.draw_component = draw_frame;
	frame->comp_head.next = NULL;
	frame->comp_head.parent = NULL;
	frame->back_color = back_color;

	ret = frame->set_position(frame, x, y, lcd);
	if (ret < 0) {
		perror("set frame position");
		return -1;
	}

	printf("%s() line %d\n", __FUNCTION__, __LINE__);

	ret = frame->set_size(frame, width, height, lcd);
	if (ret < 0) {
		perror("set frame size");
		return -1;
	}

	return 0;
}
#endif

#if 0
struct form {
	struct cell cell[CELL_NUM];
};

struct diamond {
	struct position pos;
	struct form *form;
	int form_num;
	int index_form;
	struct move_operations *d_ops;

	int (*change_form)(struct panel *panel, struct diamond *diamond);
	void (*draw_diamond)(struct panel *panel, struct diamond *diamond,
		struct lcd_info *lcd, int rect_n);
};
#endif

#if 0
void draw_diamond(struct panel *panel, struct diamond *diamond,
	struct lcd_info *lcd, int rect_n);


struct form chang_tiao[] = {
	[0] = {
		.cell[0] = {.pos = {.x = -1, .y = 0}, },
		.cell[1] = {.pos = {.x = 0, .y = 0}, },
		.cell[2] = {.pos = {.x = 1, .y = 0}, },
		.cell[3] = {.pos = {.x = 2, .y = 0}, },
	},
	[1] = {
		.cell[0] = {.pos = {0, -1}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {0, 1}, },
		.cell[3] = {.pos = {0, 2}, },
	},
};

struct form tian_zi[] = {
	[0] = {
		.cell[0] = {.pos = {0, -1}, },
		.cell[1] = {.pos = {1, -1}, },
		.cell[2] = {.pos = {0, 0}, },
		.cell[3] = {.pos = {1, 0}, },
	},
};

struct form tu_zi[] = {
	[0] = {
		.cell[0] = {.pos = {0, -1}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {-1, 0}, },
		.cell[3] = {.pos = {1, 0}, },
	},
	[1] = {
		.cell[0] = {.pos = {0, -1}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {0, 1}, },
		.cell[3] = {.pos = {1, 0}, },
	},
	[2] = {
		.cell[0] = {.pos = {-1, 0}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {1, 0}, },
		.cell[3] = {.pos = {0, 1}, },
	},
	[3] = {
		.cell[0] = {.pos = {0, -1}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {0, 1}, },
		.cell[3] = {.pos = {-1, 0}, },
	},
};

struct form l_zi[] = {
	[0] = {
		.cell[0] = {.pos = {0, -1}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {1, 0}, },
		.cell[3] = {.pos = {2, 0}, },
	},
	[1] = {
		.cell[0] = {.pos = {1, 0}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {0, 1}, },
		.cell[3] = {.pos = {0, 2}, },
	},
	[2] = {
		.cell[0] = {.pos = {0, 1}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {-1, 0}, },
		.cell[3] = {.pos = {-2, 0}, },
	},
	[3] = {
		.cell[0] = {.pos = {-1, 0}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {0, -1}, },
		.cell[3] = {.pos = {0, -2}, },
	},
};

struct form j_zi[] = {
	[0] = {
		.cell[0] = {.pos = {0, -1}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {-1, 0}, },
		.cell[3] = {.pos = {-2, 0}, },
	},
	[1] = {
		.cell[0] = {.pos = {1, 0}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {0, -1}, },
		.cell[3] = {.pos = {0, -2}, },
	},
	[2] = {
		.cell[0] = {.pos = {0, 1}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {1, 0}, },
		.cell[3] = {.pos = {2, 0}, },
	},
	[3] = {
		.cell[0] = {.pos = {-1, 0}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {0, 1}, },
		.cell[3] = {.pos = {0, 2}, },
	},
};

struct form z_zi[] = {
	[0] = {
		.cell[0] = {.pos = {-1, -1}, },
		.cell[1] = {.pos = {0, -1}, },
		.cell[2] = {.pos = {0, 0}, },
		.cell[3] = {.pos = {1, 0}, },
	},
	[1] = {
		.cell[0] = {.pos = {1, -1}, },
		.cell[1] = {.pos = {1, 0}, },
		.cell[2] = {.pos = {0, 0}, },
		.cell[3] = {.pos = {0, 1}, },
	},
};

struct form s_zi[] = {
	[0] = {
		.cell[0] = {.pos = {-1, 0}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {0, -1}, },
		.cell[3] = {.pos = {1, -1}, },
	},
	[1] = {
		.cell[0] = {.pos = {0, -1}, },
		.cell[1] = {.pos = {0, 0}, },
		.cell[2] = {.pos = {1, 0}, },
		.cell[3] = {.pos = {1, 1}, },
	},
};

static inline int diam_level_move(struct panel *panel, struct diamond *diamond, int step)
{
	struct cell *cell;
	int i;
	int x, y;

	for (i = 0; i < CELL_NUM; i++) {
		cell = &diamond->form[diamond->index_form].cell[i];
		x = diamond->pos.x + cell->pos.x;
		y = diamond->pos.y + cell->pos.y;

		if (x + step < 0 || x + step >= panel->columns ||
			panel->val[x + step + y * panel->columns] != panel->back_rect) {
			printf("+++++++index_form = %d, step = %d, x = %d\n",
				diamond->index_form, step, x);

			return HIT;
		}
	}
	printf("before level move: diamond->pos.x = %d\n", diamond->pos.x);
/*
	if (ret == HIT) {
		step = step + (step > 0 ? -1 : 1);
		while (step * step > 0) {
			if (x + step < 0 || x + step >= panel->columns ||
				panel->val[x + step + y * panel->columns] != panel->back_rect)
				step = step + (step > 0 ? -1 : 1);
			else
				break;
		}
		printf("======step = %d\n", step);
	}
*/
	diamond->pos.x += step;

	return 0;
}

int diam_left_move(void *base, void *dest, int step)
{
	struct panel *panel = base;
	struct diamond *diamond = dest;
//	int tmp_ret;
	int ret = 0;
	int i;

/*	ret = diam_level_move(panel, diamond, -1 * step);
	printf("left: atfer level move: diamond->pos.x = %d, ret = %d\n", diamond->pos.x, ret);
	if (ret == HIT) {
		step--;
		while (step > 0) {
			tmp_ret = diam_level_move(panel, diamond, -1 * step);
			if (tmp_ret == HIT)
				step--;
			else
				break;
		}
		printf("left: diamond->pos.x = %d\n", diamond->pos.x);
	}
*/
	for (i = 1; i <= step; i++) {
		ret = diam_level_move(panel, diamond, -1);
		if (ret == HIT)
			break;
	}

	return ret;
}

int dima_right_move(void *base, void *dest, int step)
{
	struct panel *panel = base;
	struct diamond *diamond = dest;
//	int tmp_ret;
	int ret = 0;
	int i;

/*	ret = diam_level_move(panel, diamond, step);
	printf("right: atfer level move: diamond->pos.x = %d, ret = %d\n", diamond->pos.x, ret);
	if (ret == HIT) {
		step--;
		while (step > 0) {
			tmp_ret = diam_level_move(panel, diamond, step);
			if (tmp_ret == HIT)
				step--;
			else
				break;
		}
		printf("right: diamond->pos.x = %d\n", diamond->pos.x);
	}
*/
	for (i = 0; i < step; i++) {
		ret = diam_level_move(panel, diamond, 1);
		if (ret == HIT)
			break;
	}

	return ret;
}

int diam_down_move(void *base, void *dest, int step)
{
	struct panel *panel = base;
	struct diamond *diamond = dest;
	struct cell *cell;
	struct form *form;
	int i, ret = 0, n;
	int x, y;
//	int tmp_ret = HIT;

	form = &diamond->form[diamond->index_form];

	for (n = 0; n < step && n < panel->lines; n++) {
		for (i = 0; i < CELL_NUM; i++) {
			cell = &form->cell[i];
			x = diamond->pos.x + cell->pos.x;
			y = diamond->pos.y + cell->pos.y;

//			printf("down line %d: panel->val[%d][%d] = %d, panel->back_rect = %d\n",
//				__LINE__, x, y, panel->val[x + y * panel->columns], panel->back_rect);

			if (x < 0 || y < 0)
				continue;
			else if (y + 1 >= panel->lines ||
				panel->val[x + (y  + 1) * panel->columns] != panel->back_rect) {

				printf("%s() down HIT line %d: panel->val[%d][%d] = %d, panel->back_rect = %d\n",
					__FUNCTION__, __LINE__, y, x, panel->val[x + y * panel->columns], panel->back_rect);

				ret = HIT;
				break;
			} else
				ret = 0;
		}

		if (ret == HIT)
			break;

		diamond->pos.y++;
	}

/*	while (step > 0) {
		for (i = 0; i < CELL_NUM; i++) {
			cell = &form->cell[i];
			x = diamond->pos.x + cell->pos.x;
			y = diamond->pos.y + cell->pos.y;

//			printf("down line %d: panel->val[%d][%d] = %d, panel->back_rect = %d\n",
//				__LINE__, x, y, panel->val[x + y * panel->columns], panel->back_rect);

			if (y + step >= panel->lines ||
				panel->val[x + (y  + step) * panel->columns] != panel->back_rect) {

				printf("down HIT line %d: panel->val[%d][%d] = %d, panel->back_rect = %d\n",
					__LINE__, y, x, panel->val[x + y * panel->columns], panel->back_rect);

				tmp_ret = HIT;
				break;
			} else
				tmp_ret = 0;
		}

		if (tmp_ret == HIT) {
			step--;
			ret = HIT;
		} else {
			ret = 0;
			break;
		}
	}

	diamond->pos.y += step;
*/
	return ret;
}

static struct move_operations diam_ops = {
	.left_move = diam_left_move,
	.right_move = dima_right_move,
	.down_move = diam_down_move,
};

static int diam_change_form(struct panel *panel, struct diamond *diamond)
{
	struct cell *cell;
	int i, ret;
	int x, y;
	int index;

	if (diamond->form_num == 1)
		return 0;

	index =  (diamond->index_form + 1) % diamond->form_num;
	for (i = 0; i < CELL_NUM; i++) {
		cell = &diamond->form[index].cell[i];
		x = diamond->pos.x + cell->pos.x;
		y = diamond->pos.y + cell->pos.y;

		if (x < 0 || x >= panel->columns ||y < 0 || y >= panel->lines ||
			panel->val[x + y * panel->columns] != panel->back_rect) {
			printf("change form line %d--HIT: x = %d, y = %d\n", __LINE__, x, y);
			ret = HIT;
			break;
		} else
			ret = 0;
	}

	if (ret == 0)
		diamond->index_form = index;

	return ret;
}


struct diamond tts_diam[DIAM_NUM] = {
	[0] = {
		.form = chang_tiao,
		.form_num = 2,
		.index_form = 0,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[1] = {
		.form = tian_zi,
		.form_num = 1,
		.index_form = 0,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[2] = {
		.form = tu_zi,
		.form_num = 4,
		.index_form = 0,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[3] = {
		.form = l_zi,
		.form_num = 4,
		.index_form = 0,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[4] = {
		.form = j_zi,
		.form_num = 4,
		.index_form = 0,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[5] = {
		.form = z_zi,
		.form_num = 2,
		.index_form = 0,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[6] = {
		.form = s_zi,
		.form_num = 2,
		.index_form = 0,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
};

#if 0
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
#endif

void draw_diamond(struct panel *panel, struct diamond *diamond,
	struct lcd_info *lcd, int rect_n)
{
	int i, j;
	struct cell cell;
	int x, y;
	struct form *form;


		form = &diamond->form[diamond->index_form];
		for (j = 0; j < CELL_NUM; j++) {
			form->cell[j].rect = &panel->rects[rect_n];
			form->cell[j].param = (void *)rect_n;
		}
	for (i = 0; i < CELL_NUM; i++) {
		cell.rect = form->cell[i].rect;
		x = diamond->pos.x + form->cell[i].pos.x;
		y = diamond->pos.y + form->cell[i].pos.y;

		if (x >= 0 && x < panel->columns && y >= 0 && y < panel->lines
			&& panel->val[y * panel->columns + x] == panel->back_rect) {
			cell.pos.x = x;
			cell.pos.y = y;
			draw_cell(&panel->pix_pos, &cell, lcd);
		}
//		else {
//			printf("%s() line %d: ", __FUNCTION__, __LINE__);
//			printf("diamond.pos.x = %d, diamond.pos.y = %d\n", diamond->pos.x, diamond->pos.y);
//			printf("			 cell.pos.x = %d, cell.pos.y = %d\n", form->cell[i].pos.x, form->cell[i].pos.y);
//			printf("((((((((((((((((draw_diamond: panel->val[%d][%d] = %d\n",
//				y, x, panel->val[y * panel->columns + x]);
//		}
	}
}
#endif

#if 0
int draw_panel(struct component *parent, struct component *dest, struct lcd_info *lcd)
{
	struct cell cell;
	struct position pix_pos;
	struct frame *frame = container_of(parent, struct frame, comp_head);
	struct panel *panel = container_of(dest, struct panel, comp_node);
	int i, j;
//	printf("%s() line %d: lines = %d, columns = %d\n", __FUNCTION__, __LINE__, panel->lines, panel->columns);
	for (i = 0; i < panel->lines; i++) {
//		printf("line %-2d: ", i);
		for (j = 0; j < panel->columns; j++) {
			cell.rect = &panel->rects[(int)panel->val[j +i * panel->columns]];

			cell.pos.x = j;
			cell.pos.y = i;

//			printf("%3d", panel->val[j +i * panel->columns]);

			pix_pos.x = panel->pix_pos.x + frame->pix_pos.x;
			pix_pos.y = panel->pix_pos.y + frame->pix_pos.y;
			draw_cell(&pix_pos, &cell, lcd);
		}
//		printf("\n");
	}

	return 0;
}
#endif

#if 0
int draw_label(struct component *parent, struct component *dest, struct lcd_info *lcd)
{
//	int i, j;
	struct frame *frame = container_of(parent, struct frame, comp_head);
	struct label *label = container_of(dest, struct label, comp_node);
	const struct font_desc *font = label->font;
	char *base = lcd->lcd_base;
	int len;      //需要显示的字符个数
	int lines;   //需要显示的字符行数
	int count; //每行最多显示的字符个数
	int n;  	 //每行将要显示的字符个数
	char *text = (!label->minor_text) ? label->text : label->minor_text;
	int label_x, label_y, text_y;
	int proch_x, proch_y;
	int tmp_height, tmp_width;

//	start_x = frame->pix_pos.x + label->pix_pos.x +
//		(label->width - label->text_len * (font->width + font->word_gap * 2)) / 2;
//	start_y = frame->pix_pos.y + label->pix_pos.y +
//		(label->height - lines * (font->height + font->line_gap * 2)) / 2;

	tmp_width = font->width + font->word_gap * 2;
	tmp_height = font->height + font->line_gap * 2;

	len = label->text_len;
	count = (label->width / tmp_width);
	lines = (len / count) + (len % count > 0 ? 1 : 0);
	printf("%s() line %d: count = %d, lines = %d, len = %d\n", __FUNCTION__, __LINE__, count, lines, len);

	label_x = frame->pix_pos.x + label->pix_pos.x;
	label_y = frame->pix_pos.y + label->pix_pos.y;

	//	text_x = (label->width - label->text_len * tmp_width) / 2;
	//	text_y = (label->height - lines * tmp_height) / 2;
	proch_x = (label->width - (lines > 1 ? count : len) * tmp_width) / 2;
	while ((proch_y = (label->height - lines * tmp_height) / 2) < 0)
		lines--;

	base += label_y * lcd->line_length;
	draw_boder(base, lcd, label_x, label_x + label->width, label_y, label_y + proch_y, label->back_color);
	base += proch_y * lcd->line_length;
//	for (i = label_y; i < text_y; i++) {
//		for (j = label_x; j < lcd->width && j < label_x + label->width; j +=label->back_color.size )
//			memcpy(base + j * lcd->bpp, label->back_color, label->back_color.size);
//		base += lcd->line_length;
//	}

	printf("proch_x = %d, proch_y = %d, label->height = %d, tmp_height = %d\n",
		proch_x, proch_y, label->height, tmp_height);

	text_y = proch_y;
	while (len > 0 && text_y + tmp_height + proch_y <= label->height) {
		draw_boder(base, lcd, label_x, label_x + proch_x,
			label_y + text_y, label_y + text_y + tmp_height, label->back_color);

//		for (i = text_y; i < text_y + tmp_height; i++) {
//			for (j = label_x; j < lcd->width && j < label_x + text_x; j++)
//				memcpy(base + j * lcd->bpp, label->back_color, label->back_color.size);
//			base += lcd->line_length;
//		}

		n = len > count ? count : len;
		draw_n_char(lcd, label_x + proch_x, label_y + text_y,
			font, label->text_color, label->back_color, text, n);

		draw_boder(base, lcd, label_x + proch_x + n * tmp_width, label_x + label->width,
			label_y + text_y, label_y + text_y + tmp_height, label->back_color);
		base += tmp_height * lcd->line_length;

	//	for (i = text_y; i < text_y + tmp_height; i++) {
	//		for (j = label_x + text_x + n * tmp_width; j < lcd->width && j < label_x + label->width; j++)
	//			memcpy(base + j * lcd->bpp, label->back_color, label->back_color.size);
	//		base += lcd->line_length;
	//	}
		text += n;
		text_y += tmp_height;
		len -= n;
		printf("text_y = %d\n", text_y);
	}

	draw_boder(base, lcd, label_x, label_x + label->width,
		label_y + text_y, label_y + label->height, label->back_color);

	return 0;
}
#endif


#ifdef G_BIOS

static inline char get_input()
{
	return uart_recv_byte();
}

static inline unsigned char get_chose()
{
	unsigned char c;
	int ret;
//	int i, j;

//	c =  uart_recv_byte_no_wait();
/*	if (c > 0 && c != 'e' && c != ' ' && c == 'a' && c == 's' && c == 'd' && c == 'w' && c == 'c') {
		for (j = 0; j < 1000; j++)
		for (i = 0; i < 1000000; i++)
			uart_recv_byte_no_wait();
	}
*/
	ret = uart_recv_byte_timeout(&c, 1000 * 1000 * 10);
	if (ret < 0)
		c = 0;

	return c;
}

#else
static int __get_input(int sec)
{
	fd_set rfds;
	struct timeval tv;
	int ret;

	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	if (sec >= 0) {
		tv.tv_sec  = sec;
		tv.tv_usec = 0;

		ret = select(1, &rfds, NULL, NULL, &tv);
	} else
		ret = select(1, &rfds, NULL, NULL, NULL);

	if (ret < 0)
		perror("select()");

	printf("++++======+++++=========ret = %d\n", ret);
	return ret;
}

static inline int get_input()
{
	return __get_input(-1);
}

static inline int get_chose()
{
	return __get_input(1);
}

#endif

static inline int utoa(char *dest, unsigned int val)
{
	int n, i;
	int tmp = val;

	n = 0;
	do {
		n++;
		tmp /= 10;
	} while (tmp != 0);

	for (i = 0; i < n; i++, val /= 10)
		dest[n - i - 1] = val % 10 + '0';

	return n;
}

static int tetris_view_score(struct tetris *tetris)
{
	char str[17] = {'s', 'c', 'o', 'r', 'e', ':', ' '};
	struct label *score_label = &tetris->score_label;
	int n = strlen("score: ");

	n += utoa(str + n, tetris->score);
	score_label->set_text(score_label, (const char *)str, n);

	score_label->comp_node.draw_component(&tetris->frame.comp_head,
		&score_label->comp_node, tetris->lcd);

	return 0;
}

static int tetris_view_max_score(struct tetris *tetris)
{
	char str[21] = {'m', 'a', 'x', ' ', 's', 'c', 'o', 'r', 'e', ':', ' '};
	struct label *max_score_label = &tetris->max_score_label;
	int n = strlen("max score: ");

	n += utoa(str + n, tetris->max_score);
	max_score_label->set_text(max_score_label, (const char *)str, n);

	max_score_label->comp_node.draw_component(&tetris->frame.comp_head,
		&max_score_label->comp_node, tetris->lcd);

	return 0;
}

static int tetris_find_full_line(struct tetris *tetris)
{
	int i, j;
	struct panel *major_panel;
	int flag = 0;

	major_panel = &tetris->major_panel;
	for (i = major_panel->lines - 1; i >= tetris->top_line; i--) {
		flag = 1;
		for (j = 0; j < major_panel->columns; j++) {
			if (major_panel->val[i * major_panel->columns + j] == major_panel->back_rect) {
				flag = 0;
				break;
			}
		}

		if (flag == 1)
			break;
	}

	if (flag == 0)
		return -1;
	printf("%s() line %d: fine full line = %d\n",__FUNCTION__, __LINE__, i);
	tetris->full_line = i;
	return 0;
}

static inline void tetris_update_top_line(struct tetris *tetris)
{
	int i, j;
	struct panel *panel = &tetris->major_panel;
	int flag = 0;

	for (i = tetris->top_line; i < panel->lines; i++) {
		for (j = 0; j < panel->columns; j++) {
			if (panel->val[i * panel->columns + j] != panel->back_rect) {
				flag = 1;
				break;
			}
		}

		if (flag)
			break;
	}

	printf("update top_line: top_line = %d\n", i);

	tetris->top_line = i;
}

static int check_fault(struct panel *panel, int line, int column, int top_line, int up_step, int level_step, int *flags)
{
	int flag_up, flag_level;
	int i;

	if (column >= 0 && column < panel->columns && line < top_line)
		return 1;

	if (column < 0 || column >= panel->columns)
		return 1;

	//if (column < 0 || column >= panel->columns
	//	|| panel->val[line * panel->columns + column] != panel->back_rect)	
	if (panel->val[line * panel->columns + column] != panel->back_rect)
		return 0;

	for (i = line; i >= top_line; i--) {
		if (panel->val[i * panel->columns + column] != panel->back_rect)
			break;
	}
	if (i < top_line)
		return 1;

	flag_up = check_fault(panel, line + up_step, column, top_line, up_step, level_step, flags);
	flag_level = check_fault(panel, line, column + level_step, top_line, up_step, level_step, flags);

	if (i >= top_line && (flag_up || flag_level) && flags[column] < 0)
		flags[column] = line;

	return flag_up || flag_level;
}

static void fault_down_move(struct panel *panel, int full_line, int top_line, int *flags)
{
	int i, j;
	int min_height = panel->lines;
	int line_up, line_down;

	//求得悬空快需要下移的行数
	for (j = 0; j < panel->columns; j++) {
		line_up = 0;
		line_down = 0;
		if (flags[j] >= 0) {
			printf("%s() line %d: flags[%d] = %d\n", __FUNCTION__, __LINE__, j, flags[j]);
			for (i = flags[j]; i >= top_line; i--) {
				if (panel->val[i * panel->columns + j] != panel->back_rect) {
					line_up = i;
					break;
				}
			}

			for (i = flags[j]; i < panel->lines; i++) {
				if (panel->val[i * panel->columns + j] != panel->back_rect)
					break;
			}
			line_down = i;

			flags[j] = line_up;
			if (min_height > line_down - line_up - 1)
				min_height = line_down - line_up - 1;
			printf("			min_height = %d\n", min_height);
		}
	}

	//先整体下移
	for (j = 0; j < panel->columns; j++) {
		for (i = full_line; i >= top_line; i--) {
			panel->val[i * panel->columns + j]
				= panel->val[(i - 1) * panel->columns + j];
			panel->val[(i - 1) * panel->columns + j] = panel->back_rect;
		}
		if (flags[j] >= 0)
			flags[j]++;
	}
	min_height--;
	top_line++;

	//再悬空快下移
	for (j = 0; min_height && j < panel->columns; j++) {
		if (flags[j] >= 0) {
			for (i = flags[j]; i >= top_line; i--) {
				panel->val[(i + min_height) * panel->columns + j]
					= panel->val[i * panel->columns + j];
				panel->val[i  * panel->columns + j] = panel->back_rect;
			}
		}
	}
}

static void tetris_down_move(struct tetris *tetris)
{
	int i, j;
	struct panel *panel = &tetris->major_panel;
	int flag;
	int column, line;
	int flags[panel->columns];

	if (tetris->full_line < 0)
		return;

	printf("full_line = %d, top_line = %d\n", tetris->full_line, tetris->top_line);

	line = tetris->full_line;
	for (column = 0; column < panel->columns; column++) {
		flag = 0;
		for (j = column; j < panel->columns && flag == 0; j++) {
			if (panel->val[line * panel->columns + j] == panel->back_rect) {
				printf("in tetris_down_move: line = %d, column = %d, top_line = %d\n", line, j, tetris->top_line);
				for (i = line - 1; i >= tetris->top_line; i--) {
					printf("line = %d, column = %d\n", i, j);
					if (panel->val[i * panel->columns + j] != panel->back_rect) {
						flag = 1;
						column = j;
						break;
					}
				}
			}
		}

		for (i = 0; flag && i < panel->columns; i++)
			flags[i] = -1;

		if (flag)
			printf("before check_fault: line = %d, column = %d\n", line, column);

		if (flag && check_fault(panel, line, column, tetris->top_line, -1, -1, flags)
			&& check_fault(panel, line, column, tetris->top_line, -1, 1, flags)) {
			printf("-------has fault: column = %d, line = %d\n", column, line);

			fault_down_move( panel, tetris->full_line, tetris->top_line, flags);

			for (i = 0; i < panel->columns; i++) {
				if (flags[i] >= 0)
					printf("-------flags[%d] = %d\n", i, flags[i]);
			}

			tetris_update_top_line(tetris);
		}
	}
}

static int tetris_delete_line(struct tetris *tetris)
{
	int j;
	struct panel *major_panel  = &tetris->major_panel;

	if (tetris->full_line < 0 || tetris->full_line >= major_panel->lines) {
		tetris->full_line = -1;
		return -1;
	}
	printf("%s() line %d: ++++++++delete line, full_line = %d\n",
		__FUNCTION__, __LINE__, tetris->full_line);
	for (j = 0; j < major_panel->columns; j++)
		major_panel->val[tetris->full_line * major_panel->columns + j] = major_panel->back_rect;

	tetris->score++;
	return 0;
}

static int tetris_read_score(struct tetris *tetris)
{
#ifdef G_BIOS
	tetris->max_score = 300;
	return 0;
#else
	int fd;
	unsigned int score = 0;
	int ret;

	printf("-----------------------read_score: line %d\n", __LINE__);
	fd = open(SCORE_FILE,  O_RDWR);
	if (fd < 0 && errno != ENOENT) {
		perror("open");
		return fd;
	} else if (errno == ENOENT) {
		tetris->max_score = 0;
		return tetris->write_score(tetris);
	}

	ret = read(fd, &score, sizeof(score));
	if (ret < 0) {
		close(fd);
		perror("read");
		return ret;
	}

	close(fd);

	tetris->max_score = score;

	return 0;
#endif
}

static int tetris_write_score(struct tetris *tetris)
{
#ifndef G_BIOS
	int fd;
	int ret;
	unsigned int score;

	if (tetris == NULL)
		return 0;

	score = tetris->max_score;

	fd = open(SCORE_FILE,  O_RDWR | O_CREAT, 0666);
	if (fd < 0) {
		perror("open");
		return fd;
	}

	ret = write(fd, &score, sizeof(score));
	if (ret < 0) {
		close(fd);
		perror("write");
		return ret;
	}

	close(fd);
#endif
	return 0;
}


static int build_tetris_body(struct tetris *tetris
	, int x, int y, int width, int height, struct lcd_info *lcd)
{
	int w, h;
	int ret;
	struct frame *frame;
	struct panel *major_panel;
	struct panel *minor_panel;
	struct label *max_score_label;
	struct label *score_label;
	struct rectangle *rects = tetris->rects;
	int rect_num = tetris->rect_num;
	int major_x, major_y;
	int minor_y;
//	int major_width, major_height;
	const struct font_desc *font;
	int label_x, label_w, label_h;

	if (lcd->width >= 800)
		font = find_font("SUN12x22");
	else
		font = find_font("VGA8x16");
	if (font == NULL)
		printf("%s() line %d: ---------- font == NULL\n", __FUNCTION__, __LINE__);
	frame = &tetris->frame;
	printf("%s() line %d: before init_frame\n", __FUNCTION__, __LINE__);
	ret = init_frame(frame, x, y, width, height, tetris->colors[tetris->color_num - 1], lcd);
	if (ret < 0) {
		perror("init frame");
		return -1;
	}

//	w = (width / 2) / (rects[0].w + rects[0].gap * 2);
//	h = height / (rects[0].h + rects[0].gap * 2);
	w = rects[0].w + rects[0].gap * 2;
	h = rects[0].h + rects[0].gap * 2;
	printf("%s() line %d: before init major panel\n", __FUNCTION__, __LINE__);

	major_panel = &tetris->major_panel;
//	major_x = w > 14 ? (w - 14) * rects[0].w : 0;
//	major_y = h > 24 ? (h - 24) / 2 * rects[0].h : 0;
	major_x = lcd->width / 2 - MAJOR_COLUMNS * w;
	major_y = (lcd->height - MAJOR_LINES * h) / 2;
//	major_width = w > 14 ? 14 : w;
//	major_height = h > 24 ? 24 : h;
	ret = init_panel(major_panel, major_x, major_y, MAJOR_COLUMNS, MAJOR_LINES,
		rects, rect_num, 0, lcd);
	if (ret < 0) {
		perror("init major panel");
		return -1;
	}

	frame->add_component(frame, &major_panel->comp_node);
	printf("%s() line %d: after add_component of major_panel\n", __FUNCTION__, __LINE__);

	max_score_label = &tetris->max_score_label;
	score_label = &tetris->score_label;

	label_w = 16 * (font->width + font->word_gap * 2);
	label_h = font->height+ font->line_gap * 2 + 10;
	label_x = lcd->width / 2;
	ret = init_label(max_score_label, label_x, major_y,
		label_w, label_h, tetris->colors[0], tetris->colors[3], font, lcd);
	printf("%s() line %d: after init max_score_label\n", __FUNCTION__, __LINE__);
	frame->add_component(frame, &max_score_label->comp_node);

	ret = init_label(score_label, label_x, major_y + label_h + 10,
		label_w, label_h, tetris->colors[0], tetris->colors[3], font, lcd);
	printf("%s() line %d: after init score_label\n", __FUNCTION__, __LINE__);
	frame->add_component(frame, &score_label->comp_node);
	
	printf("%s() line %d: before init minor panel\n", __FUNCTION__, __LINE__);
	minor_panel = &tetris->minor_panel;
	minor_y = (lcd->height - MINOR_LINES * h) / 2;
	minor_y = major_y + label_h * 2 + 10 >= minor_y ? major_y + label_h * 2 + 20 : minor_y;
	ret = init_panel(minor_panel, label_x, minor_y,
		MINOR_COLUMNS, MINOR_LINES, rects, rect_num, 3, lcd);
	if (ret < 0) {
		perror("init minor panel");
		return -1;
	}
	printf("%s() line %d: after init minor panel\n", __FUNCTION__, __LINE__);

	frame->add_component(frame, &minor_panel->comp_node);
	printf("%s() line %d: after add_component of minor_panel\n", __FUNCTION__, __LINE__);

	return 0;
}

static int tetris_run(struct tetris *tetris)
{
	struct panel *major_panel;
	struct panel *minor_panel;
	struct diamond *diamond;
	struct diamond *next_diam;
	struct lcd_info *lcd;
	struct form *form;
	struct frame *frame;
	int i = 0, n = 0;
	int x, y;
	int ret, tmp_ret;
	int cell_n;
	unsigned char c;
//	int t;
//	int flag1, flag2;

	printf("%s() line %d\n", __FUNCTION__, __LINE__);

	lcd = tetris->lcd;
	frame = &tetris->frame;
	frame->comp_head.draw_component(NULL, &frame->comp_head, lcd);

	major_panel = &tetris->major_panel;
	printf("%s() line %d before draw major panel\n", __FUNCTION__, __LINE__);
	major_panel->comp_node.draw_component(&frame->comp_head,
		&major_panel->comp_node, lcd);

	printf("%s() line %d before draw minor panel\n", __FUNCTION__, __LINE__);
	minor_panel = &tetris->minor_panel;
	minor_panel->comp_node.draw_component(&frame->comp_head,
		&minor_panel->comp_node, lcd);

	tetris->next_diam = 0;
	next_diam = &tetris->diamond[tetris->next_diam];
	next_diam->pos.x = 2;
	next_diam->pos.y = 2;
	next_diam->draw_diamond(minor_panel, next_diam, lcd, 1);

	printf("%s() line %d before view max score\n", __FUNCTION__, __LINE__);
	tetris->view_max_score(tetris);
	printf("%s() line %d before view score\n", __FUNCTION__, __LINE__);
	tetris->view_score(tetris);

	printf("%s() line %d start run\n", __FUNCTION__, __LINE__);
	sleep(1);
#ifdef G_BIOS
	for (j = 1; ; j++) {
		srandom(j);
#else
	while (1) {
		srandom(time(NULL));
#endif
		tetris->curr_diam = tetris->next_diam;
		tetris->next_diam = random() % tetris->diam_num;
		printf("%s() line %d running============\n", __FUNCTION__, __LINE__);

		minor_panel->comp_node.draw_component(&frame->comp_head,
		&minor_panel->comp_node, lcd);
		printf("%s() line %d running============\n", __FUNCTION__, __LINE__);

		next_diam = &tetris->diamond[tetris->next_diam];
		next_diam->pos.x = 2;
		next_diam->pos.y = 2;
		next_diam->index_form = 0;
		next_diam->draw_diamond(minor_panel, next_diam, lcd, 1);
		printf("%s() line %d running============\n", __FUNCTION__, __LINE__);

		diamond = &tetris->diamond[tetris->curr_diam];
		diamond->pos.x = 6;
		diamond->pos.y = 0;
		diamond->index_form = 0;
		diamond->draw_diamond(major_panel, diamond, lcd, 1);
		printf("%s() line %d running============\n", __FUNCTION__, __LINE__);

		ret = 0;
		for (i = 0; ret != HIT; i++) {

#ifdef G_BIOS
			n = 0;
			while ((c = get_chose()) && (n++ < 3)) {
#else
			c = 0;
			clock_t start;
			clock_t end;
			start = times(NULL);
			printf("----------time-start: = %ld\n", start);
			while (get_chose() > 0) {
				c = getchar();
#endif
				printf("chose: %d\n", c);
				switch (c) {
				case 'a':
					diamond->draw_diamond(major_panel, diamond, lcd, 0);
					diamond->d_ops->left_move(major_panel, diamond, 1);
					diamond->draw_diamond(major_panel, diamond, lcd, 1);
					printf("left\n");
					break;
				case 's':
					diamond->draw_diamond(major_panel, diamond, lcd, 0);
					ret = diamond->d_ops->down_move(major_panel, diamond, major_panel->lines);
					diamond->draw_diamond(major_panel, diamond, lcd, 1);
					printf("down\n");
					break;
				case 'd':
					diamond->draw_diamond(major_panel, diamond, lcd, 0);
					diamond->d_ops->right_move(major_panel, diamond, 1);
					diamond->draw_diamond(major_panel, diamond, lcd, 1);
					printf("right\n");
					break;
				case 'w':
					diamond->draw_diamond(major_panel, diamond, lcd, 0);
					diamond->change_form(major_panel, diamond);
					diamond->draw_diamond(major_panel, diamond, lcd, 1);
					break;
				case ' ':
					while ((c = get_input()) != ' ') {
						if (c == 'e')
							goto EXIT;
					}
					break;
				case 'e':
					goto EXIT;
				default:
					goto OUT;
				}

				if (ret == HIT)
					break;

				end = times(NULL);
				printf("----------time-end: = %ld\n", end);
				if ((end - start ) / 100 >= 1) {
					printf("----------time = %ld\n", (end - start) / 100);
					break;
				}

			}

OUT:
//			sleep(1);

			if (ret != HIT) {
				diamond->draw_diamond(major_panel, diamond, lcd, 0);
				ret = diamond->d_ops->down_move(major_panel, diamond, 1);
				diamond->draw_diamond(major_panel, diamond, lcd, 1);
			}
		}

//		if (ret == HIT) {
		diamond->draw_diamond(major_panel, diamond, lcd, 0);
		ret = diamond->d_ops->down_move(major_panel, diamond, 1);
		diamond->draw_diamond(major_panel, diamond, lcd, 2);

		printf("%s() line %d: ---------------down move HIT\n", __FUNCTION__, __LINE__);

		tmp_ret = 0;
		form = &diamond->form[diamond->index_form];
		cell_n = sizeof(form->cell) / sizeof(form->cell[0]);
		for (n = 0; n < cell_n; n++) {
			x = diamond->pos.x + form->cell[n].pos.x;
			y = diamond->pos.y + form->cell[n].pos.y;
			if (y >= 0 && tetris->top_line > y)
				tetris->top_line = y;

			tmp_ret += major_panel->set_val(major_panel, x, y, 2);
		}

		if (tmp_ret == -1 * cell_n) {
			printf("%s() line %d: -------game over!--------\n", __FUNCTION__, __LINE__);
			break;
		}

		major_panel->comp_node.draw_component(&frame->comp_head,
				&major_panel->comp_node, tetris->lcd);

		while (tetris->find_full_line(tetris) == 0) {
			ret = tetris->delete_line(tetris);
			major_panel->comp_node.draw_component(&frame->comp_head,
				&major_panel->comp_node, tetris->lcd);
			tetris->down_move(tetris);
#ifdef G_BIOS
			sleep(1);
#else
			usleep(400 * 1000);
#endif
			major_panel->comp_node.draw_component(&frame->comp_head,
				&major_panel->comp_node, tetris->lcd);

			if (tetris->score > tetris->max_score) {
				tetris->max_score = tetris->score;
				tetris->view_max_score(tetris);
			}
			tetris->view_score(tetris);
		}
//	}

		printf("%s() line %d: tetris->top_line = %d\n", __FUNCTION__, __LINE__, tetris->top_line);
		if (tetris->top_line <= 0)
			break;
	}

EXIT:
	return 0;
}

static int tetris_close(struct tetris *tetris)
{
	if (tetris->write_score != NULL)
		tetris->write_score(tetris);

	if (tetris->frame.comp_head.release)
		tetris->frame.comp_head.release(&tetris->frame.comp_head);

//	if (tetris->colors)
//		free(tetris->colors);

	return 0;
}

int init_tetris(struct tetris *tetris, int x, int y, int width, int height,
	struct lcd_info *lcd, color_t *colors, int color_num, struct rectangle *rects, int rect_num)
{
	int ret;

	if (x < 0 || y < 0 || width < 0 || height < 0 ||
		x + width > lcd->width || y + height > lcd->height) {
		printf("invalid arguments\n");
		return -1;
	}

	printf("%s() line %d\n", __FUNCTION__, __LINE__);

	tetris->lcd = lcd;

	tetris->run = tetris_run;
	tetris->close = tetris_close;

	tetris->diamond = tts_diam;
	tetris->diam_num = DIAM_NUM;
	tetris->curr_diam = 0;
	tetris->next_diam = 1;

	tetris->find_full_line = tetris_find_full_line;
	tetris->delete_line = tetris_delete_line;
	tetris->down_move = tetris_down_move;
	tetris->top_line = height;
	tetris->full_line = -1;

	tetris->colors = colors;
	tetris->color_num = color_num;
	tetris->rects = rects;
	tetris->rect_num = rect_num;

	tetris->read_score = tetris_read_score;
	tetris->write_score = tetris_write_score;
	tetris->view_score = tetris_view_score;
	tetris->view_max_score = tetris_view_max_score;
	tetris->score = 0;
	tetris->read_score(tetris);

	printf("%s() line %d before build_tetris_body\n", __FUNCTION__, __LINE__);
	ret = build_tetris_body(tetris, x, y, width, height, lcd);
	if (ret < 0) {
		perror("build_tetris_body");
		tetris->close(tetris);
		return -1;
	}
	printf("%s() line %d\n", __FUNCTION__, __LINE__);

	return 0;
}

#if 0
int main(int argc, char **argv)
{
	int fd;
	int ret;
	char *map;
	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;
	struct lcd_info lcd;
	struct tetris tetris;
	color_t out_color = get_color(0, 200, 200, 200);
	color_t in_color_on = get_color(0, 255, 0, 0);
	color_t in_color_off = get_color(0, 100, 120, 100);
	color_t in_color_end = get_color(0, 0, 0, 255);
	color_t minor_panel_color = get_color(0, 0, 0, 0);
	color_t colors[5];
	struct rectangle rects[4];
	int w, h;

/*
	rects = malloc(sizeof(*rects) * 3);
	if (!rects) {
		perror("malloc rectangles");
		return -1;
	}
*/

	memcpy(&colors[0], &out_color, sizeof(out_color));
	memcpy(&colors[1], &in_color_on, sizeof(in_color_on));
	memcpy(&colors[2], &in_color_off, sizeof(in_color_off));
	memcpy(&colors[3], &in_color_end, sizeof(in_color_end));
	memcpy(&colors[4], &minor_panel_color, sizeof(minor_panel_color));

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		printf("open /dev/fb0 error!\n");
		return -1;
	}

	ret = ioctl(fd, FBIOGET_FSCREENINFO, &fix);
	if (ret < 0) {
		perror("ioctl");
		return -1;
	}

	lcd.line_length = fix.line_length;

	ret = ioctl(fd, FBIOGET_VSCREENINFO, &var);
	if (ret < 0) {
		perror("ioctl");
		return -1;
	}

	lcd.width = var.xres_virtual;
	lcd.height = var.yres_virtual;
	lcd.bpp = (var.bits_per_pixel + 7) / 8;

	printf("width = %d, height = %d, bpp = %d\n", lcd.width, lcd.height, lcd.bpp);

	w = lcd.width / 2 / MAJOR_COLUMNS;
	w = w > 20 ? 18 : w - 2;

	h = lcd.height / MAJOR_LINES;
	h = h > 20 ? 18 : h - 2;

	printf("%s() line %d: w = %d, h = %d\n", __FUNCTION__, __LINE__, w, h);

	rects[0].w = w;
	rects[0].h = h;
	rects[0].gap = 1;
	rects[0].out_color = colors[0];
	rects[0].in_color = colors[2];

	rects[1].w = w;
	rects[1].h = h;
	rects[1].gap = 1;
	rects[1].out_color = colors[0];
	rects[1].in_color = colors[1];

	rects[2].w = w;
	rects[2].h = h;
	rects[2].gap = 1;
	rects[2].out_color = colors[0];
	rects[2].in_color = colors[3];

	rects[3].w = w;
	rects[3].h = h;
	rects[3].gap = 1;
	rects[3].out_color = colors[4];
	rects[3].in_color = colors[4];

	map = mmap(NULL, lcd.height * lcd.line_length, PROT_WRITE, MAP_SHARED, fd, 0);
	if (!map) {
		perror("mmap");
		return -1;
	}
	lcd.lcd_base = map;
	printf("%s() line %d: before init_tetris\n", __FUNCTION__, __LINE__);

#ifndef G_BIOS
	font_init();
	printf("%s() line %d: atfer new_tetris\n", __FUNCTION__, __LINE__);
#endif

	ret = init_tetris(&tetris, 0, 0, lcd.width, lcd.height, &lcd,
		colors, sizeof(colors) / sizeof(colors[0]),
		rects, sizeof(rects) / sizeof(rects[0]));
	if (ret < 0) {
		perror("init_tetris");
		munmap(map, lcd.height * lcd.line_length);
		close(fd);
		return -1;
	}
	printf("%s() line %d: atfer new_tetris\n", __FUNCTION__, __LINE__);

#ifndef G_BIOS
	tty_set();
#endif
	ret = tetris.run(&tetris);
#ifndef G_BIOS
	tty_reset();
#endif

	tetris.close(&tetris);
	munmap(map, lcd.height * lcd.line_length);
	close(fd);
	return ret;
}
#endif
