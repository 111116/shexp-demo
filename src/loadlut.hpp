#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include "loadtexture.hpp"

const int n_entry = 1024;


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
	create_2D_float_texture(n, n_entry, log_lut, GL_LINEAR);
	glActiveTexture(GL_TEXTURE2);
	create_2D_float_texture(2, n_entry, ab_lut, GL_LINEAR);
}

