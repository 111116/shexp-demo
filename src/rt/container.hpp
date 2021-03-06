#pragma once

#include <vector>
#include "geometry.hpp"
#include "lib/consolelog.hpp"

class BasicContainer: public Primitive
{
	AABox bound;
	struct HitTmp {
		BasicPrimitive const* primitive = NULL;
		float t;
		HitTmp(){}
		HitTmp(BasicPrimitive const* primitive, const float t):
			primitive(primitive), t(t) {}
		operator bool() {
			return primitive!=NULL;
		}
	};
protected:
	std::vector<BasicPrimitive*> list;
public:
	BasicContainer(const std::vector<BasicPrimitive*>& list): list(list)
	{
		console.log("building SAH BVH of", list.size(), "primitives");
		build(list, root);
		bound = list[0]->boundingVolume();
		for (int i=1; i<list.size(); ++i) {
			bound = bound + list[i]->boundingVolume();
		}
	}

	bool intersect(const Ray& ray, Hit* result) const override
	{
		HitTmp tmp = treehit(ray, root);
		if (!tmp) return false;
		point p = ray.atParam(tmp.t);
		*result = Hit(p, tmp.primitive->Ns(p), tmp.primitive->Ng(p), tmp.primitive->uv(p));
		auto jacobi = dynamic_cast<Triangle const*>(tmp.primitive)->jacobi();
		// TODO implement for other types of primitive
		result->dpdu = jacobi.column(0);
		result->dpdv = jacobi.column(1);
		return true;
	}
	 
	AABox boundingVolume() const override
	{
		return bound;
	}

	SampleInfo sampleSurface(Sampler& sampler) const override
	{
		unsigned id = sampler.get1u(list.size());
		SampleInfo info = list[id]->sampleSurface(sampler);
		info.pdf /= list.size();
		return info;
	}

private:
	struct treenode {
		treenode* lc = NULL;
		treenode* rc = NULL;
		BasicPrimitive* shape = NULL;
		AABox bound;
	};
	treenode* root = NULL;

	void build(std::vector<BasicPrimitive*> list, treenode*& cur)
	{
		// create tree node
		if (!cur) {
			cur = new treenode();
		}
		// bind shape to leaf nodes
		if (list.size() == 1)
		{
			cur->shape = list[0];
			cur->bound = list[0]->boundingVolume();
			return;
		}
		// compute bounding box
		AABox& bound = cur->bound;
		bound = list[0]->boundingVolume();
		for (int i=1; i<list.size(); ++i) {
			bound = bound + list[i]->boundingVolume();
		}
		// sort in sparsest dimension
		auto cmp = +[](BasicPrimitive* a, BasicPrimitive* b) {
			return a->boundingVolume().x1 < b->boundingVolume().x1;
		};
		if (bound.y2 - bound.y1 > bound.x2 - bound.x1) {
			cmp = +[](BasicPrimitive* a, BasicPrimitive* b) {
				return a->boundingVolume().y1 < b->boundingVolume().y1;
			};
		}
		if (bound.z2 - bound.z1 > bound.x2 - bound.x1 &&
			bound.z2 - bound.z1 > bound.y2 - bound.y1) {
			cmp = +[](BasicPrimitive* a, BasicPrimitive* b) {
				return a->boundingVolume().z1 < b->boundingVolume().z1;
			};
		}
		std::sort(list.begin(), list.end(), cmp);
		// determine split position to minimize sum of surface area of bounding boxes
		std::vector<float> prefix_area, suffix_area;
		prefix_area.resize(list.size());
		suffix_area.resize(list.size());
		AABox accu = list[0]->boundingVolume();
		for (int i=0; i<list.size(); ++i) {
			accu = accu + list[i]->boundingVolume();
			prefix_area[i] = accu.surfaceArea();
		}
		accu = list[list.size()-1]->boundingVolume();
		for (int i=list.size()-1; i>=0; --i) {
			accu = accu + list[i]->boundingVolume();
			suffix_area[i] = accu.surfaceArea();
		}
		int rangemin = round(list.size()*0.15);
		int rangemax = round(list.size()*0.85)-1;
		// limit split point from being too close to side
		int best = rangemin;
		for (int i=rangemin; i<rangemax; ++i) {
			if (prefix_area[i] + suffix_area[i+1] < prefix_area[best] + suffix_area[best+1])
				best = i;
		}
		// recursive partition
		build(std::vector<BasicPrimitive*>(list.begin(), list.begin()+best+1), cur->lc);
		build(std::vector<BasicPrimitive*>(list.begin()+best+1, list.end()), cur->rc);
	}

	HitTmp treehit(const Ray& ray, treenode* node) const {
		if (node == NULL) return HitTmp();
		if (!node->bound.intersect(ray)) return HitTmp();
		if (node->shape != NULL) {
			float p;
			bool t = node->shape->intersect(ray, p);
			return HitTmp(t? node->shape: NULL, p);
		}
		HitTmp resl = treehit(ray, node->lc);
		if (!resl) return treehit(ray, node->rc);
		HitTmp resr = treehit(ray, node->rc);
		return (!resr || resl.t < resr.t)? resl: resr;
	}
};
