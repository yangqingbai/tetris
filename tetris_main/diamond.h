#ifndef ___DIAMOND_H___
#define ___DIAMOND_H___

#define CELL_NUM 4
#define DIAM_NUM 7

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

#define NOTHIT 0
#define HIT 1

extern struct diamond tts_diam[] ;

#endif