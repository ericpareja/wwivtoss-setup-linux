wsetup: wsetup.cpp fidoadr.o
	astyle wsetup.cpp
	g++ -o wsetup fidoadr.o wsetup.cpp -ggdb -D_DEBUG -std=c++11 -lmenu -lncurses -fpermissive

share.o: share.cpp
	g++ -c -o share.o share.cpp -std=c++11

export.o: export.cpp
	g++ -c -o export.o export.cpp -std=c++11

dawg.o: dawg.cpp
	g++ -c -o dawg.o dawg.cpp -std=c++11

fidoadr.o: fidoadr.cpp
	g++ -c -o fidoadr.o fidoadr.cpp -std=c++11

wwivtoss: wwivtoss.cpp share.o export.o dawg.o
	g++ -o wwivtoss wwivtoss.cpp share.o export.o dawg.o -ggdb -D_DEBUG -std=c++11 -fpermissive


clean:
	rm wsetup

#-fpermissive
