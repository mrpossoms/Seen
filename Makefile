$(eval OS := $(shell uname))

CXX=g++
CFLAGS=--std=c++11 -g -Wall -fPIC
INC=-I/usr/local/include
SRCS=camera.cpp envmap.cpp geo.cpp texture.cpp shader.cpp renderergl.cpp listscene.cpp core.cpp custompass.cpp
LINK=-lode -lpng
OBJS=$(addprefix obj/,$(SRCS:.cpp=.o))

ifeq ($(OS),Darwin)
	LINK +=-lpthread -lm -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
	LINK += -lopencv_videoio
else
	LINK +=-lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl
	CFLAGS += -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -DGL_GLEXT_PROTOTYPES
endif

all: static shared
	@echo "Built all"

static: lib $(addprefix obj/,$(SRCS:.cpp=.o))
	$(AR) rcs lib/libseen.a $(OBJS)

shared: lib $(OBJS)
	gcc $(CFLAGS) -shared -o ./lib/libseen.so $(OBJS)

obj:
	mkdir obj

lib:
	mkdir lib

obj/%.o: src/%.cpp obj
	$(CXX) $(CFLAGS) $(INC) -c $< -o $@

install-static: static
	cp lib/*.a /usr/local/lib
	mkdir -p /usr/local/include/seen
	cp src/*.h* /usr/local/include/seen

install: static shared
	cp lib/* /usr/local/lib
	mkdir -p /usr/local/include/seen
	cp src/*.h* /usr/local/include/seen

# botshop: $(OBJS)
# 	$(CXX) $(CFLAGS) $(INC) $^ -o $@ $(LINK)

clean:
	rm -rf obj lib
