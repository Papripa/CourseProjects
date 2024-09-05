
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<math.h>
//生成了52个城市稍的序号、x坐标、y坐标、名字（这个随机生成了）
int main(){
    srand((unsigned)time(NULL));
    FILE *fp=fopen("D:\\code-c\\C-single\\蚁群算法\\coords.txt","a");
    int i=0,j;
    int x,y;
    char name[15];
    for(i=0;i<52;i++){
        x=rand()%52+1;
        y=rand()%52+1;
        for(j=0;j<10;j++)
            name[j]='a'+rand()%24;
        fprintf(fp,"%d %d %d %s\n",i,x,y,name);
    }
    fclose(fp);
}