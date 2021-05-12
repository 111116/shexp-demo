#pragma once

#include "sh.hpp"
#include "readgamma.hpp"


template <int n>
matrix<n> operator* (const matrix<n>& a, const matrix<n>& b)
{
    matrix<n> c;
    memset(c.a, 0, sizeof c.a);
    for (int i=0; i<n; ++i)
        for (int j=0; j<n; ++j)
            for (int k=0; k<n; ++k)
                c.a[i][j] += a.a[i][k] * b.a[k][j];
    return c;
}

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


// projection of product as transformation lambda B: A*B
template <int n>
matrix<n*n> SH<n>::prodMatrix() const
{
    static std::vector<TensorEntry> SparseGamma = readGamma(n);
    matrix<n*n> m;
    for (auto e: SparseGamma)
    	m.a[e.a][e.b] += e.val * a[e.c];
    return m;
}

template <int n>
SH<n> operator*(const matrix<n*n>& a, const SH<n>& b)
{
    SH<n> c;
    for (int i=0; i<n*n; ++i)
        for (int j=0; j<n*n; ++j)
            c.a[i] += a.a[i][j] * b.a[j];
    return c;
}
