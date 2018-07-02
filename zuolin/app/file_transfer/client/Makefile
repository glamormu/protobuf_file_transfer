CC = gcc
CFLAGS = -lssl -lcrypto -lpthread
TARGET = zuolin_file_client
OBJS = zuolin_file_client.o
TARGET : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
zuolin_file_server.o:zuolin_file_client.c

.PHONY : clean
clean:
	rm -rf *.o
	rm -f $(TARGET)