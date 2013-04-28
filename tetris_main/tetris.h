#ifndef _____TETRIS_H_____
#define _____TETRIS_H____

#include <graphic/draw.h>
#include <graphic/graphic.h>

struct tetris {
	struct frame frame;
	struct lcd_info *lcd;
	struct rectangle *rects;
	int rect_num;
	struct panel major_panel;
	struct panel minor_panel;
	struct label max_score_label;
	struct label score_label;
	struct diamond *diamond;
	int diam_num;
	int curr_diam;
	int next_diam;
	color_t *colors;
	int color_num;
	unsigned int score;
	unsigned int max_score;
	int top_line;
	int full_line;

	int (*run)(struct tetris *tetris);
	int (*close)(struct tetris *tetris);
	int (*read_score)(struct tetris *tetris);
	int (*write_score)(struct tetris *tetris);
	int (*delete_line)(struct tetris *tetris);
	int (*find_full_line)(struct tetris *tetris);
	int (*view_score)(struct tetris *tetris);
	int (*view_max_score)(struct tetris *tetris);
	void (*down_move)(struct tetris *tetris);
};

int init_tetris(struct tetris *tetris, int x, int y, int width, int height,
	struct lcd_info *lcd, color_t *colors, int color_num, struct rectangle *rects, int rect_num);

#endif
