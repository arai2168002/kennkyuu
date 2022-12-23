/*Machigasira Yuki*/
/*もしかしたらいらないヘッダファイルもあるかも・・・*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

//#define P 2  
#define P 4	
//#define P 1			//プロセッサ数
//#define MAX 1000000000		//起動していない時
//#define alpha 1000.0		//換算レート
#define S 1024

#define NIL ((List)0)
#define LNIL ((LList)0) 
#define S 1024
#define FIXEDPOINT 2	//????????????????2?i?????????V?t?g?????

typedef long long FIXPOINTDECIMAL;		//long long?^????????
#define ITFD(x) ((FIXPOINTDECIMAL)(x << FIXEDPOINT))	//int?^????l???????????



FIXPOINTDECIMAL MAX=2147483647; //?N????????????

int TN = 0;					//タスク数
int valTN=P*2;				//?]???l???茵?I?????^?X?N??
double dead_max = 0;		//相対デッドラインの最大値
int rand_memory[S][S];		//消費メモリ増分
double ET[S];				//Taskの1ステップの実行時間の平均（本実験では1ステップ1単位時間で実行）
int schedule[S];			//Taskがスケジュールされるとき1，そうでないとき0

int state[S];				//Taskが起動している場合は値を1,起動していない場合は0
int finish[S];				//タスクの処理がすべて終了していれば1，そうでなければ0

int step[S];				//現在のステップ
int sys_step = 0;			//全体のステップ
int tmp_step[S];			//作業用ステップ

double save_laxity[S];		//Taskが終了した時のLaxity Time

int Worst_Memory = 0;		//最悪メモリ消費量
int Current_Memory = 0;		//現在のメモリ消費量

int deadline_miss_task[S];	//デッドラインミスの数
int Deadline_Miss = 0;		//デッドラインミスの数の総和

int ability = 0;			//0の時は，LLFスケジュール可能性は判定をする
int LLF_used = 0;			//スイッチしたかしていないか

FIXPOINTDECIMAL alpha=ITFD(1),alphadiff=0; //?p?????[?^??(?????l)0?A????C??????

/*sort用変数(LLF)*/
double sort_laxity[S];
int sort_num_LLF[S];
/*sort用変数(LMCLF)*/
FIXPOINTDECIMAL sort_priority[S];
int sort_num_LMCLF[S];

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

typedef struct cell {
  FIXPOINTDECIMAL element;
  struct cell *next;
} Cell,*List;

typedef struct llcell {
  List element;
  struct llcell *next;
} LLCell,*LList;

data task_data[S];

void *thread_Tasks(void *num);			//タスク
void *thread_LLF_AcII_LMCLF();			//LLF AcII-LMCLF-combined Schedular

void Schedulability_Analysis_Func();	//Schedulability Analysis
void LLF();  							//余裕時間をソートする関数
void AcII_LMCLF();  					//優先度関数値をソートする関数(関数LMCLFとは少し違うので注意)
void memory_recode(int Memory);			//最悪メモリ消費量を記憶する関数

void calc();							//Taskの処理
void load();							//計算負荷

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
LList firstnList2(LList,List n);
List restnList(List l,unsigned int n);		//リストの末尾からn番目までの要素を削除
int memberList(int element,List l);     //リストにその要素が含まれているかどうかを調べる関数
int memberList2(List l);
void freeList(List l);		//リストのメモリ解放
void freeList2(LList l);
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

int pthread_yield(void);				//コンパイラに警告を出させないためのプロトタイプ宣言である.なお,この関数はPOSIXでは非標準であり,sched_yield()を使うのが正しいとある.


/*スレッド間で変数を保護するmutex*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*スレッド間で状態を判断するcond*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int main(void) {

	fprintf(stderr, "\n-------------LLF AcII-LMCLF-combined Scheduling in %d-Processor Environment-------------\n", P);
	
	int i = 0, j = 0, k1 = 0, k2 = 0;
	
	pthread_t Task[S];
	pthread_t Combined_scheduler;
	
	/*ファイル読み込み用変数*/
	char file_mem[256];								//消費メモリ増分のファイル
	char *file_dl = "rand_period_tasks.txt";		//周期のファイル（本実験では周期=相対デッドライン）
	char *file_ut = "rand_utilization_tasks.txt";	//タスク利用率のファイル
	FILE *fMemory;
	FILE *fdl, *fut;
	int ret, x;
	double y;
	
	int ptask;										//仮のタスク数
	int pdeadline[S];								//仮のデッドライン
	double utilization[S];							//タスク利用率
	double aver_ut = 0;								//タスク利用率（平均）
	int period_count = 0;							//最小公倍数までのステップで何度リリースされるか
	
	/*ファイル読み込み*/
	fdl = fopen(file_dl,"r");
	ptask = 0;
	while(ret = fscanf(fdl,"%d",&x) != EOF){
		pdeadline[ptask] = x;
		//fprintf(stderr, "Relative Deadline [Task%d] = %5.0lf\n", ptask+1, task_data[ptask].Relative_Deadline);
		ptask++;
	}
	
	fut = fopen(file_ut,"r");
	i = 0;
	while(ret = fscanf(fut,"%lf",&y) != EOF){
		utilization[i] = y;
		//fprintf(stderr, "Relative Deadline [Task%d] = %5.0lf\n", ptask+1, task_data[ptask].Relative_Deadline);
		aver_ut += utilization[i];
		i++;
	}
	aver_ut = aver_ut / ptask;
	
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
		tmp_step[i] = 0;
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
	pthread_create(&Combined_scheduler, NULL, thread_LLF_AcII_LMCLF, NULL);

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
	fprintf(stderr, "Worst Case Memory Consumption (LLF AcII-LMCLF) = %d\n", Worst_Memory);
	
	/*ログの出力*/
	char log[256];
	FILE *fpout_Combined;
	sprintf(log, "datafile_LLF_AcII_LMCLF_%lf.txt", alpha);
	fpout_Combined = fopen(log, "a");
	fprintf(fpout_Combined, "%d\t", Worst_Memory);		
	fprintf(fpout_Combined, "%d\t", Deadline_Miss);
	fprintf(fpout_Combined, "%d\r\n", LLF_used);	
	fclose(fpout_Combined);
	
	FILE *fpout_Task_Set;
	fpout_Task_Set = fopen("datafile_Task_Set.txt","a");
	fprintf(fpout_Task_Set, "%d\t", ptask);
	fprintf(fpout_Task_Set, "%lf\r\n", aver_ut);
	fclose(fpout_Task_Set);

	fprintf(stderr, "\n-------------LLF AcII-LMCLF-combined Scheduling in %d-Processor Environment-------------\n\n", P);

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

/*Improved LLF LMCLF-combined Schedular*/
void *thread_LLF_AcII_LMCLF(){
	
	int s1,s2,i,j;			//カウント用変数
	int hooker = 0;			//フックがかかっているタスク数
	int fin = -1;			//全処理が終了しているタスク数
	int total_memory;		//スケジュールされるタスクの総消費メモリ増分
	int count;				//起動しているタスクの数
	int loop;				//0でwhileから脱出
	int MIN_MCI,MCI;		//最小の消費メモリ増分，消費メモリ増分
	int processor;			//用いるプロセッサ数
	int intask;				//スケジュールされるタスクの数
	int twice[S];			//二回目選ばれたとき2
	
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
		
		if(fin != TN && count > 0){
			
			/*ステップを作業用変数に格納*/
			for(i=0;i<TN;i++){
				tmp_step[i] = step[i];
				twice[i] = 0;
			}
			
			i = 0;
			loop = 1;
			intask = 0;
			while(i<P && loop==1){	
		
				AcII_LMCLF();		//優先度関数の小さい順にソート
			
				MIN_MCI = MAX-1;
				for(s1=0;s1<P;s1++){
					MCI = 0;
					for(s2=0;s2<=s1;s2++){
						MCI = MCI + sort_priority[s2];
					}
				
					if(MCI < MIN_MCI){
						MIN_MCI = MCI;
						processor = s1+1;
					}
				}
			
				for(s1=0;s1<processor;s1++){
				
					twice[sort_num_LMCLF[s1]]++;
					if(twice[sort_num_LMCLF[s1]] == 2){
						loop = 0;
						break;
					}
				
					/*スケジュールされるタスクの選定および総消費メモリ増分の計算*/
					if(state[sort_num_LMCLF[s1]] == 1){
						schedule[sort_num_LMCLF[s1]] = 1;
						intask++;
					}
					
					if(intask >= P){
						loop = 0;
						break;
					}
				
					tmp_step[sort_num_LMCLF[s1]]++;
				}
		
				i++;
			}	
		}
		
		/*ソートの初期化*/
		for(i=0;i<TN;i++)
			tmp_step[i] = step[i];
			
		AcII_LMCLF();
		
		/*スケジュール可能性判定*/
		if(count > 1 && fin != TN){
			
			if(count >= P){
				i = P;
			}else{
				i = count;
			}
			
			ability = 1;
			while(processor <= i && ability == 1){
				
				Schedulability_Analysis_Func();
				
				if(ability == 1){
					processor++;
					for(j=0;j<processor;j++){
						if(state[sort_num_LMCLF[j]] == 1)
							schedule[sort_num_LMCLF[j]] = 1;
					}
				}
				
			}
			
			if(ability == 1)
				LLF_used++;
		}
		
		
		if(fin != TN && count > 0){
			if(ability == 0){
				/*以下のコメントアウトを外すと出力が見れる(1)ただし，(2)もはずすこと*/
			
				for(i=0;i<TN;i++)
					tmp_step[i] = step[i];
			
				AcII_LMCLF();
			
				fprintf(stderr, "\n");
				for(j=0;j<TN;j++)
					fprintf(stderr, "Sort LMCLF[%d] = (%s * %d) + (%0.0lf * %0.0lf) = %0.1lf\n", sort_num_LMCLF[j]+1, FixPointDecimalToString(alpha), rand_memory[sort_num_LMCLF[j]][step[sort_num_LMCLF[j]]], task_data[sort_num_LMCLF[j]].WCET-step[sort_num_LMCLF[j]], task_data[sort_num_LMCLF[j]].Laxity_Time, FixPointDecimalToString(ITFD(sort_priority[j])));
			
			}else{
				/*巻き戻し*/
				for(j=0;j<TN;j++)
					schedule[j] = 0;
				
				LLF();
				/*以下のコメントアウトを外すと出力が見れる(2)*/
			
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
	
	ability = 0;
	
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
	fprintf(stderr, "k_max = %0.0lf\n", dead_max);	//kの最大
	fprintf(stderr, "ability = %d\n", ability);		//f(k,i)を計算するか
	for(i=0;i<TN;i++){
		if(schedule[i] == 1)
			fprintf(stderr, "Scheduled Task by AcII-LMCLF = %d\n", i+1);
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
		
			Fk = (k*P) - R1 - R2;
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


/*優先度関数値が小さい順にソートする関数(関数LMCLFと違うところはtmp_stepで計算しているところ)*/
void AcII_LMCLF(){

	FIXPOINTDECIMAL priority_func[S];		//?D??x????l?i?[????i???p?j
	FIXPOINTDECIMAL val=0,minval=MAX;               //?]???l?i?[???
	FIXPOINTDECIMAL priority_func1=0,priority_func2=0;
	int i = 0,j = 0,k = 0,l = 0,a = 0,b = 0,c = 0;		//?J?E???g?p???
	int set1=0,set2=0; //1,2?X?e?b?v???^?X?N??g???????
	FIXPOINTDECIMAL prealpha=alpha;	//?O????????
	FIXPOINTDECIMAL alphauppermin=MAX,alphalowermax=0;   //??????????
	FIXPOINTDECIMAL alphaupperminsav=MAX,alphalowermaxsav=0;   //??????????(????p)
    FIXPOINTDECIMAL s1val=0,s2val=0;  //1?X?e?b?v??,2?X?e?b?v???]???l????v
	FIXPOINTDECIMAL tempalpha1=0,tempalpha2=0;  //??????????????i?[???
	int valketa=0;	//?]???l?v?Z??????????????????????
	int besti,bestk;   // ?????????????i??k???L??
	int mintasknum=0;	//?]???l?????????^?X?N???
	int setP1num=0,setP2num=0;	//1,2?X?e?b?v???]???l????????^?X?N?????W????v?f??
	int Laxityjudge=MAX;	//?]?T?????????????
	int NewLaxityjudge=MAX;	//????]?T?????????????

  	List setP1=createList();	//1?X?e?b?v???]???l????????^?X?N?????W??
	List setP2=createList();	//2?X?e?b?v???]???l????????^?X?N?????W??
	List val1=createList();		//1?X?e?b?v???]???l?W??
	List val2=createList();		//2?X?e?b?v???]???l?W??
	List copyval1=createList();		//1?X?e?b?v???]???l?W??
	List copyval2=createList();		//2?X?e?b?v???]???l?W??
  	LList Aset1=createList2();	//1?X?e?b?v????^?X?N??g???????S??W??
	LList Aset2=createList2();    //?Q?X?e?b?v????^?X?N??g???????S??W??
	LList Aset1orig=createList2();	//Aset1????|?C???^???L??
	LList Aset2orig=createList2();	//Aset2????|?C???^???L??
    List kumi1=createList();		//1?X?e?b?v????^?X?N??g??????????W??
    List kumi2=createList();		//2?X?e?b?v????^?X?N??g??????????W??
	List bestkumi1=createList();	//1?X?e?b?v????????????????^?X?N??g????????L??
	List bestkumi2=createList();	//2?X?e?b?v????????????????^?X?N??g????????L??
	
	pthread_mutex_lock(&mutex);
    alphadiff=0;
	minval=MAX;
	/*???Z???[?g???????*/
  	for(i=TN-1;i>=0;i--){//val1??]???l???i?[
	  if(state[i]==1){
		//??????????????????????邱?????????????????p???]???l??v?Z
		//valketa=shiftoperationtimes((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time) - shiftoperationtimes(rand_memory[i][step[i]]);
		//val1=insertList(((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time) + ((valketa>=0)?(rand_memory[i][step[i]] << valketa):(rand_memory[i][step[i]] >> abs(valketa))),val1);
		val1=insertList((FTFD(task_data[i].WCET - tmp_step[i]) * FTFD(task_data[i].Laxity_Time)) + (prealpha * ITFD(rand_memory[i][tmp_step[i]])),val1);
	  }else{
		val1=insertList(MAX,val1);
	  }
  	}
	//fprintf(stderr,"val1:"); fprintFPList(val1);

	for(i=0;i<valTN;i++){//?]???l?????????^?X?N??????i?[
		mintasknum=minList(val1,0,-1,MAX);
		if(mintasknum!=-1){
			setP1=insertList(mintasknum,setP1);
			copyval1=val1;
			val1=ideleatList(val1,copyval1,mintasknum);
		}
	}
	freeList(copyval1);

	setP1num=lengthList(setP1);

	//fprintf(stderr,"step1?T????????????^?X?N???:"); fprintList(setP1);

	if(setP1num>=P){
  		Aset1=Makesubsetset(setP1,1,P);
	}else{
		Aset1=insertList2(setP1,Aset1);
	}

	//fprintf(stderr,"step1?^?X?N????W??:"); fprintList(Aset1);
	freeList(setP1);
	Aset1orig=Aset1;

  	for(set1=1;set1<lengthList2(Aset1orig);set1++){
		if(!nullpList(kumi1)){
			freeList(kumi1);
		}

    	kumi1=headList2(Aset1);
        Aset1=tailList2(Aset1);
		//fprintf(stderr,"kumi1 %d組目:",set1); fprintList(kumi1);

		alphauppermin=MAX,alphalowermax=0;    
		for(i=0;i<TN;i++){
			if(memberList(i,kumi1)==1 && state[i] == 1){  /* i?^?X?N????I??????????^?X?N???N??????????? */
				for(j=0;j<TN;j++){
					if(memberList(j,kumi1)==0 && state[j] == 1){  //j?^?X?N????I????????????^?X?N???N???????????
						if(rand_memory[i][tmp_step[i]] == rand_memory[j][tmp_step[j]]) {  // 修正提案2022.12.1中田 0除算を回避するため
					    	continue;
					  } else
						if(rand_memory[i][tmp_step[i]] < rand_memory[j][tmp_step[j]]){// m(i)??+Ci*Li<m(j)??+Cj*Lj && m(j)>m(i) --> Ci*Li-Cj*Lj<(m(j)-m(i))?? --> ??>(Ci*Li-Cj*Lj)/(m(j)-m(i))
							tempalpha1=((FTFD(task_data[i].WCET - tmp_step[i]) * FTFD(task_data[i].Laxity_Time))-(FTFD(task_data[j].WCET - tmp_step[j]) * FTFD(task_data[j].Laxity_Time)))/(ITFD(rand_memory[j][tmp_step[j]])-ITFD(rand_memory[i][tmp_step[i]]));
							if(alphalowermax<tempalpha1){	
		    					alphalowermax=tempalpha1;
							}else{
							}
						}else{// m(i)??+Ci*Li<m(j)??+Cj*Lj && m(j)<m(i) --> (m(i)-m(j))??<Cj*Lj-Ci*Li --> ??<(Cj*Lj-Ci*Li)/(m(i)-m(j))
							tempalpha2=((FTFD(task_data[j].WCET - tmp_step[j]) * FTFD(task_data[j].Laxity_Time))-(FTFD(task_data[i].WCET - tmp_step[i]) * FTFD(task_data[i].Laxity_Time)))/(ITFD(rand_memory[i][tmp_step[i]]) - ITFD(rand_memory[j][tmp_step[j]]));
							if(alphauppermin>tempalpha2){
		    					alphauppermin=tempalpha2;								
							}
						}
					}
				}
            }
        }
		
	    if(!(alphauppermin>0 && alphalowermax < alphauppermin)){
	    	continue;
	    }
		
	    //fprintf(stderr,"%lf <= alpha <= %lf for scheduling task %d first\n",alphalowermax,alphauppermin,i+1);
	    alphaupperminsav=alphauppermin; alphalowermaxsav=alphalowermax;

		//val2=createList();
		for(k=TN-1,val2=createList();k>=0;k--){//val2??]???l???i?[
			if(state[k]==1){
				//??????????????????????邱?????????????????p???]???l??v?Z
				//valketa=shiftoperationtimes((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]) * task_data[k].Laxity_Time) - shiftoperationtimes(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]]);
				//val2=insertList(((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]) * task_data[k].Laxity_Time) + (valketa>=0)?(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]] << valketa):(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]] >> abs(valketa)),val2);
				val2=insertList(((FTFD((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k])) * FTFD(task_data[k].Laxity_Time)) + (prealpha * ITFD(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]]))),val2);
			}else{
				val2=insertList(MAX,val2);
			}
		}

		//fprintf(stderr,"val2:"); fprintFPList(val2);

		//setP2=createList();
		for(k=0,setP2=createList();k<valTN;k++){//?]???l?????????^?X?N??????i?[
			mintasknum=minList(val2,0,-1,MAX);
			if(mintasknum!=-1){
			setP2=insertList(mintasknum,setP2);
			copyval2=val2;
			val2=ideleatList(val2,copyval2,mintasknum);
			}
		}
		freeList(copyval2);

		setP2num=lengthList(setP2);
		//fprintf(stderr,"step2?T????????????^?X?N???:"); fprintList(setP2);


		if(setP2num>=P){
			//Aset2=setList(setP2,createList(),0,setP2num-P+1);
			Aset2=Makesubsetset(setP2,1,P);
		}else{
			Aset2=insertList2(setP2,Aset2);
		}

		//fprintf(stderr,"step2?^?X?N????W??:"); fprintList(Aset2);
		freeList(setP2);

		Aset2orig=Aset2;

  		for(set2=1;set2<lengthList2(Aset2orig);set2++){
			if(!nullpList(kumi2)){
			freeList(kumi2);
			}
            kumi2=headList2(Aset2);
            Aset2=tailList2(Aset2);
			//fprintf(stderr,"kumi2 %d組目:",set2); fprintList(kumi2);

			alphauppermin=alphaupperminsav; alphalowermax=alphalowermaxsav;
	    	for(k=0;k<TN;k++){
	    		if(memberList(k,kumi2)==1 && state[k] == 1){ /* k?^?X?N????I??????????^?X?N???N??????????? */
					for(l=0;l<TN;l++){
						if(memberList(l,kumi2)==0 && state[l] == 1){  /* l?^?X?N????I????????????^?X?N???N??????????? */
							if(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]] == rand_memory[l][(memberList(l,kumi1)==1)?(tmp_step[l]+1):tmp_step[l]] ) {  // 修正提案2022.12.1中田 0除算を回避するため
						    	continue;
						  	} else 
		    				if(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]] < rand_memory[l][(memberList(l,kumi1)==1)?(tmp_step[l]+1):tmp_step[l]] ){  // m(k)??+Ck*Lk<m(l)??+Cl*Ll && m(l)>m(k) --> Ck*Lk-Cl*Ll<(m(l)-m(k))?? --> ??>(Ck*Lk-Cl*Ll)/(m(l)-m(k))
		    					tempalpha1=((FTFD((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k])) * FTFD(task_data[k].Laxity_Time))-(FTFD((task_data[l].WCET - (memberList(l,kumi1)==1)?(tmp_step[l]+1):tmp_step[l])) * FTFD(task_data[l].Laxity_Time)))/(ITFD(rand_memory[l][(memberList(l,kumi1)==1)?(tmp_step[l]+1):tmp_step[l]]) - ITFD(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]]));	
		    					if(alphalowermax<tempalpha1){
									alphalowermax=tempalpha1;
		    					}else{
		    					}
		    				}else{// m(k)??+Ck*Lk<m(l)??+Cl*Ll && m(l)<m(k) --> (m(k)-m(l))??<Cl*Ll-Ck*Lk  -->  ??<(Cl*Ll-Ck*Lk)/(m(k)-m(l))
		    					tempalpha2=((FTFD((task_data[l].WCET - (memberList(l,kumi1)==1)?(tmp_step[l]+1):tmp_step[l])) * FTFD(task_data[l].Laxity_Time))-(FTFD((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k])) * FTFD(task_data[k].Laxity_Time)))/(ITFD(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]]) - ITFD(rand_memory[l][(memberList(l,kumi1)==1)?(tmp_step[l]+1):tmp_step[l]]));	
		    					if(alphauppermin>tempalpha2){
									alphauppermin=tempalpha2;
		    					}
                            }
						}
					}
                }
            }

            if(alphauppermin>0 && alphalowermax < alphauppermin){ //求めたいαが条件を満たしている時
				//fprintf(stderr,"true");
                //fprintf(stderr,"%lf <= alpha <= %lf for scheduling task %d and then task %d\n",alphalowermax,alphauppermin,i+1,k+1);
				val=0;
				Laxityjudge=0;
                //1ステップ目の評価値の合計
                for(i=0,s1val=0;i<TN;i++){
                    if(memberList(i,kumi1)==1 && state[i] == 1){ /* set1のiビット目が1ならば */
						//valketa=shiftoperationtimes((task_data[i].WCET - tmp_step[i]) * task_data[i].Laxity_Time) - shiftoperationtimes(rand_memory[i][tmp_step[i]]);
                        //s1val+=(((task_data[i].WCET - tmp_step[i]) * task_data[i].Laxity_Time) + ((valketa>=0)?(rand_memory[i][tmp_step[i]] << valketa):(rand_memory[i][tmp_step[i]] >> abs(valketa))));
						s1val+=(FTFD((task_data[i].WCET - tmp_step[i])) * FTFD(task_data[i].Laxity_Time)) + (prealpha * ITFD(rand_memory[i][tmp_step[i]]));
						//fprintf(stderr,"い");
					}
					
							/*if(task_data[i].Laxity_Time>0){
						//fprintf(stderr,"%d %d",i,(task_data[i].Laxity_Time));
					  NewLaxityjudge=(FTFD((double)1/shiftoperationtimes(task_data[i].Laxity_Time))); // 修正提案2022.12.1中田 左辺が固定小数点型なのに、右辺の除算が整数型になっており、小数点以下が求まらないので修正
						//fprintf(stderr,"う");
					}else{
						NewLaxityjudge=MAX;
					}
					if(NewLaxityjudge>Laxityjudge){
						Laxityjudge=NewLaxityjudge;
					}	*/					
                }
				//fprintf(stderr,"あ");
                //2ステップ目の評価値の合計とLaxityjudgeの設定
                for(k=0,s2val=0;k<TN;k++){
                    if(memberList(k,kumi2)==1 && state[k] == 1){ /* set2??i?r?b?g???1????s2val??]???l???i?[ */
						//valketa=shiftoperationtimes((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]) * task_data[k].Laxity_Time) - shiftoperationtimes(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]]);
                        //s2val+=((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]) * task_data[k].Laxity_Time) + (valketa>=0)?(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]] << valketa):(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]] >> abs(valketa));
						s2val+=((FTFD((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k])) * FTFD(task_data[k].Laxity_Time)) + (prealpha * ITFD(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]])));
						
					}
					/*if(task_data[k].Laxity_Time!=0){//0??????V?t?g???Z?????????????????Laxityjudge?????????t????????????????
					  	NewLaxityjudge=FTFD((double)1/shiftoperationtimes((task_data[k].Laxity_Time))); // 修正提案2022.12.1中田 左辺が固定小数点型なのに、右辺の除算が整数型になっており、小数点以下が求まらないので修正
					}else{
						NewLaxityjudge=MAX;
					}

					if(NewLaxityjudge>Laxityjudge){
						Laxityjudge=NewLaxityjudge;
					}*/
                }

                if(s1val>s1val+s2val){  //1?X?e?b?v???]???l????v??1?X?e?b?v??,2?X?e?b?v???]???l????v????????
                    val=s1val;
                }else{  //????????
                    val=s1val+s2val;
                }
				
              	if(minval>val){  //??????????????]???l?????????????
		    		minval=val;  bestkumi1=copyList(kumi1); bestkumi2=copyList(kumi2);
	    			if(alphauppermin<MAX){//???????????l??MAX????????????????????l?????????l?????2????????l???????????A?????]?T?????????????????l???
						//alpha=((alphalowermax + alphauppermin)/2) >> Laxityjudge;
						alpha=((alphalowermax + alphauppermin)/2);
					}else if(alphauppermin>=MAX){//????????????????????l???????????A?????]?T?????????????????l???
						//alpha=(alphalowermax) >> Laxityjudge;
						alpha=(alphalowermax);
					}
				}
			}			
		}
		freeList2(Aset2orig);
   	}
	freeList2(Aset1orig);	
		
		
	
	
	fprintf(stderr,"prealpha=%s,newalpha=%s\n",FixPointDecimalToString(prealpha),FixPointDecimalToString(alpha));
	fprintf(stderr,"tmp_step1:"); fprintList(bestkumi1);
	fprintf(stderr,"tmp_step2:"); fprintList(bestkumi2);
	fprintf(stderr,"minval=%s \n",FixPointDecimalToString(minval));
	fprintf(stderr,"Laxityjudge=%d \n",Laxityjudge);
	fprintf(stderr,"\n");
	pthread_mutex_unlock(&mutex);


	/*優先度関数値の初期化*/
	for(i=0;i<TN;i++){
		if(state[i] == 1 && tmp_step[i] != task_data[i].WCET){

			priority_func[i] = (alpha * ITFD(rand_memory[i][tmp_step[i]])) + (FTFD(task_data[i].WCET - tmp_step[i]) * FTFD(task_data[i].Laxity_Time));		

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

//?????????
int get_digit(int n){
	int digit=1;

	while(n/=10){
		digit++;
	}

	return digit;
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


//先頭からn番目までの要素をリストに入れて返す
/*
LList firstnList2(LList l,List n){

  	if(nullpList2(l) || n==0){  //lが空でないまたはnが0のとき空のリストを返す
    	return createList2();
  	}else{  //それ以外の時lの先頭要素をfirstnListに書き込む
    	return insertList2(headList2(l),firstnList2(tailList2(l),--n));
  	}
}
*/


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


//リストのリストに要素が含まれているかどうかを探索
/*
int memberList2(int element,List l){

  if(nullpList2(l)){
    return 0;
  }else if(element==headList2(l)){
    return 1;
  }else{
    return memberList2(element,tailList2(l));
  }
}
*/

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


//リストのメモリ解放
void freeList2(LList l){
	if(nullpList2(l)){
		return;
	}else{
		freeList2(tailList2(l));
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

//リストの複製を作成
/*
LList copyList2(LList l){

	return firstnList2(l,lengthList2(l));

}
*/

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

