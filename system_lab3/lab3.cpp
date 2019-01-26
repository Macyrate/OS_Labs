#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>

const size_t CACHE_LENGTH = 1020;


//采用信号灯机制实现p、v操作的数据结构和函数
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

int P(int semid, int semnum) {
	struct sembuf sops = {semnum, -1, SEM_UNDO};
	return (semop(semid, &sops, 1));
}

int V(int semid, int semnum) {
	struct sembuf sops = {semnum, +1, SEM_UNDO};
	return (semop(semid, &sops, 1));
}

int main(int argc, char *argv[]){

	//定义共享缓冲区及其信号灯
	int ringbuf_id = shmget(IPC_PRIVATE, CACHE_LENGTH + sizeof (unsigned), IPC_CREAT | IPC_EXCL | 0666);
	int rbuf_empty = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
	int rbuf_max = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
	int rbuf_mutex = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
	int part_over = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
	int over = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
	int get_id = 0;
	int put_id = 0;

	if (( rbuf_empty < 0 )||( rbuf_max < 0 )||( rbuf_mutex < 0 )||( part_over < 0) || ( over < 0))
	{
		printf( "semget error!!! \n");
		printf("errno is: %d\n",errno);
		exit (1 );
	}

	union semun semopts;
	FILE *out;
	FILE *in;

	//打开源和目标文件
	if (argc != 3) {
		puts("arguments error");
		return 0;
	}
	if ((in = fopen(argv[1], "rb")) == NULL) {
		puts("can't open input file");
		return 0;
	}
	if ((out = fopen(argv[2], "wb")) == NULL) {
		puts("can't open output file");
		fclose(in);
		return 0;
	}
	
	//信号灯赋值
	semopts.val = CACHE_LENGTH;
	semctl(rbuf_empty, 0, SETVAL, semopts);

	semopts.val = 1;
	semctl(rbuf_mutex, 0, SETVAL, semopts);

	semopts.val = 0;
	semctl(rbuf_max, 0, SETVAL, semopts);
	semctl(part_over, 0, SETVAL, semopts);
	semctl(over, 0, SETVAL, semopts);

   	//get
	if ((get_id = fork()) == 0) {
		unsigned char* s = (unsigned char*)shmat(ringbuf_id, 0, 0);
		char ch;
		int i = 0;
        	puts("get:start");
		do {
			P(rbuf_empty, 0);
			P(rbuf_mutex, 0);

			//加入内容
			if(i >= CACHE_LENGTH){
                		i = 0;
				shmdt(s);
				s = (unsigned char*)shmat(ringbuf_id, 0, 0);
			}

			ch = fgetc(in);
			*s = ch;
			printf("get:%c\n", *s);

			V(rbuf_mutex, 0);
			V(rbuf_max, 0);

			if (ch == EOF) break;
            		else{
				s++;
                		i++;
            		}

		} while (1);
		fclose(in);
		shmdt(s);

		//同步结束进程
		P(part_over,0);
		P(over, 0);

		puts("get:ended");
		return 0;
	}

	//put
	if ((put_id = fork()) == 0) {
		unsigned char * s = (unsigned char*)shmat(ringbuf_id, 0, 0);
		int i = 0;
		char ch;
		puts("put:start");
        	do {
            		P(rbuf_max, 0);
			P(rbuf_mutex, 0);

		//加入内容
			if(i >= CACHE_LENGTH){
				i = 0;
				shmdt(s);
				s = (unsigned char*)shmat(ringbuf_id, 0, 0);
			}

            		ch = *s;
            		printf("put:%c\n", ch);

            		V(rbuf_mutex, 0);
			V(rbuf_empty, 0);

            		if(ch != EOF){
                		fputc(ch, out);
            		}

            		if (ch == EOF) break;
            		else{
				s++;
 				i++;
			}
        	} while (1);
        	shmdt(s);
        	fclose(out);

		//同步结束进程
		V(over, 0);
		V(over, 0);

        	puts("put:ended");
        	return 0;
	}

	wait(0);	wait(0);

//释放缓冲区和信号灯
	shmctl(ringbuf_id, IPC_RMID, 0);
	semctl(rbuf_empty, 0, IPC_RMID, 0);
	semctl(rbuf_mutex, 0, IPC_RMID, 0);
	semctl(rbuf_max, 0, IPC_RMID, 0);
	semctl(part_over, 0, IPC_RMID, 0);
	semctl(over, 0, IPC_RMID, 0);

	return 0;
}