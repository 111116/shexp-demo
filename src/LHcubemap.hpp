#pragma once

const int N_COEFFS = 9;

// cubemap stores SH projection of product of environment light L and cosine-weight function H at different direction
void calculate_LHcubemap(float* data, const int size)
// layout: SHindex, face, position, rgb
{
	assert(N_COEFFS == 9);
	const int n = 3;
	// load SH projection of environment light
	vec3f L[9];
	L[0] = vec3f(0.735427, 0.613381, 0.58883);
	L[1] = vec3f(0.102978, 0.138365, 0.166129);
	L[2] = vec3f(0.508975, 0.420545, 0.433161);
	L[3] = vec3f(-0.0336926, -0.254128, -0.414039);
	L[4] = vec3f(0.00985182, -0.113192, -0.184917);
	L[5] = vec3f(0.103119, 0.141603, 0.168878);
	L[6] = vec3f(-0.0445019, -0.06456, -0.0479999);
	L[7] = vec3f(0.036427, -0.21701, -0.410711);
	L[8] = vec3f(0.114988, 0.113818, 0.116976);
	SH<n> L_r, L_g, L_b;
	for (int i=0; i<N_COEFFS; ++i) {
		L_r.a[i] = L[i].x;
		L_g.a[i] = L[i].y;
		L_b.a[i] = L[i].z;
	}
	// SH-project cosine-weight function H
	SymmSH<n> H([](float theta){return std::max(cos(theta), 0.0f);});

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

GLuint buildLHcubemap()
{
	console.log("building cubemap L_H(N)...");
	const int size = 128;
	float *data = new float[N_COEFFS*6*size*size*3];

	calculate_LHcubemap(data, size);

	GL_TEXTURE_CUBE_MAP;
	GL_TEXTURE_CUBE_MAP_ARRAY;
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texture);

	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage3D( GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB32F, size, size, N_COEFFS*6, 0, GL_RGB, GL_FLOAT, data);

	delete [] data;
	return texture;
}