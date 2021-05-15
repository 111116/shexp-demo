#include <iostream>
#include "sh.hpp"
#include "image.hpp"

const int n = 10;

int main(int argc, char* argv[])
{
	SH<n> a;
	for (int i=0; i<n*n; ++i)
		std::cin >> a.a[i];

	int yres = 400;
    int xres = 2 * yres;
    Image img(xres, yres);
    for (int y = 0; y < yres; ++y) {
        float theta = (float(y) + 0.5f) / float(yres) * PI;
        for (int x = 0; x < xres; ++x) {
            float phi = (float(x) + 0.5f) / float(xres) * 2.f * PI;
            // Compute RGB color for direction $(\theta,\phi)$ from SH coefficients
            vec3f w(sin(theta) * cos(phi), sin(theta) * sinf(phi), cos(theta));
            float val = a.eval(w);
            val = std::max(0.0f, std::min(1.0f, val));
            img.setPixel(x,y,Color(val,val,val));
        }
    }

	img.saveFile("preview.exr");
}
