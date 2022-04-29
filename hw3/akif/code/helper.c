#include "helper.h"

static char msg[250];

void checkArguments(int argc, char **argv, args *givenArgs)
{

    int opt,res;
    //initialize all values to zero
    givenArgs->bArg = 0;
    givenArgs->cArg = 0;
    givenArgs->nArg = 0;
    givenArgs->tArg = 0;
    givenArgs->vArg = 0;
    if (argc < 13)
    {
        showUsageAndExit();
    }
    //beforehand get t and c values
    /*res = atoi(argv[10]);
    if (res < 1)
    {
        showUsageAndExit();
    }
    else{
         givenArgs->tArg = res;
    }
    res = atoi(argv[6]);
    if (res < 3)
    {
        showUsageAndExit();
    }
    else{
         givenArgs->cArg = res;
    }*/
    while ((opt = getopt(argc, argv, "n:v:c:b:t:i:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            res = atoi(optarg);
            if (res < 2)
            {
                showUsageAndExit();
            }
            givenArgs->nArg = res;
            break;
        case 'v':
            res = atoi(optarg);
            if (res < 2)
            {
                showUsageAndExit();
            }
            givenArgs->vArg = res;
            break;
        case 'c':
            res = atoi(optarg);
            if (res < 3)
            {
                showUsageAndExit();
            }
            givenArgs->cArg = res;
            break;
         case 'b':
            res = atoi(optarg);
            if (res < 1)
            {
                showUsageAndExit();
            }
            givenArgs->bArg = res;
            break;
        case 't':
            res = atoi(optarg);
            if (res < 1)
            {
                showUsageAndExit();
            }
            givenArgs->tArg = res;
            break;
       
        case 'i':
            strcpy(givenArgs->iArg , optarg);
            break;
        case '?':
            showUsageAndExit();
            break;
        default:
            showUsageAndExit();
            break;
        }
    }
    if (givenArgs->bArg < (givenArgs->tArg*givenArgs->cArg) + 1)
    {
        showUsageAndExit();
    }
     
}

void showUsageAndExit()
{
    printf("Usage: ./program [FLAGS] and [PARAMETERS]\n"
           "Flags and Parameters:\n"
           "-n >= 2: the number of nurses (integer)\n"
           "-v >= 2: the number of vaccinators (integer)\n"
           "-c >= 3: the number of citizens (integer)\n"
           "-b >= tc+1: size of the buffer (integer)\n"
           "-t >= 1: how many times each citizen must receive the 2 shots (integer)\n"
           "-i: pathname of the input file\n"
           "Example\n"
           "./program -n 3 -v 2 -c 3 -b 11 -t 3 -i /home/user/Desktop/test\n");
        exit(EXIT_FAILURE);
}

void errExit(char *msg){
    //In case of an arbitrary error, 
    //exit by printing to stderr a nicely formatted informative message.
    fprintf(stderr, "%s:%s\n",msg,strerror(errno));
    exit(EXIT_FAILURE);
}
int safeRead(int fd, void *buf, size_t size)
{
    int rd = read(fd, buf, size);
    if (rd == -1)
    {
        errExit("reading error!");
    }
    return rd;
}
int safeOpen(const char *file, int oflag)
{
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
    int fd = open(file, oflag, mode);
    if (fd < 0)
    {
        errExit("open error!");
    }
    return fd;
}
int safeLseek(int fd, int offset, int whence)
{
    int pos = lseek(fd, offset, whence);
    if (pos == -1)
    {
        errExit("lseek error!");
    }
    return pos;
}
char readOneChar(int fd){
    char c;
    int eof;
    // x represent end of file
    eof = safeRead(fd, &c, 1);
    if (eof != 0)
    {
        if (c != '\n')
        {
            return c;
        }
        eof = safeRead(fd, &c, 1);
        if (eof != 0)
        {
            return c;
        }
        return 'x';
        
    }
    return 'x'; 
}
int getMin(int a, int b){
    if (a < b)
    {
        return a;
    }
    else{
        return b;
    }
    
}
void printNurseMsg(int index,pid_t pid, char vacc, clinic *info){
    sprintf(msg,"Nurse %d (pid=%ld) has brought vaccine %c:the clinic has %d vaccine1 and %d vaccine2.",index, (long)pid,vacc,info->dose1,info->dose2);
    printf("%s\n",msg);
}
void printCitizenMsg(int index,pid_t pid, int time, clinic *info){
    if (time == 1)
    {
        sprintf(msg,"Citizen %d (pid=%ld) is vaccinated for the %dst time: the clinic has %d vaccine1 and %d vaccine2",index,(long)pid,time,info->dose1,info->dose2);
    }
    else if (time == 2)
    {
        sprintf(msg,"Citizen %d (pid=%ld) is vaccinated for the %dnd time: the clinic has %d vaccine1 and %d vaccine2",index,(long)pid,time,info->dose1,info->dose2);
    }
    else if (time == 3)
    {
        sprintf(msg,"Citizen %d (pid=%ld) is vaccinated for the %drd time: the clinic has %d vaccine1 and %d vaccine2",index,(long)pid,time,info->dose1,info->dose2);
    }
    else{
        sprintf(msg,"Citizen %d (pid=%ld) is vaccinated for the %dth time: the clinic has %d vaccine1 and %d vaccine2",index,(long)pid,time,info->dose1,info->dose2);
    }
    printf("%s\n",msg);
}
void printVaccinatorMsg(int index,pid_t pid, pid_t citPid){
    sprintf(msg,"Vaccinator %d (pid=%ld) is inviting citizen pid=%ld to the clinic",index,(long)pid,(long)citPid);
    printf("%s\n",msg);
}
void printStartMsg(clinic *info){
    sprintf(msg,"Welcome to the GTU344 clinic. Number of citizens to vaccinate c=%d with t=%d doses.",info->givenParams.cArg,info->givenParams.tArg);
    printf("%s\n",msg);
}
void citizenLeaveMsg(int leftCiti){
    sprintf(msg,"Citizen is leaving. Remaining citizens to vaccinate: %d",leftCiti);
    printf("%s\n",msg);
}
void nurseLeaveMsg(){
    printf("Nurses have carried all vaccines to the buffer, terminating.\n");
}
void allCityMsg(){
    printf("All citizens have been vaccinated.\n");
}
void vaccDoseMsg(int index,pid_t pid, int times){
    sprintf(msg,"Vaccinator %d (pid=%ld) vaccinated %d doses.",index,(long)pid,times);
    printf("%s\n",msg);
}
void clinicClosedMsg(){
    printf("The clinic is now closed. Stay healthy.\n");
}
/*int main(int argc, char *argv[])
{
    //start search operation
    args givenParams;
    checkArguments(argc, argv, &givenParams);
    printf("n:%d\n",givenParams.nArg);
    printf("v:%d\n",givenParams.vArg);
    printf("c:%d\n",givenParams.cArg);
    printf("b:%d\n",givenParams.bArg);
    printf("t:%d\n",givenParams.tArg);
    printf("i:%s\n",givenParams.iArg);


    //-------------------------------

    printf("n:%d\n",biontech->givenParams.nArg);
    printf("v:%d\n",biontech->givenParams.vArg);
    printf("c:%d\n",biontech->givenParams.cArg);
    printf("b:%d\n",biontech->givenParams.bArg);
    printf("t:%d\n",biontech->givenParams.tArg);
    printf("i:%s\n",biontech->givenParams.iArg);
    printf("dose1:%d\n",biontech->dose1);
    printf("dose2:%d\n",biontech->dose2);
    printf("fd:%d\n",biontech->fd);

    return 0;

}*/

