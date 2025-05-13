CC = gcc
CFLAGS = -pedantic -Wall

twoShell: twoShell.o linked_list.o dstring.o helper.o
	$(CC) $(CFLAGS) -o twoShell twoShell.o linked_list.o dstring.o helper.o
twoShell.o: twoShell.c linked_list.h dstring.h helper.h
	$(CC) $(CFLAGS) -c twoShell.c
linked_list.o: linked_list.c linked_list.h
	$(CC) $(CFLAGS) -c linked_list.c
dstring.o: dstring.c dstring.h
	$(CC) $(CFLAGS) -c dstring.c
helper.o: helper.c helper.h
	$(CC) $(CFLAGS) -c helper.c
