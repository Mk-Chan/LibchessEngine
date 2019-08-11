CXX = g++
CXXFLAGS = -std=c++17 -Wall -pipe -fopenmp $(EXTRACXXFLAGS)
LDFLAGS = -pthread -Wl,--no-as-needed $(CXXFLAGS) $(EXTRALDFLAGS)

OBJS = main.o search.o evaluation.o

BINDIR = /usr/local/bin

EXE = engine

ifeq ($(BUILD),debug)
	CXXFLAGS += -O0 -g -fno-omit-frame-pointer
else
	CXXFLAGS += -O3 -DNDEBUG
endif

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

install:
	-cp $(EXE) $(BINDIR)
	-strip $(BINDIR)/$(EXE)

uninstall:
	-rm -f $(BINDIR)/$(EXE)

clean:
	-rm -f $(OBJS) $(EXE)
