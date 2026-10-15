CXX := g++
CXXFLAGS := -std=c++17 -Wall

MUJOCO := /home/nata-negara/mujoco
EIGEN := /home/nata-negara/mujoco/build/_deps/eigen3-src
GLFW := $(shell pkg-config --cflags --libs glfw3)

INCLUDES := -I$(MUJOCO)/include -I$(EIGEN)
LIBS := -L$(MUJOCO) -lmujoco -Wl,-rpath,$(MUJOCO) $(GLFW)

sim: callback.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) callback.cpp -o sim $(LIBS)

clean:
	rm -f sim
