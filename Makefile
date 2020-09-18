# Dependency rules for non-file targets
all: add1 add2 chat displaygame referee
clobber: clean
	rm -f *~ \#*\#
clean:
	rm -f add1 add2 chat displaygame referee *.o
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
displaygame: point.o board.o game.o displaygame.o
	gcc217 point.o board.o game.o displaygame.o -o displaygame
point.o: point.c point.h
	gcc217 -c point.c
board.o: board.c board.h
	gcc217 -c board.c
game.o: game.c game.h
	gcc217 -c game.c
displaygame.o: displaygame.c game.h board.h point.h
	gcc217 -c displaygame.c
referee: point.o board.o game.o referee.o
	gcc217 point.o board.o game.o referee.o -o referee
referee.o: referee.c game.h board.h point.h
	gcc217 -c referee.c
