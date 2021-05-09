#pragma once

#include "sh.hpp"

// symmetric projection
template <int n>
SymmSH<n>::SymmSH(std::function<float(float)> func, int nsample)
{
    double acc[n];
    memset(acc, 0, sizeof acc);
    float y[n*n];
    for (int i=0; i<nsample; ++i)
    {
        float theta = PI*(i+0.5)/nsample;
        vec3f w(sin(theta), 0, cos(theta));
        SHEvaluate(w, n-1, y);
        float f = func(theta);
        for (int i=0; i<n; ++i)
            acc[i] += f * y[SHIndex(i,0)] * sin(theta);
    }
    for (int i=0; i<n; ++i)
        a[i] = acc[i] * 2*PI*PI / nsample;
}


// SH projection
template <int n>
SH<n>::SH(sphfunc func, int nsample)
{
    RTcore::MT19937Sampler sampler;
    double acc[n*n];
    memset(acc, 0, sizeof acc);
    float y[n*n];
    for (int _=0; _<nsample; ++_)
    {
        vec3f w = sampler.sampleUnitSphereSurface();
        SHEvaluate(w, lmax, y);
        float f = func(w);
        for (int i=0; i<n*n; ++i)
            acc[i] += f * y[i];
    }
    for (int i=0; i<n*n; ++i)
        a[i] = acc[i] * 4*PI / nsample;
}


// reconstruction
template <int n>
float SH<n>::eval(vec3f w) const
{
    float y[n*n];
    SHEvaluate(w, lmax, y);
    float s = 0.0f;
    for (int i=0; i<n*n; ++i)
        s += a[i] * y[i];
    return s;
}

// phi-theta visualization
template <int n>
Image SH<n>::visualized(int yres) {
    int xres = 2 * yres;
    Image img(xres, yres);
    for (int y = 0; y < yres; ++y) {
        float theta = (float(y) + 0.5f) / float(yres) * PI;
        for (int x = 0; x < xres; ++x) {
            float phi = (float(x) + 0.5f) / float(xres) * 2.f * PI;
            // Compute RGB color for direction $(\theta,\phi)$ from SH coefficients
            vec3f w(sin(theta) * cos(phi), sin(theta) * sinf(phi), cos(theta));
            float val = eval(w);
            if (val > 0)
                img.setPixel(x,y,Color(val,0,0));
            else
                img.setPixel(x,y,Color(0,0,-val));
        }
    }
    return img;
}