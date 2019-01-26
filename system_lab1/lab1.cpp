//头文件，包含了各种系统调用的库
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

//全局变量声明，包括一些输出信息和管道指针等。
pid_t pid1;
pid_t pid2;
char killed1[40] = "Child Process l is Killed by Parent!\n";
char killed2[40] = "Child Process 2 is Killed by Parent!\n";
char buf[30];
int n=1;
int fd[2];
int *write_fd = &fd[1];
int *read_fd = &fd[0];
int status;

//主进程收到SIGINT信号后的处理函数
void shutall(int sig_no){
	kill(pid1,SIGUSR1);		//向两个子进程发送自定义SIGUSR1信号
	usleep(1000);	//略微错开时间，以使子进程1先结束
	kill(pid2,SIGUSR1);
}

//子进程1收到SIGUSR1信号后的处理函数
void down1(int sig_no){
	printf("%s",killed1);	//输出进程结束信息
	usleep(300);
	kill(pid1,SIGKILL);		//发送自杀信号，结束进程
}

//子进程2收到SIGUSR1信号后的处理函数
void down2(int sig_no){
	printf("%s",killed2);	//输出进程结束信息
	usleep(300);
	kill(pid2,SIGKILL);		//发送自杀信号，结束进程
}

//子进程1行为
int child1(){
	signal(SIGINT,SIG_IGN);
	signal(SIGUSR1,down1);
	while(1){
		sprintf(buf,"I send you %d times.\n",n);
		write(fd[1],buf,30);	//将字符串送入管道
		n++;
		sleep(1);	//休眠1秒
	}
}

//子进程2行为
int child2(){
	signal(SIGINT,SIG_IGN);
	signal(SIGUSR1,down2);
	while(1){
		usleep(300000);		//休眠0.3秒后进行read
		read(fd[0],buf,30);
		printf("%s",&buf);
		usleep(700000);		//休眠0.7秒，则一周期为1秒
	}
}

int main(void){
	signal(SIGINT,shutall);
	if(pipe(fd)==-1){
		perror("make pipe");
		exit(-1);
	}
	pid1=fork();	//fork两个子进程
	pid2=fork();

	if(pid1==0&&pid2!=0) child1();		//判定是哪个子进程，以执行相应行为函数
	if(pid2==0&&pid1!=0) child2();

	waitpid(pid1,&status,0);	//等待子进程结束
	waitpid(pid2,&status,0);
	close(fd[0]);	//关闭管道
	close(fd[1]);
	return 0;	//主进程结束
}



