# Dependency rules for non-file targets
all: add1 add2 chat 
clobber: clean
	rm -f *~ \#*\#
clean:
	rm -f add1 add2 chat  *.o
# Dependency rules for file targets
add1: add1.o
	gcc217 add1.o -o add1
add1.o: add1.c
	gcc217 -c add1.c
add2: add2.o
	gcc217 add2.o -o add2
add2.o: add2.c
	gcc217 -c add2.c
chat: chat.o
	gcc217 chat.o -o chat
chat.o: chat.c
	gcc217 -c chat.c


