all: test_binary test_list

SRCS =  ../code/interp.c test_binary.c
test_binary: $(SRCS) ../code/listlib.h ../code/keyword_dict.h
	$(CC) -o $@ -O2 -I../code $(SRCS)
test_list: ../code/listtest.cpp ../code/listlib.h ../code/keyword_dict.h
	$(CC) -o $@ -O2 -I../code ../code/listtest.cpp
