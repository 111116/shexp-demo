#pragma once

#include "sphere_hierarchy.hpp"

// output scene file for visualization of each layer


void visualize(std::vector<Sphere> spheres, std::string outfile = "scene.json", std::string imagefile = "demo.png", vec3f camerapos = vec3f(3,2,-5.76543))
{
	console.info("For visualization, run: tungsten", outfile);
	std::ofstream fout(outfile);
	if (!fout) throw "failed open output file";
	fout << "{\"primitives\":[\n";
	for (auto sph : spheres)
	{
		fout << "{\"type\": \"sphere\",\"transform\":{\"position\":["
		<< sph.center.x << "," << sph.center.y << "," << sph.center.z << "],\"scale\":" << sph.radius << "},\"bsdf\":{\"type\":\"lambert\",\"ior\":1.3,\"albedo\":[1,0.6,0.6]}},\n";
	}
    fout << "{\"type\": \"skydome\",\"temperature\": 4777.0,\"gamma_scale\": 1.0,\"turbidity\": 3.0,\"intensity\": 6.0,\"sample\": true},";
	fout << "],\n";
	fout << "\"camera\":{\"resolution\":[2048,2048],\"transform\":{";
	// print camera pos
	fout << "\"position\": [" << camerapos.x << "," << camerapos.y << "," << camerapos.z << "], \"look_at\": [0,0,0], \"up\": [0,1,0]";

	fout << "},";
	fout << "\"type\":\"pinhole\",\"fov\":40},\n";
    fout << "\"integrator\": {\"min_bounces\": 0,\"max_bounces\": 16,\"type\": \"path_tracer\"},\n";
    fout << "\"renderer\": {\"scene_bvh\": true,\"spp\": 16,\"output_file\": \"" << imagefile << "\"},\n";
    fout << "}\n";
}

std::vector<SphereTree> nextlayer(const std::vector<SphereTree>& v)
{
	std::vector<SphereTree> cur;
	for (auto t: v)
		cur.insert(cur.end(), t.child.begin(), t.child.end());
	return cur;
}

void visualize(SphereTree tree)
{
	std::vector<SphereTree> layer = {tree};
	for (int id=0; !layer.empty(); ++id) {
		std::vector<Sphere> spheres;
		for (auto t: layer)
			spheres.push_back(t.bound);
		visualize(spheres, std::string("hierarchy/") + std::to_string(id) + ".json", std::to_string(id) + ".png");
		layer = nextlayer(layer);
	}
}