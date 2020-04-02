CC = clang

# Flags needed for code coverage
# CFlags = -fprofile-instr-generate -fcoverage-mapping -g -pthread -Wall 

CFlags = -g -pthread -Wall 
LDFLAGS = -lcheck 

CHECK_TEST = check_test

LIB_OBJS = llist.o utils.o
CHECK_OBJS = check_llist.o $(LIB_OBJS)
ALL_OBJS = check_llist.o $(LIB_OBJS)

SRC_DIR_PATH = ./src
TEST_DIR_PATH = ./tests

default: check

#-----------------#
# Objects         #
#-----------------#
llist.o: $(SRC_DIR_PATH)/llist.c
	$(CC) $(CFLAGS) -c $(SRC_DIR_PATH)/llist.c

utils.o: $(SRC_DIR_PATH)/utils.c
	$(CC) $(CFLAGS) -c $(SRC_DIR_PATH)/utils.c

check_llist.o: $(TEST_DIR_PATH)/check_llist.c
	$(CC) $(CFLAGS) -c $(TEST_DIR_PATH)/check_llist.c

#-----------------#
# Unit Test Build #
#-----------------#
check: $(CHECK_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CHECK_OBJS) -o $(CHECK_TEST)

ckcov: $(CHECK_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CHECK_OBJS) -o $(CHECK_TEST)

#-----------------#
# Memory Tests    #
#-----------------#
memtest: check
	valgrind --leak-check=full $(CHECK_TEST)

# .PHONY is a built-in target name used to declare phony targets.
# A phony target is one whose recipe does not generate a target file.
# - https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: clean

# The hyphen is used to ignore errors in commands
# - https://stackoverflow.com/questions/2670130/make-how-to-continue-after-a-command-fails
clean:
	-rm $(CHECK_TEST) $(ALL_OBJS)
