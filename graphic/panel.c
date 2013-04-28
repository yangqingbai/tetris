#include <stdio.h>
#include <graphic/draw.h>
#include <graphic/graphic.h>

#define container_of(ptr, type, member) \
	(type *)((char *)ptr - (long)(&((type *)0)->member))

char val1[MAJOR_COLUMNS * MAJOR_LINES];
char val2[MINOR_COLUMNS * MINOR_LINES];

static int panel_set_val(struct panel *panel, int x, int y, int val)
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

static int panel_set_back_rect(struct panel *panel, int back_rect)
{
	if (back_rect >= panel->rect_num)
		return -1;

	panel->back_rect = back_rect;
	return 0;
}

static void panel_set_rectangles(struct panel *panel, struct rectangle *rects, int rect_num)
{
	panel->rects = rects;
	panel->rect_num = rect_num;
}

static int panel_release(struct component *comp)
{
//	struct panel *panel = container_of(comp, struct panel, comp_node);

//	free(panel->val);
	return 0;
}

static int draw_panel(struct component *parent, struct component *dest, struct lcd_info *lcd)
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

