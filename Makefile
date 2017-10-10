CXX ?= g++
INCLUDEDIR := -I./include
CXXFLAGS := -O3 -march=native -std=c++11 -g -Wall -Wextra ${INCLUDEDIR}
LDFLAGS := -L ./lib -lmxnet_predict -lblas -lpthread -lopencv_core -lopencv_imgproc -lopencv_highgui

SRC := main.cpp
OBJ := $(SRC:.cpp=.o)

predict: ${OBJ}
	@echo LD $@
	@${CXX} $< -o $@ ${LDFLAGS}

%.o: %.cpp
	@echo CXX $<
	@${CXX} -MMD ${CXXFLAGS} -o $@ -c $<

-include src/*.d

clean:
	rm -f ${OBJ} predict *.d

.PHONY: clean fmt
.SECONDARY:

