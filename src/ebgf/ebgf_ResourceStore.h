#ifndef __EBGF_RESOURCESTORE_H
#define __EBGF_RESOURCESTORE_H

class CResource
{
	public:
		CResource();
		virtual ~CResource();

		/* all for internal EBGF use only! */
		CResource *HashNext;
		int RefCount;
		char *Filename;
		virtual void Backup() = 0;
		virtual bool Restore();
		virtual bool Open(const char *name) {return false;};
};

void EBGF_ReturnResource(CResource *, bool dealloc = true);

/* for internal use only */
void __EBGF_BackupResources();
void __EBGF_RestoreResources();
void __EBGF_DestroyResourceList();
void __EBGF_CreateResourceList();

void __EBGF_StoreHash(CResource *res);
CResource *__EBGF_FindFirstResource(const char *name);
CResource *__EBGF_FindNextResource(const char *name);

#endif
