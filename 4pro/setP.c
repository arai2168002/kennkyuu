#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define P 2 
#define TN 8
#define NIL ((List)0)

typedef struct cell {
  int element;
  struct cell *next;
} Cell,*List;

typedef int(*INTFUNCP)(int);

#define NIL ((List)0)

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
  if (nullpList(l)) { printf("\n"); }
  else {
    printf("%d ",headList(l));
    printList(tailList(l));
  }
}





List setList(List sourceList,List subsetList,int begin,int end){
    List p=createList();
    List temp=createList();
    int i=0;

    for(i=begin;i<end;i++){
        temp=appendList(subsetList,insertList(iList(sourceList,i),createList()));
        if(end+1<=TN){
          p=appendList(p,setList(sourceList,temp,i+1,end+1));
        }else{
          p=appendList(p,temp);
        }
    }
    return p;

}



int main(void){
  int i=0;
  List setP=createList();
  List p=malloc(sizeof(Cell));

  for(i=0;i<TN;i++){
  setP=insertList(i,setP);
  }

  
  p=setList(setP,createList(),0,TN-P+1);

  printList(p);
  return 0;
}