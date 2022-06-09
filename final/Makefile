output: 
	gcc  -o servant servant.c helper.c utility.c bst_for_files.c -Wall -pthread -lrt 
	gcc  -o client client.c helper.c utility.c bst_for_files.c networking.c -Wall -pthread -lrt 
	gcc  -o server server.c helper.c utility.c bst_for_files.c networking.c -Wall -pthread -lrt 

clean: 
	rm  servant client server
