#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>

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

void loadlut(int n, float& maxmagn)
{
	console.log("loading log & OL lut...");
	float log_lut[n_entry*n];
	float ab_lut[n_entry*2];
	float a[n_entry], b[n_entry], magn[n_entry];
	// read lut from file
	std::string filename = "../res/lut" + std::to_string(n);
	std::ifstream fin(filename);
	if (!fin) throw "cannot open lut";
	for (int i=0; i<n_entry; ++i)
	{
		// calculate magnitude of f_hat (f without DC component)
		float s = 0;
		for (int j=0; j<n; ++j) {
			fin >> log_lut[i*n+j];
			if (j>0)
				s += log_lut[i*n+j] * log_lut[i*n+j];
		}
		magn[i] = sqrt(s);
		// fin >> ab_lut[i*2+0] >> ab_lut[i*2+1];
		fin >> a[i] >> b[i];
	}
	// magnitude is increasing then decreasing
	maxmagn = 0;
	for (int i=0; i<n_entry; ++i)
		maxmagn = std::max(maxmagn, magn[i]);
	// convert AB_lut from angle-indexed to magnitude-indexed
	int current_id = 0;
	for (int i=0; i<n_entry; ++i)
	{
		float ratio = (float)i / (n_entry-1);
		auto lerp = [](float a, float b, float t){return a*(1-t)+b*t;};
		float target_magn = lerp(magn[0], maxmagn, ratio);
		while (current_id < n_entry-1 and target_magn > magn[current_id+1])
			current_id += 1;
		float r = (target_magn - magn[current_id]) / (magn[current_id+1] - magn[current_id]);
		ab_lut[i*2+0] = lerp(a[current_id], a[current_id+1], r);
		ab_lut[i*2+1] = lerp(b[current_id], b[current_id+1], r);
	}
	// build textures
	glActiveTexture(GL_TEXTURE1);
	createFloat2DTexture(n, n_entry, log_lut);
	glActiveTexture(GL_TEXTURE2);
	createFloat2DTexture(2, n_entry, ab_lut);
}

