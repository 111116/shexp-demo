#include <iostream>
#include <fstream>
#include <cmath>

double sqr(double a){
	return a*a;
}
int main(int argc, char* argv[])
{
	std::ifstream f1(argv[1]);
	std::ifstream f2(argv[2]);
	double t1,t2, sd=0, s2=0;
	while (f1 >> t1) {
		f2 >> t2;
		sd += sqr(t1-t2);
		s2 += sqr(t2);
	}
	std::cout << "Relative L2 Error: " << sqrt(sd) / sqrt(s2) << "\n";
}
// TODO calculate relative error