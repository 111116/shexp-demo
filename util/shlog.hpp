#pragma once

#include "sh.hpp"
#include "lib/Eigen/Eigen"


template <int n>
SH<n> eigen_to_sh(Eigen::Matrix<float, n*n, 1> v)
{
	SH<n> a;
	for (int i=0; i<n*n; ++i)
		a.a[i] = v(i);
	return a;
}

template <int n>
Eigen::Matrix<float, n*n, 1> sh_to_eigen(SH<n> a)
{
	Eigen::Matrix<float, n*n, 1> v;
	for (int i=0; i<n*n; ++i)
		v(i) = a.a[i];
	return v;
}


template <int n>
SH<n> log(const SH<n>& a)
{
    typedef Eigen::Matrix<float, n*n, n*n> EMatrix;
    EMatrix Mf;
    matrix<n*n> Mf_tmp = a.prodMatrix();
    for (int i=0; i<n*n; ++i)
    for (int j=0; j<n*n; ++j)
        Mf(i,j) = Mf_tmp.a[i][j];
    // Eigendecomposition Mf = R^T D R
    Eigen::SelfAdjointEigenSolver<EMatrix> eigensolver(Mf);
    if (eigensolver.info() != Eigen::Success)
    	throw "Eigendecomposition failed";
    auto RT = eigensolver.eigenvectors();
    auto D = eigensolver.eigenvalues();
    assert(-D.minCoeff() <= D.maxCoeff());
    double epsilon = 0.02 * D.maxCoeff();
    // calculate q'(max(D,eps))
    EMatrix qD;
    qD.setZero();
    auto diag = qD.diagonal();
    for (int i=0; i<n*n; ++i)
    {
    	float t = fmax(epsilon, D(i));
    	diag[i] = log(t) / (t-1);
    }
    // log(f) = R^T q'(D) R (f-1)
    Eigen::Matrix<float, n*n, 1> f_minus_unit = sh_to_eigen(a) - sh_to_eigen(SH<n>::unit());
    auto logf = RT * qD * RT.transpose() * f_minus_unit;
    return eigen_to_sh<n>(logf);
}


template <int n>
SymmSH<n> log(const SymmSH<n>& a)
{
	auto l = log(SH<n>(a));
	SymmSH<n> b;
	for (int i=0; i<n; ++i)
		b.a[i] = l.at(i,0);
	return b;
}
