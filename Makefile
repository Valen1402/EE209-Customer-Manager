CC = gcc
CFLAGS += -Wall

all: array hash

array:	customer_manager1.o
	$(CC) customer_manager1.o -o testclient1

hash:	customer_manager2.o
	$(CC) customer_manager2.o -o testclient2

customer_manager1.o: customer_manager1.h

customer_manager2.o: customer_manager2.h

clean:
	rm -f *.o *~

