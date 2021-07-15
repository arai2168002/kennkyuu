/*2019/11/20_17:41 Machigasira Yuki*/
/*もしかしたらいらないヘッダファイルもあるかも・・・*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define S 1024		//入力ファイル：状態の数
#define F 3			//αの種類（現在は3種類だが，種類が増えるときは，このFの値と，以下のlogファイル名を変更するだけでよい．減るときも同様にする．）

int main (){
	
	/*入力ファイル*/
	char data_EDF[S] = "datafile_EDF.txt";
	char data_EDZL[S] = "datafile_EDZL.txt";
	char data_LLF[S] = "datafile_LLF.txt";
	char data_LMCF[S] = "datafile_LMCF.txt";
	char data_LMCLF[F][S] = {
		"datafile_LMCLF_1000.000000.txt", "datafile_LMCLF_1.000000.txt", "datafile_LMCLF_0.000000.txt"	//No.1, No.2, No.3という表示で対応（以下同様）　αの値を変更すると吐き出すlogファイル名が変わるので，ここも変更
	};
	char data_AcII_LMCF[S] = "datafile_AcII_LMCF.txt";
	char data_AcII_LMCLF[F][S] = {
		"datafile_AcII_LMCLF_1000.000000.txt", "datafile_AcII_LMCLF_1.000000.txt", "datafile_AcII_LMCLF_0.000000.txt"
	};
	char data_LLF_LMCLF[F][S] = {
		"datafile_LLF_LMCLF_1000.000000.txt", "datafile_LLF_LMCLF_1.000000.txt", "datafile_LLF_LMCLF_0.000000.txt"
	};
	char data_LLF_AcII_LMCLF[F][S] = {
		"datafile_LLF_AcII_LMCLF_1000.000000.txt", "datafile_LLF_AcII_LMCLF_1.000000.txt", "datafile_LLF_AcII_LMCLF_0.000000.txt"
	};
	char data_Task_Set[S] = "datafile_Task_Set.txt";
	
	/*読み込み変数*/
	double mem_EDF, dm_EDF;
	double mem_EDZL, dm_EDZL;
	double mem_LLF, dm_LLF;
	double mem_LMCF, dm_LMCF;
	double mem_LMCLF[F], dm_LMCLF[F];
	double mem_AcII_LMCF, dm_AcII_LMCF;
	double mem_AcII_LMCLF[F], dm_AcII_LMCLF[F];
	double mem_LLF_LMCLF[F], dm_LLF_LMCLF[F], switching1[F];
	double mem_LLF_AcII_LMCLF[F], dm_LLF_AcII_LMCLF[F], switching2[F];
	double ptasks, ut_aver; 
	FILE *fp_EDF, *fp_EDZL, *fp_LLF, *fp_LMCF, *fp_LMCLF[F], *fp_AcII_LMCF, *fp_AcII_LMCLF[F], *fp_LLF_LMCLF[F], *fp_LLF_AcII_LMCLF[F], *fp_Task_Set;
	char line_EDF[S], line_EDZL[S], line_LLF[S], line_LMCF[S], line_LMCLF[F][S], line_AcII_LMCF[S], line_AcII_LMCLF[F][S], line_LLF_LMCLF[F][S], line_LLF_AcII_LMCLF[F][S], line_Task_Set[S];
	
	/*書き込み変数*/
	double memory_EDF[S], deadline_miss_EDF[S];												//EDF										
	double memory_EDZL[S], deadline_miss_EDZL[S];											//EDZL										
	double memory_LLF[S], deadline_miss_LLF[S];												//LLF										
	double memory_LMCF[S], deadline_miss_LMCF[S];											//LMCF
	double memory_LMCLF[F][S], deadline_miss_LMCLF[F][S];									//LMCLF
	double memory_AcII_LMCF[S], deadline_miss_AcII_LMCF[S];									//AcII-LMCF
	double memory_AcII_LMCLF[F][S], deadline_miss_AcII_LMCLF[F][S];							//AcII-LMLCF		
	double memory_LLF_LMCLF[F][S], deadline_miss_LLF_LMCLF[F][S], sw1[F][S];				//LLF LMCLF
	double memory_LLF_AcII_LMCLF[F][S], deadline_miss_LLF_AcII_LMCLF[F][S], sw2[F][S];		//LLF AcII-LMCLF
	double ptask[S], utilization_aver[S];													//Task Sets
	int n = 0;																				//データ数
		
	/*結果(EDF)*/
	double aver_mem_EDF = 0;																//最悪メモリ消費量の平均
	double aver_dm_EDF = 0;																	//デッドラインミスの平均
	
	/*結果(EDZL)*/
	double aver_mem_EDZL = 0;																//最悪メモリ消費量の平均
	double aver_dm_EDZL = 0;																//デッドラインミスの平均
	
	/*結果(LLF)*/
	double aver_mem_LLF = 0;																//最悪メモリ消費量の平均
	double aver_dm_LLF = 0;																	//デッドラインミスの平均
	
	/*結果(LMCF)*/
	double aver_reduction_LMCF = 0;															//メモリ削減率の平均
	double max_reduction_LMCF = 0, min_reduction_LMCF = 200;								//メモリ削減率の最大と最小
	double aver_mem_LMCF = 0;																//最悪メモリ消費量の平均
	double aver_dm_LMCF = 0;																//デッドラインミスの平均
	
	/*結果(LMCLF)*/
	double aver_reduction_LMCLF[F];															//メモリ削減率の平均
	double max_reduction_LMCLF[F], min_reduction_LMCLF[F];									//メモリ削減率の最大と最小
	double aver_mem_LMCLF[F];																//最悪メモリ消費量の平均
	double aver_dm_LMCLF[F];																//デッドラインミスの平均	
	
	/*結果(AcII-LMCF)*/
	double aver_reduction_AcII_LMCF = 0;													//メモリ削減率の平均
	double max_reduction_AcII_LMCF = 0, min_reduction_AcII_LMCF = 200;						//メモリ削減率の最大と最小
	double aver_mem_AcII_LMCF = 0;															//最悪メモリ消費量の平均
	double aver_dm_AcII_LMCF = 0;															//デッドラインミスの平均
	
	/*結果(AcII-LMCLF)*/
	double aver_reduction_AcII_LMCLF[F];													//メモリ削減率の平均
	double max_reduction_AcII_LMCLF[F], min_reduction_AcII_LMCLF[F];						//メモリ削減率の最大と最小
	double aver_mem_AcII_LMCLF[F];															//最悪メモリ消費量の平均
	double aver_dm_AcII_LMCLF[F];															//デッドラインミスの平均
	
	/*結果(LLF LMCLF)*/
	double aver_reduction_LLF_LMCLF[F];														//メモリ削減率の平均
	double max_reduction_LLF_LMCLF[F], min_reduction_LLF_LMCLF[F];							//メモリ削減率の最大と最小
	double aver_mem_LLF_LMCLF[F];															//最悪メモリ消費量の平均
	double aver_dm_LLF_LMCLF[F];															//デッドラインミスの平均
	double aver_sw1[F];																		//スケジュール切り替えの平均
	
	/*結果(LLF AcII-LMCLF)*/
	double aver_reduction_LLF_AcII_LMCLF[F];												//メモリ削減率の平均
	double max_reduction_LLF_AcII_LMCLF[F], min_reduction_LLF_AcII_LMCLF[F];				//メモリ削減率の最大と最小
	double aver_mem_LLF_AcII_LMCLF[F];														//最悪メモリ消費量の平均
	double aver_dm_LLF_AcII_LMCLF[F];														//デッドラインミスの平均
	double aver_sw2[F];																		//スケジュール切り替えの平均
	
	/*結果(Task Set)*/
	double aver_task = 0;																	//タスク数
	double aver_utilization = 0, max_utilization = 0;										//タスク利用率（平均，平均の最大）

	/*作業用変数*/
	char *p;
	int i, j;																				//カウント用変数
	double reduction[S];																	//タスクセット[i]のメモリ削減率
	
	
	/*結果ファイルの読み込み(EDF)*/
	fp_EDF = fopen(data_EDF,"r");
	i = 0;
	while(fgets(line_EDF,S,fp_EDF) != NULL){
		sscanf(line_EDF,"%lf\t%lf %[^\r\n]", &mem_EDF, &dm_EDF, line_EDF);
		memory_EDF[i] = mem_EDF;        									//最悪メモリ消費量
		deadline_miss_EDF[i] = dm_EDF;										//EDFのデッドラインミス
		p = strtok(line_EDF," \r\n\t");		   	  							//最初分解対象を取得
		while(p != NULL){           										//line_EDFにpがなくなるまで繰り返す  
			p = strtok(NULL," \r\n\t");   									//次のpを取得
		}
		i++;
	}
	fclose(fp_EDF);
	
	/*結果ファイルの読み込み(EDZL)*/
	fp_EDZL = fopen(data_EDZL,"r");
	i = 0;
	while(fgets(line_EDZL,S,fp_EDZL) != NULL){
		sscanf(line_EDZL,"%lf\t%lf %[^\r\n]", &mem_EDZL, &dm_EDZL, line_EDZL);
		memory_EDZL[i] = mem_EDZL;        									//最悪メモリ消費量
		deadline_miss_EDZL[i] = dm_EDZL;									//EDZLのデッドラインミス
		p = strtok(line_EDZL," \r\n\t");		   	  						//最初分解対象を取得
		while(p != NULL){           										//line_EDZLにpがなくなるまで繰り返す  
			p = strtok(NULL," \r\n\t");   									//次のpを取得
		}
		i++;
	}
	fclose(fp_EDZL);
	
	/*結果ファイルの読み込み(LLF)*/
	fp_LLF = fopen(data_LLF,"r");
	i = 0;
	while(fgets(line_LLF,S,fp_LLF) != NULL){
		sscanf(line_LLF,"%lf\t%lf %[^\r\n]", &mem_LLF, &dm_LLF, line_LLF);
		memory_LLF[i] = mem_LLF;        									//最悪メモリ消費量
		deadline_miss_LLF[i] = dm_LLF;										//LLFのデッドラインミス
		p = strtok(line_LLF," \r\n\t");		   	  							//最初分解対象を取得
		while(p != NULL){           										//line_LLFにpがなくなるまで繰り返す  
			p = strtok(NULL," \r\n\t");   									//次のpを取得
		}
		i++;
	}
	fclose(fp_LLF);
	
	
	/*結果ファイルの読み込み(LMCF)*/
	fp_LMCF = fopen(data_LMCF,"r");
	i = 0;
	while(fgets(line_LMCF,S,fp_LMCF) != NULL){
		sscanf(line_LMCF,"%lf\t%lf %[^\r\n]", &mem_LMCF, &dm_LMCF, line_LMCF);
		memory_LMCF[i] = mem_LMCF;        									//最悪メモリ消費量
		deadline_miss_LMCF[i] = dm_LMCF;									//LMCFのデッドラインミス
		p = strtok(line_LMCF," \r\n\t");	      							//最初分解対象を取得
		while(p != NULL){           										//line_LMCFにpがなくなるまで繰り返す  
			p = strtok(NULL," \r\n\t");   									//次のpを取得
		}
		i++;
	}
	fclose(fp_LMCF);
	
	
	/*結果ファイルの読み込み(LMCLF)*/
	for(j=0;j<F;j++){
		fp_LMCLF[j] = fopen(data_LMCLF[j],"r");
		i = 0;
		while(fgets(line_LMCLF[j],S,fp_LMCLF[j]) != NULL){
			sscanf(line_LMCLF[j],"%lf\t%lf %[^\r\n]", &mem_LMCLF[j], &dm_LMCLF[j], line_LMCLF[j]);
			memory_LMCLF[j][i] = mem_LMCLF[j];        						//最悪メモリ消費量
			deadline_miss_LMCLF[j][i] = dm_LMCLF[j];						//LMCLF[j]のデッドラインミス
			p = strtok(line_LMCLF[j]," \r\n\t");      						//最初分解対象を取得
			while(p != NULL){           									//line_LMCLF[j]にpがなくなるまで繰り返す  
				p = strtok(NULL," \r\n\t");   								//次のpを取得
			}
			i++;
		}
		fclose(fp_LMCLF[j]);
	}
	
	
	/*結果ファイルの読み込み(AcII-LMCF)*/
	fp_AcII_LMCF = fopen(data_AcII_LMCF,"r");
	i = 0;
	while(fgets(line_AcII_LMCF,S,fp_AcII_LMCF) != NULL){
		sscanf(line_AcII_LMCF,"%lf\t%lf %[^\r\n]", &mem_AcII_LMCF, &dm_AcII_LMCF, line_AcII_LMCF);
		memory_AcII_LMCF[i] = mem_AcII_LMCF;      							//最悪メモリ消費量
		deadline_miss_AcII_LMCF[i] = dm_AcII_LMCF;							//AcII-LMCFのデッドラインミス
		p = strtok(line_AcII_LMCF," \r\n\t");     			 				//最初分解対象を取得
		while(p != NULL){           										//line_AcII_LMCFにpがなくなるまで繰り返す  
			p = strtok(NULL," \r\n\t");   									//次のpを取得
		}
		i++;
	}
	fclose(fp_AcII_LMCF);
	
	
	/*結果ファイルの読み込み(AcII-LMCLF)*/
	for(j=0;j<F;j++){
		fp_AcII_LMCLF[j] = fopen(data_AcII_LMCLF[j],"r");
		i = 0;
		while(fgets(line_AcII_LMCLF[j],S,fp_AcII_LMCLF[j]) != NULL){
			sscanf(line_AcII_LMCLF[j],"%lf\t%lf %[^\r\n]", &mem_AcII_LMCLF[j], &dm_AcII_LMCLF[j], line_AcII_LMCLF[j]);
			memory_AcII_LMCLF[j][i] = mem_AcII_LMCLF[j];        			//最悪メモリ消費量
			deadline_miss_AcII_LMCLF[j][i] = dm_AcII_LMCLF[j];				//AcII-LMCLF[j]のデッドラインミス
			p = strtok(line_AcII_LMCLF[j]," \r\n\t");      					//最初分解対象を取得
			while(p != NULL){           									//line_AcII_LMCLF[j]にpがなくなるまで繰り返す  
				p = strtok(NULL," \r\n\t");   								//次のpを取得
			}
			i++;
		}
		fclose(fp_AcII_LMCLF[j]);
	}
	
	
	/*結果ファイルの読み込み(LLF LMCLF)*/
	for(j=0;j<F;j++){
		fp_LLF_LMCLF[j] = fopen(data_LLF_LMCLF[j],"r");
		i = 0;
		while(fgets(line_LLF_LMCLF[j],S,fp_LLF_LMCLF[j]) != NULL){
			sscanf(line_LLF_LMCLF[j],"%lf\t%lf\t%lf %[^\r\n]", &mem_LLF_LMCLF[j], &dm_LLF_LMCLF[j], &switching1[j], line_LLF_LMCLF[j]);
			memory_LLF_LMCLF[j][i] = mem_LLF_LMCLF[j];      				//最悪メモリ消費量
			deadline_miss_LLF_LMCLF[j][i] = dm_LLF_LMCLF[j];				//LLF　LMCLF[j]のデッドラインミス
			sw1[j][i] = switching1[j];										//スケジュール切り替え数
			p = strtok(line_LLF_LMCLF[j]," \r\n\t");      					//最初分解対象を取得
			while(p != NULL){           									//line_LLF_LMCLF[j]にpがなくなるまで繰り返す  
				p = strtok(NULL," \r\n\t");   								//次のpを取得
			}
			i++;
		}
		fclose(fp_LLF_LMCLF[j]);
	}
	
	
	/*結果ファイルの読み込み(LLF AcII-LMCLF)*/
	for(j=0;j<F;j++){
		fp_LLF_AcII_LMCLF[j] = fopen(data_LLF_AcII_LMCLF[j],"r");
		i = 0;
		while(fgets(line_LLF_AcII_LMCLF[j],S,fp_LLF_AcII_LMCLF[j]) != NULL){
			sscanf(line_LLF_AcII_LMCLF[j],"%lf\t%lf\t%lf %[^\r\n]", &mem_LLF_AcII_LMCLF[j], &dm_LLF_AcII_LMCLF[j], &switching2[j], line_LLF_AcII_LMCLF[j]);
			memory_LLF_AcII_LMCLF[j][i] = mem_LLF_AcII_LMCLF[j];     		//最悪メモリ消費量
			deadline_miss_LLF_AcII_LMCLF[j][i] = dm_LLF_AcII_LMCLF[j];		//LLF　AcII-LMCLF[j]のデッドラインミス
			sw2[j][i] = switching2[j];										//スケジュール切り替え数
			p = strtok(line_LLF_AcII_LMCLF[j]," \r\n\t");  	    			//最初分解対象を取得
			while(p != NULL){           									//line_LLF_AcII_LMCLF[j]にpがなくなるまで繰り返す  
				p = strtok(NULL," \r\n\t");   								//次のpを取得
			}
			i++;
		}
		fclose(fp_LLF_AcII_LMCLF[j]);
	}
	
	
	/*結果ファイルの読み込み(Task Set)*/
	fp_Task_Set = fopen(data_Task_Set,"r");
	i = 0;
	while(fgets(line_Task_Set,S,fp_Task_Set) != NULL){
		sscanf(line_Task_Set,"%lf\t%lf %[^\r\n]", &ptasks, &ut_aver, line_Task_Set);
		ptask[i] = ptasks;													//タスク数
		utilization_aver[i] = ut_aver;										//タスク利用率（平均）
		n++;																//データ数
		p = strtok(line_Task_Set," \r\n\t");      							//最初分解対象を取得
		while(p != NULL){           										//line_Task_Setにpがなくなるまで繰り返す  
			p = strtok(NULL," \r\n\t");   									//次のpを取得
		}
		i++;
	}
	fclose(fp_Task_Set);
	
	
	/*変数の初期化*/
	for(j=0;j<F;j++){
		
		max_reduction_LMCLF[j] = 0;
		min_reduction_LMCLF[j] = 200;
		
		max_reduction_AcII_LMCLF[j] = 0;
		min_reduction_AcII_LMCLF[j] = 200;
		
		max_reduction_LLF_LMCLF[j] = 0;
		min_reduction_LLF_LMCLF[j] = 200;
		
		max_reduction_LLF_AcII_LMCLF[j] = 0;
		min_reduction_LLF_AcII_LMCLF[j] = 200;
		
	}


	/*結果の導出(EDF, EDZL, LLF, LMCF)*/
	for(i=0;i<n;i++){
		reduction[i] = (1 - (memory_LMCF[i] / memory_LLF[i])) * 100;
	
		aver_mem_EDF += memory_EDF[i];
		aver_mem_EDZL += memory_EDZL[i];
		aver_mem_LLF += memory_LLF[i];
		aver_mem_LMCF += memory_LMCF[i];
		
		if(deadline_miss_EDF[i] > 0)
			aver_dm_EDF++;
		
		if(deadline_miss_EDZL[i] > 0)
			aver_dm_EDZL++;
		
		if(deadline_miss_LLF[i] > 0)
			aver_dm_LLF++;

		if(deadline_miss_LMCF[i] > 0)
			aver_dm_LMCF++;
		
		if(max_reduction_LMCF < reduction[i])
			max_reduction_LMCF = reduction[i];
	
		if(min_reduction_LMCF > reduction[i])
			min_reduction_LMCF = reduction[i];
		
	}
	aver_reduction_LMCF = (1 - (aver_mem_LMCF / aver_mem_LLF)) * 100;
	
	
	/*結果の導出(LMCLF)*/
	for(j=0;j<F;j++){
		for(i=0;i<n;i++){
			reduction[i] = (1 - (memory_LMCLF[j][i] / memory_LLF[i])) * 100;
	
			aver_mem_LMCLF[j] += memory_LMCLF[j][i];

			if(deadline_miss_LMCLF[j][i] > 0)
				aver_dm_LMCLF[j]++;
		
			if(max_reduction_LMCLF[j] < reduction[i])
				max_reduction_LMCLF[j] = reduction[i];
	
			if(min_reduction_LMCLF[j] > reduction[i])
				min_reduction_LMCLF[j] = reduction[i];
		
		}
		aver_reduction_LMCLF[j] = (1 - (aver_mem_LMCLF[j] / aver_mem_LLF)) * 100;
	}
	

	/*結果の導出(AcII-LMCF)*/
	for(i=0;i<n;i++){
		reduction[i] = (1 - (memory_AcII_LMCF[i] / memory_LLF[i])) * 100;
	
		aver_mem_AcII_LMCF += memory_AcII_LMCF[i];
		
		if(deadline_miss_AcII_LMCF[i] > 0)
			aver_dm_AcII_LMCF++;
		
		if(max_reduction_AcII_LMCF < reduction[i])
			max_reduction_AcII_LMCF = reduction[i];
	
		if(min_reduction_AcII_LMCF > reduction[i])
			min_reduction_AcII_LMCF = reduction[i];
		
	}
	aver_reduction_AcII_LMCF = (1 - (aver_mem_AcII_LMCF / aver_mem_LLF)) * 100;
	
	
	/*結果の導出(AcII-LMCLF)*/
	for(j=0;j<F;j++){
		for(i=0;i<n;i++){
			reduction[i] = (1 - (memory_AcII_LMCLF[j][i] / memory_LLF[i])) * 100;
	
			aver_mem_AcII_LMCLF[j] += memory_AcII_LMCLF[j][i];

			if(deadline_miss_AcII_LMCLF[j][i] > 0)
				aver_dm_AcII_LMCLF[j]++;
			
			if(max_reduction_AcII_LMCLF[j] < reduction[i])
				max_reduction_AcII_LMCLF[j] = reduction[i];
	
			if(min_reduction_AcII_LMCLF[j] > reduction[i])
				min_reduction_AcII_LMCLF[j] = reduction[i];
		
		}
		aver_reduction_AcII_LMCLF[j] = (1 - (aver_mem_AcII_LMCLF[j] / aver_mem_LLF)) * 100;
	}
	
	
	/*結果の導出(LLF LMCLF)*/
	for(j=0;j<F;j++){
		for(i=0;i<n;i++){
			reduction[i] = (1 - (memory_LLF_LMCLF[j][i] / memory_LLF[i])) * 100;
	
			aver_mem_LLF_LMCLF[j] += memory_LLF_LMCLF[j][i];
			aver_sw1[j] += sw1[j][i];

			if(deadline_miss_LLF_LMCLF[j][i] > 0)
				aver_dm_LLF_LMCLF[j]++;
		
			if(max_reduction_LLF_LMCLF[j] < reduction[i])
				max_reduction_LLF_LMCLF[j] = reduction[i];
	
			if(min_reduction_LLF_LMCLF[j] > reduction[i])
				min_reduction_LLF_LMCLF[j] = reduction[i];
		
		}
		aver_reduction_LLF_LMCLF[j] = (1 - (aver_mem_LLF_LMCLF[j] / aver_mem_LLF)) * 100;
	}
	
	
	/*結果の導出(LLF AcII-LMCLF)*/
	for(j=0;j<F;j++){
		for(i=0;i<n;i++){
			reduction[i] = (1 - (memory_LLF_AcII_LMCLF[j][i] / memory_LLF[i])) * 100;
	
			aver_mem_LLF_AcII_LMCLF[j] += memory_LLF_AcII_LMCLF[j][i];
			aver_sw2[j] += sw2[j][i];

			if(deadline_miss_LLF_AcII_LMCLF[j][i] > 0)
				aver_dm_LLF_AcII_LMCLF[j]++;
		
			if(max_reduction_LLF_AcII_LMCLF[j] < reduction[i])
				max_reduction_LLF_AcII_LMCLF[j] = reduction[i];
	
			if(min_reduction_LLF_AcII_LMCLF[j] > reduction[i])
				min_reduction_LLF_AcII_LMCLF[j] = reduction[i];
		
		}
		aver_reduction_LLF_AcII_LMCLF[j] = (1 - (aver_mem_LLF_AcII_LMCLF[j] / aver_mem_LLF)) * 100;
	}
		
	/*結果の導出(Task Set)*/
	for(i=0;i<n;i++){
		aver_task += ptask[i];
		aver_utilization += utilization_aver[i];
		
		if(max_utilization < utilization_aver[i])
			max_utilization = utilization_aver[i];
		
		if(max_utilization > 1)
			max_utilization = floor(max_utilization);
	}
	
	
	/*結果出力(最悪メモリ消費量WCMC，デッドラインミス発生率)*/
	printf("\n---------------------------Result (WCMC and Deadline Miss)---------------------------\n\n");
	
	printf("No. %d\n", n);
	
	printf("EDF\t\t\t");
	printf("WCMC:%0.2lf\tDeadline Miss:%0.1lf\n", aver_mem_EDF/n, aver_dm_EDF*100/n);
	
	printf("EDZL\t\t\t");
	printf("WCMC:%0.2lf\tDeadline Miss:%0.1lf\n", aver_mem_EDZL/n, aver_dm_EDZL*100/n);
	
	printf("LLF\t\t\t");
	printf("WCMC:%0.2lf\tDeadline Miss:%0.1lf\n", aver_mem_LLF/n, aver_dm_LLF*100/n);
	
	printf("LMCF\t\t\t");
	printf("WCMC:%0.2lf\tDeadline Miss:%0.1lf\n", aver_mem_LMCF/n, aver_dm_LMCF*100/n);
	
	for(j=0;j<F;j++){
		printf("LMCLF (No.%d)\t\t", j+1);
		printf("WCMC:%0.2lf\tDeadline Miss:%0.1lf\n", aver_mem_LMCLF[j]/n, aver_dm_LMCLF[j]*100/n);
	}
	
	printf("AcII-LMCF\t\t");
	printf("WCMC:%0.2lf\tDeadline Miss:%0.1lf\n", aver_mem_AcII_LMCF/n, aver_dm_AcII_LMCF*100/n);
	
	for(j=0;j<F;j++){
		printf("AcII-LMCLF (No.%d)\t", j+1);
		printf("WCMC:%0.2lf\tDeadline Miss:%0.1lf\n", aver_mem_AcII_LMCLF[j]/n, aver_dm_AcII_LMCLF[j]*100/n);
	}
	
	for(j=0;j<F;j++){
		printf("LLF LMCLF (No.%d)\t", j+1);
		printf("WCMC:%0.2lf\tDeadline Miss:%0.1lf\tsw1:%0.1lfTimes\n", aver_mem_LLF_LMCLF[j]/n, aver_dm_LLF_LMCLF[j]*100/n, aver_sw1[j]/n);
	}
	
	for(j=0;j<F;j++){
		printf("LLF AcII-LMCLF (No.%d)\t", j+1);
		printf("WCMC:%0.2lf\tDeadline Miss:%0.1lf\tsw2:%0.1lfTimes\n", aver_mem_LLF_AcII_LMCLF[j]/n, aver_dm_LLF_AcII_LMCLF[j]*100/n, aver_sw2[j]/n);
	}
	printf("\n-------------------------------------------------------------------------------------\n\n");
	
	
	/*結果出力(メモリ削減率)*/
	printf("\n-----------------------Result (Memory Reduction Ratio for LLF)-----------------------\n\n");
	
	printf("No. %d\n", n);
	printf("LMCF\t\t\t");
	printf("AVG:%0.2lf%%\tMAX:%0.2lf%%\tMIN:%0.2lf%%\n", aver_reduction_LMCF, max_reduction_LMCF, min_reduction_LMCF);
	
	for(j=0;j<F;j++){
		printf("LMCLF (No.%d)\t\t", j+1);
		printf("AVG:%0.2lf%%\tMAX:%0.2lf%%\tMIN:%0.2lf%%\n", aver_reduction_LMCLF[j], max_reduction_LMCLF[j], min_reduction_LMCLF[j]);
	}
	
	printf("AcII-LMCF\t\t");
	printf("AVG:%0.2lf%%\tMAX:%0.2lf%%\tMIN:%0.2lf%%\n", aver_reduction_AcII_LMCF, max_reduction_AcII_LMCF, min_reduction_AcII_LMCF);
	
	for(j=0;j<F;j++){
		printf("AcII-LMCLF (No.%d)\t", j+1);
		printf("AVG:%0.2lf%%\tMAX:%0.2lf%%\tMIN:%0.2lf%%\n", aver_reduction_AcII_LMCLF[j], max_reduction_AcII_LMCLF[j], min_reduction_AcII_LMCLF[j]);
	}
	
	for(j=0;j<F;j++){
		printf("LLF LMCLF (No.%d)\t", j+1);
		printf("AVG:%0.2lf%%\tMAX:%0.2lf%%\tMIN:%0.2lf%%\n", aver_reduction_LLF_LMCLF[j], max_reduction_LLF_LMCLF[j], min_reduction_LLF_LMCLF[j]);
	}

	for(j=0;j<F;j++){
		printf("LLF AcII-LMCLF (No.%d)\t", j+1);
		printf("AVG:%0.2lf%%\tMAX:%0.2lf%%\tMIN:%0.2lf%%\n", aver_reduction_LLF_AcII_LMCLF[j], max_reduction_LLF_AcII_LMCLF[j], min_reduction_LLF_AcII_LMCLF[j]);
	}
	printf("\n-------------------------------------------------------------------------------------\n\n");
	
	
	/*結果出力(タスクセット)*/
	printf("\n----------------------------------Result (Task Set)----------------------------------\n\n");
	printf("No. %d\n", n);	
	printf("Tasks(AVG)=%0.1lf\t\tUtilization(AVG)=%0.2lf\t\tUtilization(MAX)=%0.2lf\n", aver_task/n, aver_utilization/n, max_utilization);
	printf("\n-------------------------------------------------------------------------------------\n\n");	
	
	
	/*ログの出力*/
	system("rm -f log.txt");
	
	char log[S];
	FILE *fplog;
		
	sprintf(log, "log.txt");
	fplog = fopen(log, "a");
		
	fprintf(fplog, "%0.2lf\t%0.1lf\r\n", aver_mem_EDF/n, aver_dm_EDF*100/n);
		
	fprintf(fplog, "%0.2lf\t%0.1lf\r\n", aver_mem_EDZL/n, aver_dm_EDZL*100/n);
		
	fprintf(fplog, "%0.2lf\t%0.1lf\r\n", aver_mem_LLF/n, aver_dm_LLF*100/n);
	
	fprintf(fplog, "%0.2lf\t%0.1lf\r\n", aver_mem_LMCF/n, aver_dm_LMCF*100/n);
	
	for(j=0;j<F;j++)
		fprintf(fplog, "%0.2lf\t%0.1lf\r\n", aver_mem_LMCLF[j]/n, aver_dm_LMCLF[j]*100/n);
	
	fprintf(fplog, "%0.2lf\t%0.1lf\r\n", aver_mem_AcII_LMCF/n, aver_dm_AcII_LMCF*100/n);
	
	for(j=0;j<F;j++)
		fprintf(fplog, "%0.2lf\t%0.1lf\r\n", aver_mem_AcII_LMCLF[j]/n, aver_dm_AcII_LMCLF[j]*100/n);
	
	for(j=0;j<F;j++)
		fprintf(fplog, "%0.2lf\t%0.1lf\t%0.1lf\r\n", aver_mem_LLF_LMCLF[j]/n, aver_dm_LLF_LMCLF[j]*100/n, aver_sw1[j]/n);
		
	for(j=0;j<F;j++)
		fprintf(fplog, "%0.2lf\t%0.1lf\t%0.1lf\r\n", aver_mem_LLF_AcII_LMCLF[j]/n, aver_dm_LLF_AcII_LMCLF[j]*100/n, aver_sw2[j]/n);
		
	fprintf(fplog, "%0.1lf\t%0.2lf\t%0.2lf\r\n", aver_task/n, aver_utilization/n, max_utilization);
	
	fclose(fplog);
	
	
	return 0;
}
