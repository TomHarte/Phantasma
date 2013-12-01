#ifndef __EBGF_SHADER_H
#define __EBGF_SHADER_H

#include "ebgf_GLExts.h"
#include "ebgf_ResourceStore.h"

class CShaderPair: public CResource
{
	public:
		CShaderPair();
		~CShaderPair();
		bool Open(const char *Vertex, const char *Fragment);
		bool CreateFromListing(const char *Vertex, const char *Fragment);

		bool Activate();

		void Backup();
		bool Restore();
		
		GLhandleARB GetVertexHandle() {return VertexID;}
		GLhandleARB GetFragmentHandle() {return FragmentID;}
		GLhandleARB GetProgramHandle() {return ProgramID;}

	private:
		GLchar *Vertex, *Fragment;
		GLhandleARB VertexID, FragmentID;
		GLhandleARB ProgramID;
};

#endif
