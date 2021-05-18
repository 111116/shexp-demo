#include <iostream>
#include "sh.hpp"
#include "image.hpp"

const int n = 15;

int main(int argc, char* argv[])
{
	Image texture(argv[1]);

	int yres = 200;
    int xres = 2 * yres;
    Image img(xres, yres);
    for (int y = 0; y < yres; ++y) {
        float theta = (float(y) + 0.5f) / float(yres) * PI;
        for (int x = 0; x < xres; ++x) {
            float phi = (float(x) + 0.5f) / float(xres) * 2.f * PI;
            // Compute RGB color for direction $(\theta,\phi)$ from SH coefficients
            vec3f w(-sin(theta) * cos(phi), -sin(theta) * sinf(phi), cos(theta));
            vec3f val = texture.sample(vec2f( 0.5+(0.5/PI)*atan2(w.z, w.x), (1/PI)*acos(w.y)));
            img.setPixel(x,y,Color(val));
        }
    }
	img.saveFile("preview.exr");
}
