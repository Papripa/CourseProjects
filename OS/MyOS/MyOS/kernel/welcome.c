#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

void welcomeAnimation(){
	int t=200;
	milli_delay(t);
	int i = 0;
	for (i = 1; i <= 100; ++i)
	{
		printf("-");
	}
	for (i = 1; i <= 100; ++i)
	{
		printf("\b");
	}
	for (i = 1; i <= 240; ++i)
	{
		printf(">");
		milli_delay(t);
	}

	printf("\n");
	milli_delay(t);
	printf("\n");
}

PUBLIC void welcome(){   
printf("\n\n\n\n\n\n\n\n\n\n\n\n");
printf("                         __   __         __   __\n" );
printf("                        |  \\/  |       / __\\/  _|  \n");
printf("                        | \\  / | _   _ ||  || (__  \n");
printf("                        | |\\/| || | | |||  ||\\__ \\    \n");
printf("                        | |  | || |_| |||__||___)|      \n");
printf("                        |_|  |_|\\____ | \\__/|___/              \n");
printf("                                  __/ |\n");
printf("                                 |___/  \n");
printf("\n");
printf("\n");

printf("               Please print CRTL+F2 or CRTL+F3 to change your tty.");
printf("\n");

printf("\n");
printf("\n");
printf("\n");

printf("\n");
}

