#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>



typedef struct arg
{
    
   int nArg;
   int vArg;
   int cArg;
   int bArg;
   int tArg;
   char iArg[41];

}args;

typedef struct GTU344
{
    args givenParams;
    int dose1;
    int dose2;
    int totalLeft;
    int isRead;
    int fd;
    int leftCiti;
}clinic;
enum ProcessType{
    PARENT = 0,
    UNKNOWN,
    NURSE,
    VACCINATOR,
    CITIZEN
};
typedef struct ProcessInfo {
    pid_t pid;
    enum ProcessType type;
    int index;
}process;

void checkArguments(int argc, char **argv, args *givenArgs);
void showUsageAndExit();
void errExit(char *msg);
int safeLseek(int fd, int offset, int whence);
int safeRead(int fd, void *buf, size_t size);
int safeOpen(const char *file, int oflag);
char readOneChar(int fd);
int getMin(int a, int b);
void printNurseMsg(int index,pid_t pid, char vacc, clinic *info);
void printCitizenMsg(int index,pid_t pid, int time, clinic *info);
void printVaccinatorMsg(int index,pid_t pid, pid_t citPid);
void printStartMsg(clinic *info);
void citizenLeaveMsg(int leftCiti);
void nurseLeaveMsg();
void allCityMsg();
void vaccDoseMsg(int index,pid_t pid, int times);
void clinicClosedMsg();
#endif