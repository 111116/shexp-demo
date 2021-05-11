#include "image.hpp"

vec3f Simplelight(float theta, float phi)
{
    /*if (theta <= M_PI / 8.0f)return glm::vec3(4.0f,4.0f, 4.0f);
    return glm::vec3(0.0f, 0.0f, 0.0f);*/

    const float scalar = 8.0f;
    float monochromatic = std::max(0.0f, 5 * cos(theta) - 4) + std::max(
        0.0, -4 * sin(theta - M_PI) * cos(phi - 2.5) - 3);
    return scalar * vec3f(monochromatic, monochromatic, monochromatic);
}

int main()
{
	int yres = 200;
    int xres = 2 * yres;
    Image img(xres, yres);
    for (int y = 0; y < yres; ++y) {
        float theta = (float(y) + 0.5f) / float(yres) * PI;
        for (int x = 0; x < xres; ++x) {
            float phi = (float(x) + 0.5f) / float(xres) * 2.f * PI;
            // Compute RGB color for direction $(\theta,\phi)$ from SH coefficients
            vec3f w = Simplelight(theta, phi);
            float val_r = std::max(0.0f, w.x);
            float val_g = std::max(0.0f, w.y);
            float val_b = std::max(0.0f, w.z);
            img.setPixel(x,y,Color(val_r, val_g, val_b));
        }
    }
	img.saveFile("preview.exr");
}