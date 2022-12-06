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
//#define alpha 1000.0		//換算レート
#define S 1024

int TN = 0;					//タスク数
double dead_max = 0;		//相対デッドラインの最大値
int rand_memory[S][S];		//消費メモリ増分
double ET[S];				//Taskの1ステップの実行時間の平均（本実験では1ステップ1単位時間で実行）
int schedule[S];			//Taskがスケジュールされるとき1，そうでないとき0

int state[S];				//Taskが起動している場合は値を1,起動していない場合は0
int finish[S];				//タスクの処理がすべて終了していれば1，そうでなければ0

int step[S];				//現在のステップ
int sys_step = 0;			//全体のステップ

double save_laxity[S];		//Taskが終了した時のLaxity Time

int Worst_Memory = 0;		//最悪メモリ消費量
int Current_Memory = 0;		//現在のメモリ消費量

int deadline_miss_task[S];	//デッドラインミスの数
int Deadline_Miss = 0;		//デッドラインミスの数の総和

int ability = 0;			//0の時は，LLFスケジュール可能性は判定をする
int LLF_used = 0;			//スイッチしたかしていないか

/*sort用変数(LLF)*/
double sort_laxity[S];
int sort_num_LLF[S];
/*sort用変数(LMCLF)*/
double sort_priority[S];
int sort_num_LMCLF[S];

int hook[S];				//一時停止している（フックがかかっている）場合1，そうでなければ0
double alpha=0;	            //換算レート

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
void *thread_LLF_LMCLF();				//LLF LMCLF-combined Schedular

void Schedulability_Analysis_Func();	//Schedulability Analysis
void LLF();  							//余裕時間をソートする関数
void LMCLF();  							//優先度関数値をソートする関数
void memory_recode(int Memory);			//最悪メモリ消費量を記憶する関数

void calc();							//Taskの処理
void load();							//計算負荷

int GCD(int x, int y);					//最大公約数を返す
int LCM(int x, int y);					//最小公倍数を返す

int pthread_yield(void);				//コンパイラに警告を出させないためのプロトタイプ宣言である.なお,この関数はPOSIXでは非標準であり,sched_yield()を使うのが正しいとある


/*スレッド間で変数を保護するmutex*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*スレッド間で状態を判断するcond*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int main(void) {

	fprintf(stderr, "\n-------------LLF LMCLF-combined Scheduling in %d-Processor Environment-------------\n", P);
	
	int i = 0, j = 0, k1 = 0, k2 = 0;
	
	pthread_t Task[S];
	pthread_t Combined_scheduler;
	
	/*ファイル読み込み用変数*/
	char file_mem[256];								//消費メモリ増分のファイル
	char *file_dl = "rand_period_tasks.txt";		//周期のファイル（本実験では周期=相対デッドライン）
	FILE *fMemory;
	FILE *fdl;
	int ret, x;
	
	int ptask;										//仮のタスク数
	int period_count = 0;							//最小公倍数までのステップで何度リリースされるか
	int pdeadline[S];								//仮のデッドライン
	
	
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
	pthread_create(&Combined_scheduler, NULL, thread_LLF_LMCLF, NULL);
	
	/*スレッドの停止*/
	pthread_join(Combined_scheduler,NULL);
	
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
	
	if(LLF_used > 0)
		fprintf(stderr, "\nSchedule Switched\n");
	
	fprintf(stderr, "The Number of Deadline Miss = %d\n", Deadline_Miss);
	fprintf(stderr, "Worst Case Memory Consumption (LLF LMCLF) = %d\n", Worst_Memory);
	
	/*ログの出力*/
	char log[256];
	FILE *fpout_Combined;
	sprintf(log, "datafile_LLF_LMCLF_%lf.txt", alpha);
	fpout_Combined = fopen(log, "a");
	fprintf(fpout_Combined, "%d\t", Worst_Memory);		
	fprintf(fpout_Combined, "%d\t", Deadline_Miss);
	fprintf(fpout_Combined, "%d\r\n", LLF_used);	
	fclose(fpout_Combined);

	fprintf(stderr, "\n-------------LLF LMCLF-combined Scheduling in %d-Processor Environment-------------\n\n", P);

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

/*LLF LMCLF-combined Schedular*/
void *thread_LLF_LMCLF(){
	
	int i,j;				//カウント用変数
	int hooker = 0;			//フックがかかっているタスク数
	int fin = -1;			//全処理が終了しているタスク数
	int total_memory;		//スケジュールされるタスクの総消費メモリ増分
	int count;				//起動しているタスクの数
	int loop;
	
	pthread_t Task[TN];
	int ready[TN];			//スレッドを止めたら0
	
	
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
		
		LMCLF();		//優先度関数の小さい順にソート
		/*スケジュールされるタスクの選定*/
		for(i=0;i<P;i++){
			/*LMCLFでタスクをスケジュールすると仮定*/
			if(state[sort_num_LMCLF[i]] == 1)
				schedule[sort_num_LMCLF[i]] = 1;
			
		}
		
		/*スケジュール可能性判定*/
		if(count > P && fin != TN)
			Schedulability_Analysis_Func();
		
		if(ability == 1)
			LLF_used++;
		
		if(fin != TN && count > 0){
			if(ability == 0){
				/*以下のコメントアウトを外すと出力が見れる*/
			
				fprintf(stderr, "\n");
				for(j=0;j<TN;j++)
					fprintf(stderr, "Sort LMCLF[%d] = (%lf * %d) + (%0.0lf * %0.0lf) = %0.1lf\n", sort_num_LMCLF[j]+1, alpha, rand_memory[sort_num_LMCLF[j]][step[sort_num_LMCLF[j]]], task_data[sort_num_LMCLF[j]].WCET-step[sort_num_LMCLF[j]], task_data[sort_num_LMCLF[j]].Laxity_Time, sort_priority[j]);
			
			}else{
				
				/*巻き戻し*/
				for(j=0;j<TN;j++)
					schedule[j]=0;

				LLF();
				/*以下のコメントアウトを外すと出力が見れる*/
			
				fprintf(stderr, "\n");
				for(j=0;j<TN;j++)
					fprintf(stderr, "Sort Laxity[%d] = %lf\n", sort_num_LLF[j]+1, sort_laxity[j]);
			
				/*スケジュールされるタスクの選定*/
				for(i=0;i<P;i++){
					if(state[sort_num_LLF[i]] == 1)
						schedule[sort_num_LLF[i]] = 1;
				}
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
		
		ability = 0;
		hooker = 0;
		
	}
	
	return 0;
}


/*Schedulability Analysis*/
void Schedulability_Analysis_Func(){
	
	int i = 0, j = 0;					//カウント用変数
	int p_num = 0;						//使うプロセッサ数
	int state_all = 0;					//起動しているタスク数
	
	double SAF_Relative_Deadline[TN];	//スケジュール可能性判定用のデッドライン
	double SAF_Laxity_Time[TN];			//スケジュール可能性判定用の余裕時間
	double SAF_Run_Time[TN];			//スケジュール可能性判定用の実行時間
	
	
	/*現在のスケジューリングサイクルのパラメータについて*/
	for(i=0;i<TN;i++){
			
		if(state[i] == 1){
			SAF_Laxity_Time[i] = task_data[i].Relative_Deadline - task_data[i].WCET + task_data[i].Run_Time;
			/*全タスク時刻0で起動したとする*/
			SAF_Relative_Deadline[i] = task_data[i].Relative_Deadline;
		}else{
			SAF_Laxity_Time[i] = task_data[i].Relative_Deadline + task_data[i].Release_Time - sys_step - task_data[i].WCET + task_data[i].Run_Time;
			/*全タスク時刻0で起動したとする*/
			SAF_Relative_Deadline[i] = task_data[i].Relative_Deadline + task_data[i].Release_Time - sys_step;
		}
			

	}
	
	/*パラメータ出力*/
	
	fprintf(stderr, "\n");
	
	fprintf(stderr, "Current Scheduling Cycle\n");
	
	for(i=0;i<TN;i++){
		if(finish[i] == 0){
			
			fprintf(stderr, "D [Task%d] = %0.0lf\t", i+1, SAF_Relative_Deadline[i]);
			fprintf(stderr, "C [Task%d] = %0.0lf\t", i+1, (task_data[i].WCET - task_data[i].Run_Time));
			fprintf(stderr, "L [Task%d] = %0.0lf\n", i+1, SAF_Laxity_Time[i]);
			
		}
	}
	
	/*全タスクのデータを作業用変数に格納(次のスケジューリングサイクル)*/
	dead_max = 0;
	for(i=0;i<TN;i++){
		
		if(schedule[i] == 1){
			SAF_Run_Time[i] = task_data[i].Run_Time + ET[i];
			p_num++;
		}else{
			SAF_Run_Time[i] = task_data[i].Run_Time;
		}
		
		if(finish[i] == 0){
			SAF_Relative_Deadline[i] -= ET[i];
			
			if(dead_max < SAF_Relative_Deadline[i])
				dead_max = SAF_Relative_Deadline[i];
			
			SAF_Laxity_Time[i] = SAF_Relative_Deadline[i] - task_data[i].WCET + SAF_Run_Time[i];
			
			if(SAF_Laxity_Time[i] < 0)
				ability = 1;
		}
		
	}
	
	/*パラメータ出力*/
	
	fprintf(stderr, "\n");
	
	fprintf(stderr, "Next Scheduling Cycle\n");
	
	fprintf(stderr, "Processor = %d\n", p_num);		//使用するプロセッサ数
	fprintf(stderr, "k_max = %0.0lf\n", dead_max);		//kの最大
	for(i=0;i<TN;i++){
		if(schedule[i] == 1)
			fprintf(stderr, "Scheduled Task by LMCLF = %d\n", i+1);
	}
	
	for(i=0;i<TN;i++){
		if(finish[i] == 0){
			fprintf(stderr, "D [Task%d] = %0.0lf\t", i+1, SAF_Relative_Deadline[i]);
			fprintf(stderr, "C [Task%d] = %0.0lf\t", i+1, (task_data[i].WCET - SAF_Run_Time[i]));
			fprintf(stderr, "L [Task%d] = %0.0lf\n", i+1, SAF_Laxity_Time[i]);
			
		}
	}
	
	
	fprintf(stderr, "\n");

	for(i=0;i<TN;i++)
		state_all += state[i];
	
	/*余剰計算力を計算*/
	double Fk;
	double R1,R2;
	int k = 1;
	
	if(state_all > 0){
		
		while(ability == 0 && k <= dead_max){
			Fk = 0;
			R1 = 0, R2 = 0;
			
			/*fkの計算*/
			for(i=0;i<TN;i++){
				if(finish[i] == 0){
					if(SAF_Relative_Deadline[i] <= k){
						R1 = R1 + (task_data[i].WCET - SAF_Run_Time[i]);
					}
					else if(SAF_Laxity_Time[i] <= k && SAF_Relative_Deadline[i] > k){
						R2 = R2 + (k - SAF_Laxity_Time[i]);
					}else{
						//fprintf(stderr, "FLAG\n");
					}
				}
			}
		
			Fk = (k*p_num) - R1 - R2;
			if(k == dead_max)
				fprintf(stderr, "Fk[%d] = %lf\n", k, Fk);
		
			if(Fk < 0){
				fprintf(stderr, "\nSchedule_Switch[%d] = %lf\n", k, Fk);
				ability = 1;
			}
			
			k++;
		}
	}
	
	/*スケジュール可能性判定用の変数の初期化*/
	for(i=0;i<TN;i++){
		SAF_Laxity_Time[i] = MAX;
		SAF_Relative_Deadline[i] = MAX;
		SAF_Run_Time[i] = MAX;
	}
		
	
}


/*余裕時間の少ない順にソートする関数*/
void LLF(){

	double laxity[S];				//余裕時間格納変数（作業用）
	int i = 0, j = 0, k = 0;		//カウント用変数

	/*余裕時間の初期化*/
	for(i=0;i<TN;i++){
		if(state[i] == 1 && step[i] != task_data[i].WCET){
			laxity[i] = task_data[i].Laxity_Time;			
		}else{
			laxity[i] = MAX;
		}
	}
	
	/*Taskの余裕時間をソート*/
	/*ソート用変数初期化*/
	for(k=0;k<TN;k++){
		sort_laxity[k] = laxity[k];
		sort_num_LLF[k] = k;
		//fprintf(stderr, "sort_laxity[%d] = %lf\n", k, sort_laxity[k]);
	}

	int t1 = 0, t2 = 0;
	/*余裕時間の少ない順にソートする.*/
	for(j=0;j<TN-1;j++){
		for(k=TN-1;k>j;k--){
			if(sort_laxity[k] < sort_laxity[k-1]){
				t1 = sort_laxity[k];
				sort_laxity[k] = sort_laxity[k-1];
				sort_laxity[k-1] = t1;
				
				t2 = sort_num_LLF[k];
				sort_num_LLF[k] = sort_num_LLF[k-1];
				sort_num_LLF[k-1] = t2;
			}
		}
	}
	/*デバッグ用*/
	/*
	for(j=0;j<TN;j++){
		fprintf(stderr, "Laxity Time of Task%d = %lf\n", sort_num_LLF[j]+1, sort_laxity[j]);
	}
	fprintf(stderr, "\n");
	*/
	
	
}


/*優先度関数値が小さい順にソートする関数*/
void LMCLF(){

	double priority_func[S];		//優先度関数値格納変数（作業用）
	double memory=0,minmemory=MAX;               //メモリ消費量格納変数
	double priority_func1=0,priority_func2=0;
	int i = 0, j = 0, k = 0,l = 0;		//カウント用変数
	double alphauppermin=MAX,alphalowermax=0;  //αの範囲最大最小
	int tempalpha1=0,tempalpha2=0;  //仮のαの上限下限格納関数

for(i=0;i<TN;i++){
			for(j=0;j<TN;j++){
				if(i==j){

				}else if(rand_memory[i][step[0]] > rand_memory[j][step[0]]){
						tempalpha1=((task_data[j].WCET - step[0]) * task_data[j].Laxity_Time)-((task_data[i].WCET - step[0]) * task_data[i].Laxity_Time)/(rand_memory[i][step[0]])-(rand_memory[j][step[0]]);
						if(alphauppermin>tempalpha1){	
							alphauppermin=tempalpha1;
						}else{
						}
				}else{
						tempalpha2=((task_data[j].WCET - step[0]) * task_data[j].Laxity_Time)-((task_data[i].WCET - step[0]) * task_data[i].Laxity_Time)/(rand_memory[i][step[0]])-(rand_memory[j][step[0]]);
						if(alphalowermax<tempalpha2){
							alphalowermax=tempalpha2;
						}
				}
			}
		
				

			for(k=0;k<TN;k++){
				for(l=0;l<TN;l++){
					if(rand_memory[k][step[1]] > rand_memory[l][step[0]] && i==k){
						tempalpha1=((task_data[k].WCET - step[1]) * task_data[k].Laxity_Time)-((task_data[l].WCET - step[0]) * task_data[l].Laxity_Time)/(rand_memory[k][step[1]])-(rand_memory[l][step[0]]);	
						if(alphauppermin>tempalpha1){
							alphauppermin=tempalpha1;
						}else{
							tempalpha2=((task_data[k].WCET - step[1]) * task_data[k].Laxity_Time)-((task_data[l].WCET - step[0]) * task_data[l].Laxity_Time)/(rand_memory[k][step[1]])-(rand_memory[l][step[0]]);	
						}
						if(alphalowermax<tempalpha2){
							alphalowermax=tempalpha2;
						}
					}
						

					if(alphauppermin>0 && alphalowermax < alphauppermin){
						if(i==k){
							memory=rand_memory[i][step[0]] + rand_memory[i][step[1]];
						}else{
							memory=rand_memory[i][step[0]] + rand_memory[k][step[0]];
						}
						if(minmemory>memory){
							minmemory=memory;
							alpha=(alphalowermax + alphauppermin)/2;
						}
					}	
				}	
			}
	}






	/*優先度関数値の初期化*/
	for(i=0;i<TN;i++){
		if(state[i] == 1 && step[i] != task_data[i].WCET){
			
			priority_func[i] = (alpha * rand_memory[i][step[i]]) + ((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time);		

		}else{
			priority_func[i] = MAX;
		}
	}

	/*ソート用変数初期化*/
	for(k=0;k<TN;k++){
		sort_priority[k] = priority_func[k];
		sort_num_LMCLF[k] = k;
		//fprintf(stderr, "sort_priority[%d] = %lf\n", k, sort_priority[k]);
	}
	
	int t1 = 0, t2 = 0;
	/*優先度関数値が小さい順にソートする.*/
	for(j=0;j<TN-1;j++){
		for(k=TN-1;k>j;k--){
			if(sort_priority[k] < sort_priority[k-1]){
				t1 = sort_priority[k];
				sort_priority[k] = sort_priority[k-1];
				sort_priority[k-1] = t1;
				
				t2 = sort_num_LMCLF[k];
				sort_num_LMCLF[k] = sort_num_LMCLF[k-1];
				sort_num_LMCLF[k-1] = t2;
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


