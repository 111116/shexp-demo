#pragma once

#include <fstream>
#include <iostream>
#include <cstring>
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


// unoptimized
template <int n>
SH<n> SH<n>::squared()
{
    // SH<n> c;
    // for (auto e: SH<n>::SquareSparseGamma)
    //     c.a[e.c] += e.val * a[e.a] * a[e.b];
    // return c;
    return (*this)*(*this);
    // TODO optimize
}


template <int n>
float dot(const SH<n>& a, const SH<n>& b)
{
    float t = 0;
    for (int i=0; i<n*n; ++i)
        t += a.a[i] * b.a[i];
    return t;
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


// template <int n>
// SH<n> operator*(const matrix<n*n>& a, const SH<n>& b)
// {
//     SH<n> c;
//     for (int i=0; i<n*n; ++i)
//         for (int j=0; j<n*n; ++j)
//             c.a[i] += a.a[i][j] * b.a[j];
//     return c;
// }
