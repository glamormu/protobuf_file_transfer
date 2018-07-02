CC = gcc
CFLAGS = -lssl -lcrypto -lpthread
TARGET = zuolin_file_server
OBJS = zuolin_file_server.o
TARGET : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
zuolin_file_server.o:zuolin_file_server.c

.PHONY : clean
clean:
	rm -rf *.o
	rm -f $(TARGET)