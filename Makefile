$(eval OS := $(shell uname))

CXX=g++
CFLAGS=--std=c++11 -g -Wall -fPIC -O0
INC=-I/usr/local/include -I./src
SRCS=camera.cpp cubemap.cpp geo.cpp texture.cpp shader.cpp shader_factory.cpp shader_factory_expression.cpp renderergl.cpp listscene.cpp core.cpp custompass.cpp
LINK=-lode -lpng
OBJS=$(addprefix obj/,$(SRCS:.cpp=.o))

TST_SRC=shader_def

ifeq ($(OS),Darwin)
	LINK +=-lpthread -lm -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
	LINK += -lopencv_videoio
else
	LINK +=-lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl
	CFLAGS += -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -DGL_GLEXT_PROTOTYPES
endif

all: static shared
	@echo "Built all"

lib/libseen.a: lib $(addprefix obj/,$(SRCS:.cpp=.o))
	$(AR) rcs lib/libseen.a $(OBJS)

static: lib/libseen.a
	@echo "Built static"

shared: lib $(OBJS)
	gcc $(CFLAGS) -shared -o ./lib/libseen.so $(OBJS)

obj:
	mkdir obj

lib:
	mkdir lib

obj/%.o: src/%.cpp obj
	$(CXX) $(CFLAGS) $(INC) -c $< -o $@

bin/tests:
	mkdir -p bin/tests

tests: bin/tests lib/libseen.a
	@echo "Building tests..."
	@for source in $(TST_SRC); do\
		($(CXX) $(INC) $(CFLAGS) src/tests/$$source.cpp  -o bin/tests/$${source%.*}.bin ./lib/libseen.a $(LINK)) || (exit 1);\
	done

test: tests
	@./test_runner.py

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
