all: test_binary

SRCS =  ../code/interp.c test_binary.c
test_binary: $(SRCS) ../code/keyword_dict.h
	$(CC) -o $@ -O2 -I../code $(SRCS)
