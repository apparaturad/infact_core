# CFLAGS = -O3
# CFLAGS = -O3 -fcx-limited-range -ffast-math -ffinite-math-only -fgcse-las -fgcse-sm -fno-trapping-math -fpeel-loops -fprofile-generate -fprofile-use -fprofile-values -fsingle-precision-constant -ftree-loop-im -funsafe-loop-optimizations -funsafe-math-optimizations
CFLAGS = -g -Wall

BINS = e
LIBS = -L. -lgh_core.27
CC = g++
ARCH = libgh_core.27.a
HED = gh_core.27.h

######################################################
$(BINS): main.o $(ARCH)
	$(CC) $(CFLAGS) -o $(BINS) main.o $(LIBS)

$(ARCH): $(patsubst   %.cpp,%.o,$(wildcard   *.cpp))
	ar cr $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -MMD $<

include $(wildcard   *.d)

clean:
	rm -f ./*.o
	rm -f ./*.d
	rm -f ./*.log
	rm -f ./*.gcda
	rm -f $(ARCH)
	rm -f $(BINS)

install:
	sudo cp $(ARCH) /usr/lib/
	sudo cp ./gh_core.h /usr/include/gh/$(HED)
