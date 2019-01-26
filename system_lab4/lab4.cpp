#include <unistd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <pwd.h>

void getRWX(const unsigned short m);

void printdir(char *dir, int depth){
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if  ((dp = opendir(dir)) ==NULL){
        printf("fail to open the dir");
        return;
    }
    chdir(dir);
    while((entry = readdir(dp))!=NULL){
      //以该目录项的名字为参数,调用lstat得到该目录项的相关信息;
      lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)){
            if(strcmp(entry->d_name,"..")==0||strcmp(entry->d_name,".")==0)
               	continue;
           // 打印目录项的深度、目录名等信息
           // 递归调用printdir,打印子目录的信息,其中的depth+4;
              getRWX(statbuf.st_mode);
              printf("%d   ",depth);   //深度
               struct passwd *pwd;
               pwd = getpwuid(statbuf.st_uid);
               printf("%s   ",pwd->pw_name);  //文件所有者ID

               struct passwd *gpwd;
               if((gpwd = getpwuid(statbuf.st_gid))!=NULL)
              {
              printf("%s\t",gpwd->pw_name);  //文件所有者组ID
              }
              printf("%ld\t",statbuf.st_size); //文件大小

              printf("%s\t",strtok(ctime(&statbuf.st_mtime)+4,"\n")); //最后修改内容时间
	          printf("%s\n",entry->d_name);
              printdir(entry->d_name,depth+4);
              }else{
             // printf("%d\t",statbuf.st_mode);
              getRWX(statbuf.st_mode);
              printf("%d   ",depth);   //深度
               struct passwd *pwd;
               pwd = getpwuid(statbuf.st_uid);
               printf("%s   ",pwd->pw_name);  //文件所有者ID
              struct passwd *gpwd;
              if((gpwd = getpwuid(statbuf.st_gid))!=NULL)
              {
              printf("%s\t",gpwd->pw_name);  //文件所有者组ID
              }
              printf("%ld\t",statbuf.st_size); //文件大小
              printf("%s\t",strtok(ctime(&statbuf.st_ctime)+4, "\n")); //最后修改内容时间
              printf("%s\n",entry->d_name);
              }
              }
    chdir("..");
    closedir(dp);
}


void getRWX(const unsigned short m)
{
    if (m& S_IRUSR)
       printf("r");
   else
       printf("-");

    if (m & S_IWUSR)
         printf("w");
    else
        printf("-");

    if (m & S_IXUSR)
         printf("x");
    else
        printf("-");

    if (m & S_IRGRP)
        printf("r");
    else
        printf("-");

    if (m & S_IWGRP)
         printf("w");
    else
        printf("-");

    if (m & S_IXGRP)
        printf("x");
    else
        printf("-");

    if (m & S_IROTH)
        printf("r");
    else
        printf("-");

    if (m & S_IWOTH)
        printf("w");
    else
        printf("-");

    if (m & S_IXOTH)
        printf("x   ");
    else
        printf("-   ");
        return;
}

int main(){
    char dirr[50];
	printf("please input the dir name\n");
	scanf("%s",dirr);
	printdir(dirr,1);
	return 0;

}
 
