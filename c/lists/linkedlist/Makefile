CC = gcc  
CFlags = -ggdb -pthread -Wall 
EXEC = exec

target:
	$(CC) $(CFLAGS) main.c ./src/* -o $(EXEC)
memtest: target
	valgrind --leak-check=full $(EXEC)
clean:
	rm -rf $(EXEC)*
