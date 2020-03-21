target:
	gcc -o exec -ggdb -pthread -Wall main.c ./src/bintree.c ./src/utils.c
memtest: target
	valgrind --leak-check=full ./exec
clean:
	rm -rf exec*
