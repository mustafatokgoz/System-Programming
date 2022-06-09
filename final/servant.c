#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <dirent.h> 

#include "helper.h"
#include "utility.h"
#include "bst_for_files.h"


int main(int argc, char*argv[]){
    char ch;
    int n,m;
    char *director_path,*c_param,*ip;
    int port;
    int check_input = 0;
    char *err_mass = "You should enter the correct command (Run Way: ./servant -d directoryPath -c 10-19 -r IP -p PORT)\n";
    int i,j;
    char buff[256];
    void *ret;
    int low_bound,up_bound;
    
    while ((ch = getopt (argc, argv, "d:c:r:p:")) != -1){
      switch (ch){
        case 'd':
            director_path = optarg;
            check_input++;
            break;
        case 'c':
            c_param = optarg;
            check_input++;
            break;
        case 'r':
            ip = optarg;
            check_input++;
            break;
        case 'p':
            port = atoi(optarg);
            check_input++;
            break;      
        case '?':
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n"));
          write(2,err_mass,strlen(err_mass)); 
          return 1;
        default:
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n"));  
          write(2,err_mass,strlen(err_mass));
          return 1;
        }
    }
    if(check_input != 4){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }
    if(port < 2000){
        write(2,"You should enter port number bigger than 2000\n",strlen("You should enter port number bigger than 2000\n"));
        exit(0);
    }
    if (seperate_c_paramater(&low_bound,&up_bound,c_param) == -1){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }

    printf("%s %s %d %d %d \n",director_path,ip,port,low_bound,up_bound);
    int count = 0,count2= 0;
    node* root = NULL;
    root = read_from_disk(director_path,low_bound,up_bound,root);

    search(root,"00-01-2000","20-11-2055","VILLA","ADANA",1,&count);
    search(root,"00-01-2000","20-11-2055","VILLA","",0,&count2);
    printf("%d %d\n",count,count2);

    free_tree(root);
    /*
    char **content;
    content = malloc(5 * sizeof(char *));
    for(i = 0; i < 5; i++){
        content[i] = malloc(50 * sizeof(char));
    }
    strcpy(content[0],"mustafa tokgoz");
    strcpy(content[1],"musokgoz  dksjbfdkjs");
    strcpy(content[2],"must fdlksnfkdlsnf dsoz");
    strcpy(content[3],"musknfdk oz");
    node* root = NULL;
    root = insert(root,"10-11-2001","ANKARA","TARLA",content,5);
    insert(root,"20-11-2002","ISTANBUL","BINA",content,5);
    strcpy(content[2],"serhatoz  dksjbfdkjs");
    strcpy(content[3],"mserhatnfkdlsnf dsoz");
    insert(root,"14-01-2000","ANKARA","KAPI",content,5);
    insert(root,"20-11-2011","ISTANBUL","TARLA",content,5);
    insert(root,"18-11-2020","ANKARA","EV",content,5);
 
    node* root2 = root;
    

    int count;
    
    // print inoder traversal of the BST
    printf("%d \n",count);
    inorder(root);
    

    free_array2(content,5);

    */



    


    return 0;

}    


