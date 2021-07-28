#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define P 4
#define TN 8
#define NIL ((List)0)

typedef struct cell {
  int element;
  struct cell *next;
} Cell,*List;

typedef int(*INTFUNCP)(int);


List createList(void) {
  return NIL;
}

List insertList(int element,List l) {
  List temp=malloc(sizeof(Cell));
  temp->element=element;
  temp->next=l;
  return temp;
}

int nullpList(List l) {
  return (l==NIL);
}

int headList(List l) {
  return l->element;
}

List tailList(List l) {
  return l->next;
}

int tailarray(List l,int i){
  return i++;
}


List appendList(List l1,List l2){

  if (nullpList(l1)){
    return l2;
  }else{
    return insertList(headList(l1),appendList(tailList(l1),l2));

  }

}

int iList(List l1,int i){

  if(i==0){
    return headList(l1);
  }else{
    return iList(tailList(l1),i-1);
  }

}

void printList(List l) {

  if (nullpList(l)){ 
    printf("\n"); 
  }else{
    printf("%d ",headList(l));
    printList(tailList(l));
  }
}

List firstnList(List l,unsigned int n){

  if(nullpList(l) || n==0){  //lが空でないまたはnが0のとき空のリストを返す
    return createList();
  }else{  //それ以外の時lの先頭要素をfirstnListに書き込む
    return insertList(headList(l),firstnList(tailList(l),--n));
  }
}

List restnList(List l,unsigned int n){
  if(nullpList(l) || n==0){  //lが空でないまたはnが0のとき空のリストを返す
    return l;
  }else{  //それ以外の時先頭の要素を取り除く
    restnList(tailList(l),--n);
  }
}


List setList(List sourceList,List subsetList,int begin,int end){
  List p=createList();
  List temp=createList();
  int i=0;

  for(i=begin;i<end;i++){
    temp=appendList(subsetList,insertList(iList(sourceList,i),createList()));
    //printf("temp:");
    //printList(temp);
    if(end+1<=TN){
      //printf("再帰前:\n");
      //printf("i=%d,end=%d\n",i,end);
      p=appendList(p,setList(sourceList,temp,i+1,end+1));
    }else{
      p=appendList(p,temp);
      //printf("一回目:");
      //printList(p);
    }
  }
  //printf("return前:");
  //printList(p);
  return p;
}

int calcNumOfCombination(int n, int r){
    int num = 1;
    for(int i = 1; i <= r; i++){
        num = num * (n - i + 1) / i;
    }
    return num;
}

int main(void){
  int i,j;
  List setP=createList();
  List p=malloc(sizeof(Cell));
  List z=createList();
  List kumi=createList();

  for(i=TN-1;i>=0;i--){
    setP=insertList(i,setP);
  }

  printf("タスク番号：");
  printList(setP);

  p=setList(setP,createList(),0,TN-P+1);

  printf("組み合わせ：");
  printList(p);

  for(i=0;i<calcNumOfCombination(TN,P);i++){
    kumi=firstnList(p,P);
    p=restnList(p,P);
    printf("kumi%d:",i);
    printList(kumi);
  }
  return 0;
}