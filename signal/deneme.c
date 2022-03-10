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


  

#define SYNC_SIG SIGUSR1 /* Synchronization signal */

/* Signal handler - does nothing but return */

static void handler(int sig)

{

}

 

int main(int argc, char *argv[])

{

   setbuf(stdout, NULL); /* Disable buffering of stdout */

   struct sigaction sa;

 

   /* STEP 1: create a signal set of type struct sigset_t */

   //Sigset_t is a data structure to specify what signals are affected.

   sigset_t blockMask, origMask, emptyMask;

 

   /* STEP 2: add user defined signal to user set by sigaddset() */

   //This function removes every signal on blockMask signal set. It always returns 0.

   sigemptyset(&blockMask);

   //This function adds SYNC_SIG to the signal set blockMask. SYNC_SIG is SIGUSR1 or user-defined signal.

   sigaddset(&blockMask, SYNC_SIG); /* Block signal */

 

   /* STEP 3: create a signal mask containing user defined signal by using sigpromask(). If there is error, print error msg */

   //Sigprocmask sets the signal MASK of the process to be union of (&blockMask, &origMask), and saves current process' signal set to &original

   if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1) 
            printf("sigprocmask error");

 

   /* STEP 4: defines actions to take once the signal is catch by the process */

   //set signal set "sa_mask" in sa structure to be empty

   sigemptyset(&sa.sa_mask);

   //sa_flags specifies a set of flags which modify the behavior of the signal

   sa.sa_flags = SA_RESTART;

   //sa_handler specifies the action to be associated with signum

   sa.sa_handler = handler;

   //sigaction is a system call, used to change the action taken by a process upon receipt of a specific signal.

   //upon SYNC_SIG is received, handler will run.

   if (sigaction(SYNC_SIG, &sa, NULL) == -1) 
            printf("sigaction error");

 

   /* STEP 5: Creating process ID for child process. */

   // pid_t is a data type, same as int or long int, representing process ID

   pid_t childPid;

 

   /* STEP 6: runs child process and parent process in parallel */

   switch (childPid = fork()) {

      case -1:

         printf("fork");

      /* STEP 6.2: Run Child process, send to parent process a user defined signal to trigger actions performed by Parent signal by kill().*/

      case 0:

         /* Child does some required action here... */

         printf("Child process PID: %ld started  - doing some work\n", (long) getpid());

         sleep(2); /* Simulate time spent doing some work */

         /* And then signals parent that it's done */

         printf("Child process PID: %ld about to signal Parent PID %ld \n", (long) getpid(), (long) getppid() );

         if (kill(getppid(), SYNC_SIG) == -1) 
                printf("kill caught error");

         /* Now child can do other things... */

         _exit(EXIT_SUCCESS);

      /* STEP 6.1: If Parent process is running, suspend the process if the user-defined signal is caught by using sigsuspend().*/

      default:

         /* Parent may do some work here, and then waits for child to

         complete the required action */

         printf("Parent PID: %ld about to wait for signal from Child Process \n", (long) getpid());

 

         // set signal set &emptyMask for Parent process to be empty????????????????

         sigemptyset(&emptyMask);

         printf("emptyMask is %ld \n",emptyMask.__val[0]);

         if (sigsuspend(&emptyMask) == -1 && errno != EINTR)

            printf("sigsuspend caught error");

         printf("Parent PID: %ld got signal from Child Process \n", (long) getpid());

         /* If required, return signal mask to its original state */

         if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1)

            printf("sigprocmask");

         /* Parent carries on to do other things... */

         exit(EXIT_SUCCESS);

   }

}
