#include <string.h>

#include <graphic/draw.h>
#include <font/font.h>

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

