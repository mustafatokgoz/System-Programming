#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


#define FORK_NUMBER 3

typedef struct pid_store{
    int many;
    pid_t pid_arr[FORK_NUMBER];
}pid_store;

void errExit(char *inf){
    char *errInf = "Error was occured for given information : ";
    write(2, errInf, strlen(errInf));
    write(2, inf, strlen(inf));    
    write(2, "\n", 1);
    exit(0);
}

void exitInf(char *inf){
    write(1, inf, strlen(inf));    
    write(1, "\n", 1);
    exit(0);
}

volatile sig_atomic_t child_action = 0;
volatile sig_atomic_t user_action = 0;
pid_store store;

void handler(int signal_number){
    int status;
    pid_t childPid;
    while ((childPid = waitpid(-1, &status, WNOHANG)) > 0){

    }
    errExit(" Exiting by control c");
}

void child_handler(int signal_number){
        ++child_action;
}
void user_handler(int signal_number){
        ++user_action;
}

void errAllExit(char *err){
    int status;
    pid_t childPid;
    while ((childPid = waitpid(-1, &status, WNOHANG)) > 0){

    }
    errExit(err);    
}

void fork_function(){
    sigset_t mask1;
    sigemptyset(&mask1);
    sigaddset(&mask1, SIGCHLD);

    printf("This is the fork part before of parent part of fork pid %d\n",getpid());
    store.pid_arr[store.many] = getpid();
    store.many = store.many + 1;

    if (kill(getppid(),SIGUSR1) == -1){
        errAllExit("error");
    }

    while(child_action < FORK_NUMBER){
        if (sigsuspend(&mask1) == -1 && errno != EINTR)
                errAllExit("Sigsupend error\n");
    }

    printf("This is the fork part after of parent part\n");
    kill(getppid(),SIGUSR1);
}

void parent_function(){
    int i = 0;
    sigset_t mask2;
    sigemptyset(&mask2);
    //sigaddset(&mask2, SIGUSR1);
                
    //while(user_action < FORK_NUMBER){
    if (sigsuspend(&mask2) == -1 && errno != EINTR)
            errAllExit("Sigsupend error\n");
    //}
    printf("\n number %d \n",user_action);
    printf("\n store many %d  \n",store.many);
    if (store.many < FORK_NUMBER){
        errAllExit("Store pid number error\n");
    }
    for(i = 0; i < FORK_NUMBER; i++){
        kill(store.pid_arr[i], SIGCHLD);
    }
    
    user_action = 0;
    while(user_action < FORK_NUMBER){
        if (sigsuspend(&mask2) == -1 && errno != EINTR)
            errAllExit("Sigsupend error\n");
    }

}

int main(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa , NULL);

    int i = 0;

    struct sigaction child_sa;
    memset(&child_sa, 0, sizeof(child_sa));
    child_sa.sa_handler = &child_handler;
    child_sa.sa_flags = 0;
    if (sigaction(SIGCHLD, &child_sa, NULL) != 0){
        errExit("Sigaction error");
    }

    struct sigaction parent_sa;
    memset(&parent_sa, 0, sizeof(parent_sa));
    parent_sa.sa_handler = &user_handler;
    parent_sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &parent_sa, NULL) != 0){
        errExit("Sigaction error");
    }
    
    store.many = 0;
    // parent part before the fork
    printf("This is the parent part before the fork\n");
    for(i = 0; i < FORK_NUMBER ; i++){
        switch (fork())
        {
        case -1:
            errAllExit("Fork error");
        case 0:    
            fork_function();
            break;
        default:
            parent_function();
            break;
        }

    }
    // Last parent part
    printf("This is the parent part of the program after fork\n");
    fflush(stdout);
    errAllExit(" All processes are finished\n");

    return 0;

}
