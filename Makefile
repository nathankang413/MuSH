CC = gcc
CFLAGS = -Wall -Werror -pedantic -Og
LD = gcc
PROGRAM = mush2
HANDIN_CLASS = pn-cs357
HANDIN_LOC = asgn6
DEMO = ~pn-cs357/demos/$(PROGRAM)
# DEBUG = -D DEBUG
TEST_FILE = test-cmds

all: $(PROGRAM)

# compile program
OBJS = $(PROGRAM).o
INC = -I ~pn-cs357/Given/Mush/include
LIBS = -L ~pn-cs357/Given/Mush/lib64 -lmush
$(PROGRAM): $(OBJS)
	$(LD) $(CFLAGS) $(DEBUG) -o $(PROGRAM) $(OBJS) $(LIBS)

$(PROGRAM).o: $(PROGRAM).c
	$(CC) $(CFLAGS) $(DEBUG) -c $(INC) -o $(PROGRAM).o $(PROGRAM).c

submit: 
	~pn-cs357/bin/longlines.pl *.c *.h
	handin $(HANDIN_CLASS) $(HANDIN_LOC) *.c
	handin $(HANDIN_CLASS) $(HANDIN_LOC) *.h
	handin $(HANDIN_CLASS) $(HANDIN_LOC) Makefile
	handin $(HANDIN_CLASS) $(HANDIN_LOC) README

clean:
	rm -f *.o
	rm -f test_out
	rm -f test_ref

mem: $(PROGRAM) mem_stdin mem_arg

mem_stdin: $(PROGRAM)
	valgrind --leak-check=full --track-origins=yes ./$(PROGRAM) < $(TEST_FILE) > test_out

mem_arg: $(PROGRAM)
	valgrind --leak-check=full --track-origins=yes ./$(PROGRAM) $(TEST_FILE) > test_out


test: $(PROGRAM) test_stdin test_arg

test_stdin: $(PROGRAM)
	- $(DEMO) < $(TEST_FILE) > test_ref 2>&1
	- ./$(PROGRAM) < $(TEST_FILE) > test_out 2>&1
	diff test_out test_ref

test_arg: $(PROGRAM)
	- $(DEMO) $(TEST_FILE) > test_ref	2>&1
	- ./$(PROGRAM) $(TEST_FILE) > test_out 2>&1
	diff test_out test_ref

test_parse: $(PROGRAM)	# both stdin and arg
	$(DEMO) -vp $(TEST_FILE) > test_ref 2>&1
	./$(PROGRAM) -vp $(TEST_FILE) > test_out 2>&1
	diff test_out test_ref

	$(DEMO) -vp < $(TEST_FILE) > test_ref 2>&1
	./$(PROGRAM) -vp < $(TEST_FILE) > test_out 2>&1
	diff test_out test_ref