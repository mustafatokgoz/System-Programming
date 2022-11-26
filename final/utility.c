#include "utility.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "bst_for_files.h"



int seperate_c_paramater(int *lower_bound,int *upper_bound,char *c_parameter){
    char *token = strtok(c_parameter,"-");
    *lower_bound = atoi(token);
    if(*lower_bound == 0){
        return -1;
    }
    token = strtok(NULL,"-");
    *upper_bound = atoi(token);
    if(*upper_bound == 0){
        return -1;
    }
    return 0;
}

void sort_cities(char arr[][70], int n){
    char temp[70];
    int i,j;
    for (i=0; i<n-1; i++){
        for (j=0; j<n-1-i; j++){
            if (strcmp(arr[j], arr[j+1]) > 0){
                strcpy(temp, arr[j]);
                strcpy(arr[j], arr[j+1]);
                strcpy(arr[j+1], temp);
            }
        }
    }
}
 



node* read_from_disk(char *director_path,int low_bound,int up_bound,node* root,char *city1,char *city2){
    DIR *directory = opendir(director_path);  /*to open directory*/
    struct dirent *dir=NULL;
    char buff[500],buff2[500];
    node *first_addr;
    int city_count = 0;
    int index = 0;
    int i = 0;
    int check = 0;
    int len;
    
    if(directory == NULL){
        return 0;
    }
    
    while ((dir = readdir(directory)) != NULL){
        if(dir->d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0){
            city_count++;
        } 
    }    
    closedir(directory);
   
    

    directory = opendir(director_path); 
   
    char sorted_array[city_count][70];
    index = 0;
    while ((dir = readdir(directory)) != NULL){
        if(dir->d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0){
            strcpy(sorted_array[index],dir->d_name);
            index++;
        } 
        
    }   
    index = 0;
    closedir(directory);
    
    
    sort_cities(sorted_array,city_count);
    /*
    for(i = 0; i < city_count;i++){
        printf("%s \n",sorted_array[i]);
    }
    */

    int j = 0;
    char **fcontent = NULL;
    for(i = 0; i < up_bound-low_bound + 1 ; i++){
        len = sprintf(buff,"%s/%s",director_path,sorted_array[i+low_bound-1]);
        if(i == 0){
            strcpy(city1,sorted_array[i+low_bound-1]);
        }
        else if(i == up_bound-low_bound){
             strcpy(city2,sorted_array[i+low_bound-1]);
        }
        buff[len] = '\0';
        directory = opendir(buff);
        if(directory == NULL){
            return NULL;
        }
        
        while( (dir = readdir(directory)) != NULL){
            
            if(dir->d_type == DT_REG){
                
                len = sprintf(buff2,"%s/%s",buff,dir->d_name);
                buff2[len] = '\0';
                
                FILE * fp;
                
                
                fp = fopen(buff2, "r");
                if (fp == NULL)
                    exit(0);
                   
                j = 0;
                
                fcontent = NULL;
                char buffer[1024];
                while (fgets(buffer, 1024, fp)){
                    if(strlen(buffer) > 5 && buffer[0]!= '\n' ){
                        char **new_fcontent = realloc(fcontent,(j+1) * sizeof(char*));
                        if(new_fcontent == NULL){
                            perror("null");
                        }
                        fcontent = new_fcontent;
                        buffer[strcspn(buffer, "\n")] = 0;
                        if(strlen(buffer) > 5 ){
                            fcontent[j] = malloc((strlen(buffer) + 2) * sizeof(char));
                            strncpy(fcontent[j],buffer,strlen(buffer));
                            fcontent[j][strlen(buffer)]='\0';
                            if(strlen(fcontent[j]) > 4){
                                j++;
                            }
                        }
                    }    
                }
               
                if(fcontent != NULL){
                    if(check == 0){
                        root = insert(root,dir->d_name,sorted_array[i+low_bound-1],fcontent,j);
                        first_addr = root;
                        check++;
                    }
                    else{
                        
                        insert(root,dir->d_name,sorted_array[i+low_bound-1],fcontent,j);
                    }
                }
                
                
                fclose(fp);
                
            } 
        }
        closedir(directory);
    }
    return first_addr;
}

void free_array2(char **arr, int n){
  int i = 0;
  for ( i = 0; i < n; i++ ){
        free(arr[i]);
  }
  free(arr);
}

char **get_requests(char *request_file,int *n){
    FILE * fp;
    char **requests;
    char buffer[500];
    int j = 0;
    fp = fopen(request_file, "r");
    if(fp == NULL){
        return NULL;
    }
    requests = malloc(1 * sizeof(char *));
    while (fgets(buffer, 500, fp)){
            requests = realloc(requests,(j+1) * sizeof(char*));
            buffer[strcspn(buffer, "\n")] = 0;
            if(strlen(buffer) > 5){
                requests[j] = malloc((strlen(buffer) + 2) * sizeof(char));
                strncpy(requests[j],buffer,strlen(buffer));
                requests[j][strlen(buffer)]='\0';
                if(strlen(requests[j]) > 4){
                    j++;
                }
            }
    }
    if(requests[0] == NULL){
        free(requests);
        return NULL;
    }
    if(strlen(requests[0]) < 3){
        free_array2(requests,j+1);
        return NULL;
    }
    *n = j;
    fclose(fp);
    return requests;

}