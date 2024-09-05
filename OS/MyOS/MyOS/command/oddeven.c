
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
struct posix_tar_header
{				/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
	/* 500 */
};

void main(int argc, char * argv[]){
	char *filename="/cmd.tar";
	int fd = open(filename, O_RDWR);
	assert(fd != -1);
	int cmd=0;
	int len[10];
	char name[10][100];
	char buf[SECTOR_SIZE * 16];
	int chunk = sizeof(buf);
	while (1) {
		read(fd, buf, SECTOR_SIZE);
		if (buf[0] == 0)
			break;
		struct posix_tar_header * phdr = (struct posix_tar_header *)buf;
		int flag =0;
		//name[num]=&(phdr->name); 
		if(strcmp(phdr->name,argv[1])==1){
			flag=1;
		}   
		 //calculate the file size 
		char * p = phdr->size;//size of the file
		int f_len = 0;
		while (*p)
			f_len = (f_len * 8) + (*p++ - '0'); //octal 
		
		int fdout = open(phdr->name, O_RDWR);
		int bytes_left = f_len; 
		int now_checknum=0;                		
		while (bytes_left) {
			int iobytes = min(chunk, bytes_left);
			read(fd, buf,((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			for(int j = 0; j < ((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE && flag == 1; j++) {
				now_checknum = now_checknum ^ buf[j];
			}
			bytes_left -= iobytes;
			
		}
		int x;
		for(x=0;x<=100 && flag==1;x++){
			if(strcmp(check_value[x].name,phdr->name)!=0){
				break;
			}
		}
		if(flag == 1){
			printl("ELF Name: %s\nnow checksum : %d\norigin checksum : %d", check_value[x].name, now_checknum,check_value[x].check_value);
		}
		cmd++;
		// len of the file
	}
	close(fd);
}
