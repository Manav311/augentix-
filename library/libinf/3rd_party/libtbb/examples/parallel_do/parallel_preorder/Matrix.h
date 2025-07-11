/*
    Copyright 2005-2016 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks. Threading Building Blocks is free software;
    you can redistribute it and/or modify it under the terms of the GNU General Public License
    version 2  as  published  by  the  Free Software Foundation.  Threading Building Blocks is
    distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See  the GNU General Public License for more details.   You should have received a copy of
    the  GNU General Public License along with Threading Building Blocks; if not, write to the
    Free Software Foundation, Inc.,  51 Franklin St,  Fifth Floor,  Boston,  MA 02110-1301 USA

    As a special exception,  you may use this file  as part of a free software library without
    restriction.  Specifically,  if other files instantiate templates  or use macros or inline
    functions from this file, or you compile this file and link it with other files to produce
    an executable,  this file does not by itself cause the resulting executable to be covered
    by the GNU General Public License. This exception does not however invalidate any other
    reasons why the executable file might be covered by the GNU General Public License.
*/

class Matrix {
	static const int n = 20;
	float array[n][n];

    public:
	Matrix()
	{
	}
	Matrix(float z)
	{
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				array[i][j] = i == j ? z : 0;
	}
	friend Matrix operator-(const Matrix &x)
	{
		Matrix result;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				result.array[i][j] = -x.array[i][j];
		return result;
	}
	friend Matrix operator+(const Matrix &x, const Matrix &y)
	{
		Matrix result;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				result.array[i][j] = x.array[i][j] + y.array[i][j];
		return result;
	}
	friend Matrix operator-(const Matrix &x, const Matrix &y)
	{
		Matrix result;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				result.array[i][j] = x.array[i][j] - y.array[i][j];
		return result;
	}
	friend Matrix operator*(const Matrix &x, const Matrix &y)
	{
		Matrix result(0);
		for (int i = 0; i < n; ++i)
			for (int k = 0; k < n; ++k)
				for (int j = 0; j < n; ++j)
					result.array[i][j] += x.array[i][k] * y.array[k][j];
		return result;
	}
};
