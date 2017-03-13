all: test_binary

SRCS =  ../code/interp.c test_binary.c
test_binary: $(SRCS) ../code/string_dict.h
	$(CC) -o $@ -O2 -I../code $(SRCS)
