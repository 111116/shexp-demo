CXXFLAGS = -std=c++11 -O3

all: env_sh test makelut

env_sh: env_sh.cpp *.hpp
	$(CXX) $(CXXFLAGS) $< sh.cpp lib/*.cpp -o $@

test: test.cpp *.hpp
	$(CXX) $(CXXFLAGS) $< sh.cpp lib/*.cpp -o $@

makelut: makelut.cpp *.hpp
	$(CXX) $(CXXFLAGS) $< sh.cpp lib/*.cpp -o $@

sh2img: sh2img.cpp *.hpp
	$(CXX) $(CXXFLAGS) $< sh.cpp lib/*.cpp -o $@

ad: ad.cpp *.hpp
	$(CXX) $(CXXFLAGS) $< sh.cpp lib/*.cpp -o $@
