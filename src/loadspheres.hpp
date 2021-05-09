#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include "sphere_hierarchy.hpp"
#include "sphere_hierarchy_visualize.hpp"


GLuint create_2D_vec4_texture(int width, int height, const float* data)
{
	// create & bind a named texture
	GLuint texture;
	glGenTextures(1, &texture);  
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	// we use nestest interpolation since this texture is used for data retrieval
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
	// glGenerateMipmap(GL_TEXTURE_2D);
	return texture;
}

// return value: number of spheres
int loadspheres(const char* filename)
{
	std::vector<Sphere> spheres;
	// read file
	std::ifstream fin(filename);
	if (!fin)
		throw "load spheres: cannot open file";
	Sphere t(vec3f(),0);
	while (fin >> t.center >> t.radius) {
		spheres.push_back(t);
	}
	visualize(sphere_hierarchy(spheres));
	throw "exiting...";

	std::vector<float> v;
	float x;
	while (fin >> x)
		v.push_back(x);
	// checks
	if (v.size()==0 || v.size()%4!=0)
		throw "load spheres: invalid";
	int n = v.size()/4;
	for (int i=0; i<n; ++i)
		if (v[4*i+3]<0) // negative radius
			throw "load spheres: invalid";
	// pad to 1024*64
	if (n>1024*64)
		throw "load spheres: too many spheres";
	for (int i=n; i<1024*64; ++i)
		for (int c=0; c<4; ++c)
			v.push_back(0.0f);
	// build texture
	glActiveTexture(GL_TEXTURE4);
	create_2D_vec4_texture(1024, 64, &v[0]);
	return n;
}