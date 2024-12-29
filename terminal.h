// https://gist.github.com/dagon666/8194870
//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
void cm_off(void)
{
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag &= ~ICANON; 
	t.c_lflag &= ~ECHO;

	// Apply the new settings
	tcsetattr(STDIN_FILENO, TCSANOW, &t); 
}

void cm_on(void)
{
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag |= ICANON; 
	t.c_lflag |= ECHO;

	// Apply the new settings
	tcsetattr(STDIN_FILENO, TCSANOW, &t); 
}
