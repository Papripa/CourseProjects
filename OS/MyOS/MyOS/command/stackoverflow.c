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
/*

esp------>main

|
|

esp
*/
int main(int argc, char* argv[]) {
	overflow_retaddr();
	printf("You are safe\n");
}

void flow() {
	printf("You are hacked!\n");
}

unsigned char maldata[20];

void overflow_retaddr() {
	unsigned char testbuf[4];
	unsigned long test = (unsigned long)flow;
	memcpy(maldata + 16, &test, 4);
	memcpy(testbuf, maldata, 20);	
}

