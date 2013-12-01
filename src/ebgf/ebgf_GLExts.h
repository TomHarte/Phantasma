#ifndef __EBGF_GLEXTS_H
#define __EBGF_GLEXTS_H

#include "SDL.h"
#include "SDL_opengl.h"

#define GLEXT_VBO			0x001	//GL_ARB_vertex_buffer_object
#define GLEXT_MULTITEX		0x002	//GL_ARB_multitexture
#define GLEXT_TEXCOMBINE	0x004	//GL_ARB_texture_env_combine
#define GLEXT_TEXDOT3		0x008	//GL_ARB_texture_env_dot3
#define GLEXT_TEXCOMPRESS	0x010	//GL_ARB_texture_compression
#define GLEXT_CUBEMAP		0x020	//GL_ARB_texture_cube_map
#define GLEXT_DEPTHTEX		0x040	//GL_ARB_depth_texture
#define GLEXT_SHADOW		0x080	//GL_ARB_shadow
#define GLEXT_SHADEROBJS	0x100	//GL_ARB_shader_objects
#define GLEXT_SHADELANG100	0x200	//GL_ARB_shading_language_100
#define GLEXT_VERTEXSHADER	0x400	//GL_ARB_vertex_shader
#define GLEXT_PIXELSHADER	0x800	//GL_ARB_fragment_shader
//#define GLEX

extern Uint32 AvailableGLExtensions;

/* vertex buffer object */
#ifndef __APPLE__
extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;
extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
extern PFNGLISBUFFERARBPROC glIsBufferARB;
#else
#define glBindBufferARB		glBindBuffer
#define glGenBuffersARB		glGenBuffers
#define glBufferDataARB		glBufferData
#define glDeleteBuffersARB	glDeleteBuffers
#define glIsBufferARB		glIsBuffer
#endif

/* multi texturing */
#ifndef __APPLE__
extern PFNGLACTIVETEXTUREPROC glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTextureARB;
extern PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;
extern PFNGLMULTITEXCOORD2FVPROC glMultiTexCoord2fv;
#else
#define glActiveTextureARB			glActiveTexture
#define glClientActiveTextureARB	glClientActiveTexture
#define glMultiTexCoord2fARB		glMultiTexCoord2f
#define glMultiTexCoord2fvARB		glMultiTexCoord2fv
#endif

/* shaders */
#ifndef __APPLE__
#else
#define glCreateShaderObjectARB		glCreateShader
#define glShaderSourceARB			glShaderSource
#define GLhandleARB					GLuint
#define glCompileShaderARB			glCompileShader
#define glGetObjectParameterivARB	glGetProgramiv
#define glCreateProgramObjectARB	glCreateProgram
#define glAttachObjectARB			glAttachShader
#define glLinkProgramARB			glLinkProgram
#define glUseProgramObjectARB		glUseProgram
#define glDeleteShaderARB			glDeleteShader
#define glDeleteProgramARB			glDeleteProgram
#define glGetAttribLocationARB		glGetAttribLocation
#define glVertexAttribPointerARB	glVertexAttribPointer
#define glEnableVertexAttribArrayARB	glEnableVertexAttribArray
#define glDisableVertexAttribArrayARB	glDisableVertexAttribArray
#define glGetUniformLocationARB		glGetUniformLocation
#define glUniform1iARB				glUniform1i
#endif

/* function to fill in all of the above */
void __ebgf_GetExtensions();

#endif
