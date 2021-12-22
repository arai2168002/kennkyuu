TARGETS = Rand.exe EDF.exe EDZL.exe LLF.exe LMCF.exe LMCLF_no1.exe LMCLF_no1_a.exe LMCLF_no1_a2.exe LMCLF_no1_a3.exe LMCLF_MP.exe  LMCLF_MP_LINER.exe LMCLF_MP_LOG.exe LMCLF_MP_LOG_STEP1.exe LMCLF_MP_LOG_SHIFT.exe LMCLF_MP_LOG_SHIFT_TRANS.exe LMCLF_no2.exe LMCLF_no3.exe AcII_LMCF.exe AcII_LMCLF_no1.exe AcII_LMCLF_no2.exe AcII_LMCLF_no3.exe LLF_LMCLF_no1.exe LLF_LMCLF_no2.exe LLF_LMCLF_no3.exe LLF_AcII_LMCLF_no1.exe LLF_AcII_LMCLF_no2.exe LLF_AcII_LMCLF_no3.exe Result.exe

OBJS = Rand.o EDF.o EDZL.o LLF.o LMCF.o LMCLF_no1.o LMCLF_no1_a.o LMCLF_no1_a2.o LMCLF_no1_a3.o LMCLF_MP.o  LMCLF_MP_LINER.o LMCLF_MP_LOG.o LMCLF_MP_LOG_STEP1.o LMCLF_MP_LOG_SHIFT.o LMCLF_MP_LOG_SHIFT_TRANS.o LMCLF_no2.o LMCLF_no3.o AcII_LMCF.o AcII_LMCLF_no1.o AcII_LMCLF_no2.o AcII_LMCLF_no3.o LLF_LMCLF_no1.o LLF_LMCLF_no2.o LLF_LMCLF_no3.o LLF_AcII_LMCLF_no1.o LLF_AcII_LMCLF_no2.o LLF_AcII_LMCLF_no3.o Result.o

CC =/usr/bin/gcc
RM =/bin/rm


all : $(TARGETS)

Rand.exe : Rand.o
	$(CC) -o $@ $< -lm
	
EDF.exe : EDF.o
	$(CC) -o $@ $< -lpthread

EDZL.exe : EDZL.o
	$(CC) -o $@ $< -lpthread

LLF.exe : LLF.o
	$(CC) -o $@ $< -lpthread
	
LMCF.exe : LMCF.o 
	$(CC) -o $@ $< -lpthread 

LMCLF_no1.exe : LMCLF_no1.o
	$(CC) -o $@ $<  -lpthread

LMCLF_no1_a.exe : LMCLF_no1_a.o
	$(CC) -o $@ $<  -lpthread

LMCLF_no1_a2.exe : LMCLF_no1_a2.o
	$(CC) -o $@ $<  -lpthread

LMCLF_no1_a3.exe : LMCLF_no1_a3.o
	$(CC) -o $@ $<  -lpthread

LMCLF_MP.exe : LMCLF_MP.o
	$(CC) -o $@ $<  -lpthread

LMCLF_MP_LINER.exe : LMCLF_MP_LINER.o
	$(CC) -o $@ $<  -lpthread

LMCLF_MP_LOG.exe : LMCLF_MP_LOG.o
	$(CC) -o $@ $<  -lpthread

LMCLF_MP_LOG_STEP1.exe : LMCLF_MP_LOG_STEP1.o
	$(CC) -o $@ $<  -lpthread

LMCLF_MP_LOG_SHIFT.exe : LMCLF_MP_LOG_SHIFT.o
	$(CC) -o $@ $<  -lpthread

LMCLF_MP_LOG_SHIFT_TRANS.exe : LMCLF_MP_LOG_SHIFT_TRANS.o
	$(CC) -o $@ $<  -lpthread

LMCLF_no2.exe : LMCLF_no2.o
	$(CC) -o $@ $<  -lpthread

LMCLF_no3.exe : LMCLF_no3.o
	$(CC) -o $@ $<	-lpthread
	
AcII_LMCF.exe : AcII_LMCF.o
	$(CC) -o $@ $<  -lpthread
	
AcII_LMCLF_no1.exe : AcII_LMCLF_no1.o
	$(CC) -o $@ $<  -lpthread
	
AcII_LMCLF_no2.exe : AcII_LMCLF_no2.o
	$(CC) -o $@ $<  -lpthread
	
AcII_LMCLF_no3.exe : AcII_LMCLF_no3.o
	$(CC) -o $@ $<  -lpthread
	
LLF_LMCLF_no1.exe : LLF_LMCLF_no1.o
	$(CC) -o $@ $<	-lpthread
	
LLF_LMCLF_no2.exe : LLF_LMCLF_no2.o
	$(CC) -o $@ $<	-lpthread
	
LLF_LMCLF_no3.exe : LLF_LMCLF_no3.o
	$(CC) -o $@ $<	-lpthread

LLF_AcII_LMCLF_no1.exe : LLF_AcII_LMCLF_no1.o 
	$(CC) -o $@ $< -lpthread
	
LLF_AcII_LMCLF_no2.exe : LLF_AcII_LMCLF_no2.o 
	$(CC) -o $@ $< -lpthread

LLF_AcII_LMCLF_no3.exe : LLF_AcII_LMCLF_no3.o 
	$(CC) -o $@ $< -lpthread

	
Result.exe : Result.o
	$(CC) -o $@ $<
	

Rand.o : Rand.c MT.h

EDF.o : EDF.c

EDZL.o : EDZL.c

LLF.o : LLF.c

LMCF.o : LMCF.c

LMCLF_no1.o : LMCLF_no1.c
LMCLF_no1_a.o : LMCLF_no1_a.c
LMCLF_no1_a2.o : LMCLF_no1_a2.c
LMCLF_no1_a3.o : LMCLF_no1_a3.c
LMCLF_MP.o : LMCLF_MP.c
LMCLF_MP_LINER.o : LMCLF_MP_LINER.c
LMCLF_MP_LOG.o : LMCLF_MP_LOG.c
LMCLF_MP_LOG_STEP1.o : LMCLF_MP_LOG_STEP1.c
LMCLF_MP_LOG_SHIFT.o : LMCLF_MP_LOG_SHIFT.c
LMCLF_MP_LOG_SHIFT_TRANS.o : LMCLF_MP_LOG_SHIFT_TRANS.c
LMCLF_no2.o : LMCLF_no2.c
LMCLF_no3.o : LMCLF_no3.c

AcII_LMCF.o : AcII_LMCF.c

AcII_LMCLF_no1.o : AcII_LMCLF_no1.c
AcII_LMCLF_no2.o : AcII_LMCLF_no2.c
AcII_LMCLF_no3.o : AcII_LMCLF_no3.c

LLF_LMCLF_no1.o : LLF_LMCLF_no1.c
LLF_LMCLF_no2.o : LLF_LMCLF_no2.c
LLF_LMCLF_no3.o : LLF_LMCLF_no3.c

LLF_AcII_LMCLF_no1.o : LLF_AcII_LMCLF_no1.c
LLF_AcII_LMCLF_no2.o : LLF_AcII_LMCLF_no2.c
LLF_AcII_LMCLF_no3.o : LLF_AcII_LMCLF_no3.c

Result.o : Result.c


clean:
	$(RM) -f $(OBJS) *~
	$(RM) -f $(TARGETS) *.txt
	$(RM) -f $(TARGETS) *.stackdump


