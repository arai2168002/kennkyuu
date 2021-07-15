#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define P 4 
//#define P 2  				//プロセッサ数
//#define P 1  				//プロセッサ数
#define MAX 1000000000000		//起動していない時
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

double save_laxity[S];		//Taskが終了した時のLaxity Time

int Worst_Memory = 0;		//最悪メモリ消費量
int Current_Memory = 0;		//現在のメモリ消費量

int deadline_miss_task[S];	//デッドラインミスの数
int Deadline_Miss = 0;		//デッドラインミスの数の総和

double alpha=0,alphadiff=0; //パラメータα(初期値)0、αの修正差分

/*sort用変数*/
double sort_priority[S];
int sort_num_LMCLF[S];

int hook[S];				//一時停止している（フックがかかっている）場合1，そうでなければ0


/*構造体の要素*/
/*Task番号(Task1は0),相対デッドライン,最悪実行時間,起動時刻,実行済みの時間,余裕時間,選ばれたタスク,選ばれなかったタスク*/
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
void *thread_LMCLF();					//LMCLF Schedular

void LMCLF();  							//優先度関数値をソートする関数
void memory_recode(int Memory);			//最悪メモリ消費量を記憶する関数

void calc();							//Taskの処理
void load();							//計算負荷

int pthread_yield(void);				//コンパイラに警告を出させないためのプロトタイプ宣言である.なお,この関数はPOSIXでは非標準であり,sched_yield()を使うのが正しいとある.

int get_digit(int n);                //桁数を知る




/*スレッド間で変数を保護するmutex*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*スレッド間で状態を判断するcond*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int main(void) {

	fprintf(stderr, "\n-------------LMCLF Scheduling in %d-Processor Environment-------------\n", P);
	
	int i = 0, j = 0, k1 = 0, k2 = 0;
	
	pthread_t Task[S];
	pthread_t LMCLF_scheduler;
	
	/*ファイル読み込み用変数*/
	char file_mem[256];								//消費メモリ増分のファイル
	char *file_dl = "rand_period_tasks.txt";		//周期のファイル（本実験では周期=相対デッドライン）
	FILE *fMemory;
	FILE *fdl;
	int ret, x;
	
	int ptask;										//仮のタスク数
	int pdeadline[S];								//仮のデッドライン
	int period_count = 0;							//最小公倍数までのステップで何度リリースされるか
	
	
	pthread_mutex_lock(&mutex);
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
	pthread_mutex_unlock(&mutex);

	/*スレッドの生成*/
	pthread_create(&LMCLF_scheduler, NULL, thread_LMCLF, NULL);

	/*スレッドの停止*/
	pthread_join(LMCLF_scheduler,NULL);
	
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
	fprintf(stderr, "Worst Case Memory Consumption (LMCLF) = %d\n", Worst_Memory);
	
	/*ログの出力*/
	char log[256];
	FILE *fpout_LMCLF;
	sprintf(log, "datafile_LMCLF_%lf.txt", alpha);
	fpout_LMCLF = fopen(log, "a");
	fprintf(fpout_LMCLF, "%d\t", Worst_Memory);		
	fprintf(fpout_LMCLF, "%d\r\n", Deadline_Miss);
	fclose(fpout_LMCLF);

	fprintf(stderr, "\n-------------LMCLF Scheduling in %d-Processor Environment-------------\n\n", P);

	return 0;
}


/*タスク*/
void *thread_Tasks(void *num){

	int *task_number = (int *)num;	//タスクの識別子
	
	int i;							//カウント用変数
	
	fprintf(stderr, "Task%d next starts\n", *task_number+1);
	pthread_mutex_lock(&mutex);
	state[*task_number] = 1;
	hook[*task_number] = 1;			//タスクにフックをかける
	pthread_mutex_unlock(&mutex);
	
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
				pthread_mutex_lock(&mutex);
				finish[*task_number] = 1;
				pthread_mutex_unlock(&mutex);
	
				save_laxity[*task_number] = task_data[*task_number].Laxity_Time;
				save_laxity[*task_number] += 0;
		
				/*デッドラインミスしたタスクを測定*/
				if(task_data[*task_number].Laxity_Time < -0.0)
					deadline_miss_task[*task_number]++;
	
				if(deadline_miss_task[*task_number] > 0)
					fprintf(stderr, "task%d missed deadline\n", *task_number+1);
		
				pthread_mutex_lock(&mutex);
				state[*task_number] = 0;
				pthread_mutex_unlock(&mutex);
				task_data[*task_number].Laxity_Time = MAX;
				
			}
			
			pthread_mutex_lock(&mutex);
			step[*task_number]++;
			pthread_mutex_unlock(&mutex);
			
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
		
         	pthread_mutex_lock(&mutex);
		schedule[*task_number] = 0;
		hook[*task_number] = 1;
        	pthread_mutex_unlock(&mutex);
	
	}
	
	pthread_mutex_lock(&mutex);
	rand_memory[*task_number][step[*task_number]] = 0;
	step[*task_number] = task_data[*task_number].WCET;
	hook[*task_number] = 0;
	pthread_mutex_unlock(&mutex);
	
	return 0;
}

/*LMCLF Schedular*/
void *thread_LMCLF(){
	
	int i,j;					//カウント用変数
	int hooker = 0;				//フックがかかっているタスク数
	int fin = -1;				//全処理が終了しているタスク数
	int total_memory;			//スケジュールされるタスクの総消費メモリ増分
	int count;					//起動しているタスクの数
	int sys_step = 0;			//全体のステップ
	
	pthread_t Task[TN];
	int ready[TN];				//スレッドを止めたら0
	
	
	fprintf(stderr,"Scheduler thread starts...\n");
	/*全タスクの処理が終了するまで繰り返す*/
	while(fin != TN)  {

		/*スケジュールに参加するタスクがあるか判定*/
		for(i=0;i<TN;i++){
		  //fprintf(stderr,"sys_step=%d,task_data[%d].Release_Time=%d\n",sys_step,i,task_data[i].Release_Time);
			if(sys_step == task_data[i].Release_Time){
			  fprintf(stderr,"Task %d/%d thread starts...\n",i+1,TN);
				pthread_create(&Task[i], NULL, thread_Tasks, &task_data[i].Number);
				pthread_mutex_lock(&mutex);
				state[i] = 1;
				pthread_mutex_unlock(&mutex);
				ready[i] = 1;

			}
		}
	
		/*全処理が終了しているタスク数の測定とスレッドの停止*/
		fin = 0;
		for(i=0;i<TN;i++){
			
			fin = fin + finish[i];
			
			/*スレッドの停止*/
			if(finish[i] == 1 && ready[i] == 1){
			  fprintf(stderr,"Task %d/%d thread stops...\n",i+1,TN);
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

		fprintf(stderr,"fin=%d,count=%d\n",fin,count);
		if(fin != TN && count > 0){
			
			LMCLF();			//優先度関数の小さい順にソート
			/*以下のコメントアウトを外すと出力が見れる*/		
		
			fprintf(stderr, "\n");
			for(j=0;j<TN;j++)
				fprintf(stderr, "Sort LMCLF[%d] = (%lf * %d) + (%0.0lf * %0.0lf) = %0.1lf\n", sort_num_LMCLF[j]+1, alpha, rand_memory[sort_num_LMCLF[j]][step[sort_num_LMCLF[j]]], task_data[sort_num_LMCLF[j]].WCET-step[sort_num_LMCLF[j]], task_data[sort_num_LMCLF[j]].Laxity_Time, sort_priority[j]);
			
			/*スケジュールされるタスクの選定*/
			for(i=0;i<P;i++){
			  if(state[sort_num_LMCLF[i]] == 1) {
				  pthread_mutex_lock(&mutex);
				  schedule[sort_num_LMCLF[i]] = 1;
				  pthread_mutex_unlock(&mutex);
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
		for(i=0;i<TN;i++) {
		  pthread_mutex_lock(&mutex);
		  hook[i] = 0;
		  pthread_mutex_unlock(&mutex);
		}
		
		hooker = 0;

	}
	
	return 0;
}


/*優先度関数値が小さい順にソートする関数*/
void LMCLF(){

	double priority_func[S];		//優先度関数値格納変数（作業用）
	double memory=0,minmemory=MAX;               //メモリ消費量格納変数
	double priority_func1=0,priority_func2=0;
	int i = 0, j = 0, k = 0,l = 0,c = 0;		//カウント用変数
	unsigned long long  set1=0,set2=0; //2ステップ分のプロセッサ数分のタスク集合
	double alphauppermin=MAX,alphalowermax=0;   //αの範囲最大最小
	double alphaupperminsav=MAX,alphalowermaxsav=0;   //αの範囲最大最小(保存用)
    double s1memory=0,s2memory=0;  //1ステップ目,2ステップ目のメモリの合計
	double tempalpha1=0,tempalpha2=0;  //仮のαの上限下限格納関数
	double WCETLaxity1=0,WCETLaxity2=0; //残余実行時間×余裕時間
	double randma1=0,randma2=0;	//α×消費メモリ増分
	int keta1=0,keta2=0; //α×消費メモリ増分と残余実行時間×余裕時間の桁数を引いたもの
	int besti,bestk;   // 最小メモリとなるiとkを記憶
	
	pthread_mutex_lock(&mutex);
        alphadiff=0;

       //fprintf(stderr,"%lld\n",set1);
	/*換算レートαの決定*/
    for(set1=1;set1<pow(2,TN);set1++){ //1ステップ目のプロセッサ分のタスク集合
	    for(i=0,c=0;i<TN || c<=P;i++){
			if((set1&(int)pow(2,i))!=0){  /* set1のiビット目が1ならば */
				c++;  /* Iの要素数cを数える */
			}
		}
		if(c!=P){/* |I|!=Pなら現在のset1は無効として次のset1を試す */
			continue;
		}	
		alphauppermin=MAX,alphalowermax=0;    
		for(i=0;i<TN;i++){
			//alphauppermin=MAX,alphalowermax=0;
			if((set1&(int)pow(2,i))!=0 && state[i] == 1){  /* set1のiビット目が1ならば */
				for(j=0;j<TN;j++){
					if((set1&(int)pow(2,j))==0 && state[j] == 1){  //set1のiビット目が0ならば
						if(rand_memory[i][step[i]] < rand_memory[j][step[j]]){// m(i)α+Ci*Li<m(j)α+Cj*Lj && m(j)>m(i) --> Ci*Li-Cj*Lj<(m(j)-m(i))α --> α>(Ci*Li-Cj*Lj)/(m(j)-m(i))
							tempalpha1=(((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time)-((task_data[j].WCET - step[j]) * task_data[j].Laxity_Time))/((rand_memory[j][step[j]])-(rand_memory[i][step[i]]));
							WCETLaxity1=(((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time) + ((task_data[j].WCET - step[j]) * task_data[j].Laxity_Time))/2;
							randma1=((fabs(tempalpha1)*fabs(rand_memory[i][step[i]])) + (fabs(tempalpha1)*fabs(rand_memory[j][step[j]])))/2;
							if(alphalowermax<tempalpha1){	
		    					alphalowermax=tempalpha1;
		    					keta1=get_digit(randma1) - get_digit(WCETLaxity1);
								fprintf(stderr,"1steplower 選ばれたi,jはi=%d,j=%d,alphalowermax=%lf,\n",i,j,alphalowermax);
							}else{
							}
						}else{// m(i)α+Ci*Li<m(j)α+Cj*Lj && m(j)<m(i) --> (m(i)-m(j))α<Cj*Lj-Ci*Li --> α<(Cj*Lj-Ci*Li)/(m(i)-m(j))
							tempalpha2=(((task_data[j].WCET - step[j]) * task_data[j].Laxity_Time)-((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time))/((rand_memory[i][step[i]])-(rand_memory[j][step[j]]));
							WCETLaxity1=(((task_data[j].WCET - step[j]) * task_data[j].Laxity_Time) + ((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time))/2;
							randma1=((fabs(tempalpha2)*fabs(rand_memory[i][step[i]])) + (fabs(tempalpha2)*fabs(rand_memory[j][step[j]])))/2;
							if(alphauppermin>tempalpha2){
		    					alphauppermin=tempalpha2;
		    					keta1=get_digit(randma1) - get_digit(WCETLaxity1);								
								fprintf(stderr,"1stepupper 選ばれたi,jはi=%d,j=%d,alphauppermin=%lf,\n",i,j,alphauppermin);
							}
						}
					}
				}
            }
        }
	    if(!(alphauppermin>0 && alphalowermax < alphauppermin)){
	    	continue;
	    }
	    fprintf(stderr,"%lf <= alpha <= %lf for scheduling task %d first\n",alphalowermax,alphauppermin,i+1);
	    alphaupperminsav=alphauppermin; alphalowermaxsav=alphalowermax;

		for(set2=1;set2<pow(2,TN);set2++){ //2ステップ目のプロセッサ分のタスク集合
	    	for(k=0,c=0;k<TN || c<=P;k++){
				if((set2&(int)pow(2,k))!=0 ){  /* set2のiビット目が1ならば */
					c++;  /* Iの要素数cを数える */
				}
			}
			if(c!=P){/* |I|>P または|I|>Pなら現在のset2は無効として次のset2を試す */
				continue;
			}
			alphauppermin=alphaupperminsav; alphalowermax=alphalowermaxsav;
	    	for(k=0;k<TN;k++){
	      		//alphauppermin=alphaupperminsav; alphalowermax=alphalowermaxsav;
	    		if((set2&(int)pow(2,k))!=0 && state[k] == 1){ /* set2のiビット目が1ならば */
					for(l=0;l<TN;l++){
						if((set2&(int)pow(2,l))==0 && state[l] == 1){  /* set2のiビット目が0ならば */
		    				if(rand_memory[k][(k==i)?(step[k]+1):step[k]] < rand_memory[l][(l==i)?(step[l]+1):step[l]] ){  // m(k)α+Ck*Lk<m(l)α+Cl*Ll && m(l)>m(k) --> Ck*Lk-Cl*Ll<(m(l)-m(k))α --> α>(Ck*Lk-Cl*Ll)/(m(l)-m(k))
		    					tempalpha1=(((task_data[k].WCET - (k==i)?(step[k]+1):step[k]) * task_data[k].Laxity_Time)-((task_data[l].WCET - (l==i)?(step[l]+1):step[l]) * task_data[l].Laxity_Time))/((rand_memory[l][(l==i)?(step[l]+1):step[l]])-(rand_memory[k][(k==i)?(step[k]+1):step[k]]));	
		    					WCETLaxity2=(((task_data[k].WCET - step[k]) * task_data[k].Laxity_Time) + ((task_data[l].WCET - step[l]) * task_data[l].Laxity_Time))/2;
		    					randma2=((fabs(tempalpha1)*fabs(rand_memory[k][step[k]])) + (fabs(tempalpha1)*fabs(rand_memory[l][step[l]])))/2;
		    					if(alphalowermax<tempalpha1){
									alphalowermax=tempalpha1;
									keta2=get_digit(randma2) - get_digit(WCETLaxity2);
									fprintf(stderr,"2steplower:選ばれたk,lはk=%d,l=%d,alphalowermax=%lf,\n",k,l,alphalowermax);
		    					}else{
		    					}
		    				}else{// m(k)α+Ck*Lk<m(l)α+Cl*Ll && m(l)<m(k) --> (m(k)-m(l))α<Cl*Ll-Ck*Lk  -->  α<(Cl*Ll-Ck*Lk)/(m(k)-m(l))
		    					tempalpha2=(((task_data[l].WCET - (l==i)?(step[l]+1):step[l]) * task_data[l].Laxity_Time)-((task_data[k].WCET - (k==i)?(step[k]+1):step[k]) * task_data[k].Laxity_Time))/((rand_memory[k][(k==i)?(step[k]+1):step[k]])-(rand_memory[l][(l==i)?(step[l]+1):step[l]]));	
		    					WCETLaxity2=(((task_data[l].WCET - step[l]) * task_data[l].Laxity_Time) + ((task_data[k].WCET - step[k]) * task_data[k].Laxity_Time))/2;
		    					randma2=((fabs(tempalpha2)*fabs(rand_memory[l][step[l]])) + (fabs(tempalpha2)*fabs(rand_memory[k][step[k]])))/2;
		    					if(alphauppermin>tempalpha2){
									alphauppermin=tempalpha2;
									keta2=get_digit(randma2) - get_digit(WCETLaxity2);
									fprintf(stderr,"2stepupper:選ばれたk,lはk=%d,l=%d,alphauppermin=%lf,\n",k,l,alphauppermin);
		    					}
		    				}	
						}
					}
                }
            }
            if(alphauppermin>0 && alphalowermax < alphauppermin){ //求めたいαが条件を満たしている時
                fprintf(stderr,"%lf <= alpha <= %lf for scheduling task %d and then task %d\n",alphalowermax,alphauppermin,i+1,k+1);
                //1ステップ目のメモリ増分の合計
                for(i=0;i<TN;i++){
                    if((set1&(int)pow(2,i))!=0){ /* set1のiビット目が1ならば */
                        s1memory=rand_memory[i][step[i]];
                    }
                }
                //2ステップ目のメモリ増分の合計
                for(i=0;i<TN;i++){
                    if((set1&(int)pow(2,i))!=0){ /* set1のiビット目が1ならば */
                        for(k=0;k<TN;k++){
                            if((set2&(int)pow(2,k))!=0){ /* set2のiビット目が1ならば */
                                s2memory=rand_memory[k][(k==i)?(step[k]+1):step[k]];
                            }
                        }
                    }
                }
                if(s1memory>s1memory+s2memory){  //1ステップ目のメモリ増分の合計が1ステップ目,2ステップ目のメモリ増分の合計より大きい場合
                    memory=s1memory;
                }else{  //小さい場合
                    memory=s1memory+s2memory;
                }

              	if(minmemory>memory){  //今まで求めた最悪メモリ消費量よりも小さいとき
		    		minmemory=memory;  besti=i; bestk=k;
		    		if(alphauppermin<MAX){
						if(((keta1+keta2)/2)>0){
							alpha=((alphalowermax + alphauppermin)/2)/(pow(10,(keta1+keta2)/2));
	    				}else{
        					alpha=((alphalowermax + alphauppermin)/2);
		    			}
		    		}else{
		    		    if(((keta1+keta2)/2)>0){
						    alpha=(alphalowermax)/(pow(10,(keta1+keta2)/2));
						}else{
							alpha=(alphalowermax);
						}
	    			}
				}
			}			
		}
   	}		
		
		
	
	fprintf(stderr,"oldalpha=%lf, alphadiff=%lf\n",alpha,alphadiff);
	fprintf(stderr,"newalpha=%lf\n",alpha);
	fprintf(stderr,"minmemory=%lf when i=%d, k=%d\n",minmemory, besti+1, bestk+1);
	pthread_mutex_unlock(&mutex);

		

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
	fprintf(stderr, "Current_Memory = %d\n", Current_Memory);
	
	if(Worst_Memory < Current_Memory){
		Worst_Memory = Current_Memory;
		fprintf(stderr, "Worst_Memory = %d\n", Worst_Memory);
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

int get_digit(int n){
	int digit=1;

	while(n/=10){
		digit++;
	}

	return digit;
}