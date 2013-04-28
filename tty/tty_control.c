#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <unistd.h>

static struct termios old_attr, cur_attr;

int tty_reset(void)
{
	if (tcsetattr(STDIN_FILENO, TCSANOW, &old_attr) != 0)
		return -1;

	return 0;
}


int tty_set(void)
{
	if ( tcgetattr(STDIN_FILENO, &old_attr) )
		return -1;

	memcpy(&cur_attr, &old_attr, sizeof(cur_attr) );
	cur_attr.c_lflag &= ~ICANON;
	//cur_attr.c_lflag &= ~ECHO; //no echo
	cur_attr.c_cc[VMIN] = 1;
	cur_attr.c_cc[VTIME] = 0;

	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_attr) != 0)
		return -1;

	return 0;
}

/*
int get_input(void)
{
	fd_set rfds;
	struct timeval tv;
	int ret;

	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	tv.tv_sec  = 1;
	tv.tv_usec = 0;

	ret = select(1, &rfds, NULL, NULL, &tv);

	if (ret < 0) {
		perror("select()");
		return -1;
	} else if (ret > 0)
		return 1;

	return 0;
}

int main()
{
	int tty_set_flag;
	tty_set_flag = tty_set();
	while(1) {
		if(get_input()) {
			const int key = getchar();

			printf("%c pressed\n", key);
			if(key == 'q')
				break;
		} else
			fprintf(stderr, "<no key detected>\n");
	}

	if(tty_set_flag == 0)
		tty_reset();

	return 0;
}
*/
