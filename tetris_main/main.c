#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#include <graphic/draw.h>
#include <graphic/graphic.h>
#include <font/font.h>
#include <tty/tty_control.h>
#include "tetris.h"

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

