#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bst_for_files.h"

node* newNode(char *date,char *city,char **content,int n){
    struct node* temp = (struct node*)malloc(sizeof(struct node));
    strcpy(temp->date,date);
    strcpy(temp->city,city);
    temp->file_content = content;
    temp->n = n;
    temp->left = temp->right = NULL;
    return temp;
}
 
void inorder(node* root){
    if (root != NULL) {
        inorder(root->left);
        printf("%s %s \n", root->date,root->city);
        printf("%s  \n", root->file_content[2]);
        inorder(root->right);
    }
}

node* insert(node* node, char *date,char *city,char **content,int n)
{
    
    if (node == NULL)
        return newNode(date,city,content,n);
 
    if (strcmp(date,node->date) < 0)
        node->left = insert(node->left, date,city,content,n);
    else if (strcmp(date,node->date) > 0)
        node->right = insert(node->right,date,city,content,n);
 
    return node;
}

int check_types(char **content,char *type,int n){
    int i = 0;
    int count = 0;
    if(content !=NULL){
        for (i = 0; i< n; i++){
            if(content[i]!=NULL){
                char temp[strlen(content[i])];
                strcpy(temp,content[i]);
                char *token = strtok(temp," ");
                token = strtok(NULL," ");
                if(strcmp(token,type) == 0){
                    count++;
                }
            }
        }
    }
    return count;
}


int* search(node *root, char *date , char *date2,char *type,char *city,int city_enable,int *count){
    
    if (root != NULL) {
        search(root->left,date,date2,type,city,city_enable,count);
        char temp[20],temp2[20],temp3[20];
        int d1,m1,y1,d2,m2,y2,d3,m3,y3;
    

        strcpy(temp,root->date);
        char *token = strtok(temp,"-");
        d1 = atoi(token);
        token = strtok(NULL,"-");
        m1 = atoi(token);
        token = strtok(NULL,"-");
        y1 = atoi(token);

        strcpy(temp2,date);
        char *token2 = strtok(temp2,"-");
        d2 = atoi(token2);
        token2 = strtok(NULL,"-");
        m2 = atoi(token2);
        token2 = strtok(NULL,"-");
        y2 = atoi(token2);

        strcpy(temp3,date2);
        char *token3 = strtok(temp3,"-");
        d3 = atoi(token3);
        token3 = strtok(NULL,"-");
        m3 = atoi(token3);
        token3 = strtok(NULL,"-");
        y3 = atoi(token3);

        if(city_enable == 1){
            if(strcmp(root->city,city) == 0){
                if(y1 > y2  && y1 < y3){
                    *count = *count + check_types(root->file_content,type,root->n);
                }
                else if(y1 == y2 && m1 > m2 ){
                    *count = *count + check_types(root->file_content,type,root->n);
                }   
                else if(y1 == y3 && m1 < m3 ){
                    *count = *count + check_types(root->file_content,type,root->n);
                }   
                else if(y1 == y2 && m1 == m2 && d1 >= d2){
                    *count = *count + check_types(root->file_content,type,root->n);
                }   
                else if(y1 == y3 && m1 == m3 && d1 <= d3){
                    *count = *count + check_types(root->file_content,type,root->n);
                }   
            }
        }
        else{
                if(y1 > y2  && y1 < y3){
                    *count = *count + check_types(root->file_content,type,root->n);
                }
                else if(y1 == y2 && m1 > m2 ){
                    *count = *count + check_types(root->file_content,type,root->n);
                }   
                else if(y1 == y3 && m1 < m3 ){
                    *count = *count + check_types(root->file_content,type,root->n);
                }   
                else if(y1 == y2 && m1 == m2 && d1 >= d2){
                    *count = *count + check_types(root->file_content,type,root->n);
                }   
                else if(y1 == y3 && m1 == m3 && d1 <= d3){
                    *count = *count + check_types(root->file_content,type,root->n);
                }  
        }
    
    
        search(root->right,date,date2,type,city,city_enable,count);
    }
    return 0;
}

void free_tree(node *root){
    int i = 0;
    if (root != NULL) {
        free_tree(root->right);
        free_tree(root->left);
        
        if(root->file_content!=NULL){
            for(i = 0; i< root->n;i++){
                if(root->file_content[i]!=NULL){
                    free(root->file_content[i]);
                }
            }
            free(root->file_content);
        }
        free(root);
    }
}