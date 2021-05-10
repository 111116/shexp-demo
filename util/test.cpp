#include "lib/consolelog.hpp"
#include "sh.hpp"

int main()
{
	const int n = 3;
	SH<n> one([](vec3f w){return 1;});
	console.log(one.a[0]);
	SH<n> h(SymmSH<n>([](float theta){return std::max(cos(theta), 0.0f);}));
	// SH<n> h([](vec3f w){return std::max(w.z, 0.0f);});
	for (int i=0; i<n*n; ++i)
		console.log(h.a[i]/acos(-1));
	console.log("---");
	float a[n*n];
	SHEvaluate(vec3f(0,0,1), n-1, a);
	for (int i=0; i<n*n; ++i)
		console.log(a[i]);
}
