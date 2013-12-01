#include "ebgf_Matrix.h"
#include <math.h>
#include <string.h>

CMatrix::CMatrix()
{
	/* load identity */
	Contents[0] = Contents[5] = Contents[10] = Contents[15] = 1;
	Contents[1] = Contents[2] = Contents[3] = 
	Contents[4] = Contents[6] = Contents[7] = 
	Contents[8] = Contents[9] = Contents[11] = 
	Contents[12] = Contents[13] = Contents[14] = 0;
}

void CMatrix::Multiply()
{
	glMultMatrixf(Contents);
}

void CMatrix::MultiplyInverse()
{
	/* invert matrix and multiply by that - good for positioning cameras */
	GLfloat Inverse[16];

	/* transpost 3x3 */
	Inverse[0] = Contents[0]; Inverse[1] = Contents[4]; Inverse[2] = Contents[8];
	Inverse[4] = Contents[1]; Inverse[5] = Contents[5]; Inverse[6] = Contents[9];
	Inverse[8] = Contents[2]; Inverse[9] = Contents[6]; Inverse[10] = Contents[10];
	
	/* bottom = 0, 0, 0, 1 */
	Inverse[3] = Inverse[7] = Inverse[11] = 0; Inverse[15] = 1;

	/* figure out translation ... */
	Inverse[12] = -(Inverse[0] * Contents[12] + Inverse[4] * Contents[13] + Inverse[8] * Contents[14]);
	Inverse[13] = -(Inverse[1] * Contents[12] + Inverse[5] * Contents[13] + Inverse[9] * Contents[14]);
	Inverse[14] = -(Inverse[2] * Contents[12] + Inverse[6] * Contents[13] + Inverse[10] * Contents[14]);

	glMultMatrixf(Inverse);
}

void CMatrix::Translate(float x, float y, float z)
{
	/* this works in local space */
	Contents[12] += x*Contents[0] + y*Contents[4] + z*Contents[8];
	Contents[13] += x*Contents[1] + y*Contents[5] + z*Contents[9];
	Contents[14] += x*Contents[2] + y*Contents[6] + z*Contents[10];
}

void CMatrix::Transpose()
{
	GLfloat Inverse[16];
	Inverse[1] = Contents[4]; Inverse[2] = Contents[8];
	Inverse[4] = Contents[1]; Inverse[6] = Contents[9];
	Inverse[8] = Contents[2]; Inverse[9] = Contents[6];

	Contents[1] = Inverse[1]; Contents[2] = Inverse[2];
	Contents[4] = Inverse[4]; Contents[6] = Inverse[6];
	Contents[8] = Inverse[8]; Contents[9] = Inverse[9];
}

void CMatrix::Rotate(float angle, float x, float y, float z)
{
	/* compose rotation matrix */
	GLfloat RotMatrix[16], Result[16], Cos, Sine;

	angle = (angle * M_PI) / 180.0f;
	Cos = cos(angle);
	Sine = sin(angle);

	float l = x*x + y*y + z*z;
	if(l > 1.01f || l < 0.99f)
	{
		float m = 1.0f / sqrt(l);
		x *= m; y *= m; z *= m;
	}

	/* formula lifted from man page for glRotatef */
	RotMatrix[0] = (x*x*(1 - Cos)) + Cos;
	RotMatrix[1] = (y*x*(1 - Cos)) + z*Sine;
	RotMatrix[2] = (x*z*(1 - Cos)) - y*Sine;

	RotMatrix[4] = (x*y*(1 - Cos)) - z*Sine;
	RotMatrix[5] = (y*y*(1 - Cos)) + Cos;
	RotMatrix[6] = (y*z*(1 - Cos)) + x*Sine;

	RotMatrix[8] = (x*z*(1 - Cos)) + y*Sine;
	RotMatrix[9] = (y*z*(1 - Cos)) - x*Sine;
	RotMatrix[10] = (z*z*(1 - Cos)) + Cos;

	/* do multiplication */
	Result[0] = Contents[0] * RotMatrix[0] +	Contents[4] * RotMatrix[1] +	Contents[8] * RotMatrix[2];
	Result[4] = Contents[0] * RotMatrix[4] +	Contents[4] * RotMatrix[5] +	Contents[8] * RotMatrix[6];
	Result[8] = Contents[0] * RotMatrix[8] +	Contents[4] * RotMatrix[9] +	Contents[8] * RotMatrix[10];

	Result[1] = Contents[1] * RotMatrix[0] +	Contents[5] * RotMatrix[1] +	Contents[9] * RotMatrix[2];
	Result[5] = Contents[1] * RotMatrix[4] +	Contents[5] * RotMatrix[5] +	Contents[9] * RotMatrix[6];
	Result[9] = Contents[1] * RotMatrix[8] +	Contents[5] * RotMatrix[9] +	Contents[9] * RotMatrix[10];

	Result[2] = Contents[2] * RotMatrix[0] +	Contents[6] * RotMatrix[1] +	Contents[10] * RotMatrix[2];
	Result[6] = Contents[2] * RotMatrix[4] +	Contents[6] * RotMatrix[5] +	Contents[10] * RotMatrix[6];
	Result[10] = Contents[2] * RotMatrix[8] +	Contents[6] * RotMatrix[9] +	Contents[10] * RotMatrix[10];

	Result[3] = Result[7] = Result[11] = 0;

	/* apply fixes */
	float len = 1.0f / sqrt(Result[0]*Result[0] + Result[1]*Result[1] + Result[2]*Result[2]);
	Result[0] *= len; Result[1] *= len; Result[2] *= len;
	len = 1.0f / sqrt(Result[4]*Result[4] + Result[5]*Result[5] + Result[6]*Result[6]);
	Result[4] *= len; Result[5] *= len; Result[6] *= len;
	len = 1.0f / sqrt(Result[8]*Result[8] + Result[9]*Result[9] + Result[10]*Result[10]);
	Result[8] *= len; Result[9] *= len; Result[10] *= len;

	/* copy back */
	memcpy(Contents, Result, sizeof(GLfloat)*12);
}

void CMatrix::Construct(CM_ConstructionType ct, CVector a, CVector b)
{
	CVector c = a ^ b;
	switch(ct)
	{
		case CM_UPRIGHT:
			Contents[0] = b.Data[0]; Contents[4] = b.Data[1]; Contents[8] = b.Data[2];
			Contents[1] = a.Data[0]; Contents[5] = a.Data[1]; Contents[9] = a.Data[2];
			Contents[2] = c.Data[0]; Contents[6] = c.Data[1]; Contents[10] = c.Data[2];
		break;
		case CM_UPFRONT:
			Contents[0] = c.Data[0]; Contents[4] = c.Data[1]; Contents[8] = c.Data[2];
			Contents[1] = a.Data[0]; Contents[5] = a.Data[1]; Contents[9] = a.Data[2];
			Contents[2] = b.Data[0]; Contents[6] = b.Data[1]; Contents[10] = b.Data[2];
		break;
		case CM_RIGHTFRONT:
			Contents[0] = a.Data[0]; Contents[4] = a.Data[1]; Contents[8] = a.Data[2];
			Contents[1] = c.Data[0]; Contents[5] = c.Data[1]; Contents[9] = c.Data[2];
			Contents[2] = b.Data[0]; Contents[6] = b.Data[1]; Contents[10] = b.Data[2];
		break;
	}
}

CVector operator *(CMatrix m, CVector v)
{
	CVector R(
		m.Contents[0] * v.Data[0] + m.Contents[4] * v.Data[1] + m.Contents[8] * v.Data[2] + m.Contents[12],
		m.Contents[1] * v.Data[0] + m.Contents[5] * v.Data[1] + m.Contents[9] * v.Data[2] + m.Contents[13],
		m.Contents[2] * v.Data[0] + m.Contents[6] * v.Data[1] + m.Contents[10] * v.Data[2] + m.Contents[14]);
	return R;
}

void CMatrix::Construct(CVector a, CVector b, CVector c)
{
	Contents[0] = a.Data[0]; Contents[4] = a.Data[1]; Contents[8] = a.Data[2];
	Contents[1] = b.Data[0]; Contents[5] = b.Data[1]; Contents[9] = b.Data[2];
	Contents[2] = c.Data[0]; Contents[6] = c.Data[1]; Contents[10] = c.Data[2];
}
