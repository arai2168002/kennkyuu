/*Machigasira Yuki*/
/*もしかしたらいらないヘッダファイルもあるかも・・・*/
/*実験に用いるタスクセットの生成方法は"EDZL Schedulability Analysis in Real-TIme Multicore Scheduling"から引用*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <windows.h>
#include "MT.h"

#define P 4											//プロセッサ数
#define CM 1										//破棄条件の調節
#define del_co CM*P									//タスクセットの破棄条件
#define paramater 0.7								//指数分布のパラメータ

#define S 1024

/*タスクセットパラメータ関連*/
double Utilization[S];								//タスク利用率
int TN = 0;											//タスク数
double U = 0;										//プロセッサ利用率

/*ファイル関連*/
char file_mem[256];
char *file_ut = "rand_utilization_tasks.txt";		//タスク利用率
char *file_tl = "TS_utilization.txt";				//タスクセットの利用率
FILE *fut;
FILE *ftl;

struct timeval tv;

void rand_utilization_decision(int task_num);		//タスク利用率を指数分布に従い決定する関数
void step1();

/*乱数変換および生成*/
double Uniform(void);
double rand_exp(double lambda);
double rand_normal(double mu,double sigma);


int main(void) {

	int i, j;
	
	/*ファイル読み込み用*/
	int err = 0;
	char line_ut[S];
	char *p;
	
	double file_utilization;
	
	printf("\n\nGenerating random numbers (%d-Processor) \n", P);
	
	if((fut = fopen(file_ut,"r")) == NULL){
		
		step1();
		
	}else{	
	
		i = 0;
		
		/*タスク利用率格納*/
		while(fgets(line_ut,S,fut) != NULL){
			sscanf(line_ut,"%lf %[^\r\n]", &file_utilization);
			Utilization[i] = file_utilization;      //タスクiの利用率を格納
			TN++;									//タスク数
			p = strtok(line_ut," \r\n\t");      	//最初分解対象を取得
			while(p != NULL){           			//line_utにpがなくなるまで繰り返す  
				p = strtok(NULL," \r\n\t");   		//次のpを取得
			}
			i++;
		}
		
		/*現在時刻による乱数の初期化*/
		gettimeofday(&tv,NULL);
		init_genrand(tv.tv_sec + tv.tv_usec);
		
		rand_utilization_decision(TN);
		
		U = 0;
	
		for(i=0;i<TN+1;i++)
			U += Utilization[i];
		
		if(U > del_co){
			
			step1();
			
		}else{
			
			/*ファイル書き込み*/
			fut = fopen(file_ut,"a");
			ftl = fopen(file_tl,"a");
			fprintf(fut, "%lf\n", Utilization[TN]);
			fprintf(ftl, "%lf\n", U);
			fclose(fut);
			fclose(ftl);
			
			/*パラメータ出力*/
			for(i=0;i<TN+1;i++)
				printf("Utilization[task%d] = %lf\n", i+1, Utilization[i]);

		}
		
	}
	
	return 0;
	
}


/*詳しくは"EDZL Schedulability Analysis in Real-TIme Multicore Scheduling"の5章*/
void step1(){
	
	int i,j;

	
	/*ファイルの初期化*/
	remove(file_ut);
	
	/*タスク数の初期化*/
	TN = P + 1;
	
	U = 10000;
	
	while(U > del_co){
		
		for(i=0;i<TN;i++)
			rand_utilization_decision(i);
		
		U = 0;
		
		for(i=0;i<TN;i++)
			U += Utilization[i];
		
	}
	
	/*パラメータ出力*/
	for(i=0;i<TN;i++)
		printf("Utilization[task%d] = %lf\n", i+1, Utilization[i]);
	
	/*ファイル書き込み*/
	fut = fopen(file_ut,"a");
	ftl = fopen(file_tl,"a");
	
	for(i=0;i<TN;i++)
		fprintf(fut, "%lf\n", Utilization[i]);
	
	fprintf(ftl, "%lf\n", U);

	fclose(fut);
	fclose(ftl);
	
}


/*タスク利用率を指数分布に従い決定する関数*/
void rand_utilization_decision(int task_num){
	
	double lam;							//λ（1/平均）
	
	lam = 1 / paramater;
	//printf("%lf\n", lam);

	/*現在時刻による乱数の初期化*/
	gettimeofday(&tv,NULL);
	init_genrand(tv.tv_sec + tv.tv_usec);
	
	Utilization[task_num] = 3;
	while(Utilization[task_num] > 1)
		Utilization[task_num] = rand_exp(lam);
	
}


/*Mersenne Twisterによる一様乱数生成*/
double Uniform(void){
	return genrand_real3();
}


/*一様分布から指数分布へ(逆関数法)*/
double rand_exp(double lambda){
   return -log(Uniform())/lambda;
}


/*一様乱数から正規分布へ(ボックス=ミュラー法)*/
double rand_normal(double mu,double sigma){
  double z = sqrt(-2.0*log(Uniform())) * sin(2.0*M_PI*Uniform());
  return mu + sigma * z;
}