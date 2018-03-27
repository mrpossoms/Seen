$(eval OS := $(shell uname))

CXX=g++
CFLAGS=--std=c++11 -g
INC=-I/usr/local/include -I../src
LINK=../lib/seen.a -lode -lpng
OBJS=$(addprefix obj/,$(SRCS:.cpp=.o))
DEMOS=demo0 demo1 demo2

ifeq ($(OS),Darwin)
	LINK +=-lpthread -lm -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
	LINK += -lopencv_videoio
else
	LINK +=-lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl
	CFLAGS += -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -DGL_GLEXT_PROTOTYPES
endif

all: $(DEMOS)
	@echo "Built all"

../lib/seen.a:
	make -C .. static

obj: ../lib/seen.a
	mkdir obj || true 

obj/%.o: src/%.cpp obj
	$(CXX) $(CFLAGS) $(INC) -c $< -o $@

demo0: obj
	$(CXX) $(CFLAGS) $(INC) src/$@.cpp -o $@ $(LINK)

demo1: obj/sky.o obj/demo1.o
	$(CXX) $(CFLAGS) $(INC) $^ -o $@ $(LINK)

demo2: obj/sky.o obj/demo2.o
	$(CXX) $(CFLAGS) $(INC) $^ -o $@ $(LINK)

clean:
	rm -rf ./obj $(DEMOS) *.dSYM
