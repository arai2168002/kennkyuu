#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

#define P 2
//#define P 2  				//プロセッサ数
//#define P 1  				//プロセッサ数
#define MAX 1000000000 //起動していない時
//#define alpha 1000.0		//換算レート
#define NIL ((List)0)
#define LNIL ((LList)0) //修正提案 (2022.11.25 中田)
#define S 1024
#define FIXEDPOINT 2	//固定小数に変換するために2進数においてシフトする回数

typedef long long FIXPOINTDECIMAL;		//long long型をまとめて宣言
#define ITFD(x) ((FIXPOINTDECIMAL)(x << FIXEDPOINT))	//int型の数値を固定小数に変換


typedef struct cell {
  FIXPOINTDECIMAL element;
  struct cell *next;
} Cell,*List;

typedef struct llcell {
  List element;
  struct llcell *next;
} LLCell,*LList;



void *thread_Tasks(void *num);			//タスク
void *thread_LMCLF();					//LMCLF Schedular

void LMCLF();  							//優先度関数値をソートする関数
void memory_recode(int Memory);			//最悪メモリ消費量を記憶する関数

void calc();							//Taskの処理
void load();							//計算負荷

int pthread_yield(void);				//コンパイラに警告を出させないためのプロトタイプ宣言である.なお,この関数はPOSIXでは非標準であり,sched_yield()を使うのが正しいとある.

int get_digit(int n);                //桁数を知る

List createList(void);				//空のリストを作成
LList createList2(void);
List insertList(FIXPOINTDECIMAL element,List l);	//リストの先頭に要素を追加する
LList insertList2(List element,LList l);
LList firstList(int element,LList l);
LList insertElementToAll(int element ,LList l);
int nullpList(List l);				//リストが空かどうかを返す
int nullpList2(LList l);
int headList(List l);				//リストの先頭要素を返す
List tailList(List l);				//リストの先頭要素を削除
List headList2(LList l);  // 修正提案（プロトタイプ宣言追加） (2022.11.25 中田)
LList tailList2(LList l);
int tailarray(List l,int i);		//
List appendList(List l1,List l2);	//リストl1の末尾にlリストl2を連結
LList appendList2(LList l1,LList l2);
int iList(List l1,int i);			//リストのi番目の要素を返す
void printList(List l);				//リストの要素を表示
void fprintList(List l);            //リストの全要素をファイルに出力
void fprintList2(LList l);
void fprintFPList(List l);			//リストの全要素文字列をファイルに出力
int lengthList(List l);				//リストの長さを取得
int lengthList2(LList l);	
List firstnList(List l,unsigned int n);		//リストの先頭からn番目までの要素を削除
List restnList(List l,unsigned int n);		//リストの末尾からn番目までの要素を削除
int memberList(int element,List l);     //リストにその要素が含まれているかどうかを調べる関数
int memberList2(List l);
void freeList(List l);		//リストのメモリ解放
List copyList(List l);		//リストの複製を生成
int minList(List l,int tasknum1,int tasknum2,FIXPOINTDECIMAL min);	//リストの評価値の最小値のタスク番号を返す
List ideleatList(List l1,List l2,int i);		//先頭からi番目の要素を書き換える
List setList(List sourceList,List subsetList,int begin,int end);		//組み合わせの全パターンを格納したリストを返す
LList Makesubsetset(List T,int i,int j);		//特定の要素数の部分集合を列挙
List subsetset(int n,List T);		//要素nを各部分集合に追加
int calcNumOfCombination(int n, int r);		//組み合わせの総数を返す
int shiftoperationtimes(FIXPOINTDECIMAL n);		//シフト演算により桁数を取得
FIXPOINTDECIMAL FTFD(double x);				//double型の数値を固定小数に変換
char *FixPointDecimalToString(FIXPOINTDECIMAL x);		//固定小数の値を整数部分と小数部分とで文字列に変換


int main(void){
    int i,j;
    int t=5;
    List set1=createList();
    LList Aset1=createList2();

    for(i=0;i<t;i++){//評価値が小さいタスク番号を格納
			set1=insertList(i,set1);
		}

    Aset1=Makesubsetset(set1,2,3);
    fprintf(stderr,"step1タスク番号集合:\n"); fprintList2(Aset1);


        return 0;

}





//空のリストを生成
List createList(void){
  	return NIL;
}


LList createList2(void){
    return (LList)0;
}

//リストの先頭に要素を入れる
List insertList(FIXPOINTDECIMAL element,List l) {
	List temp=malloc(sizeof(Cell));
  	temp->element=element;
	  	//fprintf(stderr,"temp->element:%lld ",temp->element); 
  	temp->next=l;
	  	//fprintf(stderr,"temp:"); fprintList(temp);
  	return temp;
}

//リストのリストの全要素を入れる
LList insertList2(List element,LList l) {
	LList temp=malloc(sizeof(Cell));
  	temp->element=element;

  	temp->next=l;

  	return temp;


}




/* 上記関数の修正の提案(関数名も含めて) (2022.11.25 中田) */
LList insertElementToAll(int element,LList l) {
  if(nullpList2(l)) {
    return l;
  } else {
    return insertList2(insertList(element,headList2(l)), 
		insertElementToAll(element,tailList2(l)));
  }
}




//
int nullpList(List l) {
  	return (l==NIL);
}

int nullpList2(LList l) {   // 修正提案(typo修正) (2022.11.25 中田)
  //return (l->element==NIL);
  return (l==LNIL); // 修正提案 (2022.11.25 中田)
}

//リストの先頭要素を返す
int headList(List l) {
  	return l->element;
}

//先頭要素を削除する
List tailList(List l) {
  	return l->next;
}

/* 修正提案（関数追加） (2022.11.25 中田) */
List headList2(LList l) {
  return l->element;
}


LList tailList2(LList l){
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

LList appendList2(LList l1,LList l2){
  if /* (nullpList(l1->element)) */
    (nullpList2(l1))	  {  //修正提案 (2022.11.25 中田)
    	return l2;
  	}else{
    return insertList2(/* l1->element */ headList2(l1), //修正提案 (2022.11.25 中田) 基本的にリスト処理関数の形は要素の型に関わらず変わらない
		       appendList2(tailList2(l1),l2));
    }
}

//先頭からi番目の要素を返す
int iList(List l1,int i){

  	if(i==0){
    	return headList(l1);
  	}else{
    	return iList(tailList(l1),i-1);
  	}

}

//先頭からi番目の要素を書き換える
List ideleatList(List l1,List l2,int i){
  	if(i==0){
		l1->element=MAX;
    	return l2;
  	}else{
    	return ideleatList(tailList(l1),l2,i-1);
  	}

}
//リストの全要素を表示
void printList(List l) {

  	if(nullpList(l)){ 
    	printf("\n"); 
  	}else{
    	printf("%lld ",headList(l));
    	printList(tailList(l));
  	}
}

//リストの全要素をファイル出力
void fprintList(List l) {

  	if(nullpList(l)){ 
    	fprintf(stderr, "\n"); 
  	}else{
            	fprintf(stderr, "%lld ", headList(l));
    	fprintList(tailList(l));
  	}
}

void fprintList2(LList l){
    if(nullpList2(l)){
        fprintf(stderr, "\n");
    }else{
        fprintList(headList2(l));
    	fprintList2(tailList2(l));
    }
}

void fprintFPList(List l) {

  	if(nullpList(l)){ 
    	fprintf(stderr, "\n"); 
  	}else{
    	fprintf(stderr, "%s ", FixPointDecimalToString(headList(l)));
    	fprintList(tailList(l));
  	}
}

//先頭からn番目までの要素をリストに入れて返す
List firstnList(List l,unsigned int n){

  	if(nullpList(l) || n==0){  //lが空でないまたはnが0のとき空のリストを返す
    	return createList();
  	}else{  //それ以外の時lの先頭要素をfirstnListに書き込む
    	return insertList(headList(l),firstnList(tailList(l),--n));
  	}
}



//先頭からn番目までの要素を削除
List restnList(List l,unsigned int n){
  	if(nullpList(l) || n==0){  //lが空でないまたはnが0のとき空のリストを返す
    	return l;
  	}else{  //それ以外の時先頭の要素を取り除く
    	restnList(tailList(l),--n);
  	}
}

//リストに要素が含まれているかどうかを探索
int memberList(int element,List l){

  if(nullpList(l)){
    return 0;
  }else if(element==headList(l)){
    return 1;
  }else{
    return memberList(element,tailList(l));
  }

}

//リスト空であるかどうか確認
int memberList2(List l){

	if(nullpList(l)){
		return 0;
	}else{
		return 1;
	}
}

//リストのメモリ解放
void freeList(List l){
	if(nullpList(l)){
		return;
	}else{
		freeList(tailList(l));
		free(l);
		return;
	}

}

//リストの総数を数える
int lengthList(List l){

  if(nullpList(l)){
    return 0;
  }else{
    return lengthList(tailList(l))+1;
  }
}

int lengthList2(LList l){

  if(nullpList2(l)){
    return 0;
  }else{
    return lengthList2(tailList2(l))+1;
  }
}
//リストの複製を作成
List copyList(List l){

	return firstnList(l,lengthList(l));

}

//リストの評価値が最小であるタスク番号を返す関数
int minList(List l,int tasknum1,int tasknum2,FIXPOINTDECIMAL min){
	if(nullpList(l)){
		return tasknum2;
	}else if(headList(l)<min){
		min=headList(l);
		tasknum2=tasknum1;
		return minList(tailList(l),++tasknum1,tasknum2,min);
	}else{
		return minList(tailList(l),++tasknum1,tasknum2,min);
	}
}

//選ばれるタスクの選定
List setList(List sourceList,List subsetList,int begin,int end){
  	List p=createList();
	List oldp=createList();
  	List temp=createList();
  	int i=0;

  	for(i=begin;i<end;i++){
    	temp=copyList(appendList(subsetList,insertList(iList(sourceList,i),createList())));
    	if(end+1<=lengthList(sourceList)){
			oldp=p;
      		p=copyList(appendList(p,setList(sourceList,temp,i+1,end+1)));
			freeList(oldp);
			freeList(temp);
    	}else{
			oldp=p;
      		p=copyList(appendList(p,temp));
			freeList(oldp);
			freeList(temp);
    	}
  	}
  	return p;
}

//部分集合の集合を求める
LList Makesubsetset(List T,int i,int j){
    LList subsetList=createList2();
    LList subsubsetList=createList2();
    LList temp=createList2();

    if(nullpList(T)){
      return (i<=0 && 0<=j)?(insertList2(createList(),createList2())):(createList2());  /* 修正提案 (2022.11.25 中田) 「空集合１つを要素として持つ集合」と「空集合」は違うことに注意 */
    }else{
        temp=Makesubsetset(tailList(T),i-1,j-1);
        subsubsetList=Makesubsetset(tailList(T),i,j);

        subsetList=insertElementToAll(headList(T),temp);

        return appendList2(subsetList,subsubsetList);
    }

}



//組み合わせの総数を求める
int calcNumOfCombination(int n, int r){
    int num = 1;
	
	if(n<=r){
		return 1;
	}else{
		for(int i = 1; i <= r; i++){
        	num = num * (n - i + 1) / i;
		}
		return num;
    }
}

//シフト演算によりシフトする回数を取得
int shiftoperationtimes(FIXPOINTDECIMAL n){
	
	if(n==0 || n==-1){
		return 0;
	}else{
		return 1 + shiftoperationtimes(n >> 1);
	}
}

FIXPOINTDECIMAL FTFD(double x){
	return (FIXPOINTDECIMAL)(x*pow(2,FIXEDPOINT));
}

char *FixPointDecimalToString(FIXPOINTDECIMAL x){
	int ipart=0,fpart=0;
	char buf[S];

	ipart= x >>FIXEDPOINT;
	fpart= x - (ipart << FIXEDPOINT);

	snprintf(buf,sizeof(buf),"%d.%0d",ipart,fpart);

	return strdup(buf);
}
