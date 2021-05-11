#pragma once

GLuint create_2D_float_array(int width, int height, float* data)
{
	// create & bind a named texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	// we use nestest interpolation since this texture is used for data retrieval
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
	return texture;
}

GLuint create_2D_int_array(int width, int height, int* data)
{
	// create & bind a named texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	// we use nestest interpolation since this texture is used for data retrieval
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, width, height, 0, GL_RED, GL_INT, data);
	return texture;
}

int upload_gamma(int n)
{
	auto v = readGamma(n);
	const int width = 1024;
	const int height = 1024;
	int* data_a = new int[width * height];
	int* data_b = new int[width * height];
	int* data_c = new int[width * height];
	float* data_val = new float[width * height];
	for (int i=0; i<v.size(); ++i)
	{
		data_a[i] = v[i].a;
		data_b[i] = v[i].b;
		data_c[i] = v[i].c;
		data_val[i] = v[i].val;
	}
	glActiveTexture(GL_TEXTURE11);
	create_2D_int_array(width, height, data_a);
	glActiveTexture(GL_TEXTURE12);
	create_2D_int_array(width, height, data_b);
	glActiveTexture(GL_TEXTURE13);
	create_2D_int_array(width, height, data_c);
	glActiveTexture(GL_TEXTURE14);
	create_2D_float_array(width, height, data_val);
	delete[] data_a;
	delete[] data_b;
	delete[] data_c;
	delete[] data_val;
	return v.size();
}