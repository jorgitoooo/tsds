CC = gcc
CFlags = -g -pthread -Wall 
DL_FLAGS = -lcheck

EXEC = exec
CHECK_TEST = check_test

LIB_OBJS = llist.o utils.o
OBJS = main.o $(LIB_OBJS)
CHECK_OBJS = check_llist.o $(LIB_OBJS)
ALL_OBJS = main.o check_llist.o $(LIB_OBJS)

SRC_DIR_PATH = ./src
TEST_DIR_PATH = ./tests

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

llist.o: $(SRC_DIR_PATH)/llist.c
	$(CC) $(CFLAGS) -c $(SRC_DIR_PATH)/llist.c

utils.o: $(SRC_DIR_PATH)/utils.c
	$(CC) $(CFLAGS) -c $(SRC_DIR_PATH)/utils.c

#-----------------#
# Unit Test Build #
#-----------------#
check: $(CHECK_OBJS)
	$(CC) $(CFLAGS) $(DL_FLAGS) -o $(CHECK_TEST) $(CHECK_OBJS) 

check_llist.o: $(TEST_DIR_PATH)/check_llist.c
	$(CC) $(CFLAGS) -c $(TEST_DIR_PATH)/check_llist.c

#-----------------#
# Memory Tests    #
#-----------------#
memtest: $(EXEC)
	valgrind --leak-check=full $(EXEC)

# .PHONY is a built-in target name used to declare phony targets.
# A phony target is one whose recipe does not generate a target file.
# - https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: clean

# The hyphen is used to ignore errors in commands
# - https://stackoverflow.com/questions/2670130/make-how-to-continue-after-a-command-fails
clean:
	-rm $(EXEC) $(CHECK_TEST) $(ALL_OBJS)
