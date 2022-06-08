#ifndef BST_FOR_FILES_H
#define BST_FOR_FILES_H


typedef struct node {
    char date[20];
    char city[30];
    int transaction_number;
    char **file_content;
    struct node *left, *right;
}node;
node* newNode(char *date,char *city,char **content,int number);
void inorder(node* root);
node* insert(node* node, char *date,char *city,char **content,int number);
void free_tree(node *root);
int* search(node *root, char *date , char *date2,int *count);

#endif