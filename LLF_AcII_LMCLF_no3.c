/*Machigasira Yuki*/
/*�����������炢��Ȃ��w�b�_�t�@�C�������邩���E�E�E*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

//#define P 2  			
#define P 1	//�v���Z�b�T��
#define MAX 1000000000		//�N�����Ă��Ȃ���
#define alpha 0.0			//���Z���[�g
#define S 1024

int TN = 0;					//�^�X�N��
double dead_max = 0;		//���΃f�b�h���C���̍ő�l
int rand_memory[S][S];		//�����������
double ET[S];				//Task��1�X�e�b�v�̎��s���Ԃ̕��ρi�{�����ł�1�X�e�b�v1�P�ʎ��ԂŎ��s�j
int schedule[S];			//Task���X�P�W���[�������Ƃ�1�C�����łȂ��Ƃ�0

int state[S];				//Task���N�����Ă���ꍇ�͒l��1,�N�����Ă��Ȃ��ꍇ��0
int finish[S];				//�^�X�N�̏��������ׂďI�����Ă����1�C�����łȂ����0

int step[S];				//���݂̃X�e�b�v
int sys_step = 0;			//�S�̂̃X�e�b�v
int tmp_step[S];			//��Ɨp�X�e�b�v

double save_laxity[S];		//Task���I����������Laxity Time

int Worst_Memory = 0;		//�ň������������
int Current_Memory = 0;		//���݂̃����������

int deadline_miss_task[S];	//�f�b�h���C���~�X�̐�
int Deadline_Miss = 0;		//�f�b�h���C���~�X�̐��̑��a

int ability = 0;			//0�̎��́CLLF�X�P�W���[���\���͔��������
int LLF_used = 0;			//�X�C�b�`���������Ă��Ȃ���

/*sort�p�ϐ�(LLF)*/
double sort_laxity[S];
int sort_num_LLF[S];
/*sort�p�ϐ�(LMCLF)*/
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
void *thread_LLF_AcII_LMCLF();			//LLF AcII-LMCLF-combined Schedular

void Schedulability_Analysis_Func();	//Schedulability Analysis
void LLF();  							//�]�T���Ԃ��\�[�g����֐�
void AcII_LMCLF();  					//�D��x�֐��l���\�[�g����֐�(�֐�LMCLF�Ƃ͏����Ⴄ�̂Œ���)
void memory_recode(int Memory);			//�ň�����������ʂ��L������֐�

void calc();							//Task�̏���
void load();							//�v�Z����

int pthread_yield(void);				//�R���p�C���Ɍx�����o�����Ȃ����߂̃v���g�^�C�v�錾�ł���.�Ȃ�,���̊֐���POSIX�ł͔�W���ł���,sched_yield()���g���̂��������Ƃ���.


/*�X���b�h�Ԃŕϐ���ی삷��mutex*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*�X���b�h�Ԃŏ�Ԃ𔻒f����cond*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int main(void) {

	fprintf(stderr, "\n-------------LLF AcII-LMCLF-combined Scheduling in %d-Processor Environment-------------\n", P);
	
	int i = 0, j = 0, k1 = 0, k2 = 0;
	
	pthread_t Task[S];
	pthread_t Combined_scheduler;
	
	/*�t�@�C���ǂݍ��ݗp�ϐ�*/
	char file_mem[256];								//������������̃t�@�C��
	char *file_dl = "rand_period_tasks.txt";		//�����̃t�@�C���i�{�����ł͎���=���΃f�b�h���C���j
	char *file_ut = "rand_utilization_tasks.txt";	//�^�X�N���p���̃t�@�C��
	FILE *fMemory;
	FILE *fdl, *fut;
	int ret, x;
	double y;
	
	int ptask;										//���̃^�X�N��
	int pdeadline[S];								//���̃f�b�h���C��
	double utilization[S];							//�^�X�N���p��
	double aver_ut = 0;								//�^�X�N���p���i���ρj
	int period_count = 0;							//�ŏ����{���܂ł̃X�e�b�v�ŉ��x�����[�X����邩
	
	/*�t�@�C���ǂݍ���*/
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
		tmp_step[i] = 0;
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

	/*�X���b�h�̐���*/
	pthread_create(&Combined_scheduler, NULL, thread_LLF_AcII_LMCLF, NULL);

	/*�X���b�h�̒�~*/
	pthread_join(Combined_scheduler,NULL);
	
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
	
	if(LLF_used > 0)
		fprintf(stderr, "\nSchedule Switched\n");
	
	fprintf(stderr, "The Number of Deadline Miss = %d\n", Deadline_Miss);
	fprintf(stderr, "Worst Case Memory Consumption (LLF AcII-LMCLF) = %d\n", Worst_Memory);
	
	/*���O�̏o��*/
	char log[256];
	FILE *fpout_Combined;
	sprintf(log, "datafile_LLF_AcII_LMCLF_%lf.txt", alpha);
	fpout_Combined = fopen(log, "a");
	fprintf(fpout_Combined, "%d\t", Worst_Memory);		
	fprintf(fpout_Combined, "%d\t", Deadline_Miss);
	fprintf(fpout_Combined, "%d\r\n", LLF_used);	
	fclose(fpout_Combined);

	fprintf(stderr, "\n-------------LLF AcII-LMCLF-combined Scheduling in %d-Processor Environment-------------\n\n", P);

	return 0;
}


/*�^�X�N*/
void *thread_Tasks(void *num){

	int *task_number = (int *)num;	//�^�X�N�̎��ʎq
	
	int i;							//�J�E���g�p�ϐ�	
	
	fprintf(stderr, "Task%d next starts\n", *task_number+1);
	state[*task_number] = 1;
	hook[*task_number] = 1;			//�^�X�N�Ƀt�b�N��������

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
				finish[*task_number] = 1;
	
				save_laxity[*task_number] = task_data[*task_number].Laxity_Time;
				save_laxity[*task_number] += 0;
		
				/*�f�b�h���C���~�X�����^�X�N�𑪒�*/
				if(task_data[*task_number].Laxity_Time < -0.0)
					deadline_miss_task[*task_number]++;
	
				if(deadline_miss_task[*task_number] > 0)
					fprintf(stderr, "task%d missed deadline\n", *task_number+1);
		
				state[*task_number] = 0;
				task_data[*task_number].Laxity_Time = MAX;
				
			}
			
			step[*task_number]++;
			
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
	
	int s1,s2,i,j;			//�J�E���g�p�ϐ�
	int hooker = 0;			//�t�b�N���������Ă���^�X�N��
	int fin = -1;			//�S�������I�����Ă���^�X�N��
	int total_memory;		//�X�P�W���[�������^�X�N�̑������������
	int count;				//�N�����Ă���^�X�N�̐�
	int loop;				
	int MIN_MCI,MCI;		//�ŏ��̏�����������C�����������
	int processor;			//�p����v���Z�b�T��
	int intask;				//�X�P�W���[�������^�X�N�̐�
	int twice[S];				//���ڑI�΂ꂽ�Ƃ�2
	
	pthread_t Task[TN];
	int ready[TN];			//�X���b�h���~�߂���0
	
	
	/*�S�^�X�N�̏������I������܂ŌJ��Ԃ�*/
	while(fin != TN){
		
		/*�X�P�W���[���ɎQ������^�X�N�����邩����*/
		for(i=0;i<TN;i++){	
			if(sys_step == task_data[i].Release_Time){
				pthread_create(&Task[i], NULL, thread_Tasks, &task_data[i].Number);
				state[i] = 1;
				ready[i] = 1;
			}
		}
	
		/*�S�������I�����Ă���^�X�N���̑���ƃX���b�h�̒�~*/
		fin = 0;
		for(i=0;i<TN;i++){
			
			fin = fin + finish[i];
			
			/*�X���b�h�̒�~*/
			if(finish[i] == 1 && ready[i] == 1){
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
		
		if(fin != TN && count > 0){
			
			/*�X�e�b�v����Ɨp�ϐ��Ɋi�[*/
			for(i=0;i<TN;i++){
				tmp_step[i] = step[i];
				twice[i] = 0;
			}
			
			i = 0;
			loop = 1;
			intask = 0;
			while(i<P && loop==1){	
		
				AcII_LMCLF();		//�D��x�֐��̏��������Ƀ\�[�g
			
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
				
					/*�X�P�W���[�������^�X�N�̑I�肨��ё�������������̌v�Z*/
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
		
		/*�\�[�g�̏�����*/
		for(i=0;i<TN;i++)
			tmp_step[i] = step[i];
			
		AcII_LMCLF();
		
		/*�X�P�W���[���\������*/
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
				/*�ȉ��̃R�����g�A�E�g���O���Əo�͂������(1)�������C(2)���͂�������*/
			
				for(i=0;i<TN;i++)
					tmp_step[i] = step[i];
			
				AcII_LMCLF();
			
				fprintf(stderr, "\n");
				for(j=0;j<TN;j++)
					fprintf(stderr, "Sort LMCLF[%d] = (%lf * %d) + (%0.0lf * %0.0lf) = %0.1lf\n", sort_num_LMCLF[j]+1, alpha, rand_memory[sort_num_LMCLF[j]][step[sort_num_LMCLF[j]]], task_data[sort_num_LMCLF[j]].WCET-step[sort_num_LMCLF[j]], task_data[sort_num_LMCLF[j]].Laxity_Time, sort_priority[j]);
			
			}else{
				/*�����߂�*/
				for(j=0;j<TN;j++)
					schedule[j] = 0;
				
				LLF();
				/*�ȉ��̃R�����g�A�E�g���O���Əo�͂������(2)*/
			
				fprintf(stderr, "\n");
				for(j=0;j<TN;j++)
					fprintf(stderr, "Sort Laxity[%d] = %lf\n", sort_num_LLF[j]+1, sort_laxity[j]);
				
				/*�X�P�W���[�������^�X�N�̑I��*/
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
		memory_recode(total_memory);				//����������ʂ̋L�^
		
		sys_step++;				//�S�̂̃X�e�b�v���C���N�������g
		
		/*�t�b�N���O��(�S�^�X�N�������ĊJ)*/
		for(i=0;i<TN;i++)
			hook[i] = 0;
		
		ability = 0;
		hooker = 0;
			
	}
	
	return 0;
}


/*Schedulability Analysis*/
void Schedulability_Analysis_Func(){
	
	int i = 0, j = 0;					//�J�E���g�p�ϐ�
	int p_num = 0;						//�g���v���Z�b�T��
	int state_all = 0;					//�N�����Ă���^�X�N��
	
	double SAF_Relative_Deadline[TN];	//�X�P�W���[���\������p�̃f�b�h���C��
	double SAF_Laxity_Time[TN];			//�X�P�W���[���\������p�̗]�T����
	double SAF_Run_Time[TN];			//�X�P�W���[���\������p�̎��s����
	
	ability = 0;
	
	/*���݂̃X�P�W���[�����O�T�C�N���̃p�����[�^�ɂ���*/
	for(i=0;i<TN;i++){
			
		if(state[i] == 1){
			SAF_Laxity_Time[i] = task_data[i].Relative_Deadline - task_data[i].WCET + task_data[i].Run_Time;
			/*�S�^�X�N����0�ŋN�������Ƃ���*/
			SAF_Relative_Deadline[i] = task_data[i].Relative_Deadline;
		}else{
			SAF_Laxity_Time[i] = task_data[i].Relative_Deadline + task_data[i].Release_Time - sys_step - task_data[i].WCET + task_data[i].Run_Time;
			/*�S�^�X�N����0�ŋN�������Ƃ���*/
			SAF_Relative_Deadline[i] = task_data[i].Relative_Deadline + task_data[i].Release_Time - sys_step;
		}
			

	}
	
	/*�p�����[�^�o��*/
	
	fprintf(stderr, "\n");
	
	fprintf(stderr, "Current Scheduling Cycle\n");
	
	for(i=0;i<TN;i++){
		if(finish[i] == 0){
			
			fprintf(stderr, "D [Task%d] = %0.0lf\t", i+1, SAF_Relative_Deadline[i]);
			fprintf(stderr, "C [Task%d] = %0.0lf\t", i+1, (task_data[i].WCET - task_data[i].Run_Time));
			fprintf(stderr, "L [Task%d] = %0.0lf\n", i+1, SAF_Laxity_Time[i]);
			
		}
	}
	
	/*�S�^�X�N�̃f�[�^����Ɨp�ϐ��Ɋi�[(���̃X�P�W���[�����O�T�C�N��)*/
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
	
	/*�p�����[�^�o��*/
	
	fprintf(stderr, "\n");
	
	fprintf(stderr, "Next Scheduling Cycle\n");
	
	fprintf(stderr, "Processor = %d\n", p_num);		//�g�p����v���Z�b�T��
	fprintf(stderr, "k_max = %0.0lf\n", dead_max);	//k�̍ő�
	fprintf(stderr, "ability = %d\n", ability);		//f(k,i)���v�Z���邩
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
	
	/*�]��v�Z�͂��v�Z*/
	double Fk;
	double R1,R2;
	int k = 1;
	
	if(state_all > 0){
		
		while(ability == 0 && k <= dead_max){
			Fk = 0;
			R1 = 0, R2 = 0;
			
			/*fk�̌v�Z*/
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
	
	/*�X�P�W���[���\������p�̕ϐ��̏�����*/
	for(i=0;i<TN;i++){
		SAF_Laxity_Time[i] = MAX;
		SAF_Relative_Deadline[i] = MAX;
		SAF_Run_Time[i] = MAX;
	}
		
	
}


/*�]�T���Ԃ̏��Ȃ����Ƀ\�[�g����֐�*/
void LLF(){

	double laxity[S];				//�]�T���Ԋi�[�ϐ��i��Ɨp�j
	int i = 0, j = 0, k = 0;		//�J�E���g�p�ϐ�

	/*�]�T���Ԃ̏�����*/
	for(i=0;i<TN;i++){
		if(state[i] == 1 && step[i] != task_data[i].WCET){
			laxity[i] = task_data[i].Laxity_Time;			
		}else{
			laxity[i] = MAX;
		}
	}
		
	/*Task�̗]�T���Ԃ��\�[�g*/
	/*�\�[�g�p�ϐ�������*/
	for(k=0;k<TN;k++){
		sort_laxity[k] = laxity[k];
		sort_num_LLF[k] = k;
		//fprintf(stderr, "sort_laxity[%d] = %lf\n", k, sort_laxity[k]);
	}

	int t1 = 0, t2 = 0;
	/*�]�T���Ԃ̏��Ȃ����Ƀ\�[�g����.*/
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
	/*�f�o�b�O�p*/
	/*
	for(j=0;j<TN;j++){
		fprintf(stderr, "Laxity Time of Task%d = %lf\n", sort_num_LLF[j]+1, sort_laxity[j]);
	}
	fprintf(stderr, "\n");
	*/
	
	
}


/*�D��x�֐��l�����������Ƀ\�[�g����֐�(�֐�LMCLF�ƈႤ�Ƃ����tmp_step�Ōv�Z���Ă���Ƃ���)*/
void AcII_LMCLF(){

	double priority_func[S];		//�D��x�֐��l�i�[�ϐ��i��Ɨp�j
	int i = 0, j = 0, k = 0;		//�J�E���g�p�ϐ�
	
	/*�D��x�֐��l�̏�����*/
	for(i=0;i<TN;i++){
		if(state[i] == 1 && tmp_step[i] != task_data[i].WCET){

			priority_func[i] = (alpha * rand_memory[i][tmp_step[i]]) + ((task_data[i].WCET - tmp_step[i]) * task_data[i].Laxity_Time);		

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
	//fprintf(stderr, "Current_Memory = %d\n", Current_Memory);
	
	if(Worst_Memory < Current_Memory){
		Worst_Memory = Current_Memory;
		//fprintf(stderr, "Worst_Memory = %d\n", Worst_Memory);
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


