#pragma once

#include "shorder.hpp"
#include "rt/trianglemesh.hpp"
#include "rt/samplers/mt19937sampler.hpp"
#include "receiver_cluster.hpp" // create_2D_float_texture_array

void calculateLH(int n_vert, const vec3f* position, const vec3f* normal, float* LHbuffer, const char light_obj_file[])
{
	console.log("calculating LH...");
	TriangleMesh light(light_obj_file);
	for (int vert=0; vert<n_vert; ++vert)
	{
		if (vert%100==0)
			std::cerr << "\r    LH progress: " << vert << "/" << n_vert << "          ";
		// sphfunc LH = [&](vec3f w)
		// {
		// 	Primitive::Hit h;
		// 	if (!light.intersect(Ray(position[i],w), &h)) return 0.0f;
		// 	if (w.y < 0) return 0.0f;
		// 	return std::max(0.0f, dot(normal[i],w));
		// };

	    MT19937Sampler sampler;

	    double acc[N_COEFFS];
    	float y[N_COEFFS];
	    memset(acc, 0, sizeof acc);
	    const int n_sample = 100;
    	for (int _=0; _<n_sample; ++_) {
			auto info = light.sampleSurface(sampler);
			vec3f w = normalized(info.p - position[vert]);
			float irr = dot(info.normal, w) / sqrlen(info.p - position[vert]) / info.pdf;
			irr = fabs(irr);
			irr *= std::max(0.0f, dot(normal[vert],w));
        	SHEvaluate(w, shorder-1, y);
	        for (int i=0; i<N_COEFFS; ++i)
	            acc[i] += irr * y[i];
    	}
    	for (int j=0; j<N_COEFFS; ++j)
    		LHbuffer[vert*N_COEFFS+j] = acc[j] / n_sample;
	}
	std::cerr << "\r    LH progress: " << n_vert << "/" << n_vert << "           \n";
}


void uploadLH(int n_vert, const float* buffer)
{
	float* texdata = new float[1024 * 1024 * N_COEFFS];
	for (int i=0; i<n_vert; ++i)
		for (int j=0; j<N_COEFFS; ++j)
			texdata[j*1024*1024+i] = buffer[i*N_COEFFS+j];
	glActiveTexture(GL_TEXTURE0);
	create_2D_float_texture_array(1024, 1024, N_COEFFS, texdata);
	delete[] texdata;
}