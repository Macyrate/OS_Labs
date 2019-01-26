//头文件，包含了各种系统调用的库
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#define LOOPS 100

//由于标准库中已删去了关于union semun的定义，此处给出规范的自定义
union semun {
	int val; /* value for SETVAL */
	struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
	unsigned short *array; /* array for GETALL, SETALL */
	struct seminfo *__buf; /* buffer for IPC_INFO */
};

int semid;
pthread_t p1,p2;
int a;

//P、V操作的定义
void P(int semid,int index){
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0; //操作标记：0或IPC_NOWAIT等
	semop(semid,&sem,1); //1:表示执行命令的个数
	return;
}

void V(int semid,int index){
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op =  1;
	sem.sem_flg = 0;
	semop(semid,&sem,1);
	return;
}

//两个线程的行为定义
void *subp2(void*){
	a = 0;
	for(int n = 0;n != (LOOPS+1);n++){		//循环累加计算
		P(semid,1);
		a = a+n;
		V(semid,0);
	}
	return 0;
}

void *subp1(void*){
	for(int n = 0;n != (LOOPS+1);n++){	
		P(semid,0);
		printf("subp1:a=%d\n",a);	//打印结果
		V(semid,1);
	}
	return 0;
}

int main(void){
	union semun semopts;
	int res;
	//信号灯集定义
	semid = semget(0,2,IPC_CREAT|0666);
	semopts.val = 0;
	res = semctl(semid,0,SETVAL,semopts);
	semopts.val = 1;
	res = semctl(semid,1,SETVAL,semopts);
	//线程创建
	pthread_create(&p1,NULL,subp1,NULL);
	pthread_create(&p2,NULL,subp2,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	semctl(semid,0,IPC_RMID,0);
	return 0;
}
