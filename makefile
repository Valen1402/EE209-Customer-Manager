all1:
	gcc209 -g -D_GNU_SOURCE customer_manager1.c testclient.c -o test1
all2:
	gcc209 -g  -D_GNU_SOURCE customer_manager2.c testclient.c -o test2
