#include <stdio.h>
#include <graphic/draw.h>
#include <graphic/graphic.h>
#include "diamond.h"

#define DEF_INDEX 0

#define ARREY_SIZE(a) (sizeof(a) / sizeof(a[0]))

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

static int diam_left_move(void *base, void *dest, int step)
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

static int dima_right_move(void *base, void *dest, int step)
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

static int diam_down_move(void *base, void *dest, int step)
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

static void draw_diamond(struct panel *panel, struct diamond *diamond,
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

static struct form chang_tiao[] = {
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

static struct form tian_zi[] = {
	[0] = {
		.cell[0] = {.pos = {0, -1}, },
		.cell[1] = {.pos = {1, -1}, },
		.cell[2] = {.pos = {0, 0}, },
		.cell[3] = {.pos = {1, 0}, },
	},
};

static struct form tu_zi[] = {
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

static struct form l_zi[] = {
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

static struct form j_zi[] = {
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

static struct form z_zi[] = {
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

static struct form s_zi[] = {
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


struct diamond tts_diam[DIAM_NUM] = {
	[0] = {
		.form = chang_tiao,
		.form_num = ARREY_SIZE(chang_tiao),
		.index_form = DEF_INDEX,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[1] = {
		.form = tian_zi,
		.form_num = ARREY_SIZE(tian_zi),
		.index_form = DEF_INDEX,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[2] = {
		.form = tu_zi,
		.form_num = ARREY_SIZE(tu_zi),
		.index_form = DEF_INDEX,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[3] = {
		.form = l_zi,
		.form_num = ARREY_SIZE(l_zi),
		.index_form = DEF_INDEX,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[4] = {
		.form = j_zi,
		.form_num = ARREY_SIZE(j_zi),
		.index_form = DEF_INDEX,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[5] = {
		.form = z_zi,
		.form_num = ARREY_SIZE(z_zi),
		.index_form = DEF_INDEX,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
	[6] = {
		.form = s_zi,
		.form_num = ARREY_SIZE(s_zi),
		.index_form = DEF_INDEX,
		.d_ops = &diam_ops,
		.change_form = diam_change_form,
		.draw_diamond = draw_diamond,
	},
};
