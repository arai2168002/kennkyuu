#include<stdio.h>
#include<stdlib.h>

#define TN 8
#define P 2
int *ptn(int source[TN],int subset[TN],int begin,int end){
    int i;
    int *ip=(int*)malloc(sizeof(int) * TN);

    for(i=begin;i<end;i++){
        subset[i]=source[i];
        if(end+1<=TN){
            ip=ptn(source,subset,i+1,end+1);
        }else{
            ip=source;
        }
    }
    return ip;
}

int main(void){
    int i=0;
    int setP[TN],setQ[TN];
    int *p;

    for(i=0;i<TN;i++){
        setP[i]=i;
        setP[i]=0;
    }

    p=ptn(setP,setQ,0,TN-P+1);
    for(i=0;i<TN;i++){
    printf("%d\n",*p);
    }
    return 0;
}