$(eval OS := $(shell uname))

CXX=g++
CFLAGS=--std=c++11 -g
INC=-I/usr/local/include -I../src
LINK=../lib/seen.a -lode -lpng
OBJS=$(addprefix obj/,$(SRCS:.cpp=.o))
DEMOS=demo0 demo1:

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
	mkdir obj || True

demo0: obj
	$(CXX) $(CFLAGS) $(INC) src/$@.cpp -o $@ $(LINK)

demo1: obj
	$(CXX) $(CFLAGS) $(INC) src/sky.cpp src/$@.cpp -o $@ $(LINK)

clean:
	rm -rf ./obj $(DEMOS)
