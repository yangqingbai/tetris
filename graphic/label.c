#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <graphic/draw.h>
#include <graphic/graphic.h>
#include <font/font.h>


#define container_of(ptr, type, member) \
	(type *)((char *)ptr - (long)(&((type *)0)->member))

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

