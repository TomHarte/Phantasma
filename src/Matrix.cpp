#include "Matrix.h"
#include <math.h>
#include <string.h>

Matrix::Matrix()
{
	/* load identity */
	contents[0] = contents[5] = contents[10] = contents[15] = 1.0f;
	contents[1] = contents[2] = contents[3] = 
	contents[4] = contents[6] = contents[7] = 
	contents[8] = contents[9] = contents[11] = 
	contents[12] = contents[13] = contents[14] = 0.0f;
}

Matrix::Matrix(GLfloat *_contents)
{
	memcpy(contents, _contents, sizeof(GLfloat)*16);
}

Matrix Matrix::getRightOrthogonalInverse()
{
	/* invert matrix and multiply by that - good for positioning cameras */
	GLfloat inverse[16];

	/* transpost 3x3 */
	inverse[0] = contents[0]; inverse[1] = contents[4]; inverse[2] = contents[8];
	inverse[4] = contents[1]; inverse[5] = contents[5]; inverse[6] = contents[9];
	inverse[8] = contents[2]; inverse[9] = contents[6]; inverse[10] = contents[10];
	
	/* bottom = 0, 0, 0, 1 */
	inverse[3] = inverse[7] = inverse[11] = 0.0f; inverse[15] = 1.0f;

	/* figure out translation ... */
	inverse[12] = -(inverse[0] * contents[12] + inverse[4] * contents[13] + inverse[8] * contents[14]);
	inverse[13] = -(inverse[1] * contents[12] + inverse[5] * contents[13] + inverse[9] * contents[14]);
	inverse[14] = -(inverse[2] * contents[12] + inverse[6] * contents[13] + inverse[10] * contents[14]);

	return Matrix(inverse);
}

void Matrix::translateLocal(GLfloat x, GLfloat y, GLfloat z)
{
	contents[12] += x*contents[0] + y*contents[4] + z*contents[8];
	contents[13] += x*contents[1] + y*contents[5] + z*contents[9];
	contents[14] += x*contents[2] + y*contents[6] + z*contents[10];
}

void Matrix::translateGlobal(GLfloat x, GLfloat y, GLfloat z)
{
	contents[12] += x;
	contents[13] += y;
	contents[14] += z;
}

Matrix Matrix::rotationMatrix(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	/* compose rotation matrix, exactly as per glRotatef */
	GLfloat cosine, sine;

	angle = (GLfloat)((angle * M_PI) / 180.0f);
	cosine = cosf(angle);
	sine = sinf(angle);

	float length = x*x + y*y + z*z;
	if(length > 1.01f || length < 0.99f)
	{
		float multiplier = 1.0f / sqrtf(length);
		x *= multiplier; y *= multiplier; z *= multiplier;
	}

	GLfloat contents[16];
	contents[0] = (x*x*(1 - cosine)) + cosine;
	contents[1] = (y*x*(1 - cosine)) + z*sine;
	contents[2] = (x*z*(1 - cosine)) - y*sine;
	contents[3] = 0.0f;

	contents[4] = (x*y*(1 - cosine)) - z*sine;
	contents[5] = (y*y*(1 - cosine)) + cosine;
	contents[6] = (y*z*(1 - cosine)) + x*sine;
	contents[7] = 0.0f;

	contents[8] = (x*z*(1 - cosine)) + y*sine;
	contents[9] = (y*z*(1 - cosine)) - x*sine;
	contents[10] = (z*z*(1 - cosine)) + cosine;
	contents[11] = 0.0f;
	
	contents[12] = 0.0f;
	contents[13] = 0.0f;
	contents[14] = 0.0f;
	contents[15] = 1.0f;

	return Matrix(contents);
}

Matrix Matrix::projectionMatrix(GLfloat yFieldOfView, GLfloat aspectRatio, GLfloat zNear, GLfloat zFar)
{
	/* compose projection matrix, exactly as per gluPerspective */
	GLfloat cotangent = 1.0f / tanf(yFieldOfView * 0.5f);

	GLfloat contents[16];

	contents[0] = cotangent / aspectRatio;
	contents[1] = contents[2] = contents[3] = 0.0f;

	contents[4] = 0.0f;
	contents[5] = cotangent;
	contents[6] = contents[7] = 0.0f;
	
	contents[8] = contents[9] = 0.0f;
	contents[10] = (zFar + zNear) / (zNear - zFar);
	contents[11] = -1.0f;
	
	contents[12] = contents[13] = 0.0f;
	contents[14] = (2.0f * zFar * zNear) / (zNear - zFar);
	contents[15] = 0.0f;

	return Matrix(contents);
}

Matrix operator *(Matrix &left, Matrix &right)
{
	GLfloat result[16];

	/* do multiplication */
	result[0] = left.contents[0] * right.contents[0] +	left.contents[4] * right.contents[1] +	left.contents[8] * right.contents[2];
	result[4] = left.contents[0] * right.contents[4] +	left.contents[4] * right.contents[5] +	left.contents[8] * right.contents[6];
	result[8] = left.contents[0] * right.contents[8] +	left.contents[4] * right.contents[9] +	left.contents[8] * right.contents[10];

	result[1] = left.contents[1] * right.contents[0] +	left.contents[5] * right.contents[1] +	left.contents[9] * right.contents[2];
	result[5] = left.contents[1] * right.contents[4] +	left.contents[5] * right.contents[5] +	left.contents[9] * right.contents[6];
	result[9] = left.contents[1] * right.contents[8] +	left.contents[5] * right.contents[9] +	left.contents[9] * right.contents[10];

	result[2] = left.contents[2] * right.contents[0] +	left.contents[6] * right.contents[1] +	left.contents[10] * right.contents[2];
	result[6] = left.contents[2] * right.contents[4] +	left.contents[6] * right.contents[5] +	left.contents[10] * right.contents[6];
	result[10] = left.contents[2] * right.contents[8] +	left.contents[6] * right.contents[9] +	left.contents[10] * right.contents[10];

	result[3] = result[7] = result[11] =
	result[12] = result[13] = result[14] = 0.0f;
	result[15] = 1.0f;
	
	return Matrix(result);
}
