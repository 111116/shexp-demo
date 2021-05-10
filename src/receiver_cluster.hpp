#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "consolelog.hpp"

const char cluster_file[] = "cluster.cache";

void cluster_points_recalculate(int n, int n_cluster, const vec3f* positions, int* clusterids);

void cluster_points(int n, vec3f* positions, int* clusterids)
{
	const int n_cluster = 400;
	std::ifstream fin(cluster_file);
	if (!fin) {fin.close(); return cluster_points_recalculate(n, n_cluster, positions, clusterids);}
	int n1 = -1, n_cluster1 = -1;
	fin >> n1 >> n_cluster1;
	if (n!=n1 || n_cluster != n_cluster1) {fin.close(); return cluster_points_recalculate(n, n_cluster, positions, clusterids);}
	for (int i=0; i<n; ++i) {
		vec3f p;
		fin >> p;
		if (norm(p-positions[i])>1e-5) {fin.close(); return cluster_points_recalculate(n, n_cluster, positions, clusterids);}
	}
	console.log("cluster loading cache from disk...");
	for (int i=0; i<n; ++i)
		fin >> clusterids[i];
}


void cluster_points_recalculate(int n, int n_cluster, const vec3f* positions, int* clusterids)
{
	console.time("receiver cluster");
	// random initialize clusters
	vec3f center[n_cluster];
	for (int i=0; i<n_cluster; ++i)
		center[i] = positions[rand()%n]; // a few may coincide, doesn't matter

	std::vector<int> assigned[n_cluster];
	for (int k=0; k<10; ++k) {
		console.log("receiver cluster iteration",k);
		for (int i=0; i<n_cluster; ++i)
			assigned[i].clear();
		// assign points to nearest cluster
		for (int i=0; i<n; ++i) {
			float bestnorm = 1e18;
			int best = 0;
			for (int j=0; j<n_cluster; ++j)
				if (bestnorm > norm(positions[i] - center[j])) {
					bestnorm = norm(positions[i] - center[j]);
					best = j;
				}
			assigned[best].push_back(i);
		}
		// move cluster center to average of points
		for (int i=0; i<n_cluster; ++i) {
			vec3f sum = 0;
			for (int x: assigned[i])
				sum += positions[x];
			center[i] = sum / assigned[i].size();
		}
	}
	for (int i=0; i<n_cluster; ++i)
		for (int x: assigned[i])
			clusterids[x] = i;
	console.timeEnd("receiver cluster");
	console.log("cluster caching to disk...");
	std::ofstream fout(cluster_file);
	fout << n << "\n";
	fout << n_cluster << "\n";
	for (int i=0; i<n; ++i)
		fout << positions[i].x << " " << positions[i].y << " " << positions[i].z << "\n";
	for (int i=0; i<n; ++i)
		fout << clusterids[i] << "\n";
}



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

std::vector<Sphere> treecut(const SphereTree& hierarchy, vec3f cluster_center, float cluster_radius, float angle_limit)
{
	// stop recursing if the blockersphere subtends an angle less than angle_limit
	if (hierarchy.child.empty() || hierarchy.bound.radius < sin(angle_limit) * (norm(hierarchy.bound.center - cluster_center) - cluster_radius))
		return {hierarchy.bound};
	std::vector<Sphere> res;
	for (auto c: hierarchy.child)
	{
		auto cut = treecut(c, cluster_center, cluster_radius, angle_limit);
		res.insert(res.end(), cut.begin(), cut.end());
	}
	return res;
}

// generates 2D texture
void cluster_preprocess(int n, const vec3f* positions, const int* clusterids, const SphereTree& hierarchy, int* sphcnt)
{
	console.log("per-cluster processing...");
	// calculate number of clusters
	int maxcluster = 0;
	for (int recv=0; recv<n; ++recv)
		maxcluster = std::max(maxcluster, clusterids[recv]);
	int n_cluster = maxcluster+1;
	// calculate centers of clusters (average over all receivers)
	std::vector<int> recv_cnt(n_cluster,0);
	std::vector<vec3f> recv_sum(n_cluster,0);
	for (int recv=0; recv<n; ++recv) {
		int c = clusterids[recv];
		recv_cnt[c] += 1;
		recv_sum[c] += positions[recv];
	}
	vec3f cluster_center[n_cluster];
	for (int c=0; c<n_cluster; ++c) {
		cluster_center[c] = recv_sum[c] / recv_cnt[c];
		if (recv_cnt[c] == 0) {
			console.debug("encountering empty receiver cluster");
			cluster_center[c] = vec3f(0,0,0);
		}
	}
	// calculate bounding radius of clusters
	std::vector<float> cluster_radius(n_cluster,0);
	for (int recv=0; recv<n; ++recv) {
		int c = clusterids[recv];
		cluster_radius[c] = std::max(cluster_radius[c], norm(positions[recv] - cluster_center[c]));
	}
	// prepare texture data
	static const int texwidth = 1024; // max number of spheres per cluster
	static const int texheight = 1024; // max number of cluster
	float* texdata = new float[texwidth * texheight * 4];
	// assemble bounding sphere nodes for shading each cluster
	static const float theta_max = (float)20/180*PI;
	static const float theta_min = (float)5/180*PI;
	std::vector<float> cluster_blocker_count(n_cluster,0);
	for (int c=0; c<n_cluster; ++c) {
		// TODO save number of bounding
		vec3f center = cluster_center[c];
		float radius = cluster_radius[c];
		std::vector<Sphere> bounding = treecut(hierarchy, center, radius, theta_max);
		std::vector<Sphere> detailed = treecut(hierarchy, center, radius, theta_min);
		if (bounding.size() > texwidth || c >= texheight)
			throw "sphere texture overflowing";
		// TODO calculate ratio vector
		// store into texture data
		memcpy(texdata + texwidth*4*c, bounding.data(), bounding.size() * sizeof(Sphere));
		cluster_blocker_count[c] = bounding.size();
	}
	// build texture
	glActiveTexture(GL_TEXTURE4);
	// data layout of Sphere is exactly RGBA32F
	create_2D_vec4_texture(texwidth, texheight, texdata);
	delete[] texdata;
	// assign sphere count of each vertex
	for (int recv=0; recv<n; ++recv)
		sphcnt[recv] = cluster_blocker_count[clusterids[recv]];
	// show stats
	int s = 0, max = 0;
	for (int t: cluster_blocker_count) {
		s += t;
		max = std::max(max, t);
	}
	console.log("max #blockers per cluster:", max);
	console.log("avg #blockers per cluster:", 10 * s / n_cluster * 0.1);
}
