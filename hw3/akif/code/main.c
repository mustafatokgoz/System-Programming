#include "main.h"
#include "helper.h"

sem_t *sem_mutex;
sem_t *sem_full1;
sem_t *sem_full2;
sem_t *sem_empty;
sem_t *sem_run;
sem_t *sem_vac;
sem_t *sem_cit;
static char memoryName[50];
static char helperMemory[50];
static clinic *biontech;
static process processInfo;
static pid_t *currentCitPid;
int main(int argc, char *argv[])
{

    args givenParams;
    checkArguments(argc, argv, &givenParams);
    biontech = getSharedMemory(givenParams);
    currentCitPid = getHelperSharedMem();
    //openSem(givenParams.bArg);
    createSignalHandler();
    processInfo.pid = getpid();
    processInfo.type = PARENT;
    processInfo.index = 0;
    printStartMsg(biontech);
    createCitizens(biontech);
    createVaccinators(biontech);
    createNurses(biontech);
    reapDeadChildren();
    clinicClosedMsg();
    cleanAndExit();
}
clinic *getSharedMemory(args givenArgs)
{
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
    strcpy(memoryName, "clinic_sinovac344");
    int memFd = shm_open(memoryName, O_CREAT | O_RDWR, mode);
    if (memFd == -1)
        errExit("shm_open error!");
    if (ftruncate(memFd, sizeof(*biontech)) == -1)
        errExit("ftruncate error");

    clinic *gata = (clinic *)mmap(NULL, sizeof(*biontech), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
    if (gata == MAP_FAILED)
        errExit("mmap");

    gata->givenParams = givenArgs;
    gata->dose1 = 0;
    gata->dose2 = 0;
    gata->totalLeft = 2 * (givenArgs.tArg * givenArgs.cArg);
    gata->isRead = 0;
    gata->leftCiti = givenArgs.cArg;
    gata->fd = safeOpen(givenArgs.iArg, O_RDONLY);
    return gata;
}
pid_t *getHelperSharedMem()
{
    mode_t mode2 = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
    strcpy(helperMemory, "comm_between_vac_cit");
    int memFd2 = shm_open(helperMemory, O_CREAT | O_RDWR, mode2);
    if (memFd2 == -1)
        errExit("shm_open error!");
    if (ftruncate(memFd2, sizeof(*currentCitPid)) == -1)
        errExit("ftruncate error");
    pid_t *cit_pid = (pid_t *)mmap(NULL, sizeof(*currentCitPid), PROT_READ | PROT_WRITE, MAP_SHARED, memFd2, 0);
    *cit_pid = 0;
    return cit_pid;
}
void openSem(int bufferSize)
{
    /*create named semphores*/
    sem_mutex = sem_open("mutex344", O_CREAT, 0666, 1);
    if (sem_mutex == SEM_FAILED)
        errExit("sem_open error!");

    sem_vac = sem_open("wait_vaccinator", O_CREAT, 0666, 0);
    if (sem_vac == SEM_FAILED)
        errExit("sem_open error!");

    sem_cit = sem_open("wait_citizen", O_CREAT, 0666, 0);
    if (sem_cit == SEM_FAILED)
        errExit("sem_open error!");

    sem_full1 = sem_open("full344", O_CREAT, 0666, 0);
    if (sem_full1 == SEM_FAILED)
        errExit("sem_open error!");
    sem_empty = sem_open("empty344", O_CREAT, 0666, bufferSize);
    if (sem_empty == SEM_FAILED)
        errExit("sem_open error!");
    sem_full2 = sem_open("full3442", O_CREAT, 0666, 0);
    if (sem_full2 == SEM_FAILED)
        errExit("sem_open error!");
    sem_run = sem_open("whorunfirst", O_CREAT, 0666, 0);
    if (sem_run == SEM_FAILED)
        errExit("sem_open error!");
}
void removeAll()
{
    if (sem_close(sem_full1) == -1)
        errExit("sem_close");
    if (sem_close(sem_empty) == -1)
        errExit("sem_close");
    if (sem_close(sem_full2) == -1)
        errExit("sem_close");
    if (sem_close(sem_run) == -1)
        errExit("sem_close");
    if (sem_close(sem_mutex) == -1)
        errExit("sem_close");
    if (sem_close(sem_vac) == -1)
        errExit("sem_close");
    if (sem_close(sem_cit) == -1)
        errExit("sem_close");
    sem_unlink("mutex344");
    sem_unlink("full344");
    sem_unlink("empty344");
    sem_unlink("full3442");
    sem_unlink("whorunfirst");
    sem_unlink("wait_vaccinator");
    sem_unlink("wait_citizen");
    shm_unlink(memoryName); //generic data
    shm_unlink(helperMemory); //current cit pid
}
void createNurses(clinic *biontech)
{

    for (int i = 0; i < biontech->givenParams.nArg; i++)
    {

        if (fork() == 0)
        {
            /* child process*/

            processInfo.pid = getpid();
            processInfo.type = NURSE;
            processInfo.index = i + 1;
            createSignalHandler();
            openSem(biontech->givenParams.bArg);
            nurse(biontech, &processInfo);
            cleanAndExit();
        }
    }
}
void createVaccinators(clinic *biontech)
{
    for (int i = 0; i < biontech->givenParams.vArg; i++)
    {

        if (fork() == 0)
        {
            /* child process*/

            processInfo.pid = getpid();
            processInfo.type = VACCINATOR;
            processInfo.index = i + 1;
            createSignalHandler();
            openSem(biontech->givenParams.bArg);
            vaccinator(biontech, &processInfo);
            cleanAndExit();
        }
    }
}
void createCitizens(clinic *biontech)
{
    for (int i = 0; i < biontech->givenParams.cArg; i++)
    {

        if (fork() == 0)
        {
            /* child process*/

            processInfo.pid = getpid();
            processInfo.type = CITIZEN;
            processInfo.index = i + 1;
            createSignalHandler();
            openSem(biontech->givenParams.bArg);
            citizen(biontech, &processInfo);
            cleanAndExit();
        }
    }
}
void createSignalHandler()
{
    struct sigaction sa;
    sa.sa_handler = &exitHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) < 0)
    {
        errExit("sigaction error!");
    }
}
void createSignalHandler2()
{
    struct sigaction sa1;
    sa1.sa_handler = &wakeHandler;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa1, NULL) < 0)
    {
        errExit("sigaction error!");
    }
}
void exitHandler(int signal)
{
    if (signal == SIGINT)
    {
        int savedErrno = errno;
        cleanAndExit();
        errno = savedErrno;
    }
}
void wakeHandler(int signal)
{
    if (signal == SIGUSR1)
    {
    }
}

void cleanAndExit()
{
    
    if (processInfo.type == PARENT)
    {
        reapDeadChildren();
        if (close(biontech->fd) < 0)
        {
            errExit("close file error!");
        }
    }
    else
    {
        removeAll();
    }
    exit(EXIT_SUCCESS);
}
void reapDeadChildren()
{
    pid_t childPid;
    int status;
    while ((childPid = waitpid(-1, &status, 0)) > 0)
        ;
    if (childPid == -1 && errno != ECHILD)
        errExit("waitpid");
}
void nurse(clinic *biontech, process *process)
{

    while (!biontech->isRead)
    {

        if (sem_wait(sem_empty) == -1)
            errExit("sem_wait");
        if (sem_wait(sem_mutex) == -1)
            errExit("sem_wait");

        if (!biontech->isRead)
        {
            char vaccine = readOneChar(biontech->fd);
            if (vaccine == '1')
            {
                biontech->dose1 = biontech->dose1 + 1;
                //biontech->totalLeft = biontech->totalLeft - 1;
                printNurseMsg(process->index, process->pid, '1', biontech);
                if (sem_post(sem_full1) == -1)
                    errExit("sem_post");
            }
            else if (vaccine == '2')
            {
                biontech->dose2 = biontech->dose2 + 1;
                printNurseMsg(process->index, process->pid, '2', biontech);
                if (sem_post(sem_full2) == -1)
                    errExit("sem_post");
            }
            else if (vaccine == 'x')
            {
                nurseLeaveMsg();
                biontech->isRead = 1;
                if (sem_post(sem_mutex) == -1)
                    errExit("sem_post");
                break;
            }
            else
            {
                errExit("vaccine is wrong!!");
            }
            if (sem_post(sem_mutex) == -1)
                errExit("sem_post");
        }
        else
        {
            if (sem_post(sem_mutex) == -1)
                errExit("sem_post");
            break;
        }
    }
}
void vaccinator(clinic *biontech, process *process)
{
    int counter = 0;
    while (biontech->totalLeft > 0)
    {

        if (sem_wait(sem_full1) == -1)
            errExit("sem_wait");
        if (sem_wait(sem_full2) == -1)
            errExit("sem_wait");
        if (sem_wait(sem_mutex) == -1)
            errExit("sem_wait");
        if (biontech->totalLeft > 0)
        {
            //wake the citizen
            if (sem_post(sem_vac) == -1)
                errExit("sem_post");
                //wait for citizen write its pid into shared memory
            if (sem_wait(sem_cit) == -1) 
                errExit("sem_wait");
            printVaccinatorMsg(process->index, process->pid, *currentCitPid);
            biontech->dose1 = biontech->dose1 - 1;
            biontech->dose2 = biontech->dose2 - 1;
            biontech->totalLeft = biontech->totalLeft - 2;
            counter++;
            //wake citizen again to write
            //its message on the screen
            if (sem_post(sem_run) == -1)
                errExit("sem_post");
                //wait for citizen finish its job
            if (sem_wait(sem_cit) == -1) 
                errExit("sem_wait");
            if (sem_post(sem_mutex) == -1)
                errExit("sem_post");
            if (sem_post(sem_empty) == -1)
                errExit("sem_post");
            if (sem_post(sem_empty) == -1)
                errExit("sem_post");
        }
        else
        {
            if (sem_post(sem_mutex) == -1)
                errExit("sem_post");
            break;
        }
    }
    if (sem_wait(sem_cit) == -1) //wait for cit update its pid
        errExit("sem_wait");
    if (sem_post(sem_full1) == -1)
        errExit("sem_post");
    if (sem_post(sem_full2) == -1)
        errExit("sem_post");
    if (sem_post(sem_mutex) == -1)
        errExit("sem_post");
    vaccDoseMsg(process->index, process->pid, counter);
    if (sem_post(sem_cit) == -1)
        errExit("sem_post");
}
void citizen(clinic *biontech, process *process)
{

    int left = biontech->givenParams.tArg;
    int total = biontech->givenParams.tArg;
    int last = 0;
    while (left > 0)
    {
        if (sem_wait(sem_vac) == -1)
            errExit("sem_wait");

        *currentCitPid = process->pid;
        left--;

        if (sem_post(sem_cit) == -1)
            errExit("sem_post");
        if (sem_wait(sem_run) == -1)
            errExit("sem_wait");

        printCitizenMsg(process->index, process->pid, total - left, biontech);
        if (biontech->totalLeft <= 0)
        {
            citizenLeaveMsg(0);
            last = 1;
            allCityMsg();
            if (sem_post(sem_cit) == -1)
                errExit("sem_post");
        }
        
        if (sem_post(sem_cit) == -1)
            errExit("sem_post");
        
        
    }
    //printf("totalLeft:%d\n",biontech->totalLeft);
    if (!last)
    {
        /*int res;
        if (biontech->totalLeft > biontech->givenParams.tArg * biontech->givenParams.cArg)
        {
            res = ((biontech->totalLeft) / 2) / (double)(biontech->givenParams.tArg);
        }
        else{
            res = (biontech->totalLeft) / (double)(biontech->givenParams.tArg);
        }*/
        biontech->leftCiti = biontech->leftCiti - 1;
        citizenLeaveMsg(biontech->leftCiti);
    }
}