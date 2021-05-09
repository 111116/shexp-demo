#pragma once

#include <vector>
#include <algorithm>
#include "powell.hpp"
#include "util.hpp"


struct Sphere
{
	vec3f center;
	float radius;
	Sphere(vec3f center, float radius): center(center), radius(radius) {}
	Sphere& operator= (const Sphere& s)
	{
		center = s.center;
		radius = s.radius;
		return *this;
	}
	float volume() const
	{
		return 4.0/3*PI*radius*radius*radius;
	}
};

struct SphereTree
{
	Sphere bound;
	std::vector<SphereTree> child;
	SphereTree(): bound(vec3f(),0) {}
	SphereTree(Sphere s): bound(s) {}
};



Sphere sphere_set_bounding_sphere(std::vector<Sphere> spheres)
{
	// use average center as initial position
	vec3f sum;
	for (auto s: spheres)
		sum += s.center;
	vec3f initial(1.0/spheres.size() * sum);
	// target function to optimize
	auto bound_radius = [&](vec3f center){
		float r = 0;
		for (auto s: spheres)
			r = std::max(r, norm(center-s.center) + s.radius);
		return r;
	};
	// use powell optimization
	vec3f center = optimize(initial, bound_radius);
	return Sphere(center, bound_radius(center));
}

std::vector<Sphere> sphere_tree_leaves(const SphereTree& tree)
{
	if (tree.child.empty()) return {tree.bound};
	std::vector<Sphere> res;
	for (const SphereTree& c: tree.child)
	{
		std::vector<Sphere> v = sphere_tree_leaves(c);
		res.insert(res.end(), v.begin(), v.end());
	}
	return res;
}


SphereTree sphere_hierarchy(std::vector<Sphere> spheres)
{
	// build leaf nodes
	std::vector<SphereTree> nodes;
	for (auto s: spheres)
		nodes.push_back(SphereTree(s));
	// successively form next layer
	while (nodes.size() > 1) {
		// randomly select representatives
		std::random_shuffle(nodes.begin(), nodes.end());
		std::vector<SphereTree> parents;
		for (int i=0; i<(nodes.size()-1)/4+1; ++i)
			parents.push_back(SphereTree(nodes[i].bound));
		// iteratively assign & optimize
		float lastloss = 1e18;
		while (true) {
			// stage 1. assign
			for (auto& p: parents)
				p.child.clear();
			for (auto o: nodes)
				argmin(parents, [o](const SphereTree& t){return norm(t.bound.center - o.bound.center);})->child.push_back(o);
			// state 2. optimize
			for (auto& p: parents)
				p.bound = sphere_set_bounding_sphere(sphere_tree_leaves(p));
			// calculate total loss
			float curloss = 0;
			for (auto p: parents)
				curloss += p.bound.radius;
			if (curloss > lastloss)
				console.warn("sphere clustering: loss increasing");
			if (curloss > lastloss - 1e-5)
				break;
			lastloss = curloss;
		}
		nodes = parents;
	}
	return nodes[0];
}
