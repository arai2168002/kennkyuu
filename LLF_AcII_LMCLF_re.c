/*Machigasira Yuki*/
/*�����������炢��Ȃ��w�b�_�t�@�C�������邩���E�E�E*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

//#define P 2  
#define P 4	
//#define P 1			//�v���Z�b�T��
//#define MAX 1000000000		//�N�����Ă��Ȃ���
//#define alpha 1000.0		//���Z���[�g
#define S 1024

#define NIL ((List)0)
#define LNIL ((LList)0) 
#define S 1024
#define FIXEDPOINT 2	//????????????????2?i?????????V?t?g?????

typedef long long FIXPOINTDECIMAL;		//long long?^????????
#define ITFD(x) ((FIXPOINTDECIMAL)(x << FIXEDPOINT))	//int?^????l???????????



FIXPOINTDECIMAL MAX=2147483647; //?N????????????

int TN = 0;					//�^�X�N��
int valTN=P*2;				//?]???l???�?I?????^?X?N??
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

FIXPOINTDECIMAL alpha=ITFD(1),alphadiff=0; //?p?????[?^??(?????l)0?A????C??????

/*sort�p�ϐ�(LLF)*/
double sort_laxity[S];
int sort_num_LLF[S];
/*sort�p�ϐ�(LMCLF)*/
FIXPOINTDECIMAL sort_priority[S];
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

typedef struct cell {
  FIXPOINTDECIMAL element;
  struct cell *next;
} Cell,*List;

typedef struct llcell {
  List element;
  struct llcell *next;
} LLCell,*LList;

data task_data[S];

void *thread_Tasks(void *num);			//�^�X�N
void *thread_LLF_AcII_LMCLF();			//LLF AcII-LMCLF-combined Schedular

void Schedulability_Analysis_Func();	//Schedulability Analysis
void LLF();  							//�]�T���Ԃ��\�[�g����֐�
void AcII_LMCLF();  					//�D��x�֐��l���\�[�g����֐�(�֐�LMCLF�Ƃ͏����Ⴄ�̂Œ���)
void memory_recode(int Memory);			//�ň�����������ʂ��L������֐�

void calc();							//Task�̏���
void load();							//�v�Z����

List createList(void);				//��̃��X�g���쐬
LList createList2(void);
List insertList(FIXPOINTDECIMAL element,List l);	//���X�g�̐擪�ɗv�f��ǉ�����
LList insertList2(List element,LList l);
LList firstList(int element,LList l);
LList insertElementToAll(int element ,LList l);
int nullpList(List l);				//���X�g���󂩂ǂ�����Ԃ�
int nullpList2(LList l);
int headList(List l);				//���X�g�̐擪�v�f��Ԃ�
List tailList(List l);				//���X�g�̐擪�v�f���폜
List headList2(LList l);  // �C����āi�v���g�^�C�v�錾�ǉ��j (2022.11.25 ���c)
LList tailList2(LList l);
int tailarray(List l,int i);		//
List appendList(List l1,List l2);	//���X�gl1�̖�����l���X�gl2��A��
LList appendList2(LList l1,LList l2);
int iList(List l1,int i);			//���X�g��i�Ԗڂ̗v�f��Ԃ�
void printList(List l);				//���X�g�̗v�f��\��
void fprintList(List l);            //���X�g�̑S�v�f���t�@�C���ɏo��
void fprintList2(LList l);
void fprintFPList(List l);			//���X�g�̑S�v�f��������t�@�C���ɏo��
int lengthList(List l);				//���X�g�̒������擾
int lengthList2(LList l);	
List firstnList(List l,unsigned int n);		//���X�g�̐擪����n�Ԗڂ܂ł̗v�f���폜
LList firstnList2(LList,List n);
List restnList(List l,unsigned int n);		//���X�g�̖�������n�Ԗڂ܂ł̗v�f���폜
int memberList(int element,List l);     //���X�g�ɂ��̗v�f���܂܂�Ă��邩�ǂ����𒲂ׂ�֐�
int memberList2(List l);
void freeList(List l);		//���X�g�̃��������
void freeList2(LList l);
List copyList(List l);		//���X�g�̕����𐶐�
int minList(List l,int tasknum1,int tasknum2,FIXPOINTDECIMAL min);	//���X�g�̕]���l�̍ŏ��l�̃^�X�N�ԍ���Ԃ�
List ideleatList(List l1,List l2,int i);		//�擪����i�Ԗڂ̗v�f������������
List setList(List sourceList,List subsetList,int begin,int end);		//�g�ݍ��킹�̑S�p�^�[�����i�[�������X�g��Ԃ�
LList Makesubsetset(List T,int i,int j);		//����̗v�f���̕����W�����
List subsetset(int n,List T);		//�v�fn���e�����W���ɒǉ�
int calcNumOfCombination(int n, int r);		//�g�ݍ��킹�̑�����Ԃ�
int shiftoperationtimes(FIXPOINTDECIMAL n);		//�V�t�g���Z�ɂ�茅�����擾
FIXPOINTDECIMAL FTFD(double x);				//double�^�̐��l���Œ菬���ɕϊ�
char *FixPointDecimalToString(FIXPOINTDECIMAL x);		//�Œ菬���̒l�𐮐������Ə��������Ƃŕ�����ɕϊ�

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
	
	FILE *fpout_Task_Set;
	fpout_Task_Set = fopen("datafile_Task_Set.txt","a");
	fprintf(fpout_Task_Set, "%d\t", ptask);
	fprintf(fpout_Task_Set, "%lf\r\n", aver_ut);
	fclose(fpout_Task_Set);

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
	int loop;				//0��while����E�o
	int MIN_MCI,MCI;		//�ŏ��̏�����������C�����������
	int processor;			//�p����v���Z�b�T��
	int intask;				//�X�P�W���[�������^�X�N�̐�
	int twice[S];			//���ڑI�΂ꂽ�Ƃ�2
	
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
					fprintf(stderr, "Sort LMCLF[%d] = (%s * %d) + (%0.0lf * %0.0lf) = %0.1lf\n", sort_num_LMCLF[j]+1, FixPointDecimalToString(alpha), rand_memory[sort_num_LMCLF[j]][step[sort_num_LMCLF[j]]], task_data[sort_num_LMCLF[j]].WCET-step[sort_num_LMCLF[j]], task_data[sort_num_LMCLF[j]].Laxity_Time, FixPointDecimalToString(ITFD(sort_priority[j])));
			
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
		//??????????????????????�?????????????????p???]???l??v?Z
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
		//fprintf(stderr,"kumi1 %d�g��:",set1); fprintList(kumi1);

		alphauppermin=MAX,alphalowermax=0;    
		for(i=0;i<TN;i++){
			if(memberList(i,kumi1)==1 && state[i] == 1){  /* i?^?X?N????I??????????^?X?N???N??????????? */
				for(j=0;j<TN;j++){
					if(memberList(j,kumi1)==0 && state[j] == 1){  //j?^?X?N????I????????????^?X?N???N???????????
						if(rand_memory[i][tmp_step[i]] == rand_memory[j][tmp_step[j]]) {  // �C�����2022.12.1���c 0���Z��������邽��
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
				//??????????????????????�?????????????????p???]???l??v?Z
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
			//fprintf(stderr,"kumi2 %d�g��:",set2); fprintList(kumi2);

			alphauppermin=alphaupperminsav; alphalowermax=alphalowermaxsav;
	    	for(k=0;k<TN;k++){
	    		if(memberList(k,kumi2)==1 && state[k] == 1){ /* k?^?X?N????I??????????^?X?N???N??????????? */
					for(l=0;l<TN;l++){
						if(memberList(l,kumi2)==0 && state[l] == 1){  /* l?^?X?N????I????????????^?X?N???N??????????? */
							if(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]] == rand_memory[l][(memberList(l,kumi1)==1)?(tmp_step[l]+1):tmp_step[l]] ) {  // �C�����2022.12.1���c 0���Z��������邽��
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

            if(alphauppermin>0 && alphalowermax < alphauppermin){ //���߂������������𖞂����Ă��鎞
				//fprintf(stderr,"true");
                //fprintf(stderr,"%lf <= alpha <= %lf for scheduling task %d and then task %d\n",alphalowermax,alphauppermin,i+1,k+1);
				val=0;
				Laxityjudge=0;
                //1�X�e�b�v�ڂ̕]���l�̍��v
                for(i=0,s1val=0;i<TN;i++){
                    if(memberList(i,kumi1)==1 && state[i] == 1){ /* set1��i�r�b�g�ڂ�1�Ȃ�� */
						//valketa=shiftoperationtimes((task_data[i].WCET - tmp_step[i]) * task_data[i].Laxity_Time) - shiftoperationtimes(rand_memory[i][tmp_step[i]]);
                        //s1val+=(((task_data[i].WCET - tmp_step[i]) * task_data[i].Laxity_Time) + ((valketa>=0)?(rand_memory[i][tmp_step[i]] << valketa):(rand_memory[i][tmp_step[i]] >> abs(valketa))));
						s1val+=(FTFD((task_data[i].WCET - tmp_step[i])) * FTFD(task_data[i].Laxity_Time)) + (prealpha * ITFD(rand_memory[i][tmp_step[i]]));
						//fprintf(stderr,"��");
					}
					
							/*if(task_data[i].Laxity_Time>0){
						//fprintf(stderr,"%d %d",i,(task_data[i].Laxity_Time));
					  NewLaxityjudge=(FTFD((double)1/shiftoperationtimes(task_data[i].Laxity_Time))); // �C�����2022.12.1���c ���ӂ��Œ菬���_�^�Ȃ̂ɁA�E�ӂ̏��Z�������^�ɂȂ��Ă���A�����_�ȉ������܂�Ȃ��̂ŏC��
						//fprintf(stderr,"��");
					}else{
						NewLaxityjudge=MAX;
					}
					if(NewLaxityjudge>Laxityjudge){
						Laxityjudge=NewLaxityjudge;
					}	*/					
                }
				//fprintf(stderr,"��");
                //2�X�e�b�v�ڂ̕]���l�̍��v��Laxityjudge�̐ݒ�
                for(k=0,s2val=0;k<TN;k++){
                    if(memberList(k,kumi2)==1 && state[k] == 1){ /* set2??i?r?b?g???1????s2val??]???l???i?[ */
						//valketa=shiftoperationtimes((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]) * task_data[k].Laxity_Time) - shiftoperationtimes(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]]);
                        //s2val+=((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]) * task_data[k].Laxity_Time) + (valketa>=0)?(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]] << valketa):(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]] >> abs(valketa));
						s2val+=((FTFD((task_data[k].WCET - (memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k])) * FTFD(task_data[k].Laxity_Time)) + (prealpha * ITFD(rand_memory[k][(memberList(k,kumi1)==1)?(tmp_step[k]+1):tmp_step[k]])));
						
					}
					/*if(task_data[k].Laxity_Time!=0){//0??????V?t?g???Z?????????????????Laxityjudge?????????t????????????????
					  	NewLaxityjudge=FTFD((double)1/shiftoperationtimes((task_data[k].Laxity_Time))); // �C�����2022.12.1���c ���ӂ��Œ菬���_�^�Ȃ̂ɁA�E�ӂ̏��Z�������^�ɂȂ��Ă���A�����_�ȉ������܂�Ȃ��̂ŏC��
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


	/*�D��x�֐��l�̏�����*/
	for(i=0;i<TN;i++){
		if(state[i] == 1 && tmp_step[i] != task_data[i].WCET){

			priority_func[i] = (alpha * ITFD(rand_memory[i][tmp_step[i]])) + (FTFD(task_data[i].WCET - tmp_step[i]) * FTFD(task_data[i].Laxity_Time));		

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

//?????????
int get_digit(int n){
	int digit=1;

	while(n/=10){
		digit++;
	}

	return digit;
}

//��̃��X�g�𐶐�
List createList(void){
  	return NIL;
}


LList createList2(void){
    return (LList)0;
}

//���X�g�̐擪�ɗv�f������
List insertList(FIXPOINTDECIMAL element,List l) {
	List temp=malloc(sizeof(Cell));
  	temp->element=element;
	  	//fprintf(stderr,"temp->element:%lld ",temp->element); 
  	temp->next=l;
	  	//fprintf(stderr,"temp:"); fprintList(temp);
  	return temp;
}

//���X�g�̃��X�g�̑S�v�f������
LList insertList2(List element,LList l) {
	LList temp=malloc(sizeof(Cell));
  	temp->element=element;

  	temp->next=l;

  	return temp;


}




/* ��L�֐��̏C���̒��(�֐������܂߂�) (2022.11.25 ���c) */
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

int nullpList2(LList l) {   // �C�����(typo�C��) (2022.11.25 ���c)
  //return (l->element==NIL);
  return (l==LNIL); // �C����� (2022.11.25 ���c)
}

//���X�g�̐擪�v�f��Ԃ�
int headList(List l) {
  	return l->element;
}

//�擪�v�f���폜����
List tailList(List l) {
  	return l->next;
}

/* �C����āi�֐��ǉ��j (2022.11.25 ���c) */
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
    (nullpList2(l1))	  {  //�C����� (2022.11.25 ���c)
    	return l2;
  	}else{
    return insertList2(/* l1->element */ headList2(l1), //�C����� (2022.11.25 ���c) ��{�I�Ƀ��X�g�����֐��̌`�͗v�f�̌^�Ɋւ�炸�ς��Ȃ�
		       appendList2(tailList2(l1),l2));
    }
}

//�擪����i�Ԗڂ̗v�f��Ԃ�
int iList(List l1,int i){

  	if(i==0){
    	return headList(l1);
  	}else{
    	return iList(tailList(l1),i-1);
  	}

}

//�擪����i�Ԗڂ̗v�f������������
List ideleatList(List l1,List l2,int i){
  	if(i==0){
		l1->element=MAX;
    	return l2;
  	}else{
    	return ideleatList(tailList(l1),l2,i-1);
  	}

}
//���X�g�̑S�v�f��\��
void printList(List l) {

  	if(nullpList(l)){ 
    	printf("\n"); 
  	}else{
    	printf("%lld ",headList(l));
    	printList(tailList(l));
  	}
}

//���X�g�̑S�v�f���t�@�C���o��
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

//�擪����n�Ԗڂ܂ł̗v�f�����X�g�ɓ���ĕԂ�
List firstnList(List l,unsigned int n){

  	if(nullpList(l) || n==0){  //l����łȂ��܂���n��0�̂Ƃ���̃��X�g��Ԃ�
    	return createList();
  	}else{  //����ȊO�̎�l�̐擪�v�f��firstnList�ɏ�������
    	return insertList(headList(l),firstnList(tailList(l),--n));
  	}
}


//�擪����n�Ԗڂ܂ł̗v�f�����X�g�ɓ���ĕԂ�
/*
LList firstnList2(LList l,List n){

  	if(nullpList2(l) || n==0){  //l����łȂ��܂���n��0�̂Ƃ���̃��X�g��Ԃ�
    	return createList2();
  	}else{  //����ȊO�̎�l�̐擪�v�f��firstnList�ɏ�������
    	return insertList2(headList2(l),firstnList2(tailList2(l),--n));
  	}
}
*/


//�擪����n�Ԗڂ܂ł̗v�f���폜
List restnList(List l,unsigned int n){
  	if(nullpList(l) || n==0){  //l����łȂ��܂���n��0�̂Ƃ���̃��X�g��Ԃ�
    	return l;
  	}else{  //����ȊO�̎��擪�̗v�f����菜��
    	restnList(tailList(l),--n);
  	}
}

//���X�g�ɗv�f���܂܂�Ă��邩�ǂ�����T��
int memberList(int element,List l){

  if(nullpList(l)){
    return 0;
  }else if(element==headList(l)){
    return 1;
  }else{
    return memberList(element,tailList(l));
  }
}


//���X�g�̃��X�g�ɗv�f���܂܂�Ă��邩�ǂ�����T��
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

//���X�g�̃��������
void freeList(List l){
	if(nullpList(l)){
		return;
	}else{
		freeList(tailList(l));
		free(l);
		return;
	}

}


//���X�g�̃��������
void freeList2(LList l){
	if(nullpList2(l)){
		return;
	}else{
		freeList2(tailList2(l));
		free(l);
		return;
	}

}

//���X�g�̑����𐔂���
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
//���X�g�̕������쐬
List copyList(List l){

	return firstnList(l,lengthList(l));

}

//���X�g�̕������쐬
/*
LList copyList2(LList l){

	return firstnList2(l,lengthList2(l));

}
*/

//���X�g�̕]���l���ŏ��ł���^�X�N�ԍ���Ԃ��֐�
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

//�I�΂��^�X�N�̑I��
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

//�����W���̏W�������߂�
LList Makesubsetset(List T,int i,int j){
    LList subsetList=createList2();
    LList subsubsetList=createList2();
    LList temp=createList2();

    if(nullpList(T)){
      return (i<=0 && 0<=j)?(insertList2(createList(),createList2())):(createList2());  /* �C����� (2022.11.25 ���c) �u��W���P��v�f�Ƃ��Ď��W���v�Ɓu��W���v�͈Ⴄ���Ƃɒ��� */
    }else{
        temp=Makesubsetset(tailList(T),i-1,j-1);
        subsubsetList=Makesubsetset(tailList(T),i,j);

        subsetList=insertElementToAll(headList(T),temp);

        return appendList2(subsetList,subsubsetList);
    }

}


//�g�ݍ��킹�̑��������߂�
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

//�V�t�g���Z�ɂ��V�t�g����񐔂��擾
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
