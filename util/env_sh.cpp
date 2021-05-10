#include <iostream>
#include "sh.hpp"
#include "image.hpp"

sphfunc envmap_r(std::string filename)
{
	Image texture(filename);
	return [texture](vec3f w)
	{
		// return texture.sample(vec2f(
		// 	0.5 - 0.5/PI * atan2(w.x, w.z),
		// 	0.5 - 1/PI * atan2(w.y, sqrt(w.x * w.x + w.z * w.z)))).x;
		return texture.sample(vec2f( 0.5+(0.5/PI)*atan2(w.y, w.x), (1/PI)*acos(w.z))).x;
	};
}

sphfunc envmap_g(std::string filename)
{
	Image texture(filename);
	return [texture](vec3f w)
	{
		// return texture.sample(vec2f(
		// 	0.5 - 0.5/PI * atan2(w.x, w.z),
		// 	0.5 - 1/PI * atan2(w.y, sqrt(w.x * w.x + w.z * w.z)))).y;
		return texture.sample(vec2f( 0.5+(0.5/PI)*atan2(w.y, w.x), (1/PI)*acos(w.z))).y;
	};
}

sphfunc envmap_b(std::string filename)
{
	Image texture(filename);
	return [texture](vec3f w)
	{
		// return texture.sample(vec2f(
		// 	0.5 - 0.5/PI * atan2(w.x, w.z),
		// 	0.5 - 1/PI * atan2(w.y, sqrt(w.x * w.x + w.z * w.z)))).z;
		return texture.sample(vec2f( 0.5+(0.5/PI)*atan2(w.y, w.x), (1/PI)*acos(w.z))).z;
	};
}

const int n = 15;

int main(int argc, char* argv[])
{
	SH<n> env_r(envmap_r(argv[1]));
	SH<n> env_g(envmap_g(argv[1]));
	SH<n> env_b(envmap_b(argv[1]));
	
	env_r = env_r.windowed();
	env_g = env_g.windowed();
	env_b = env_b.windowed();

	for (int i=0; i<n*n; ++i) {
		std::cout
			<< env_r.a[i] << " "
			<< env_g.a[i] << " "
			<< env_b.a[i] << "\n";
	}
	// std::cout << "\n";
	// for (int i=0; i<n*n; ++i)
	// 	std::cout << env_g.a[i] << " ";
	// std::cout << "\n";
	// for (int i=0; i<n*n; ++i)
	// 	std::cout << env_b.a[i] << " ";
	// std::cout << "\n";

	int yres = 200;
    int xres = 2 * yres;
    Image img(xres, yres);
    for (int y = 0; y < yres; ++y) {
        float theta = (float(y) + 0.5f) / float(yres) * PI;
        for (int x = 0; x < xres; ++x) {
            float phi = (float(x) + 0.5f) / float(xres) * 2.f * PI;
            // Compute RGB color for direction $(\theta,\phi)$ from SH coefficients
            vec3f w(-sin(theta) * cos(phi), -sin(theta) * sinf(phi), cos(theta));
            float val_r = std::max(0.0f, env_r.eval(w));
            float val_g = std::max(0.0f, env_g.eval(w));
            float val_b = std::max(0.0f, env_b.eval(w));
            img.setPixel(x,y,Color(val_r, val_g, val_b));
        }
    }
	img.saveFile("preview.exr");
}
