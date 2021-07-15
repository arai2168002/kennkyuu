/*Machigasira Yuki*/
/*�����������炢��Ȃ��w�b�_�t�@�C�������邩���E�E�E*/
/*�����ɗp����^�X�N�Z�b�g�̐������@��"EDZL Schedulability Analysis in Real-TIme Multicore Scheduling"������p*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <windows.h>
#include "MT.h"

#define P 4
//#define P 1										//�v���Z�b�T��
#define CM 1										//�j�������̒���
#define del_co CM*P									//�^�X�N�Z�b�g�̔j������
#define paramater 0.9								//�w�����z�̃p�����[�^

#define M_max 20000					//�������m�ۂ̍ő�l(M_max/2)
	
#define P_max 1000									//�����̍ő�l
#define P_min 100									//�����̍ŏ��l

#define S 1024

/*�^�X�N�Z�b�g�p�����[�^�֘A*/
int Period[S];										//�e�^�X�N�̎���
double Utilization[S];								//�^�X�N���p��
int Rand_Memory[S][S];								//�e�^�X�N�̏����������
int WCET[S];										//�ň����s����(�X�e�b�v��)
int TN = 0;											//�^�X�N��
double U = 0;										//�v���Z�b�T���p��

/*�t�@�C���֘A*/
char file_mem[256];
char *file_pe = "rand_period_tasks.txt";			//�{�����ł͎������f�b�h���C��
char *file_ut = "rand_utilization_tasks.txt";		//�^�X�N���p��
char *file_wc = "rand_wcet_tasks.txt";				//WCET
FILE *fmemory;
FILE *fpe;
FILE *fut;
FILE *fwc;

struct timeval tv;

void rand_period_decision(int task_num);			//��������l���z�ɏ]�����肷��֐�
void rand_utilization_decision(int task_num);		//�^�X�N���p�����w�����z�ɏ]�����肷��֐�
void rand_wcet_decision(int task_num);				//�^�X�N��WCET�����肷��֐�
void rand_mem_decision(int task_num, int TN);		//���������������l���z�ɏ]�����肷��֐�
void step1();

/*�����ϊ�����ѐ���*/
double Uniform(void);
double rand_exp(double lambda);
double rand_normal(double mu,double sigma);


int main(void) {

	int i, j;
	
	/*�t�@�C���ǂݍ��ݗp*/
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
	
	/*�ǂݍ��݂ł��邩�ǂ����i�ł��Ȃ����err++�j*/
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
		
		/*�����i�[*/
		while(fgets(line_pe,S,fpe) != NULL){
			sscanf(line_pe,"%d %[^\r\n]", &file_period);
			Period[i] = file_period;        		//�^�X�Ni�̎������i�[
			TN++;									//�^�X�N��
			p = strtok(line_pe," \r\n\t");      	//�ŏ�����Ώۂ��擾
			while(p != NULL){           			//line_pe��p���Ȃ��Ȃ�܂ŌJ��Ԃ�  
				p = strtok(NULL," \r\n\t");   		//����p���擾
			}
			i++;
		}
		
		i = 0;

		/*�^�X�N���p���i�[*/
		while(fgets(line_ut,S,fut) != NULL){
			sscanf(line_ut,"%lf %[^\r\n]", &file_utilization);
			Utilization[i] = file_utilization;      //�^�X�Ni�̗��p�����i�[
			p = strtok(line_ut," \r\n\t");      	//�ŏ�����Ώۂ��擾
			while(p != NULL){           			//line_ut��p���Ȃ��Ȃ�܂ŌJ��Ԃ�  
				p = strtok(NULL," \r\n\t");   		//����p���擾
			}
			i++;
		}
		
		i = 0;

		/*WCET�i�[*/
		while(fgets(line_wc,S,fwc) != NULL){
			sscanf(line_wc,"%d %[^\r\n]", &file_wcet);
			WCET[i] = file_wcet;        			//�^�X�Ni��WCET���i�[
			p = strtok(line_wc," \r\n\t");      	//�ŏ�����Ώۂ��擾
			while(p != NULL){           			//line_wc��p���Ȃ��Ȃ�܂ŌJ��Ԃ�  
				p = strtok(NULL," \r\n\t");   		//����p���擾
			}
			i++;
		}
		
		/*���ݎ����ɂ�闐���̏�����*/
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
			
			/*�t�@�C����������(�����C�^�X�N���p���CWCET)*/
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
		
			/*�p�����[�^�o��*/
			for(i=0;i<TN+1;i++){
				printf("Period[task%d] = %d\n", i+1, Period[i]);
				printf("Utilization[task%d] = %lf\n", i+1, Utilization[i]);
				printf("WCET[task%d] = %d\n", i+1, WCET[i]);
			}
		}
		
	}
	
	return 0;
	
}


/*�ڂ�����"EDZL Schedulability Analysis in Real-TIme Multicore Scheduling"��5��*/
void step1(){
	
	int i,j;

	
	/*�t�@�C���̏�����*/
	system("rm -f rand_memory_task*.txt");
	remove(file_pe);
	remove(file_ut);
	remove(file_wc);
	
	/*�^�X�N���̏�����*/
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
	
	/*�p�����[�^�o��*/
	for(i=0;i<TN;i++){
		printf("Period[task%d] = %d\n", i+1, Period[i]);
		printf("Utilization[task%d] = %lf\n", i+1, Utilization[i]);
		printf("WCET[task%d] = %d\n", i+1, WCET[i]);
	}
	
	/*�t�@�C����������(�����C�^�X�N���p���CWCET)*/
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
	
	/*�t�@�C����������(�����������)*/
	for(i=0;i<TN;i++){
		sprintf(file_mem,"rand_memory_task%d.txt", i+1);
		
		fmemory = fopen(file_mem,"a");
		
		rand_mem_decision(i,TN);
		
		for(j=0;j<WCET[i];j++)
			fprintf(fmemory, "%d\n", Rand_Memory[i][j]);
		
	}
	
	fclose(fmemory);
	
}


/*��������l���z�ɏ]�����肷��֐�*/
void rand_period_decision(int task_num){

	/*���ݎ����ɂ�闐���̏�����*/
	gettimeofday(&tv,NULL);
	init_genrand(tv.tv_sec + tv.tv_usec);

	Period[task_num] = genrand_int32() % (P_max - P_min + 1) + P_min;	//�͈͎w�� %(�ő�l-�ŏ��l+1)+�ŏ��l
	
}


/*�^�X�N���p�����w�����z�ɏ]�����肷��֐�*/
void rand_utilization_decision(int task_num){
	
	double lam;							//�Ɂi1/���ρj
	
	lam = 1 / paramater;
	//printf("%lf\n", lam);

	/*���ݎ����ɂ�闐���̏�����*/
	gettimeofday(&tv,NULL);
	init_genrand(tv.tv_sec + tv.tv_usec);
	
	Utilization[task_num] = 3;
	while(Utilization[task_num] > 1)
		Utilization[task_num] = rand_exp(lam);
	
}


/*�^�X�N���p���ɍ��킹��WCET(�X�e�b�v��)�����肷��֐�*/
void rand_wcet_decision(int task_num){
	
	if(Utilization[task_num] > 1)
		Utilization[task_num] = floor(Utilization[task_num]);
	
	WCET[task_num] = floor(Period[task_num] * Utilization[task_num]);
	
}


/*���������������l���z�ɏ]�����肷��֐�*/
/*�����F�^�X�N�ԍ�:task_num*/
void rand_mem_decision(int task_num, int TN){
	
	double mu_mem = 0;					//����
	double sigma_mem = 40;				//�W���΍�
	int i;								//�J�E���g�p�ϐ�
	int step = 0;						//���݂̃X�e�b�v
	int total_mem[TN];					//�������������
	
	for(i=0;i<TN+1;i++)
		total_mem[i] = 0;

	for(i=0;i<(WCET[task_num]-1);i++){
		
		/*���ݎ����ɂ�闐���̏�����*/
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
		usleep(1);						//���Ԃ��擾���ė����𔭐������Ă��邽�ߕK�v

	}
	
	/*���K���킹*/
	Rand_Memory[task_num][step] = 0 - total_mem[task_num];
	total_mem[task_num] += Rand_Memory[task_num][step];
	
	
}


/*Mersenne Twister�ɂ���l��������*/
double Uniform(void){
	return genrand_real3();
}


/*��l���z����w�����z��(�t�֐��@)*/
double rand_exp(double lambda){
   return -log(Uniform())/lambda;
}


/*��l�������琳�K���z��(�{�b�N�X=�~�����[�@)*/
double rand_normal(double mu,double sigma){
  double z = sqrt(-2.0*log(Uniform())) * sin(2.0*M_PI*Uniform());
  return mu + sigma * z;
}