/*Machigasira Yuki*/
/*�����������炢��Ȃ��w�b�_�t�@�C�������邩���E�E�E*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

//#define P 2  				//�v���Z�b�T��
#define P 1  				//�v���Z�b�T��
#define MAX 1000000000000		//�N�����Ă��Ȃ���
//#define alpha 1000.0		//���Z���[�g
#define S 1024

int TN = 0;					//�^�X�N��
double dead_max = 0;		//���΃f�b�h���C���̍ő�l
int rand_memory[S][S];		//�����������
double ET[S];				//Task��1�X�e�b�v�̎��s���Ԃ̕��ρi�{�����ł�1�X�e�b�v1�P�ʎ��ԂŎ��s�j
int schedule[S];			//Task���X�P�W���[�������Ƃ�1�C�����łȂ��Ƃ�0

int state[S];				//Task���N�����Ă���ꍇ�͒l��1,�N�����Ă��Ȃ��ꍇ��0
int finish[S];				//�^�X�N�̏��������ׂďI�����Ă����1�C�����łȂ����0

int step[S];				//���݂̃X�e�b�v

double save_laxity[S];		//Task���I����������Laxity Time

int Worst_Memory = 0;		//�ň������������
int Current_Memory = 0;		//���݂̃����������

int deadline_miss_task[S];	//�f�b�h���C���~�X�̐�
int Deadline_Miss = 0;		//�f�b�h���C���~�X�̐��̑��a

double alpha=0,alphadiff=0; //�p�����[�^��(�����l)0�A���̏C������

/*sort�p�ϐ�*/
double sort_priority[S];
int sort_num_LMCLF[S];

int hook[S];				//�ꎞ��~���Ă���i�t�b�N���������Ă���j�ꍇ1�C�����łȂ����0


/*�\���̗̂v�f*/
/*Task�ԍ�(Task1��0),���΃f�b�h���C��,�ň����s����,�N������,���s�ς݂̎���,�]�T����*/
typedef struct{
	int Number;
	double Relative_Deadline;
	double WCET;
	double Release_Time;
	double Run_Time;
	double Laxity_Time;
}data;

data task_data[S];

void *thread_Tasks(void *num);			//�^�X�N
void *thread_LMCLF();					//LMCLF Schedular

void LMCLF();  							//�D��x�֐��l���\�[�g����֐�
void memory_recode(int Memory);			//�ň�����������ʂ��L������֐�

void calc();							//Task�̏���
void load();							//�v�Z����

int pthread_yield(void);				//�R���p�C���Ɍx�����o�����Ȃ����߂̃v���g�^�C�v�錾�ł���.�Ȃ�,���̊֐���POSIX�ł͔�W���ł���,sched_yield()���g���̂��������Ƃ���.

int get_digit(int n);                //������m��

/*�X���b�h�Ԃŕϐ���ی삷��mutex*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*�X���b�h�Ԃŏ�Ԃ𔻒f����cond*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int main(void) {

	fprintf(stderr, "\n-------------LMCLF Scheduling in %d-Processor Environment-------------\n", P);
	
	int i = 0, j = 0, k1 = 0, k2 = 0;
	
	pthread_t Task[S];
	pthread_t LMCLF_scheduler;
	
	/*�t�@�C���ǂݍ��ݗp�ϐ�*/
	char file_mem[256];								//������������̃t�@�C��
	char *file_dl = "rand_period_tasks.txt";		//�����̃t�@�C���i�{�����ł͎���=���΃f�b�h���C���j
	FILE *fMemory;
	FILE *fdl;
	int ret, x;
	
	int ptask;										//���̃^�X�N��
	int pdeadline[S];								//���̃f�b�h���C��
	int period_count = 0;							//�ŏ����{���܂ł̃X�e�b�v�ŉ��x�����[�X����邩
	
	
	pthread_mutex_lock(&mutex);
	/*�t�@�C���ǂݍ���*/
	fdl = fopen(file_dl,"r");
	ptask = 0;
	while(ret = fscanf(fdl,"%d",&x) != EOF){
		pdeadline[ptask] = x;
		//fprintf(stderr, "Relative Deadline [Task%d] = %5.0lf\n", ptask+1, task_data[ptask].Relative_Deadline);
		ptask++;
	}
	
	/*���΃f�b�h���C���̍ő�l*/
	for(i=0;i<ptask;i++){
		if(dead_max < pdeadline[i])
			dead_max = pdeadline[i];
	}
	
	/*�^�X�N���̌���*/
	for(j=0;j<ptask;j++){
		TN = TN + ceil(dead_max / pdeadline[j]);
		//fprintf(stderr,"%lf\n", ceil(dead_max / pdeadline[j]));
	}
	
	//fprintf(stderr,"%d\n", TN);
	
	/*Task_Data�̏�����*/
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
	
	/*�t�@�C���ǂݍ���*/
	k2 = 0;
	for(i=0;i<ptask;i++){
		
		period_count = ceil(dead_max / pdeadline[i]);
		
		for(k1=0;k1<(int) period_count;k1++){
			
			/*�����������*/
			sprintf(file_mem,"rand_memory_task%d.txt", i+1);
			fMemory = fopen(file_mem,"r");
			j = 0;
			while(ret = fscanf(fMemory,"%d",&x) != EOF){
				rand_memory[k2][j] = x;
				j++;
			}
			
			/*����*/
			task_data[k2].Relative_Deadline = pdeadline[i];
			
			/*�����[�X*/
			task_data[k2].Release_Time = k1 * pdeadline[i];
			
			/*WCET*/
			task_data[k2].WCET = j;
			
			k2++;
		}
	}
	
	/*�o��*/
	for(i=0;i<TN;i++)
		fprintf(stderr, "Relative Deadline [Task%d] = %5.0lf\n", i+1, task_data[i].Relative_Deadline);
	for(i=0;i<TN;i++)
		fprintf(stderr, "Release Time [Task%d] = %5.0lf\n", i+1, task_data[i].Release_Time);
	for(i=0;i<TN;i++)
		fprintf(stderr, "WCET [Task%d] = %5.0lf\n", i+1, task_data[i].WCET);
	pthread_mutex_unlock(&mutex);

	/*�X���b�h�̐���*/
	pthread_create(&LMCLF_scheduler, NULL, thread_LMCLF, NULL);

	/*�X���b�h�̒�~*/
	pthread_join(LMCLF_scheduler,NULL);
	
	/*�f�b�h���C���~�X����Task���J�E���g*/
	for(i=0;i<TN;i++){
		if(deadline_miss_task[i] > 0)
			Deadline_Miss++;
	}

	fprintf(stderr, "\n");
	/*�I�����̗]�T����*/
	for(i=0;i<TN;i++){
		save_laxity[i] += 0;
		fprintf(stderr, "When Finished Laxity Time of Task%d = %5.3lf\n", task_data[i].Number+1, save_laxity[i]);
	}
	
	fprintf(stderr, "The Number of Deadline Miss = %d\n", Deadline_Miss);
	fprintf(stderr, "Worst Case Memory Consumption (LMCLF) = %d\n", Worst_Memory);
	
	/*���O�̏o��*/
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


/*�^�X�N*/
void *thread_Tasks(void *num){

	int *task_number = (int *)num;	//�^�X�N�̎��ʎq
	
	int i;							//�J�E���g�p�ϐ�
	
	fprintf(stderr, "Task%d next starts\n", *task_number+1);
	pthread_mutex_lock(&mutex);
	state[*task_number] = 1;
	hook[*task_number] = 1;			//�^�X�N�Ƀt�b�N��������
	pthread_mutex_unlock(&mutex);
	
	/*Task�̗]�T���Ԃ��v�Z*/
	task_data[*task_number].Laxity_Time = task_data[*task_number].Relative_Deadline - task_data[*task_number].WCET + task_data[*task_number].Run_Time;
	task_data[*task_number].Laxity_Time += 0;
	fprintf(stderr, "Laxity Time [Task%d] = %5.3lf\n", *task_number+1, task_data[*task_number].Laxity_Time);
	
	/*���Ƀf�b�h���C���~�X���Ă���\�������邽��*/
	if(task_data[*task_number].Laxity_Time < -0.0)
		deadline_miss_task[*task_number]++;
	
	while(step[*task_number] < task_data[*task_number].WCET){
		
		while(hook[*task_number] == 1){
			/*suspend operation*/
			//fprintf(stderr, "FLAG\n");
		}
		
		if(schedule[*task_number] == 1){

			/*Task�̏���*/
			calc();

			/*�^�X�N�f�[�^�̍X�V*/
			task_data[*task_number].Run_Time = task_data[*task_number].Run_Time + ET[*task_number];
			task_data[*task_number].Relative_Deadline = task_data[*task_number].Relative_Deadline - ET[*task_number];
			task_data[*task_number].Laxity_Time = task_data[*task_number].Relative_Deadline - task_data[*task_number].WCET + task_data[*task_number].Run_Time;
			task_data[*task_number].Laxity_Time += 0;
			//fprintf(stderr, "Laxity Time of Scheduled Task%d = %5.3lf\n", *task_number+1, task_data[*task_number].Laxity_Time);		
			
			fprintf(stderr, "Scheduled Task = %d\n", *task_number+1);	
			//fprintf(stderr, "%d\n", *task_number+1);
			
			/*�ŏI�X�e�b�v�ł̏���*/
			if(step[*task_number] == task_data[*task_number].WCET - 1){
				
				fprintf(stderr, "Task%d finishes\n", *task_number+1);
				pthread_mutex_lock(&mutex);
				finish[*task_number] = 1;
				pthread_mutex_unlock(&mutex);
	
				save_laxity[*task_number] = task_data[*task_number].Laxity_Time;
				save_laxity[*task_number] += 0;
		
				/*�f�b�h���C���~�X�����^�X�N�𑪒�*/
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
			
			/*�v�Z����*/
			load();
			
			/*�^�X�N�f�[�^�̍X�V*/
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
	
	int i,j;					//�J�E���g�p�ϐ�
	int hooker = 0;				//�t�b�N���������Ă���^�X�N��
	int fin = -1;				//�S�������I�����Ă���^�X�N��
	int total_memory;			//�X�P�W���[�������^�X�N�̑������������
	int count;					//�N�����Ă���^�X�N�̐�
	int sys_step = 0;			//�S�̂̃X�e�b�v
	
	pthread_t Task[TN];
	int ready[TN];				//�X���b�h���~�߂���0
	
	
	fprintf(stderr,"Scheduler thread starts...\n");
	/*�S�^�X�N�̏������I������܂ŌJ��Ԃ�*/
	while(fin != TN)  {

		/*�X�P�W���[���ɎQ������^�X�N�����邩����*/
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
	
		/*�S�������I�����Ă���^�X�N���̑���ƃX���b�h�̒�~*/
		fin = 0;
		for(i=0;i<TN;i++){
			
			fin = fin + finish[i];
			
			/*�X���b�h�̒�~*/
			if(finish[i] == 1 && ready[i] == 1){
			  fprintf(stderr,"Task %d/%d thread stops...\n",i+1,TN);
				ready[i] = 0;
				pthread_join(Task[i],NULL);
			}
			
		}
		//fprintf(stderr, "fin = %d\n", fin);
		
		/*�S�^�X�N�f�[�^�̍X�V���I���܂őҋ@*/
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
			
			LMCLF();			//�D��x�֐��̏��������Ƀ\�[�g
			/*�ȉ��̃R�����g�A�E�g���O���Əo�͂������*/		
		
			fprintf(stderr, "\n");
			for(j=0;j<TN;j++)
				fprintf(stderr, "Sort LMCLF[%d] = (%lf * %d) + (%0.0lf * %0.0lf) = %0.1lf\n", sort_num_LMCLF[j]+1, alpha, rand_memory[sort_num_LMCLF[j]][step[sort_num_LMCLF[j]]], task_data[sort_num_LMCLF[j]].WCET-step[sort_num_LMCLF[j]], task_data[sort_num_LMCLF[j]].Laxity_Time, sort_priority[j]);
			
			/*�X�P�W���[�������^�X�N�̑I��*/
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
		memory_recode(total_memory);				//����������ʂ̋L�^
		
		sys_step++;				//�S�̂̃X�e�b�v���C���N�������g
		
		/*�t�b�N���O��(�S�^�X�N�������ĊJ)*/
		for(i=0;i<TN;i++) {
		  pthread_mutex_lock(&mutex);
		  hook[i] = 0;
		  pthread_mutex_unlock(&mutex);
		}
		
		hooker = 0;

	}
	
	return 0;
}


/*�D��x�֐��l�����������Ƀ\�[�g����֐�*/
void LMCLF(){

	double priority_func[S];		//�D��x�֐��l�i�[�ϐ��i��Ɨp�j
	double memory=0,minmemory=MAX;               //����������ʊi�[�ϐ�
	double priority_func1=0,priority_func2=0;
	int i = 0, j = 0, k = 0,l = 0;		//�J�E���g�p�ϐ�
	double alphauppermin=MAX,alphalowermax=0;   //���͈͍̔ő�ŏ�
	double alphaupperminsav=MAX,alphalowermaxsav=0;   //���͈͍̔ő�ŏ�(�ۑ��p)
	double tempalpha1=0,tempalpha2=0;  //���̃��̏�������i�[�֐�
	double WCETLaxity1=0,WCETLaxity2=0; //�c�]���s���ԁ~�]�T����
	double randma1=0,randma2=0;	//���~�����������
	int keta1=0,keta2=0; //���~������������Ǝc�]���s���ԁ~�]�T���Ԃ̌���������������
	int besti,bestk;   // �ŏ��������ƂȂ�i��k���L��
	
	pthread_mutex_lock(&mutex);
        alphadiff=0;
	/*���Z���[�g���̌���*/
	for(i=0;i<TN;i++){
	  alphauppermin=MAX,alphalowermax=0;
		if(state[i] == 1){
	    	for(j=0;j<TN;j++){
	    		if(state[j] == 1){
					if(i==j){

					}else if(rand_memory[i][step[i]] < rand_memory[j][step[j]]){  // m(i)��+Ci*Li<m(j)��+Cj*Lj && m(j)>m(i) --> Ci*Li-Cj*Lj<(m(j)-m(i))�� --> ��>(Ci*Li-Cj*Lj)/(m(j)-m(i))
						tempalpha1=(((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time)-((task_data[j].WCET - step[j]) * task_data[j].Laxity_Time))/((rand_memory[j][step[j]])-(rand_memory[i][step[i]]));
						WCETLaxity1=(((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time) + ((task_data[j].WCET - step[j]) * task_data[j].Laxity_Time))/2;
						randma1=((fabs(tempalpha1)*fabs(rand_memory[i][step[i]])) + (fabs(tempalpha1)*fabs(rand_memory[j][step[j]])))/2;
						if(alphalowermax<tempalpha1){	
		    				alphalowermax=tempalpha1;
		    				keta1=get_digit(randma1) - get_digit(WCETLaxity1);
		    				//printf("%d\n",keta1);
		  				}else{
						}
						//fprintf(stderr,"alpha>%lf for task i=%d to win j=%d\n",tempalpha1,i+1,j+1);
					}else{ // m(i)��+Ci*Li<m(j)��+Cj*Lj && m(j)<m(i) --> (m(i)-m(j))��<Cj*Lj-Ci*Li --> ��<(Cj*Lj-Ci*Li)/(m(i)-m(j))
						tempalpha2=(((task_data[j].WCET - step[j]) * task_data[j].Laxity_Time)-((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time))/((rand_memory[i][step[i]])-(rand_memory[j][step[j]]));
						WCETLaxity1=(((task_data[j].WCET - step[j]) * task_data[j].Laxity_Time) + ((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time))/2;
						randma1=((fabs(tempalpha2)*fabs(rand_memory[i][step[i]])) + (fabs(tempalpha2)*fabs(rand_memory[j][step[j]])))/2;
						if(alphauppermin>tempalpha2){
		    				alphauppermin=tempalpha2;
		    				keta1=get_digit(randma1) - get_digit(WCETLaxity1);								
						}
						//fprintf(stderr,"alpha<%lf for task i=%d to win j=%d\n",tempalpha2,i+1,j+1);
					}
	    		}	
	    	}
	    	if(!(alphauppermin>0 && alphalowermax < alphauppermin)){
	    		continue;
	    	}
	    	fprintf(stderr,"%lf <= alpha <= %lf for scheduling task %d first\n",alphalowermax,alphauppermin,i+1);
	    	alphaupperminsav=alphauppermin; alphalowermaxsav=alphalowermax;
	    	for(k=0;k<TN;k++){
	      		alphauppermin=alphaupperminsav; alphalowermax=alphalowermaxsav;
	    		if(state[k] == 1){
					for(l=0;l<TN;l++){
						if(state[l] == 1){
		    				if(k==l){

		    				}else if(rand_memory[k][(k==i)?(step[k]+1):step[k]] < rand_memory[l][(l==i)?(step[l]+1):step[l]] ){  // m(k)��+Ck*Lk<m(l)��+Cl*Ll && m(l)>m(k) --> Ck*Lk-Cl*Ll<(m(l)-m(k))�� --> ��>(Ck*Lk-Cl*Ll)/(m(l)-m(k))
		    					tempalpha1=(((task_data[k].WCET - (k==i)?(step[k]+1):step[k]) * task_data[k].Laxity_Time)-((task_data[l].WCET - (l==i)?(step[l]+1):step[l]) * task_data[l].Laxity_Time))/((rand_memory[l][(l==i)?(step[l]+1):step[l]])-(rand_memory[k][(k==i)?(step[k]+1):step[k]]));	
		    					WCETLaxity2=(((task_data[k].WCET - step[k]) * task_data[k].Laxity_Time) + ((task_data[l].WCET - step[l]) * task_data[l].Laxity_Time))/2;
		    					randma2=((fabs(tempalpha1)*fabs(rand_memory[k][step[k]])) + (fabs(tempalpha1)*fabs(rand_memory[l][step[l]])))/2;
		    					if(alphalowermax<tempalpha1){
									alphalowermax=tempalpha1;
									keta2=get_digit(randma2) - get_digit(WCETLaxity2);
									//printf("%lf\n",get_digit(randma));
		    					}else{
		    					}
		      					//fprintf(stderr,"alpha>%lf for task k=%d to win l=%d after i=%d\n",tempalpha1,k+1,l+1,i+1);
		    				}else{// m(k)��+Ck*Lk<m(l)��+Cl*Ll && m(l)<m(k) --> (m(k)-m(l))��<Cl*Ll-Ck*Lk  -->  ��<(Cl*Ll-Ck*Lk)/(m(k)-m(l))
		    					tempalpha2=(((task_data[l].WCET - (l==i)?(step[l]+1):step[l]) * task_data[l].Laxity_Time)-((task_data[k].WCET - (k==i)?(step[k]+1):step[k]) * task_data[k].Laxity_Time))/((rand_memory[k][(k==i)?(step[k]+1):step[k]])-(rand_memory[l][(l==i)?(step[l]+1):step[l]]));	
		    					WCETLaxity2=(((task_data[l].WCET - step[l]) * task_data[l].Laxity_Time) + ((task_data[k].WCET - step[k]) * task_data[k].Laxity_Time))/2;
		    					randma2=((fabs(tempalpha2)*fabs(rand_memory[l][step[l]])) + (fabs(tempalpha2)*fabs(rand_memory[k][step[k]])))/2;
		    					if(alphauppermin>tempalpha2){
									alphauppermin=tempalpha2;
									keta2=get_digit(randma2) - get_digit(WCETLaxity2);
		    					}
		    					//fprintf(stderr,"alpha<%lf for task k=%d to win l=%d after i=%d\n",tempalpha2,k+1,l+1,i+1);
		    				}	
						}
					}
					if(alphauppermin>0 && alphalowermax < alphauppermin){
						fprintf(stderr,"%lf <= alpha <= %lf for scheduling task %d and then task %d\n",alphalowermax,alphauppermin,i+1,k+1);
		  				if(i==k){
		    				if(rand_memory[i][step[i]+1]>0){
		    					memory=rand_memory[i][step[i]] + rand_memory[i][step[i]+1];
		    				}else{
		    					memory=rand_memory[i][step[i]];
		    				}
						}else{
		    				if(rand_memory[k][step[k]]>0){
		      					memory=rand_memory[i][step[i]] + rand_memory[k][step[k]];
		    				}else{                                                                                                                                                                                                                                                                                                                                                                                           
		      					memory=rand_memory[i][step[i]];
		    				}
		  				}
		  				if(minmemory>memory){
		    				minmemory=memory;  besti=i; bestk=k;
		    				if(alphauppermin<MAX){
		    					//alpha=(alphalowermax + alphauppermin)/2;
		    					if(((keta1+keta2)/2)>0){
									alphadiff=((alphalowermax + alphauppermin)/2-alpha)/(pow(10,(keta1+keta2)/2));
		    					}else{
									alphadiff=((alphalowermax + alphauppermin)/2-alpha);
		    					}
		    					//alphadiff=((alphalowermax + alphauppermin)/2-alpha)/1000; //���̖ڕW�l��(���+����)/2�Ƃ��āA���̒l��1/100000�����߂Â���
		    				}else{
		    					//alpha=alphalowermax;
		    					if(((keta1+keta2)/2)>0){
									alphadiff=(alphalowermax-alpha)/(pow(10,(keta1+keta2)/2));
		    					}else{
									alphadiff=(alphalowermax-alpha);
		    					}
		    					//alphadiff=(alphalowermax-alpha)/1000; //���̖ڕW�l�������Ƃ��āA���̒l��1/100000�����߂Â���
		    				}
						}
					}		
	    		}
	    	}	
		}
	}
	fprintf(stderr,"oldalpha=%lf, alphadiff=%lf\n",alpha,alphadiff);
	alpha+=alphadiff;  // ���̒l���X�V
	fprintf(stderr,"newalpha=%lf\n",alpha);
	fprintf(stderr,"minmemory=%lf when i=%d, k=%d\n",minmemory, besti+1, bestk+1);
	pthread_mutex_unlock(&mutex);

		

	/*�D��x�֐��l�̏�����*/
	for(i=0;i<TN;i++){
		if(state[i] == 1 && step[i] != task_data[i].WCET){

			priority_func[i] = (alpha * rand_memory[i][step[i]]) + ((task_data[i].WCET - step[i]) * task_data[i].Laxity_Time);		

		}else{
			priority_func[i] = MAX;
		}
	}

	/*�\�[�g�p�ϐ�������*/
	for(k=0;k<TN;k++){
		sort_priority[k] = priority_func[k];
		sort_num_LMCLF[k] = k;
		//fprintf(stderr, "sort_priority[%d] = %lf\n", k, sort_priority[k]);
	}
	
	int t1 = 0, t2 = 0;
	/*�D��x�֐��l�����������Ƀ\�[�g����.*/
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


/*�ň�����������ʂ��L������֐�*/
void memory_recode(int Memory){
	
	Current_Memory = Current_Memory + Memory;	//���݂̃����������
	fprintf(stderr, "Current_Memory = %d\n", Current_Memory);
	
	if(Worst_Memory < Current_Memory){
		Worst_Memory = Current_Memory;
		fprintf(stderr, "Worst_Memory = %d\n", Worst_Memory);
	}
	
}


/*Task�̏���*/

void calc(){
	int n,j;
	n = 1;
	for(j=1;j<1000000;j++){
		n *= 1;
	}
}


/*�v�Z����*/

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
