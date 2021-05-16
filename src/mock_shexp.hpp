#pragma once

#include <vector>
#include "shorder.hpp"
#include "loadlut.hpp"

namespace {
	float max_magn;
	std::vector<SymmSH<shorder>> log_lut(n_entry);
	std::vector<float> a_lut(n_entry);
	std::vector<float> b_lut(n_entry);

	template <typename T>
	T lerp(T a, T b, float t){
		return (1-t)*a + t*b;
	}

	int loadlut_cpu()
	{
		const int n = shorder;
		console.log("loading log & OL lut (CPU)...");
		float a[n_entry], b[n_entry], magn[n_entry];
		// read lut from file
		std::string filename = "../lut/lut" + std::to_string(n);
		std::ifstream fin(filename);
		if (!fin) throw "cannot open lut";
		for (int i=0; i<n_entry; ++i)
		{
			// calculate magnitude of f_hat (f without DC component)
			float s = 0;
			for (int j=0; j<n; ++j) {
				fin >> log_lut[i].a[j];
				if (j>0)
					s += log_lut[i].a[j] * log_lut[i].a[j];
			}
			magn[i] = sqrt(s);
			fin >> a[i] >> b[i];
		}
		// magnitude is increasing then decreasing
		max_magn = 0;
		for (int i=0; i<n_entry; ++i)
			max_magn = std::max(max_magn, magn[i]);
		// convert AB_lut from angle-indexed to magnitude-indexed
		int current_id = 0;
		for (int i=0; i<n_entry; ++i)
		{
			float ratio = (float)i / (n_entry-1);
			float target_magn = lerp(magn[0], max_magn, ratio);
			while (current_id < n_entry-1 and target_magn > magn[current_id+1])
				current_id += 1;
			float r = (target_magn - magn[current_id]) / (magn[current_id+1] - magn[current_id]);
			a_lut[i] = lerp(a[current_id], a[current_id+1], r);
			b_lut[i] = lerp(b[current_id], b[current_id+1], r);
		}
		return 0;
	}

	template <typename T>
	T lut_lookup(std::vector<T> v, float position)
	{
		float p = (v.size()-1) * position;
		if (p<=0) return v[0];
		if (p>=v.size()-1) return v.back();
		int i = (int)p;
		return lerp(v[i], v[i+1], p-i);
	}

	SymmSH<shorder> log_lookup(float angle)
	{
		return lut_lookup(log_lut, angle/(PI/2));
	}

	SH<shorder> shexp_OL(SH<shorder> f)
	{
	    // calculate magnitude of f_hat
	    float magn = 0;
	    for (int i=1; i<N_COEFFS; ++i)
	        magn += f.a[i] * f.a[i];
	    magn = sqrt(magn);
	    // lookup optimal coefficients
	    float a = lut_lookup(a_lut, magn / max_magn);
	    float b = lut_lookup(b_lut, magn / max_magn);
	    SH<shorder> g;
	    // apply DC isolation
	    float e = exp(f.a[0]/sqrt(4.0/PI));
	    g.a[0] = a * sqrt(4.0*PI)*e;
	    for (int i = 1; i < N_COEFFS; i++)
	        g.a[i] = b * f.a[i]*e;
	    return g;
	}

	SH<shorder> shexp_HYB(SH<shorder> f)
	{
	    // calculate magnitude of f_hat
	    float magn = 0;
	    for (int i=1; i<N_COEFFS; ++i)
	        magn += f.a[i] * f.a[i];
	    magn = sqrt(magn);
	    // calculate times of scaling/squaring
	    int p = 0;
	    float k = 1;
	    while (magn>0.25) {
	        magn *= 0.5;
	        k *= 0.5;
	        p += 1;
	    }
	    // apply scaling/squaring
	    f = k*f;
	    SH<shorder> g = shexp_OL(f);
	    for (int i=0; i<p; ++i)
	        g = g*g;
	    return g;
	}

	SH<shorder> shexp(SH<shorder> f)
	{
		return shexp_HYB(f);
	}
}


SH<shorder> log_visibility(vec3f cluster_center, std::vector<Sphere> blockers)
{
	static int init = loadlut_cpu();
	SH<shorder> f;
	for (Sphere sph: blockers)
	{
        vec3f v = sph.center - cluster_center;
        float angle = asin(std::min(sph.radius / norm(v), 1.0f));
        SymmSH<shorder> symmlog = log_lookup(angle);
        f = f + symmlog.rotated(normalized(v));
	}
	return f;
}