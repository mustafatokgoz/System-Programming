#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bst_for_files.h"

node* newNode(char *date,char *city,char **content,int number){
    struct node* temp = (struct node*)malloc(sizeof(struct node));
    strcpy(temp->date,date);
    strcpy(temp->city,city);
    temp->file_content = content;
    temp->transaction_number = number;
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

node* insert(node* node, char *date,char *city,char **content,int number)
{
    
    if (node == NULL)
        return newNode(date,city,content,number);
 
    if (strcmp(date,node->date) < 0)
        node->left = insert(node->left, date,city,content,number);
    else if (strcmp(date,node->date) > 0)
        node->right = insert(node->right,date,city,content,number);
 
    return node;
}

int* search(node *root, char *date , char *date2,int *count){
    
    if (root != NULL) {
        search(root->left,date,date2,count);
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

        if(y1 > y2  && y1 < y3){
            *count = *count + 1;
        }
        else if(y1 == y2 && m1 > m2 ){
            *count = *count + 1;
        }   
        else if(y1 == y3 && m1 < m3 ){
            *count = *count + 1;
        }   
        else if(y1 == y2 && m1 == m2 && d1 > d2){
            *count = *count + 1;
        }   
        else if(y1 == y3 && m1 == m3 && d1 < d3){
            *count = *count + 1;
        }   
        
    
        search(root->right,date,date2,count);
    }
    return 0;
}

void free_tree(node *root){
    int i = 0;
    if (root != NULL) {
        free_tree(root->right);
        for (i = 0; i < root->transaction_number; i++ ){
            free(root->file_content[i]);
        }
        free(root->file_content);
        free_tree(root->left);
        free(root);
    }
}