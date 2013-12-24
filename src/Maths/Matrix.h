//
//  Matrix.h
//  Phantasma
//
//  Created by Thomas Harte on 21/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef Phantasma_Matrix_h
#define Phantasma_Matrix_h

class Matrix
{
	public:
		Matrix();
		Matrix(GLfloat *contents);
		
		static Matrix rotationMatrix(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
		static Matrix projectionMatrix(GLfloat yFieldOfView, GLfloat aspectRatio, GLfloat zNear, GLfloat zFar);
		static Matrix translationMatrix(GLfloat x, GLfloat y, GLfloat z);

		Matrix getRightOrthogonalInverse();

		GLfloat contents[16];
};

Matrix operator *(Matrix &, Matrix &);

#endif
