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

#define P 4
//#define P 1										//プロセッサ数
#define CM 1										//破棄条件の調節
#define del_co CM*P									//タスクセットの破棄条件
#define paramater 0.9								//指数分布のパラメータ

#define M_max 20000					//メモリ確保の最大値(M_max/2)
	
#define P_max 1000									//周期の最大値
#define P_min 100									//周期の最小値

#define S 1024

/*タスクセットパラメータ関連*/
int Period[S];										//各タスクの周期
double Utilization[S];								//タスク利用率
int Rand_Memory[S][S];								//各タスクの消費メモリ増分
int WCET[S];										//最悪実行時間(ステップ数)
int TN = 0;											//タスク数
double U = 0;										//プロセッサ利用率

/*ファイル関連*/
char file_mem[256];
char *file_pe = "rand_period_tasks.txt";			//本実験では周期＝デッドライン
char *file_ut = "rand_utilization_tasks.txt";		//タスク利用率
char *file_wc = "rand_wcet_tasks.txt";				//WCET
FILE *fmemory;
FILE *fpe;
FILE *fut;
FILE *fwc;

struct timeval tv;

void rand_period_decision(int task_num);			//周期を一様分布に従い決定する関数
void rand_utilization_decision(int task_num);		//タスク利用率を指数分布に従い決定する関数
void rand_wcet_decision(int task_num);				//タスクのWCETを決定する関数
void rand_mem_decision(int task_num, int TN);		//消費メモリ増分を一様分布に従い決定する関数
void step1();

/*乱数変換および生成*/
double Uniform(void);
double rand_exp(double lambda);
double rand_normal(double mu,double sigma);


int main(void) {

	int i, j;
	
	/*ファイル読み込み用*/
	int err = 0;
	char line_pe[S];
	char line_ut[S];
	char line_wc[S];
	char *p;
	
	int file_period, file_wcet;
	double file_utilization;
	double maxmem_LLF, dm_LLF;
	double deadline_miss_LLF[S];
	
	printf("\n\nGenerating random numbers (%d-Processor) \n", P);
	
	/*読み込みできるかどうか（できなければerr++）*/
	if((fpe = fopen(file_pe,"r")) == NULL)
		err++;
	
	if((fut = fopen(file_ut,"r")) == NULL)
		err++;
	
	if((fwc = fopen(file_wc,"r")) == NULL)
		err++;
	
	if(err > 0){
		
		step1();
		
	}else{	
	
		i = 0;
		
		/*周期格納*/
		while(fgets(line_pe,S,fpe) != NULL){
			sscanf(line_pe,"%d %[^\r\n]", &file_period);
			Period[i] = file_period;        		//タスクiの周期を格納
			TN++;									//タスク数
			p = strtok(line_pe," \r\n\t");      	//最初分解対象を取得
			while(p != NULL){           			//line_peにpがなくなるまで繰り返す  
				p = strtok(NULL," \r\n\t");   		//次のpを取得
			}
			i++;
		}
		
		i = 0;

		/*タスク利用率格納*/
		while(fgets(line_ut,S,fut) != NULL){
			sscanf(line_ut,"%lf %[^\r\n]", &file_utilization);
			Utilization[i] = file_utilization;      //タスクiの利用率を格納
			p = strtok(line_ut," \r\n\t");      	//最初分解対象を取得
			while(p != NULL){           			//line_utにpがなくなるまで繰り返す  
				p = strtok(NULL," \r\n\t");   		//次のpを取得
			}
			i++;
		}
		
		i = 0;

		/*WCET格納*/
		while(fgets(line_wc,S,fwc) != NULL){
			sscanf(line_wc,"%d %[^\r\n]", &file_wcet);
			WCET[i] = file_wcet;        			//タスクiのWCETを格納
			p = strtok(line_wc," \r\n\t");      	//最初分解対象を取得
			while(p != NULL){           			//line_wcにpがなくなるまで繰り返す  
				p = strtok(NULL," \r\n\t");   		//次のpを取得
			}
			i++;
		}
		
		/*現在時刻による乱数の初期化*/
		gettimeofday(&tv,NULL);
		init_genrand(tv.tv_sec + tv.tv_usec);
		
		WCET[TN] = 0;
		while(WCET[TN] < 1){
			rand_period_decision(TN);
			rand_utilization_decision(TN);
			rand_wcet_decision(TN);
		}
		
		U = 0;
	
		for(i=0;i<TN+1;i++)
			U += Utilization[i];
		
		if(U > del_co){
			
			step1();
			
		}else{
			
			/*ファイル書き込み(周期，タスク利用率，WCET)*/
			fpe = fopen(file_pe,"a");
			fut = fopen(file_ut,"a");
			fwc = fopen(file_wc,"a");
			fprintf(fpe, "%d\n", Period[TN]);
			fprintf(fut, "%lf\n", Utilization[TN]);
			fprintf(fwc, "%d\n", WCET[TN]);
			fclose(fpe);
			fclose(fut);
			fclose(fwc);
			
			sprintf(file_mem, "rand_memory_task%d.txt", TN+1);
				
			fmemory = fopen(file_mem, "a");
		
			rand_mem_decision(TN,TN);
		
			for(j=0;j<WCET[TN];j++)
				fprintf(fmemory, "%d\n", Rand_Memory[TN][j]);
		
			/*パラメータ出力*/
			for(i=0;i<TN+1;i++){
				printf("Period[task%d] = %d\n", i+1, Period[i]);
				printf("Utilization[task%d] = %lf\n", i+1, Utilization[i]);
				printf("WCET[task%d] = %d\n", i+1, WCET[i]);
			}
		}
		
	}
	
	return 0;
	
}


/*詳しくは"EDZL Schedulability Analysis in Real-TIme Multicore Scheduling"の5章*/
void step1(){
	
	int i,j;

	
	/*ファイルの初期化*/
	system("rm -f rand_memory_task*.txt");
	remove(file_pe);
	remove(file_ut);
	remove(file_wc);
	
	/*タスク数の初期化*/
	TN = P + 1;
	
	U = 10000;
	
	while(U > del_co){
		
		for(i=0;i<TN;i++){
			WCET[i] = 0;
			while(WCET[i] < 1){
				rand_period_decision(i);
				rand_utilization_decision(i);
				rand_wcet_decision(i);
			}
		}
		
		U = 0;
		
		for(i=0;i<TN;i++)
			U += Utilization[i];
		
	}
	
	/*パラメータ出力*/
	for(i=0;i<TN;i++){
		printf("Period[task%d] = %d\n", i+1, Period[i]);
		printf("Utilization[task%d] = %lf\n", i+1, Utilization[i]);
		printf("WCET[task%d] = %d\n", i+1, WCET[i]);
	}
	
	/*ファイル書き込み(周期，タスク利用率，WCET)*/
	fpe = fopen(file_pe,"a");
	fut = fopen(file_ut,"a");
	fwc = fopen(file_wc,"a");
	
	for(i=0;i<TN;i++){
		fprintf(fpe, "%d\n", Period[i]);
		fprintf(fut, "%lf\n", Utilization[i]);
		fprintf(fwc, "%d\n", WCET[i]);
	}
	
	fclose(fpe);
	fclose(fut);
	fclose(fwc);
	
	/*ファイル書き込み(消費メモリ増分)*/
	for(i=0;i<TN;i++){
		sprintf(file_mem,"rand_memory_task%d.txt", i+1);
		
		fmemory = fopen(file_mem,"a");
		
		rand_mem_decision(i,TN);
		
		for(j=0;j<WCET[i];j++)
			fprintf(fmemory, "%d\n", Rand_Memory[i][j]);
		
	}
	
	fclose(fmemory);
	
}


/*周期を一様分布に従い決定する関数*/
void rand_period_decision(int task_num){

	/*現在時刻による乱数の初期化*/
	gettimeofday(&tv,NULL);
	init_genrand(tv.tv_sec + tv.tv_usec);

	Period[task_num] = genrand_int32() % (P_max - P_min + 1) + P_min;	//範囲指定 %(最大値-最小値+1)+最小値
	
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


/*タスク利用率に合わせたWCET(ステップ数)を決定する関数*/
void rand_wcet_decision(int task_num){
	
	if(Utilization[task_num] > 1)
		Utilization[task_num] = floor(Utilization[task_num]);
	
	WCET[task_num] = floor(Period[task_num] * Utilization[task_num]);
	
}


/*消費メモリ増分を一様分布に従い決定する関数*/
/*引数：タスク番号:task_num*/
void rand_mem_decision(int task_num, int TN){
	
	double mu_mem = 0;					//平均
	double sigma_mem = 40;				//標準偏差
	int i;								//カウント用変数
	int step = 0;						//現在のステップ
	int total_mem[TN];					//総メモリ消費量
	
	for(i=0;i<TN+1;i++)
		total_mem[i] = 0;

	for(i=0;i<(WCET[task_num]-1);i++){
		
		/*現在時刻による乱数の初期化*/
		gettimeofday(&tv,NULL);
		init_genrand(tv.tv_sec + tv.tv_usec);

		//Rand_Memory[task_num][i] = rand_normal(mu_mem,sigma_mem);
		Rand_Memory[task_num][i] = (genrand_int32() % (M_max - 0 + 1) + 0) - M_max / 2;
		total_mem[task_num] += Rand_Memory[task_num][i];

		while(total_mem[task_num] < 0){

			total_mem[task_num] -= Rand_Memory[task_num][i];
			//Rand_Memory[task_num][i] = rand_normal(mu_mem,sigma_mem);
			Rand_Memory[task_num][i] = (genrand_int32() % (M_max - 0 + 1) + 0) - M_max / 2;
			total_mem[task_num] += Rand_Memory[task_num][i];
			
		}
		
		step++;
		usleep(1);						//時間を取得して乱数を発生させているため必要

	}
	
	/*帳尻合わせ*/
	Rand_Memory[task_num][step] = 0 - total_mem[task_num];
	total_mem[task_num] += Rand_Memory[task_num][step];
	
	
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