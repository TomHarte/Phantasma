#include "ebgf_Shader.h"

CShaderPair::CShaderPair()
{
	Vertex = Fragment = NULL;
}

CShaderPair::~CShaderPair()
{
	Backup();
	if(Vertex)
	{
		free(Vertex); Vertex = NULL;
	}
	if(Fragment)
	{
		free(Fragment); Fragment = NULL;
	}
	EBGF_ReturnResource(this, false);
}

void CShaderPair::Backup()
{
	glDeleteProgramARB(ProgramID);
	glDeleteShaderARB(FragmentID);
	glDeleteShaderARB(VertexID);
}

bool CShaderPair::Restore()
{
	if(!(AvailableGLExtensions&GLEXT_SHADEROBJS)) return false;

	// create shaders
	FragmentID = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB(FragmentID, 1, (const GLchar **)&Fragment, NULL);
	glCompileShaderARB(FragmentID);

	VertexID = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	glShaderSourceARB(VertexID, 1, (const GLchar **)&Vertex, NULL);
	glCompileShaderARB(VertexID);

	ProgramID = glCreateProgramObjectARB();
	glAttachObjectARB(ProgramID, VertexID);
	glAttachObjectARB(ProgramID, FragmentID);
	glLinkProgramARB(ProgramID);

	return true;
}

bool CShaderPair::CreateFromListing(const char *Vtx, const char *Fg)
{
	Vertex = strdup(Vtx);
	Fragment = strdup(Fg);
	Filename = (char *)malloc(20);
	sprintf(Filename, "%d", (int)this);
	__EBGF_StoreHash(this);
	return Restore();
}

bool CShaderPair::Activate()
{
	if(!(AvailableGLExtensions&GLEXT_SHADEROBJS)) return false;
	glUseProgramObjectARB(ProgramID);
	return true;
}
