CXX=g++
CXXFLAGS=-std=c++1z -O2 -g -Wall -fmessage-length=0 
LIBFLAGS=-ljsoncpp
# $@  目标的完整名称
# $^  所有的依赖文件，以空格分开，不包含重复的依赖文件
runawayGem: main.cc runawayGem.cc jsonUtil.cc move.cc
	$(CXX) $(CXXFLAGS) -I. -o build/$@ $^ $(LIBFLAGS) 

all: runawayGem

clean:
	rm -rf build