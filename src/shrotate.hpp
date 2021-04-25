#pragma once

#include "sh.hpp"

// rotate to be centered around axis w
template <int n>
SH<n> SymmSH<n>::rotated(vec3f w)
{
    float y[n*n];
    SHEvaluate(w, n-1, y);
    SH<n> sh;
    for (int l=0; l<n; ++l) {
        float gl_star = a[l] * std::sqrt(4*PI/(2*l+1));
        for (int m=-l; m<=l; ++m)
            sh.at(l,m) = gl_star * y[SHIndex(l,m)];
    }
    return sh;
}


// rotation
template <int n>
SH<n> SH<n>::rotated(vec3f w)
{
    // TODO
    throw "unimplemented";
}