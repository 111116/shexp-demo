#pragma once

#include "shorder.hpp"
#include "rt/trianglemesh.hpp"
#include "receiver_cluster.hpp" // create_2D_float_texture_array

void calculateLH(int n_vert, const vec3f* position, const vec3f* normal, float* LHbuffer, const char light_obj_file[])
{
	console.log("calculating LH...");
	TriangleMesh light(light_obj_file);
	for (int i=0; i<n_vert; ++i)
	{
		if (i%1000==0)
			std::cerr << "\r    LH progress: " << i << "/" << n_vert << "          ";
		sphfunc LH = [&](vec3f w)
		{
			// Primitive::Hit h;
			// if (!light.intersect(Ray(position[i],w), &h)) return 0.0f;
			if (w.y < 0) return 0.0f;
			return std::max(0.0f, dot(normal[i],w));
		};
		SH<shorder> LH_sh(LH, 100);
		memcpy(LHbuffer + i*N_COEFFS, LH_sh.a, N_COEFFS * sizeof(float));
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