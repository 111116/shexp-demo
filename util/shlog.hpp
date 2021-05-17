#pragma once

#include <algorithm>
#include "sh.hpp"
#include "lib/Eigen/Eigen"


template <int n>
SH<n> eigen_to_sh(Eigen::Matrix<double, n*n, 1> v)
{
	SH<n> a;
	for (int i=0; i<n*n; ++i)
		a.a[i] = v(i);
	return a;
}

template <int n>
Eigen::Matrix<double, n*n, 1> sh_to_eigen(SH<n> a)
{
	Eigen::Matrix<double, n*n, 1> v;
	for (int i=0; i<n*n; ++i)
		v(i) = a.a[i];
	return v;
}


template <int n>
SH<n> log(const SH<n>& a)
{
    using Eigen::MatrixXd;
    MatrixXd Mf = MatrixXd::Zero(n*n, n*n);
    matrix<n*n> Mf_tmp = a.prodMatrix();
    for (int i=0; i<n*n; ++i)
    for (int j=0; j<n*n; ++j)
        Mf(i,j) = Mf_tmp.a[i][j];
    // Eigendecomposition Mf = R^T D R
    Eigen::SelfAdjointEigenSolver<MatrixXd> eigensolver(Mf);
    if (eigensolver.info() != Eigen::Success)
    	throw "Eigendecomposition failed";
    auto RT = eigensolver.eigenvectors();
    auto D = eigensolver.eigenvalues();
    assert(-D.minCoeff() <= D.maxCoeff());
    double epsilon = 0.02 * D.maxCoeff();
    // calculate q'(max(D,eps))
    MatrixXd qD = MatrixXd::Zero(n*n, n*n);
    auto diag = qD.diagonal();
    for (int i=0; i<n*n; ++i)
    {
    	double t = std::max(epsilon, D(i));
    	diag[i] = log(t) / (t-1);
    }
    // log(f) = R^T q'(D) R (f-1)
    Eigen::Matrix<double, n*n, 1> f_minus_unit = sh_to_eigen(a) - sh_to_eigen(SH<n>::unit());
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
