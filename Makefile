SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin
MKDIR = mkdir -p

CPP = g++
CPPFLAGS = -Wall -fPIC -pthread -std=c++1y -I./include -I${CONDA_PREFIX}/include
LDLIBS = -L/usr/lib64 -L${CONDA_PREFIX}/lib -lzmq -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lboost_system -lboost_regex -lboost_thread -lpthread
LDFLAGS = -shared -Wl,-soname,libcpp_h5_writer.so

HEADERS = $(wildcard $(SRC_DIR)/*.hpp)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: build_dirs libcpp_h5_writer

debug: CPPFLAGS += -DDEBUG_OUTPUT -g
debug: all

libcpp_h5_writer: $(OBJS)
	$(CPP) $(LDFLAGS) -o $(BIN_DIR)/libcpp_h5_writer.so $(OBJS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CPP) $(CPPFLAGS) $(LDLIBS) -c -o $@ $<

build_dirs:
	$(MKDIR) $(OBJ_DIR) $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)