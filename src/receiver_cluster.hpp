#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "consolelog.hpp"

const char cluster_file[] = "cluster.cache";

void cluster_points_recalculate(int n, int n_cluster, vec3f* positions, int* clusterids);

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


void cluster_points_recalculate(int n, int n_cluster, vec3f* positions, int* clusterids)
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