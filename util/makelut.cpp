// tabularize log(v) and coefficients a,b used in optimal linear exponentiation
// where v is SH projected visibility function of a spherical occluder on the z-axis

#include "sh.hpp"

std::function<float(float)> occlusion(float theta0)
{
	return [theta0](float theta)
	{
		return (theta<theta0)? 0.0f: 1.0f;
	};
}

// sh order n=4 (16 coefficients for general projection)
const int n_order = 4;
typedef SH<n_order> Sh;
typedef SymmSH<n_order> SymmSh;

int main()
{
	// take n_intv+1 samples of \theta in [0,pi]
	const int n_intv = 1023;

	// special case: theta zero
    for (int l=0; l<n_order; ++l)
    	std::cout << 0 << " ";
   	std::cout << " " << 1 << " " << 1 << "\n";

	for (int i=1; i<=n_intv; ++i)
	{
		// show progress
		std::cerr << "\r" << i << "/" << n_intv << "    " << std::flush;
		// project v onto SH basis
		float theta = (float)i/n_intv*PI/2;
		Sh g = Sh(SymmSh(occlusion(theta)));
		Sh f = log(g);
		// apply DC isolation
	    Sh fhat = f;
	    fhat.at(0,0) = 0.0f;
	    Sh ghat = exp(-f.at(0,0) / std::sqrt(4*PI)) * g;
	    // find out OL coefficients
	    float a = ghat.at(0,0) / std::sqrt(4*PI);
	    float b = dot(ghat, fhat) / dot(fhat, fhat);
	    std::cerr << "Error:" << (ghat - (a * Sh::unit() + b * fhat)).magnitude() << "  magn:" << fhat.magnitude() << "\n";
	    // std::cerr << "cmp:" << (ghat - ((a*1.01) * Sh::unit() + b * fhat)).magnitude() << "\n";
	    // std::cerr << "cmp:" << (ghat - ((a*0.99) * Sh::unit() + b * fhat)).magnitude() << "\n";
	    // std::cerr << "cmp:" << (ghat - (a * Sh::unit() + (b*1.01) * fhat)).magnitude() << "\n";
	    // std::cerr << "cmp:" << (ghat - (a * Sh::unit() + (b*0.99) * fhat)).magnitude() << "\n";
	    // output table
	    for (int l=0; l<n_order; ++l)
	    	std::cout << f.at(l,0) << " ";
	   	std::cout << " " << a << " " << b << "\n";
	}
}
