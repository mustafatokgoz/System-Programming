#ifndef UTILITY_H
#define UTILITY_H
#include "bst_for_files.h"

int seperate_c_paramater(int *lower_bound,int *upper_bound,char *c_parameter);
node* read_from_disk(char *director_path,int low_bound,int up_bound,node* root);
int open_file(char *file_name);
void sort_cities(char arr[][70], int n);
void free_array2(char **arr, int n);
char **get_requests(char *request_file,int *n);

#endif