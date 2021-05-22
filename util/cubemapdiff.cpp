#include "image.hpp"

int main(int argc, char* argv[])
{
	Image im1(argv[1]);
	Image im2(argv[2]);
	if (im1.w != 800 || im1.h != 600) throw "fail";
	if (im2.w != 800 || im2.h != 600) throw "fail";
	int xoff[6] = {0,200,200,200,400,600};
	int yoff[6] = {200,0,200,400,200,200};
	double sumdiff = 0, sum2;
	double avg1 = 0, avg2 = 0;
	for (int i=0; i<6; ++i) {
		for (int x0=0; x0<200; ++x0) {
			for (int y0=0; y0<200; ++y0) {
				float val1 = pow(im1.getPixel(x0+xoff[i], y0+yoff[i]).x, 1);
				float val2 = pow(im2.getPixel(x0+xoff[i], y0+yoff[i]).x, 1);
				float dx = (x0-99.5)/100;
				float dy = (y0-99.5)/100;
				float d = sqrt(dx*dx+dy*dy+1);
				float diff = val1 - val2;
				sumdiff += 1.0/100/100 * diff*diff / (d*d*d);
				sum2 += 1.0/100/100 * val2*val2 / (d*d*d);
				avg1 +=1.0/100/100 * val1 / (d*d*d);
				avg2 +=1.0/100/100 * val2 / (d*d*d);
			}
		}
	}
	std::cout << "avg: " << avg1 << " " << avg2 << "\n";
	std::cout << "Relative L2 Error: " << sqrt(sumdiff) / sqrt(sum2) << "\n";
}
// TODO calculate relative error