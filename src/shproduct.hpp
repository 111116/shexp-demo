#pragma once

#include "sh.hpp"
#include "readgamma.hpp"

// projection of product of SH projected functions
template <int n>
SH<n> operator*(const SH<n>& a, const SH<n>& b)
{
    static std::vector<TensorEntry> SparseGamma = readGamma(n);
    SH<n> c;
    for (auto e: SparseGamma)
        c.a[e.c] += e.val * a.a[e.a] * b.a[e.b];
    return c;
}

