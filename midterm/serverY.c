#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include "helper.h"

#define CHECKFILE "checkinstantiation"

void create_Y_processes(int n);
void create_semaphores();
void seperate_argumants(int,char *argv[],char **,char **,int *,int *,int *);
void serverY(char *,char*,int);
void destroy_semaphores();
void destroy_shared();
void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);
char *timestamp();
int open_file(char *filename);
void close_file(int);
void open_for_check();
void remove_for_check();
void becomeDaemon();

sem_t *semY;
sem_t *semW;
sem_t *semZ;

typedef struct track{
	int reqNum;
	int invertNum;
	int notInvertNum;
  int farwardedNum;
}track;

void initilizeshared();
static track *keep_track;

static int filedes[2];
static int sleeptime;
static int logfd;
static int poolsizeLog;
static pid_t pid_parent;


sig_atomic_t sig_check_server=0;


void killzombies() {
    destroy_semaphores();
    destroy_shared();
    kill(0, SIGTERM);
    while (poolsizeLog > 0) {
        if (wait(NULL) != -1) {
            poolsizeLog--;
        }
    }
}

/*to handle ctrl c*/
void signal_handle(int sig) {
  if(sig == SIGINT){
    remove_for_check();
    if(keep_track!=NULL){
        dprintf(logfd, "%s , SIGINT received, terminating Z and exiting server Y. Total requests handled: %d, %d invertible, %d not. %d requests were forwarded.\n",
            timestamp(),keep_track->reqNum,keep_track->invertNum,keep_track->notInvertNum,keep_track->farwardedNum);
    }
    killzombies();
    exit(0);
  }
}




int main(int argc, char *argv[]){
  char *pathServer , *logfile;
  int poolsize, poolsize2,t;
  
  struct sigaction sa;

  open_for_check();
  becomeDaemon();

  sa.sa_handler = signal_handle;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags=0;
  sigaction(SIGINT,&sa,NULL);
  
  pid_parent = getpid();
  
  
  if(pipe(filedes) == -1){
    exitInf("pipe error");
  }
 
  
  seperate_argumants(argc,argv,&pathServer, &logfile, &poolsize, &poolsize2,&t);

  logfd = open(logfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (logfd == -1){
      exitInf("Log file doesn't open\n");
  }

  
  create_semaphores();

  initilizeshared();
  create_Y_processes(poolsize);
  
  serverY(pathServer,logfile,poolsize);

  destroy_semaphores();
  destroy_shared();
  return 0;
}

void open_for_check(){
    if(open(CHECKFILE, O_RDWR|O_CREAT|O_EXCL, 0666) == -1) {
        if (errno == EEXIST){
            dprintf(logfd,"Only one instantiation can be created!\n");
            exit(EXIT_FAILURE);
        }
    }
}
void remove_for_check(){
    remove(CHECKFILE);
}

void becomeDaemon(){
    switch (fork())
    {
        case -1:
            exitInf("fork error");
        case 0:
            break;
        default:
            exit(EXIT_SUCCESS);
    }

    if (setsid() == -1)
        exitInf("setsid error");

    //ignore some signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    switch (fork())
    {
        case -1:
            exitInf("fork error");
        case 0:
            break;
        default:
            exit(EXIT_SUCCESS);
    }
    umask(0);

    close(STDOUT_FILENO);
    close(STDERR_FILENO);


}







void serverY(char *pathServer,char *logname ,int poolsize){
  int serverFd, dummyFd;
  char clientFifo[CLIENT_FIFO_NAME_LEN];
  struct request req;

  umask(0); /* So we get the permissions we want */ 
  if (mkfifo(pathServer, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
   exitInf("server make fifo error in serverY");
  serverFd = open(pathServer, O_RDONLY); if (serverFd == -1){
    exitInf("server fifo error in serverY");
  }

  /* Open an extra write descriptor, so that we never see EOF */
  dummyFd = open(pathServer, O_WRONLY); 
  if (dummyFd == -1){
    exitInf("server fifo error in serverY");
  }
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    exitInf("signal pipe error");
  }
  dprintf(logfd,"%s , Server Y (%s, p=%d, t=%d) started\n",timestamp(),logname,poolsize,sleeptime);
  /* Create well-known FIFO, and open it for reading */
  for(;;) { 
    
    if (read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
      dprintf(logfd, "Error reading request; discarding\n"); 
      continue; /* Either partial read or error */
    }

    int arr[req.seqLen*req.seqLen];

    if (read(serverFd, &arr, req.seqLen *  req.seqLen * sizeof(int)) != (req.seqLen * req.seqLen * sizeof(int)) ) {
        dprintf(logfd, "Error reading request2; discarding\n"); 
        continue; /* Either partial read or error */
    }


    struct request temp;
    temp.pid = req.pid;
    temp.seqLen = req.seqLen;

    int value;
    if (write(filedes[1], &temp, sizeof(struct request))!= sizeof(struct request)){
        dprintf(logfd, "Error writing to pipe %s\n", clientFifo);
    }
    if (write(filedes[1], &arr, req.seqLen * req.seqLen * sizeof(int)) != (req.seqLen * req.seqLen * sizeof(int))){
        dprintf(logfd, "Error writing to pipe2 %s\n", clientFifo);
    }
    if(sem_post(semY) == -1){
        dprintf(logfd, "Errorsemaphore %s\n", clientFifo);
    }


    sem_getvalue(semW,&value); 

    if(value == poolsizeLog){
      dprintf(logfd,"%s , Forwarding request of client PID#%d to serverZ, matrix size %dx%d, pool busy %d/%d\n",timestamp(),req.pid,req.seqLen,req.seqLen,value,poolsize);
      keep_track->farwardedNum = keep_track->farwardedNum + 1;
    }
    
    
  }

}



void seperate_argumants(int argc ,char *argv[],char **pathserver,char **logfile,int *poolsize,int *poolsize2,int *t){
    char c;
    int check_input = 0;
    while ((c = getopt (argc, argv, "s:o:p:r:t:")) != -1){
      switch (c){
        case 's':
          *pathserver = optarg;
          check_input++;
          break;
        case 'o':
          *logfile = optarg;
          check_input++;
          break;
        case 'p':
          *poolsize = atoi(optarg);
          if(*poolsize >= 2){
            check_input++;
            poolsizeLog = *poolsize;
          }
          break;  
        case 'r':
          *poolsize2 = atoi(optarg);
          if(*poolsize2 >= 2)
            check_input++;
          break;
        case 't':
          *t = atoi(optarg);
          sleeptime = *t;
          if(*t > 0)
            check_input++;
          break;        
        case '?':
          write(logfd,"Something went wrong.\n",strlen("Something went wrong.\n")); 
          exit(0);
        break;  
        }
    }

    if(check_input != 5){
      exitInf("\nYou should write the correct way. Like ./serverY -s pathToServerFifo -o pathToLogFile –p poolSize -r poolSize2 -t 2\n");
      exit(0);
    }

}



void create_semaphores(){
    semY = sem_open("/semname1", O_CREAT , 0666, 0);
    if (semY == SEM_FAILED)
    {
      write(logfd, "\nERROR creating semaphore Yprocess" ,strlen("ERROR creating semaphore Yprocess\n"));
      exit(EXIT_FAILURE);
     }
    semW = sem_open("/semname2", O_CREAT , 0666, 0);
    if (semW == SEM_FAILED)
    {
      write(logfd, "\nERROR creating semaphore working" ,strlen("ERROR creating semaphore working\n"));
      exit(EXIT_FAILURE);
    }
    semZ = sem_open("/semname3", O_CREAT, 0666, 0);
    if (semZ == SEM_FAILED)
    {
      write(logfd, "\nERROR creating semaphore Zprocess" ,strlen("ERROR creating semaphore Zprocess\n"));
      exit(EXIT_FAILURE);
    }
}


void destroy_semaphores(){
  if (sem_close(semY) == -1)
      exitInf("sem close");
  if (sem_close(semW) == -1)
      exitInf("sem close");
  if (sem_close(semZ) == -1)
      exitInf("sem close");

  if (sem_unlink("/semname1") == -1){
      write(logfd,"\nSem unlink error ! \n",strlen("\nSem unlink error ! \n"));
  }
  if (sem_unlink("/semname2") == -1){
      write(logfd,"\nSem unlink error ! \n",strlen("\nSem unlink error ! \n"));
  }
  if (sem_unlink("/semname3") == -1){
      write(logfd,"\nSem unlink error ! \n",strlen("\nSem unlink error ! \n"));
  }
}

int calculate_det(int n, int copy_arr[][n]){
  int i, a, b, k, det;
  int res = 0;
  int sign = 1;
  if(n == 2){
    det = copy_arr[0][0] * copy_arr[1][1] - copy_arr[0][1] * copy_arr[1][0];
    return det;
  }
  else{
    for (k = 0; k < n ; k++){
      int arr[n-1][n-1];

      for (i = 1; i < n; i++){
          b = 0;
          for (a = 0; a < n; a++){
              if (a == k) 
                  continue;
              arr[i-1][b] = copy_arr[i][a];
              b = b + 1;
          }
      }

      res = res +  sign * copy_arr[0][k] * calculate_det(n-1,arr);
      sign = sign * -1;
    }
  }

  return res;

}


void child_Y_process(){
  struct request req;
  struct response resp;
  int value;
  char clientFifo[CLIENT_FIFO_NAME_LEN];
  int clientfd;
  
  while(1){
    if(sem_wait(semY) ==-1){
         exitInf("sem wait error semY");
    }
    if(sem_post(semW) ==-1){
         exitInf("sem wait error semY");
    }
    
    
    if (read(filedes[0], &req, sizeof(struct request)) != sizeof(struct request)) {
      dprintf(logfd, "Error reading request; discarding\n"); 
      //continue; /* Either partial read or error */
    }
     int arr[req.seqLen*req.seqLen];
    if (read(filedes[0], &arr, req.seqLen * req.seqLen * sizeof(int)) != (req.seqLen * req.seqLen * sizeof(int)) ) {
        dprintf(logfd, "Error reading request2; discarding\n"); 
      //    continue; /* Either partial read or error */
    }
    sem_getvalue(semW,&value);
    dprintf(logfd,"%s , Worker PID#%d is handling client PID#%d, matrix size %dx%d, pool busy %d/%d\n",timestamp(),getpid(),req.pid,req.seqLen,req.seqLen,value,poolsizeLog);
    keep_track->reqNum = keep_track->reqNum + 1;
    int i = 0 ;
    int result;
    int k = 0 , j = 0;
    int n = req.seqLen;
    
    int copy_arr[n][n];
    for(i = 0; i < n ; i++){
        for(j = 0; j < n ; j++){
            copy_arr[i][j] =  arr[k];
            k++;
        }
    }


    result = calculate_det(req.seqLen,copy_arr);
    
    sleep(sleeptime);

    /* Open client FIFO (previously created by client) */
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) req.pid);
    clientfd = open(clientFifo, O_WRONLY);
    if(clientfd == -1){
      exitInf("clientfd error");
    }
    resp.seqNum = req.pid;
    resp.is_invertible = result;
    if (write(clientfd, &resp, sizeof(struct response))!= sizeof(struct response)){
        fprintf(stderr, "Error writing to FIFO %s\n", clientFifo);
    }
    if (close(clientfd) == -1){
       perror("close error");
    }
    if(result == 0){
      dprintf(logfd,"%s , Worker PID#%d responding to client PID#%d: the matrix IS NOT invertible.\n",timestamp(),getpid(),req.pid);
      keep_track->notInvertNum = keep_track->notInvertNum + 1;
    }
    else{
      dprintf(logfd,"%s , Worker PID#%d responding to client PID#%d: the matrix is invertible.\n",timestamp(),getpid(),req.pid);
      keep_track->invertNum = keep_track->invertNum + 1;
    }
    if(sem_wait(semW) ==-1){
         exitInf("sem wait error semY");
    }
  }
}


void create_Y_processes(int n){
    int i = 0;
    for(i = 0; i < n ;i++){
      switch (fork()) {
        case -1: 
            exitInf("fork");  
        case 0: /* Child */
            create_semaphores();
            child_Y_process(); 
            break;
        default: /* Parent */
            break;
      }
    }



}




void lock_file(int fd, struct flock fl){
    fl.l_type = F_WRLCK;
    if (fcntl(fd, F_SETLKW, &fl) == -1){
    		perror("fcntl");
    		exit(1);
    }   
}

void unlock_file(int fd, struct flock fl){
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &fl) == -1){
    		perror("fcntl");
    		exit(1);
    }    
}



char *timestamp()
{
    time_t now;
    now = time(NULL);
    char *tstr = ctime(&now);
    char *line = strchr(tstr, '\n');
    line[0] = '\0';
    return tstr;
}

void initilizeshared(){  
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
    char memoryName[15];
    strcpy(memoryName,"serverY_track");
    shm_unlink(memoryName);
    int memFd = shm_open(memoryName, O_RDWR | O_CREAT , mode);
    if (memFd == -1)
        exitInf("shm_open error!");
    if (ftruncate(memFd, sizeof(*keep_track)) == -1)  
        exitInf("ftruncate error");

    keep_track = (track *)mmap(NULL, sizeof(*keep_track), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
    if (keep_track == MAP_FAILED)
        exitInf("mmap");
    
    keep_track->reqNum = 0;
    keep_track->invertNum = 0;
    keep_track->notInvertNum = 0;
    keep_track->farwardedNum=0;
    close(memFd);
}


void destroy_shared(){
  char memoryName[15];
  strcpy(memoryName,"serverY_track");
  shm_unlink(memoryName);
}

int open_file(char *filename){
    int fd = open (filename, O_RDWR);
    if (fd == -1) {
        exitInf("file error");
    }
    return fd;
}

void close_file(int fd){
  if(close(fd) == -1){
    exitInf("close error");
  }
}





