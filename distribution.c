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

#define P 4											//�v���Z�b�T��
#define CM 1										//�j�������̒���
#define del_co CM*P									//�^�X�N�Z�b�g�̔j������
#define paramater 0.7								//�w�����z�̃p�����[�^

#define S 1024

/*�^�X�N�Z�b�g�p�����[�^�֘A*/
double Utilization[S];								//�^�X�N���p��
int TN = 0;											//�^�X�N��
double U = 0;										//�v���Z�b�T���p��

/*�t�@�C���֘A*/
char file_mem[256];
char *file_ut = "rand_utilization_tasks.txt";		//�^�X�N���p��
char *file_tl = "TS_utilization.txt";				//�^�X�N�Z�b�g�̗��p��
FILE *fut;
FILE *ftl;

struct timeval tv;

void rand_utilization_decision(int task_num);		//�^�X�N���p�����w�����z�ɏ]�����肷��֐�
void step1();

/*�����ϊ�����ѐ���*/
double Uniform(void);
double rand_exp(double lambda);
double rand_normal(double mu,double sigma);


int main(void) {

	int i, j;
	
	/*�t�@�C���ǂݍ��ݗp*/
	int err = 0;
	char line_ut[S];
	char *p;
	
	double file_utilization;
	
	printf("\n\nGenerating random numbers (%d-Processor) \n", P);
	
	if((fut = fopen(file_ut,"r")) == NULL){
		
		step1();
		
	}else{	
	
		i = 0;
		
		/*�^�X�N���p���i�[*/
		while(fgets(line_ut,S,fut) != NULL){
			sscanf(line_ut,"%lf %[^\r\n]", &file_utilization);
			Utilization[i] = file_utilization;      //�^�X�Ni�̗��p�����i�[
			TN++;									//�^�X�N��
			p = strtok(line_ut," \r\n\t");      	//�ŏ�����Ώۂ��擾
			while(p != NULL){           			//line_ut��p���Ȃ��Ȃ�܂ŌJ��Ԃ�  
				p = strtok(NULL," \r\n\t");   		//����p���擾
			}
			i++;
		}
		
		/*���ݎ����ɂ�闐���̏�����*/
		gettimeofday(&tv,NULL);
		init_genrand(tv.tv_sec + tv.tv_usec);
		
		rand_utilization_decision(TN);
		
		U = 0;
	
		for(i=0;i<TN+1;i++)
			U += Utilization[i];
		
		if(U > del_co){
			
			step1();
			
		}else{
			
			/*�t�@�C����������*/
			fut = fopen(file_ut,"a");
			ftl = fopen(file_tl,"a");
			fprintf(fut, "%lf\n", Utilization[TN]);
			fprintf(ftl, "%lf\n", U);
			fclose(fut);
			fclose(ftl);
			
			/*�p�����[�^�o��*/
			for(i=0;i<TN+1;i++)
				printf("Utilization[task%d] = %lf\n", i+1, Utilization[i]);

		}
		
	}
	
	return 0;
	
}


/*�ڂ�����"EDZL Schedulability Analysis in Real-TIme Multicore Scheduling"��5��*/
void step1(){
	
	int i,j;

	
	/*�t�@�C���̏�����*/
	remove(file_ut);
	
	/*�^�X�N���̏�����*/
	TN = P + 1;
	
	U = 10000;
	
	while(U > del_co){
		
		for(i=0;i<TN;i++)
			rand_utilization_decision(i);
		
		U = 0;
		
		for(i=0;i<TN;i++)
			U += Utilization[i];
		
	}
	
	/*�p�����[�^�o��*/
	for(i=0;i<TN;i++)
		printf("Utilization[task%d] = %lf\n", i+1, Utilization[i]);
	
	/*�t�@�C����������*/
	fut = fopen(file_ut,"a");
	ftl = fopen(file_tl,"a");
	
	for(i=0;i<TN;i++)
		fprintf(fut, "%lf\n", Utilization[i]);
	
	fprintf(ftl, "%lf\n", U);

	fclose(fut);
	fclose(ftl);
	
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