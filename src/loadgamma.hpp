#pragma once

#include "shorder.hpp"
#include "loadtexture.hpp"


int upload_gamma(int n)
{
	auto v = readGamma(n);
	const int width = 1024;
	const int height = 1024;
	float* data = new float[width * height * 4];
	for (int i=0; i<v.size(); ++i)
	{
		data[i*4+0] = float(v[i].a);
		data[i*4+1] = float(v[i].b);
		data[i*4+2] = float(v[i].c);
		data[i*4+3] = v[i].val;
	}
	glActiveTexture(GL_TEXTURE15);
	create_2D_float_texture(1024, 1024, 4, data, GL_NEAREST);
	delete[] data;
	return v.size();
}

std::vector<TensorEntry> symmetric_fold(std::vector<TensorEntry> v)
{
	std::vector<TensorEntry> res;
	for (auto e: v) {
		if (e.a < e.b)
			res.push_back({e.a, e.b, e.c, e.val*2});
		if (e.a == e.b)
			res.push_back({e.a, e.b, e.c, e.val});
	}
	return res;
}

int upload_sqrgamma(int n)
{
	auto v = readGamma(n);
	v = symmetric_fold(v);
	const int width = 1024;
	const int height = 1024;
	float* data = new float[width * height * 4];
	for (int i=0; i<v.size(); ++i)
	{
		data[i*4+0] = float(v[i].a);
		data[i*4+1] = float(v[i].b);
		data[i*4+2] = float(v[i].c);
		data[i*4+3] = v[i].val;
	}
	glActiveTexture(GL_TEXTURE16);
	create_2D_float_texture(1024, 1024, 4, data, GL_NEAREST);
	delete[] data;
	return v.size();
}