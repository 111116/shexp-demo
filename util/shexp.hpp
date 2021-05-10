#pragma once

#include <iostream>
#include <fstream>
#include "sh.hpp"


template <int n>
struct lutdata_t
{
	SymmSH<n> data;
	float magn, a, b;
};

template <int n>
std::istream& operator>> (std::istream& is, lutdata_t<n>& data)
{
	for (int i=0; i<n; ++i)
		is >> data.data.a[i];
	SymmSH<n> t = data.data;
	t.a[0] = 0;
	data.magn = SH<n>(t).magnitude();
	return is >> data.a >> data.b;
}

template <typename T>
T lerp(const T& a, const T& b, float t)
{
	return (1-t)*a + t*b;
}

template <int n>
std::vector<lutdata_t<n>> init_lut()
{
	std::ifstream fin("log.lu" + std::to_string(n));
	lutdata_t<n> t;
	std::vector<lutdata_t<n>> v;
	while (fin >> t)
		v.push_back(t);
	std::vector<lutdata_t<n>> res;
	for (int i=0; i<=100; ++i) {
		float m = (float)i/100 * 5.0;
		int id = 0;
		while (v[id+1].magn <= m) id++;
		assert(v[id+1].magn > m);
		assert(v[id].magn <= m);
		float t = (m-v[id].magn) / (v[id+1].magn - v[id].magn);
		lutdata_t<n> data;
		data.data = lerp(v[id].data, v[id+1].data, t);
		data.magn = lerp(v[id].magn, v[id+1].magn, t);
		data.a = lerp(v[id].a, v[id+1].a, t);
		data.b = lerp(v[id].b, v[id+1].b, t);
		res.push_back(data);
	}
	return res;
}



template <int n>
SH<n> exp(const SH<n>& f)
{
	return exp_HYB(f);
}

// SH exp using optimal linear
// for testing only. not optimized
template <int n>
SH<n> exp_OL(const SH<n>& f)
{
	SH<n> fhat = f;
	fhat.at(0,0) = 0.0f;
	float m = fhat.magnitude();
	// lookup a,b
	static const std::vector<lutdata_t<n>> lut = init_lut<n>();
	float a,b;
	// bilinear interpolation
	{
		const int n_intv = lut.size()-1;
		float id = m / 5.0 * n_intv;
		if (id > n_intv + 0.5 || id < -0.5) throw "interpolation: out of range";
		int i = floor(id);
		float t = id - i;
		if (i<0) {
			a = lut[0].a;
			b = lut[0].b;
		}
		else
		if (i>=n_intv) {
			a = lut[n_intv].a;
			b = lut[n_intv].b;
		}
		else {
			a = lerp(lut[i].a, lut[i+1].a, t);
			b = lerp(lut[i].b, lut[i+1].b, t);
		}
	}
	auto t = b*fhat;
	t.at(0,0) = a*std::sqrt(4*PI);
	return exp(f.at(0,0) / std::sqrt(4*PI)) * t;
}

// SH exp using optimal linear & scaling/squaring
// for testing only. not optimized
template <int n>
SH<n> exp_HYB(SH<n> f)
{
	SH<n> fhat = f;
	fhat.at(0,0) = 0.0f;
	float m = fhat.magnitude();
	// console.log("magn",m);
	int nsqr = 0;
	while (m >= 4.8) {
		f = 0.5 * f;
		m = 0.5 * m;
		nsqr++;
	}
	SH<n> res = exp_OL(f);
	while (nsqr--)
		res = res.squared();
	return res;
}


template <int n>
SH<n> exp_PS(const SH<n>& f, int maxk)
{
	float k = 1;
	SH<n> s = SH<n>::unit();
	SH<n> a = f;
	for (int i=1; i<=maxk; ++i)
	{
		k = k/i;
		s = s+k*a;
		a = a * f;
	}
	return s;
}
