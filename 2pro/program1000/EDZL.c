/*Machigasira Yuki*/
/*�����������炢��Ȃ��w�b�_�t�@�C�������邩���E�E�E*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

//#define P 2
#define P 1  				//�v���Z�b�T��
#define MAX 1000000000		//�N�����Ă��Ȃ���
#define S 1024

int TN = 0;					//�^�X�N��
double dead_max = 0;		//���΃f�b�h���C���̍ő�l
int rand_memory[S][S];		//�����������
double ET[S];				//Task��1�X�e�b�v�̎��s���Ԃ̕��ρi�{�����ł�1�X�e�b�v1�P�ʎ��ԂŎ��s�j
int schedule[S];			//Task���X�P�W���[�������Ƃ�1�C�����łȂ��Ƃ�0

int state[S];				//Task���N�����Ă���ꍇ�͒l��1�C�N�����Ă��Ȃ��ꍇ��0
int finish[S];				//�^�X�N�̏��������ׂďI�����Ă����1�C�����łȂ����0

int step[S];				//���݂̃X�e�b�v

double save_laxity[S];		//Task���I����������Laxity Time

int Worst_Memory = 0;		//�ň������������
int Current_Memory = 0;		//���݂̃����������

int deadline_miss_task[S];	//�f�b�h���C���~�X�̐�
int Deadline_Miss = 0;		//�f�b�h���C���~�X�̐��̑��a

/*sort�p�ϐ�(EDF)*/
double sort_absolute[S];
int sort_num_EDF[S];
/*sort�p�ϐ�(LLF)*/
double sort_laxity[S];
int sort_num_LLF[S];

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
void *thread_EDZL();					//EDZL Schedular

void EDF();  							//�f�b�h���C�����\�[�g����֐�
void LLF();								//�]�T���Ԃ��\�[�g����֐�
void memory_recode(int Memory);			//�ň�����������ʂ��L������֐�

void calc();							//Task�̏���
void load();							//�v�Z����

int pthread_yield(void);				//�R���p�C���Ɍx�����o�����Ȃ����߂̃v���g�^�C�v�錾�ł���.�Ȃ�,���̊֐���POSIX�ł͔�W���ł���,sched_yield()���g���̂��������Ƃ���.


/*�X���b�h�Ԃŕϐ���ی삷��mutex*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*�X���b�h�Ԃŏ�Ԃ𔻒f����cond*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int main(void) {

	fprintf(stderr, "\n----------------EDZL Scheduling in %d-Processor Environment----------------\n", P);
	
	int i = 0, j = 0, k1 = 0, k2 = 0;
	
	pthread_t Task[S];
	pthread_t EDZL_scheduler;
	
	/*�t�@�C���ǂݍ��ݗp�ϐ�*/
	char file_mem[256];								//������������̃t�@�C��
	char *file_dl = "rand_period_tasks.txt";		//�����̃t�@�C���i�{�����ł͎���=���΃f�b�h���C���j
	FILE *fMemory;
	FILE *fdl;
	int ret, x;
	
	int ptask;										//���̃^�X�N��
	int pdeadline[S];								//���̃f�b�h���C��
	double period_count = 0;						//�ŏ����{���܂ł̃X�e�b�v�ŉ��x�����[�X����邩

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
	
	/*�t�@�C��(�p�����[�^)�ǂݍ���*/
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
	pthread_create(&EDZL_scheduler, NULL, thread_EDZL, NULL);

	/*�X���b�h�̒�~*/
	pthread_join(EDZL_scheduler,NULL);
	
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

	fprintf(stderr, "\nThe Number of Deadline Miss = %d\n", Deadline_Miss);
	fprintf(stderr, "Worst Case Memory Consumption (EDZL) = %d\n", Worst_Memory);
	
	/*���O�̏o��*/
	FILE *fpout_EDZL;
	fpout_EDZL = fopen("datafile_EDZL.txt","a");
	fprintf(fpout_EDZL, "%d\t", Worst_Memory);		
	fprintf(fpout_EDZL, "%d\r\n", Deadline_Miss);
	fclose(fpout_EDZL);

	fprintf(stderr, "\n----------------EDZL Scheduling in %d-Processor Environment----------------\n\n", P);

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
	
	hook[*task_number] = 0;
	
	return 0;
}

/*EDZL Schedular*/
void *thread_EDZL(){
	
	int i,j;					//�J�E���g�p�ϐ�
	int hooker = 0;				//�t�b�N���������Ă���^�X�N��
	int fin = -1;				//�S�������I�����Ă���^�X�N��
	int total_memory;			//�X�P�W���[�������^�X�N�̑������������
	int count;					//�N�����Ă���^�X�N�̐�
	int sys_step = 0;			//�S�̂̃X�e�b�v
	int edzl_count;				//EDZL�ŃX�P�W���[�������^�X�N��
	
	pthread_t Task[TN];
	int ready[TN];				//�X���b�h���~�߂���0
	
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
		
		if(fin != TN && count > P){
			
			LLF();			//�]�T���Ԃ̏��Ȃ����Ƀ\�[�g
			
			/*LLF�ŃX�P�W���[�������^�X�N�̑I��(�]�T���Ԃ�0�ȉ�)*/
			edzl_count = 0;
			for(i=0;i<P;i++){
				if(state[sort_num_LLF[i]] == 1 && sort_laxity[i] <= 0){
					schedule[sort_num_LLF[i]] = 1;
					edzl_count++;
				}
			}
			
			EDF();			//�f�b�h���C���̏��Ȃ����Ƀ\�[�g
			
			/*EDF�ŃX�P�W���[�������^�X�N�̑I��*/
			i = 0;
			while(edzl_count < P){
				if(state[sort_num_EDF[i]] == 1 && schedule[sort_num_EDF[i]] == 0){
					schedule[sort_num_EDF[i]] = 1;
					edzl_count++;
				}
				
				i++;
			}
			
			/*�ȉ��̃R�����g�A�E�g���O���Əo�͂������*/		
			
			fprintf(stderr, "\n");
			for(j=0;j<TN;j++){
				if(sort_laxity[j] <= 0)
					fprintf(stderr, "Sort Laxity[%d] = %lf\n", sort_num_LLF[j]+1, sort_laxity[j]);
			}
			
			/*�ȉ��̃R�����g�A�E�g���O���Əo�͂������*/		
			
			fprintf(stderr, "\n");
			for(j=0;j<TN;j++)
				fprintf(stderr, "Sort Deadline[%d] = %lf\n", sort_num_EDF[j]+1, sort_absolute[j]);
			
		}else{
			
			EDF();
			LLF();
			
			/*�^�X�N��P�ȉ��̎�*/
			for(i=0;i<TN;i++){
				if(state[i] == 1)
					schedule[i] = 1;
			}
			
			/*�ȉ��̃R�����g�A�E�g���O���Əo�͂������*/		
			
			if(count >= 1 && count <= P){
				fprintf(stderr, "\n");
				for(j=0;j<TN;j++)
					fprintf(stderr, "Sort Deadline[%d] = %lf\n", sort_num_EDF[j]+1, sort_absolute[j]);
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
		
		hooker = 0;

	}
	
	return 0;
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

/*�f�b�h���C���̏��Ȃ����Ƀ\�[�g����֐�*/
void EDF(){

	double absolute[S];				//�f�b�h���C���i�[�ϐ��i��Ɨp�j
	int i = 0, j = 0, k = 0;		//�J�E���g�p�ϐ�
	
	/*�ϐ��̏�����*/
	for(i=0;i<TN;i++){
		if(state[i] == 1 && step[i] != task_data[i].WCET){
			absolute[i] = (task_data[i].WCET - step[i]) + task_data[i].Laxity_Time;			
		}else{
			absolute[i] = MAX;
		}
	}
	
	/*�\�[�g�p�ϐ�������*/
	for(k=0;k<TN;k++){
		sort_absolute[k] = absolute[k];
		sort_num_EDF[k] = k;
		//fprintf(stderr, "sort_absolute[%d] = %lf\n", k, sort_absolute[k]);
	}

	int t1 = 0, t2 = 0;
	/*�f�b�h���C���̏��Ȃ����Ƀ\�[�g����.*/
	for(j=0;j<TN-1;j++){
		for(k=TN-1;k>j;k--){
			if(sort_absolute[k] < sort_absolute[k-1]){
				t1 = sort_absolute[k];
				sort_absolute[k] = sort_absolute[k-1];
				sort_absolute[k-1] = t1;
				
				t2 = sort_num_EDF[k];
				sort_num_EDF[k] = sort_num_EDF[k-1];
				sort_num_EDF[k-1] = t2;
			}
		}
	}
	/*�f�o�b�O�p*/
	/*
	for(j=0;j<TN;j++){
		fprintf(stderr, "Laxity Time of Task%d = %lf\n", sort_num_EDF[j]+1, sort_absolute[j]);
	}
	fprintf(stderr, "\n");
	*/
	
	
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


