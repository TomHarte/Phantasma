#include "ebgf_GLExts.h"
#include <string.h>
#include "SDL.h"

Uint32 AvailableGLExtensions;
#ifndef __APPLE__
PFNGLBINDBUFFERARBPROC glBindBufferARB;
PFNGLGENBUFFERSARBPROC glGenBuffersARB;
PFNGLBUFFERDATAARBPROC glBufferDataARB;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
PFNGLISBUFFERARBPROC glIsBufferARB;
#endif

bool isExtensionSupported(const char *extension)
{
	const GLubyte *extensions = NULL;
	const GLubyte *start;
	GLubyte *where, *terminator;

	/* Extension names should not have spaces. */
	where = (GLubyte *) strchr(extension, ' ');
	if(where || *extension == '\0') return false;
	extensions = glGetString(GL_EXTENSIONS);

	/*	It takes a bit of care to be fool-proof about parsing the
		OpenGL extensions string. Don't be fooled by sub-strings,
		etc. */
	start = extensions;
	while(1)
	{
		where = (GLubyte *)strstr((const char *) start, extension);
		if(!where)
			return false;

		terminator = where + strlen(extension);
		if(where == start || *(where - 1) == ' ')
			if(*terminator == ' ' || *terminator == '\0')
				return true;

		start = terminator;
	}
	return false;
}

void __ebgf_GetExtensions()
{
	AvailableGLExtensions = 0;
	if(isExtensionSupported("GL_ARB_vertex_buffer_object"))	AvailableGLExtensions |= GLEXT_VBO;
	if(isExtensionSupported("GL_ARB_multitexture"))			AvailableGLExtensions |= GLEXT_MULTITEX;
	if(isExtensionSupported("GL_ARB_texture_env_combine"))	AvailableGLExtensions |= GLEXT_TEXCOMBINE;
	if(isExtensionSupported("GL_ARB_texture_env_dot3"))		AvailableGLExtensions |= GLEXT_TEXDOT3;
	if(isExtensionSupported("GL_ARB_texture_compression"))	AvailableGLExtensions |= GLEXT_TEXCOMPRESS;
	if(isExtensionSupported("GL_ARB_texture_cube_map"))		AvailableGLExtensions |= GLEXT_CUBEMAP;
	if(isExtensionSupported("GL_ARB_depth_texture"))		AvailableGLExtensions |= GLEXT_DEPTHTEX;
	if(isExtensionSupported("GL_ARB_shadow"))				AvailableGLExtensions |= GLEXT_SHADOW;
	if(isExtensionSupported("GL_ARB_shader_objects"))		AvailableGLExtensions |= GLEXT_SHADEROBJS;

//	printf("%02x\n", AvailableGLExtensions);

#ifndef __APPLE__
	if(AvailableGLExtensions&GLEXT_VBO)
	{
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");
		glIsBufferARB = (PFNGLISBUFFERARBPROC)SDL_GL_GetProcAddress("glIsBufferARB");
	}
	else
	{
		glGenBuffersARB = NULL;
		glBindBufferARB = NULL;
		glBufferDataARB = NULL;
		glDeleteBuffersARB = NULL;
		glIsBufferARB = NULL;
	}

	if(AvailableGLExtensions&GLEXT_MULTITEX)
	{
		glActiveTextureARB = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
		glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glClientActiveTextureARB");
		glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
		glMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");
	}
	else
	{
		glActiveTextureARB = NULL;
		glClientActiveTextureARB = NULL;
		glMultiTexCoord2fARB = NULL;
		glMultiTexCoord2fvARB = NULL;
	}
#endif
}
