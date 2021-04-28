#pragma once

#include <iostream>
#include <fstream>
#include <string>

const int n_entry = 1024;

GLuint createFloat2DTexture(int width, int height, const float* data)
{
	// create & bind a named texture
	GLuint texture;
	glGenTextures(1, &texture);  
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
	// glGenerateMipmap(GL_TEXTURE_2D);
	return texture;
}

void loadlut(int n)
{
	float log_lut[n_entry*n];
	float ab_lut[n_entry*2];
	// read lut from file
	std::string filename = "../res/lut" + std::to_string(n);
	std::ifstream fin(filename);
	if (!fin) throw "cannot open lut";
	for (int i=0; i<n_entry; ++i)
	{
		for (int j=0; j<n; ++j)
			fin >> log_lut[i*n+j];
		fin >> ab_lut[i*2+0] >> ab_lut[i*2+1];
	}
	glActiveTexture(GL_TEXTURE1);
	createFloat2DTexture(n, n_entry, log_lut);
	glActiveTexture(GL_TEXTURE2);
	createFloat2DTexture(2, n_entry, ab_lut);
}

