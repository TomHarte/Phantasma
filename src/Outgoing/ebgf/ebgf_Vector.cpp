/*
 *  ebgf_Vector.cpp
 *  EBGFApp
 *
 *  Created by Thomas Harte on 10/02/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "ebgf_Vector.h"

GLfloat CVector::SQLength()
{
	return *this * *this;
}

CVector operator ^(CVector a, CVector b)
{
	CVector R;
	R.Data[0] = a.Data[1]*b.Data[2] - b.Data[1]*a.Data[2];
	R.Data[1] = a.Data[2]*b.Data[0] - b.Data[2]*a.Data[0];
	R.Data[2] = a.Data[0]*b.Data[1] - b.Data[0]*a.Data[1];
	return R;
}

CVector operator -(CVector a, CVector b)
{
	CVector R;
	R.Data[0] = a.Data[0] - b.Data[0];
	R.Data[1] = a.Data[1] - b.Data[1];
	R.Data[2] = a.Data[2] - b.Data[2];
	return R;
}

CVector operator +(CVector a, CVector b)
{
	CVector R;
	R.Data[0] = a.Data[0] + b.Data[0];
	R.Data[1] = a.Data[1] + b.Data[1];
	R.Data[2] = a.Data[2] + b.Data[2];
	return R;
}

GLfloat operator *(const CVector a, const CVector b)
{
	return a.Data[0]*b.Data[0] + a.Data[1]*b.Data[1] + a.Data[2]*b.Data[2];
}

CVector operator *(CVector a, GLfloat b)
{
	CVector R;
	R.Data[0] = a.Data[0] * b;
	R.Data[1] = a.Data[1] * b;
	R.Data[2] = a.Data[2] * b;
	return R;
}

CVector operator /(CVector a, GLfloat b)
{
	CVector R;
	R.Data[0] = a.Data[0] / b;
	R.Data[1] = a.Data[1] / b;
	R.Data[2] = a.Data[2] / b;
	return R;
}
