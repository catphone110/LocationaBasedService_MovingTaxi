INCLUDE = -I../../ \

CPPFLAGS = -Wall -fPIC $(INCLUDE) $(DEBUG_DEF) -std=c++11 -O0 -g
LIBS = libgrid.a

all : clean $(LIBS) hashtable_test

clean:
	rm -rf *.o *.a

hashtable_test: lbs_hashtable_test.o
		g++ $(CPPFLAGS) -o $@ $^ $(LIBS)

libgrid.a: lbs_bitmap.o lbs_index.o lbs_hashtable.o lbs_grid.o lbs_nn_heap.o
	ar cr $@ $^

.o : .cpp
	g++ $(CPPFLAGS) -c $^
