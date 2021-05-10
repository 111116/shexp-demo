#pragma once

#include "sh/sh.hpp"

// cubemap stores SH projection of product of environment light L and cosine-weight function H at different direction
void calculate_sh_lut(float* data, const int size)
// layout: SHindex, face, position, rgb
{
	const int N_COEFFS = 9;
	const int n = 3;

	// evaluate SH bases at different direction N
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			float GU = size*0.5f;
			float u = (float)j - GU + 0.5f;
			float v = (float)i - GU + 0.5f;
			// corresponding direction on each face
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
				float coeffs[N_COEFFS];
				vec3f N = normalized(d[face]);
				SHEvaluate(N, n-1, coeffs);
				// fill in data
				for(int index=0; index < N_COEFFS; ++index) {
					data[((index*6 + face) * size + i) * size + j] = coeffs[index];
				}
			}
		}
	}
}

GLuint build_sh_lut()
{
	console.log("building sh lut...");
	const int size = 128;
	float *data = new float[N_COEFFS*6*size*size];

	calculate_sh_lut(data, size);
	// create & bind a named texture
	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// generate the texture
	glTexImage3D( GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_R32F, size, size, N_COEFFS*6, 0, GL_RED, GL_FLOAT, data);

	delete [] data;
	return texture;
}