/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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

PUBLIC int test_init()
{
	struct proc *p;
	int i;
	proc_table[NR_TEST].startTime = 0;
	proc_table[NR_TEST + 1].startTime = 1;
	proc_table[NR_TEST + 2].startTime = 3;
	proc_table[NR_TEST + 3].startTime = 4;
	proc_table[NR_TEST + 4].startTime = 5;
	proc_table[NR_TEST].needTime = 3;
	proc_table[NR_TEST + 1].needTime = 8;
	proc_table[NR_TEST + 2].needTime = 4;
	proc_table[NR_TEST + 3].needTime = 5;
	proc_table[NR_TEST + 4].needTime = 7;
	proc_table[NR_TEST].useTime = 0;
	proc_table[NR_TEST + 1].useTime = 0;
	proc_table[NR_TEST + 2].useTime = 0;
	proc_table[NR_TEST + 3].useTime = 0;
	proc_table[NR_TEST + 4].useTime = 0;

	SetTime = 0;
}

PUBLIC int schedule_init()
{
	int i;
	struct proc *p;
	StackError = 0;
	block_queue.start = 0;
	block_queue.end = 0;
	Ring0_queue.start = 0;
	Ring0_queue.end = 0;
	task_queue1.start = 0;
	task_queue1.end = 0;
	task_queue2.start = 0;
	task_queue2.end = 0;
	task_queue3.start = 0;
	task_queue3.end = 0;
	task_queue1.give_ticks = 1;
	task_queue2.give_ticks = 2;
	task_queue3.give_ticks = 4;
	LastPriority = 1;
	TestFlag = 0;

	for (i = 0; i < NR_TASKS + NR_NATIVE_PROCS; i++)
	{
		if (i < NR_TASKS + 1)
		{ /* TASK + INIT*/
			p = &FIRST_PROC + i;
			p->ticks = p->priority;
		}
		else if (i < NR_TEST)
		{ /* Other USER PROC */
			task_queue1.proc_pid[task_queue1.start++] = i;
		}
	} //目前将除测试进程外的进程加入队列
	p = &FIRST_PROC + task_queue1.proc_pid[task_queue1.end];
	p->ticks = task_queue1.give_ticks;

	test_init();
	return 0;
}

void init_sceen()
{
	int i;
	disp_pos = 0;
	for (i = 0; i < 80 * 85; i++)
	{
		disp_str(" ");
	}
	disp_pos = 0;
	disp_str("Os is init success, beging running\n");
}
/*****************************************************************************
 *                               kernel_main
 *****************************************************************************/
/**
 * jmp from kernel.asm::_start. 
 * 
 *****************************************************************************/
PUBLIC int kernel_main()
{

	int i, j, eflags, prio;
	u8 rpl;
	u8 priv; /* privilege */

	struct task *t;
	struct proc *p = proc_table;
	//int AslrOffset = ASLR(1);
	char *stk = task_stack + STACK_SIZE_TOTAL;
	Aslr_flag = 1;			//1表示打开ASLR

	for (i = 0; i < NR_TASKS + NR_PROCS; i++, p++, t++)
	{
		if (i >= NR_TASKS + NR_NATIVE_PROCS)
		{
			p->p_flags = FREE_SLOT;
			continue;
		}

		if (i < NR_TASKS)
		{ /* TASK */
			t = task_table + i;
			priv = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
			prio = 15;
		}
		else
		{ /* USER PROC */
			t = user_proc_table + (i - NR_TASKS);
			priv = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202; /* IF=1, bit 2 is always 1 */
			prio = 5;
		}

		strcpy(p->name, t->name); /* name of the process */
		p->p_parent = NO_TASK;
		if (i < NR_TASKS + 1) //包含init进程（交互性强，需要内核调度）
		{
			p->ring = 0;
			p->testProc = 0;
			p->ticks = prio;
		}
		else
		{
			p->ring = 1;
			if (i < NR_TEST)
			{
				p->testProc = 0;
			}
			else
			{
				p->testProc = 1; //这是测试进程
			}
		}
		if (strcmp(t->name, "INIT") != 0)
		{
			p->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
			p->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];
			;
			/* change the DPLs */
			p->ldts[INDEX_LDT_C].attr1 = DA_C | priv << 5;
			p->ldts[INDEX_LDT_RW].attr1 = DA_DRW | priv << 5;
		}
		else
		{ /* INIT process */
			unsigned int k_base;
			unsigned int k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);
			init_desc(&p->ldts[INDEX_LDT_C],
					  0, /* bytes before the entry point
				      * are useless (wasted) for the
				      * INIT process, doesn't matter
				      */
					  (k_base + k_limit) >> LIMIT_4K_SHIFT,
					  DA_32 | DA_LIMIT_4K | DA_C | priv << 5);

			init_desc(&p->ldts[INDEX_LDT_RW],
					  0, /* bytes before the entry point
				      * are useless (wasted) for the
				      * INIT process, doesn't matter
				      */
					  (k_base + k_limit) >> LIMIT_4K_SHIFT,
					  DA_32 | DA_LIMIT_4K | DA_DRW | priv << 5);
		}

		p->regs.cs = INDEX_LDT_C << 3 | SA_TIL | rpl;
		p->regs.ds =
			p->regs.es =
				p->regs.fs =
					p->regs.ss = (INDEX_LDT_RW << 3 | SA_TIL | rpl);
		p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p->regs.eip = (u32)t->initial_eip;
		p->regs.esp = (u32)stk;
		p->regs.eflags = eflags;

		p->priority = prio;
		p->ticks = 0;
		p->p_flags = 0;
		p->p_msg = 0;
		p->p_recvfrom = NO_TASK;
		p->p_sendto = NO_TASK;
		p->has_int_msg = 0;
		p->q_sending = 0;
		p->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p->filp[j] = 0;

		stk -= t->stacksize;
	}

	k_reenter = 0;
	ticks = 0;
	p_proc_ready = proc_table;

	schedule_init();
	init_clock();
	init_keyboard();
	init_sceen();
	if (Aslr_flag == 1)
	{
		disp_str("ASLR is running\n");
	}
	restart();

	while (1)
	{
	}
}

/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/

PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}

/**
 * @struct posix_tar_header
 * Borrowed from GNU `tar'
 */
struct posix_tar_header
{						/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100]; /* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
						/* 500 */
};

/*****************************************************************************
 *                                untar
 *****************************************************************************/
/**
 * Extract the tar file and store them.
 * 
 * @param filename The tar file.
 *****************************************************************************/
void untar(const char *filename)
{
	int fd = open(filename, O_RDWR);
	assert(fd != -1);

	char buf[SECTOR_SIZE * 16];
	int chunk = sizeof(buf);
	int i = 0;
	int bytes = 0;
	u8 origin_checknum = 0;
	struct stat s1;

	while (1) {
		read(fd, buf, SECTOR_SIZE);
		if (buf[0] == 0)
			break;
		i++;
		struct posix_tar_header * phdr = (struct posix_tar_header *)buf;

		/* calculate the file size */
		char * p = phdr->size;
		int f_len = 0;
		while (*p)
			f_len = (f_len * 8) + (*p++ - '0'); /* octal */

		int bytes_left = f_len;
		int fdout = open(phdr->name, O_CREAT | O_RDWR | O_TRUNC);
		if (fdout == -1) {
			printf("    failed to extract file: %s\n", phdr->name);
			printf(" aborted]");
			return;
		}
		printf("    %s (%d bytes)\n", phdr->name, f_len);
		
		//得到文件的相关inode值
		int ret1 = stat(phdr->name, &s1);
		if (ret1 != 0) {
			printl("stat error\n");
			return -1;
		}
		
		int all_bytes = 0;
		origin_checknum = 0;
		
		while (bytes_left) {
			int iobytes = min(chunk, bytes_left);
			read(fd, buf,
			     ((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			     
			//进行文件的校验值计算
			for(int j = 0; j < ((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE; j++) {
				origin_checknum = origin_checknum ^ buf[j];
			}
			write(fdout, buf, iobytes);
			bytes_left -= iobytes;
		}
		
		//文件初始校验值存储
		check_value[s1.st_ino] = origin_checknum;
		printl("inode: %d original checksum : %d\n", s1.st_ino, origin_checknum);
		close(fdout);
	}

	close(fd);

	printf("----done, %d files extracted\n", i);
}

/*****************************************************************************
 *                                shabby_shell
 *****************************************************************************/
/**
 * A very very simple shell.
 * 
 * @param tty_name  TTY file name.
 *****************************************************************************/

void shabby_shell(const char *tty_name)
{
	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];

	while (1)
	{
		//write(1, "$ ", 2);
		int r = read(0, rdbuf, 70);
		rdbuf[r] = 0;

		int argc = 0;
		char *argv[PROC_ORIGIN_STACK];
		char *p = rdbuf;
		char *s;
		int word = 0;
		char ch;
		do
		{
			ch = *p;
			if (*p != ' ' && *p != 0 && !word)
			{
				s = p;
				word = 1;
			}
			if ((*p == ' ' || *p == 0) && word)
			{
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		} while (ch);
		argv[argc] = 0;

		// 声明一个二维数组，用于存放指令
		char *proc_argv[MAX_PROC][MAX_PROC_STACK];
		int proc_num = 1;
		int proc_argc = 0;
		int error = 0;

		// 将指令分开放入准备好的数组中
		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "&"))
			{
				proc_argv[proc_num - 1][proc_argc++] = argv[i];
			}
			else
			{
				proc_argv[proc_num - 1][proc_argc] = 0;
				proc_num++;
				proc_argc = 0;
				if (proc_num > MAX_PROC)
				{
					error = 1;
					printf("Too much Proc command!\n");
				}
			}
		}

		// 如果输入的指令集合法
		if (error == 0)
		{
			int tmp_pid[proc_num];
			int pid = -1;
			int cmd_error = 0;

			for (int i = 0; i < proc_num; i++)
			{
				int fd = open(proc_argv[i][0], O_RDWR);
				if (fd == -1)
				{
					cmd_error = 1;
					break;
				}
				close(fd);
			}

			int num;
			if (cmd_error)
			{
				write(1, "{", 1);
				write(1, rdbuf, r);
				write(1, "}\n", 2);
			}
			else
			{
				for (num = 0; num < proc_num; num++)
				{
					pid = fork();

					if (pid == 0)
					{
						break;
					}
				}
			}

			if (cmd_error == 0)
			{
				if (pid != 0)
				{
					//for (int j = 0; j < proc_num; j++)
					//{
					//	int s;
					//	wait(&s);
					//}
					//int s;
					//swait(&s);
				}
				else
				{
					execv(proc_argv[num][0], proc_argv[num]);
				}
			}
		}
	}

	close(1);
	close(0);
}


/*****************************************************************************
 *                                Init
 *****************************************************************************/
/**
 * The hen.
 * 
 *****************************************************************************/
void Init()
{
	int fd_stdin = open("/dev_tty0", O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assert(fd_stdout == 1);

	printf("Os tty is running success\n");
	printf("ALL Os proc is running success\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	/* extract `cmd.tar' */
	untar("/cmd.tar");

	printf("Now you can enter CTRL+Fn to tty\n");

	char *tty_list[] = {"/dev_tty1", "/dev_tty2"};

	int i;
	for (i = 0; i < sizeof(tty_list) / sizeof(tty_list[0]); i++)
	{
		int pid = fork();
		if (pid != 0)
		{	/* parent process */
			// printf("[parent is running, child pid:%d]\n", pid);
		}
		else
		{ /* child process */
			// printf("[child is running, pid:%d]\n", getpid());
			close(fd_stdin);
			close(fd_stdout);

			shabby_shell(tty_list[i]);
			assert(0);
		}
	}

	while (1)
	{
		int s;
		int child = wait(&s);
		// printf("child (%d) exited with status: %d.\n", child, s);
	}

	assert(0);
}

void SYSTEM()
{
	while (1)
	{
		// I don't do nothing now
	}
}
/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	while (1)
	{
		//disp_color_str("hello,I am proc D. now use CPU times: ", BRIGHT | MAKE_COLOR(BLACK, NEW2));
		//milli_delay(400);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	while (1)
	{
		//milli_delay(400);
	}
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	while (1)
	{

		//milli_delay(400);
	}
}

void TestD()
{
	while (1)
	{
		//milli_delay(400);
	}
}

void TestE()
{
	while (1)
	{
		//milli_delay(400);
	}
}

void TestF()
{
	while (1)
	{
		//milli_delay(400);
	}
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char *)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}
