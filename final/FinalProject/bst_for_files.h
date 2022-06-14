#ifndef BST_FOR_FILES_H
#define BST_FOR_FILES_H


typedef struct node {
    char date[20];
    char city[30];
    char **file_content;
    int n;
    struct node *left, *right;
}node;
node* newNode(char *date,char *city,char **content,int n);
void inorder(node* root);
node* insert(node* node, char *date,char *city,char **content,int n);
void free_tree(node *root);
int* search(node *root, char *date , char *date2,char *type,char *city,int city_enable,int *count);
int check_types(char **content,char *type,int n);

#endif