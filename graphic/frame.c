#include <stdio.h>
#include <string.h>

#include <graphic/draw.h>
#include <graphic/graphic.h>


#define container_of(ptr, type, member) \
	(type *)((char *)ptr - (long)(&((type *)0)->member))

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


int init_frame(struct frame *frame, int x, int y, int width, int height,
	color_t back_color, struct lcd_info *lcd)
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

