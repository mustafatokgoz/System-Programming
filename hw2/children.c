#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);
int open_file(char *filename);

extern char **environ;


sig_atomic_t sig_check=0;

/*to handle ctrl c*/
void signal_handle(int sig) {
    sig_check=1;
}

void quit_signal_c(){
  write(2,"\nYour program was cut by keyboard (child)\n",strlen("\nYour program was cut by keyboard (child)\n"));
  exit(0);
}

int main(int argc, char *argv[]){
    char **ep,*temp;
    int fd;
    char buffer[256];
    int length = 0;
    int i = 0, j = 0, k = 0;
    struct flock fl = { F_WRLCK, SEEK_SET, 0,       0,     0 };
    double mean[3] = {0, 0, 0};
    double covariance_matrix[3][3];
    char points[10][3];

    struct sigaction sa;

    sa.sa_handler = signal_handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGINT,&sa,NULL);

    if (argc == 2){
        fd = open_file(argv[0]);
        i = 0;
        j = 0;
        for (ep = environ; *ep != NULL; ep++){
            //write(1,*ep,strlen(*ep));
            temp = *ep;
            for(i = 0;i < 4; i++){
                points[j][i] = temp[i];
            }
            mean[0] += (int) temp[0];
            mean[1] += (int) temp[1];
            mean[2] += (int) temp[2];
            j++;
        }

        mean[0] = mean[0] / 10;
        mean[1] = mean[1] / 10;
        mean[2] = mean[2] / 10;

        if(sig_check == 1){
            quit_signal_c();
        }
        for (i = 0; i < 3; i++){
		    for (j = 0; j < 3; j++) {
			    covariance_matrix[i][j] = 0.0;
			    for (k = 0; k < 10; k++){
				    covariance_matrix[i][j] += (mean[i] - points[k][i]) * (mean[j] - points[k][j]);
                }
                covariance_matrix[i][j] /= 10;
		    }
        }
        if(sig_check == 1){
            quit_signal_c();
        }
        lock_file(fd,fl);
        write(fd,argv[1],strlen(argv[1]));
        for(i = 0; i < 3 ; i++){
            for( j = 0; j < 3; j++){
                length = sprintf(buffer, "%.3f",covariance_matrix[i][j]);
                write(fd,buffer,length);
                write(fd," ",1);
            }
            write(fd,"\n",1);
        }
        unlock_file(fd,fl);
        close(fd);
        if(sig_check == 1){
            quit_signal_c();
        }

    }    
    else{
         write(1,"wrong input arr\n",strlen("wrong input arr\n"));
    }
    return 0;
}


int open_file(char *filename){
    int fd;
    fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("file error");
        exit(0);
    }
    return fd;
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