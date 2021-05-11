#pragma once

#include "shorder.hpp"

// cubemap stores SH projection of product of environment light L and cosine-weight function H at different direction
void calculate_LHcubemap(const char* filename, float* data, const int size)
// layout: SHindex, face, position, rgb
{
	// load SH projection of environment light
	vec3f L[N_COEFFS];
	std::ifstream fin(filename);
	int _t;
	fin >> _t;
	if (_t < shorder) throw "shrgb order unmatch";
	for (int i=0; i<N_COEFFS; ++i)
		fin >> L[i];
	// separate channels
	SH<shorder> L_r, L_g, L_b;
	for (int i=0; i<N_COEFFS; ++i) {
		L_r.a[i] = L[i].x;
		L_g.a[i] = L[i].y;
		L_b.a[i] = L[i].z;
	}
	// SH-project cosine-weight function H
	SymmSH<shorder> H([](float theta){return std::max(cosf(theta), 0.0f);});

	// calculate L*H(N) at different direction N
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			float GU = size*0.5f;
			float u = (float)j - GU + 0.5f;
			float v = (float)i - GU + 0.5f;

			vec3f d[6] = {
				vec3f( GU,  -v,  -u),
				vec3f(-GU,  -v,   u),
				vec3f(  u,  GU,   v),
				vec3f(  u, -GU,  -v),
				vec3f(  u,  -v,  GU),
				vec3f( -u,  -v, -GU)
			};
			for (int face = 0; face < 6; face++)
			{
				double coeffs[N_COEFFS*3];
				vec3f N = normalized(d[face]);
				auto HN = H.rotated(N);
				auto L_H_r = HN * L_r;
				auto L_H_g = HN * L_g;
				auto L_H_b = HN * L_b;
				// note that L & H can be computed at higher order than n, to obtain better accuracy
				for(int index=0; index < N_COEFFS; ++index) {
					data[(((index*6 + face) * size + i) * size + j) * 3 + 0] = L_H_r.a[index];
					data[(((index*6 + face) * size + i) * size + j) * 3 + 1] = L_H_g.a[index];
					data[(((index*6 + face) * size + i) * size + j) * 3 + 2] = L_H_b.a[index];
				}
			}
		}
	}
}

GLuint buildLHcubemap(const char* filename)
{
	console.log("building cubemap L_H(N)...");
	const int size = 128;
	float *data = new float[N_COEFFS*6*size*size*3];

	calculate_LHcubemap(filename, data, size);
	// create & bind a named texture
	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// generate the texture
	glTexImage3D( GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB32F, size, size, N_COEFFS*6, 0, GL_RGB, GL_FLOAT, data);

	delete [] data;
	return texture;
}
