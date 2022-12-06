/*Machigasira Yuki*/
/*もしかしたらいらないヘッダファイルもあるかも・・・*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

//#define P 2  		
#define P 1			//プロセッサ数
#define MAX 1000000000		//起動していない時
#define S 1024

int TN = 0;					//タスク数
double dead_max = 0;		//相対デッドラインの最大値
int rand_memory[S][S];		//消費メモリ増分
double ET[S];				//Taskの1ステップの実行時間の平均（本実験では1ステップ1単位時間で実行）
int schedule[S];			//Taskがスケジュールされるとき1，そうでないとき0

int state[S];				//Taskが起動している場合は値を1,起動していない場合は0
int finish[S];				//タスクの処理がすべて終了していれば1，そうでなければ0

int step[S];				//現在のステップ

double save_laxity[S];		//Taskが終了した時のLaxity Time

int Worst_Memory = 0;		//最悪メモリ消費量
int Current_Memory = 0;		//現在のメモリ消費量

int deadline_miss_task[S];	//デッドラインミスの数
int Deadline_Miss = 0;		//デッドラインミスの数の総和

/*sort用変数*/
int sort_memory[S];
int sort_num_LMCF[S];

int hook[S];				//一時停止している（フックがかかっている）場合1，そうでなければ0


/*構造体の要素*/
/*Task番号(Task1は0),相対デッドライン,最悪実行時間,起動時刻,実行済みの時間,余裕時間*/
typedef struct{
	int Number;
	double Relative_Deadline;
	double WCET;
	double Release_Time;
	double Run_Time;
	double Laxity_Time;
}data;

data task_data[S];

void *thread_Tasks(void *num);			//タスク
void *thread_LMCF();					//LMCF Schedular

void LMCF();  							//消費メモリ増分をソートする関数
void memory_recode(int Memory);			//最悪メモリ消費量を記憶する関数

void calc();							//Taskの処理
void load();							//計算負荷

int pthread_yield(void);				//コンパイラに警告を出させないためのプロトタイプ宣言である.なお,この関数はPOSIXでは非標準であり,sched_yield()を使うのが正しいとある.


/*スレッド間で変数を保護するmutex*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*スレッド間で状態を判断するcond*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int main(void) {

	fprintf(stderr, "\n-------------LMCF Scheduling in %d-Processor Environment-------------\n", P);
	
	int i = 0, j = 0, k1 = 0, k2 = 0;
	
	pthread_t Task[S];
	pthread_t LMCF_scheduler;
	
	/*ファイル読み込み用変数*/
	char file_mem[256];								//消費メモリ増分のファイル
	char *file_dl = "rand_period_tasks.txt";		//周期のファイル（本実験では周期=相対デッドライン）
	FILE *fMemory;
	FILE *fdl;
	int ret, x;
	
	int ptask;										//仮のタスク数
	int pdeadline[S];								//仮のデッドライン
	int period_count = 0;							//最小公倍数までのステップで何度リリースされるか
	
	
	/*ファイル読み込み*/
	fdl = fopen(file_dl,"r");
	ptask = 0;
	while(ret = fscanf(fdl,"%d",&x) != EOF){
		pdeadline[ptask] = x;
		//fprintf(stderr, "Relative Deadline [Task%d] = %5.0lf\n", ptask+1, task_data[ptask].Relative_Deadline);
		ptask++;
	}
	
	/*相対デッドラインの最大値*/
	for(i=0;i<ptask;i++){
		if(dead_max < pdeadline[i])
			dead_max = pdeadline[i];
	}
	
	/*タスク数の決定*/
	for(j=0;j<ptask;j++){
		TN = TN + ceil(dead_max / pdeadline[j]);
		//fprintf(stderr,"%lf\n", ceil(dead_max / pdeadline[j]));
	}
	
	//fprintf(stderr,"%d\n", TN);
	
	/*Task_Dataの初期化*/
	for(i=0;i<TN;i++){
		task_data[i].Number = i;
		task_data[i].WCET = 0;
		task_data[i].Release_Time = 0;
		task_data[i].Run_Time = 0;
		task_data[i].Laxity_Time = MAX;
		
		state[i] = 0;
		deadline_miss_task[i] = 0;
		schedule[i] = 0;
		step[i] = 0;
		ET[i] = 1;
		save_laxity[i] = 0;
		hook[i] = 0;
		finish[i] = 0;
	}
	
	/*ファイル読み込み*/
	k2 = 0;
	for(i=0;i<ptask;i++){
		
		period_count = ceil(dead_max / pdeadline[i]);
		
		for(k1=0;k1<(int) period_count;k1++){
			
			/*消費メモリ増分*/
			sprintf(file_mem,"rand_memory_task%d.txt", i+1);
			fMemory = fopen(file_mem,"r");
			j = 0;
			while(ret = fscanf(fMemory,"%d",&x) != EOF){
				rand_memory[k2][j] = x;
				j++;
			}
			
			/*周期*/
			task_data[k2].Relative_Deadline = pdeadline[i];
			
			/*リリース*/
			task_data[k2].Release_Time = k1 * pdeadline[i];
			
			/*WCET*/
			task_data[k2].WCET = j;
			
			k2++;
		}
	}
	
	/*出力*/
	for(i=0;i<TN;i++)
		fprintf(stderr, "Relative Deadline [Task%d] = %5.0lf\n", i+1, task_data[i].Relative_Deadline);
	for(i=0;i<TN;i++)
		fprintf(stderr, "Release Time [Task%d] = %5.0lf\n", i+1, task_data[i].Release_Time);
	for(i=0;i<TN;i++)
		fprintf(stderr, "WCET [Task%d] = %5.0lf\n", i+1, task_data[i].WCET);

	/*スレッドの生成*/
	pthread_create(&LMCF_scheduler, NULL, thread_LMCF, NULL);

	/*スレッドの停止*/
	pthread_join(LMCF_scheduler,NULL);
	
	/*デッドラインミスしたTaskをカウント*/
	for(i=0;i<TN;i++){
		if(deadline_miss_task[i] > 0)
			Deadline_Miss++;
	}

	fprintf(stderr, "\n");
	/*終了時の余裕時間*/
	for(i=0;i<TN;i++){
		save_laxity[i] += 0;
		fprintf(stderr, "When Finished Laxity Time of Task%d = %5.3lf\n", task_data[i].Number+1, save_laxity[i]);
	}
	
	fprintf(stderr, "The Number of Deadline Miss = %d\n", Deadline_Miss);
	fprintf(stderr, "Worst Case Memory Consumption (LMCF) = %d\n", Worst_Memory);
	
	/*ログの出力*/
	FILE *fpout_Combined;
	fpout_Combined = fopen("datafile_LMCF.txt","a");
	fprintf(fpout_Combined, "%d\t", Worst_Memory);		
	fprintf(fpout_Combined, "%d\r\n", Deadline_Miss);
	fclose(fpout_Combined);

	fprintf(stderr, "\n-------------LMCF Scheduling in %d-Processor Environment-------------\n\n", P);

	return 0;
}


/*タスク*/
void *thread_Tasks(void *num){

	int *task_number = (int *)num;	//タスクの識別子
	
	int i;							//カウント用変数
	
	fprintf(stderr, "Task%d next starts\n", *task_number+1);
	state[*task_number] = 1;
	hook[*task_number] = 1;			//タスクにフックをかける

	/*Taskの余裕時間を計算*/
	task_data[*task_number].Laxity_Time = task_data[*task_number].Relative_Deadline - task_data[*task_number].WCET + task_data[*task_number].Run_Time;
	task_data[*task_number].Laxity_Time += 0;
	fprintf(stderr, "Laxity Time [Task%d] = %5.3lf\n", *task_number+1, task_data[*task_number].Laxity_Time);
	
	/*既にデッドラインミスしている可能性があるため*/
	if(task_data[*task_number].Laxity_Time < -0.0)
		deadline_miss_task[*task_number]++;
	
	while(step[*task_number] < task_data[*task_number].WCET){
		
		while(hook[*task_number] == 1){
			/*suspend operation*/
			//fprintf(stderr, "FLAG\n");
		}
		
		if(schedule[*task_number] == 1){

			/*Taskの処理*/
			calc();

			/*タスクデータの更新*/
			task_data[*task_number].Run_Time = task_data[*task_number].Run_Time + ET[*task_number];
			task_data[*task_number].Relative_Deadline = task_data[*task_number].Relative_Deadline - ET[*task_number];
			task_data[*task_number].Laxity_Time = task_data[*task_number].Relative_Deadline - task_data[*task_number].WCET + task_data[*task_number].Run_Time;
			task_data[*task_number].Laxity_Time += 0;
			//fprintf(stderr, "Laxity Time of Scheduled Task%d = %5.3lf\n", *task_number+1, task_data[*task_number].Laxity_Time);		
			
			fprintf(stderr, "Scheduled Task = %d\n", *task_number+1);	
			//fprintf(stderr, "%d\n", *task_number+1);
			
			/*最終ステップでの処理*/
			if(step[*task_number] == task_data[*task_number].WCET - 1){
				
				fprintf(stderr, "Task%d finishes\n", *task_number+1);
				finish[*task_number] = 1;
	
				save_laxity[*task_number] = task_data[*task_number].Laxity_Time;
				save_laxity[*task_number] += 0;
		
				/*デッドラインミスしたタスクを測定*/
				if(task_data[*task_number].Laxity_Time < -0.0)
					deadline_miss_task[*task_number]++;
	
				if(deadline_miss_task[*task_number] > 0)
					fprintf(stderr, "task%d missed deadline\n", *task_number+1);
		
				state[*task_number] = 0;
				task_data[*task_number].Laxity_Time = MAX;
				
			}
			
			step[*task_number]++;
			
		}else{
			
			/*計算負荷*/
			load();
			
			/*タスクデータの更新*/
			//fprintf(stderr, "Non Scheduled Task%d\n", *task_number+1);	
			task_data[*task_number].Relative_Deadline = task_data[*task_number].Relative_Deadline - ET[*task_number];
			//fprintf(stderr, "Relative Deadline of Task%d = %5.3lf\n", *task_number+1, task_data[*task_number].Relative_Deadline);
			task_data[*task_number].Laxity_Time = task_data[*task_number].Relative_Deadline - task_data[*task_number].WCET + task_data[*task_number].Run_Time;
			task_data[*task_number].Laxity_Time += 0;
			//fprintf(stderr, "Laxity Time of Non Scheduled Task%d = %5.3lf\n", *task_number+1, task_data[*task_number].Laxity_Time);	
	
		}
		
		schedule[*task_number] = 0;
		hook[*task_number] = 1;
	
	}
	
	rand_memory[*task_number][step[*task_number]] = 0;
	step[*task_number] = task_data[*task_number].WCET;
	hook[*task_number] = 0;
	
	return 0;
}

/*LMCF Schedular*/
void *thread_LMCF(){
	
	int i,j;					//カウント用変数
	int hooker = 0;				//フックがかかっているタスク数
	int fin = -1;				//全処理が終了しているタスク数
	int total_memory;			//スケジュールされるタスクの総消費メモリ増分
	int count;					//起動しているタスクの数
	int sys_step = 0;			//全体のステップ
	
	pthread_t Task[TN];
	int ready[TN];				//スレッドを止めたら0
	
	
	/*全タスクの処理が終了するまで繰り返す*/
	while(fin != TN){
		
		/*スケジュールに参加するタスクがあるか判定*/
		for(i=0;i<TN;i++){	
			if(sys_step == task_data[i].Release_Time){
				pthread_create(&Task[i], NULL, thread_Tasks, &task_data[i].Number);
				state[i] = 1;
				ready[i] = 1;
			}
		}
	
		/*全処理が終了しているタスク数の測定とスレッドの停止*/
		fin = 0;
		for(i=0;i<TN;i++){
			
			fin = fin + finish[i];
			
			/*スレッドの停止*/
			if(finish[i] == 1 && ready[i] == 1){
				ready[i] = 0;
				pthread_join(Task[i],NULL);
			}
			
		}
		//fprintf(stderr, "fin = %d\n", fin);
		
		/*全タスクデータの更新が終わるまで待機*/
		count = -1;
		while(hooker != count){
			count = 0;
			for(i=0;i<TN;i++){
				if(state[i] == 1)
					count++;
			}
			
			hooker = 0;
			for(i=0;i<TN;i++){
				if(state[i] == 1){
					hooker = hooker + hook[i];
					//fprintf(stderr, "hooker[%d] = %d\n", i, hook[i]);
				}
				
			}
			//fprintf(stderr, "hooker = %d\n", hooker);
			//fprintf(stderr, "count = %d\n", count);

		}
		
		if(fin != TN && count > 0){
			
			LMCF();			//余裕時間の少ない順にソート
			/*以下のコメントアウトを外すと出力が見れる*/		
		
			fprintf(stderr, "\n");
			for(j=0;j<TN;j++)
				fprintf(stderr, "Sort LMCF[%d] = %d\n", sort_num_LMCF[j]+1, sort_memory[j]);
			
			/*スケジュールされるタスクの選定*/
			for(i=0;i<P;i++){
				if(state[sort_num_LMCF[i]] == 1)
					schedule[sort_num_LMCF[i]] = 1;
			}
		}
		
		//fprintf(stderr, "FLAG\n");
		total_memory = 0;
		for(i=0;i<TN;i++){
			if(schedule[i] == 1)
				total_memory = total_memory + rand_memory[i][step[i]];
		}
		memory_recode(total_memory);				//メモリ消費量の記録
		
		sys_step++;				//全体のステップをインクリメント
		
		/*フックを外す(全タスク処理を再開)*/
		for(i=0;i<TN;i++)
			hook[i] = 0;
		
		hooker = 0;

	}
	
	return 0;
}


/*消費メモリ増分の少ない順にソートする関数*/
void LMCF(){

	int MCI[S];						//消費メモリ増分(Memory Consumption Increments)格納変数（作業用）
	int i = 0, j = 0, k = 0;		//カウント用変数
	
	/*消費メモリ増分の初期化*/
	for(i=0;i<TN;i++){
		if(state[i] == 1 && step[i] != task_data[i].WCET){
			MCI[i] = rand_memory[i][step[i]];		
		}else{
			MCI[i] = MAX;
		}
	}

	/*ソート用変数初期化*/
	for(k=0;k<TN;k++){
		sort_memory[k] = MCI[k];
		sort_num_LMCF[k] = k;
		//fprintf(stderr, "sort_memory[%d] = %lf\n", k, sort_memory[k]);
	}
	
	int t1 = 0, t2 = 0;
	/*消費メモリ増分の少ない順にソートする.*/
	for(j=0;j<TN-1;j++){
		for(k=TN-1;k>j;k--){
			if(sort_memory[k] < sort_memory[k-1]){
				t1 = sort_memory[k];
				sort_memory[k] = sort_memory[k-1];
				sort_memory[k-1] = t1;
				
				t2 = sort_num_LMCF[k];
				sort_num_LMCF[k] = sort_num_LMCF[k-1];
				sort_num_LMCF[k-1] = t2;
			}
		}
	}
	
}


/*最悪メモリ消費量を記憶する関数*/
void memory_recode(int Memory){
	
	Current_Memory = Current_Memory + Memory;	//現在のメモリ消費量
	//fprintf(stderr, "Current_Memory = %d\n", Current_Memory);
	
	if(Worst_Memory < Current_Memory){
		Worst_Memory = Current_Memory;
		//fprintf(stderr, "Worst_Memory = %d\n", Worst_Memory);
	}
	
}


/*Taskの処理*/

void calc(){
	int n,j;
	n = 1;
	for(j=1;j<1000000;j++){
		n *= 1;
	}
}


/*計算負荷*/

void load(){
	int n,j;
	n = 1;
	for(j=1;j<1000000;j++){
		n *= 1;
	}
}


