#include "ebgf_ResourceStore.h"
#include "SDL.h"

CResource::CResource()
{
	Filename = NULL;
	RefCount = 1;
}

CResource::~CResource()
{
	if(Filename) {free(Filename); Filename = NULL;}
}

bool CResource::Restore()
{
	return Open(Filename);
}

Uint16 __EBGF_GetHash(const char *string)
{
	if(!string) return 0;

	/* not a real hash... */
	Uint16 Result = 0;
	while(*string)
	{
		Result = (Result << 4) ^ (Result >> 12) ^ *string;
		string++;
	}

	return Result;
}

CResource *__EBGF_ResourceHashTable[65536];

void __EBGF_StoreHash(CResource *res)
{
	Uint16 Hash = __EBGF_GetHash(res->Filename);
	res->HashNext = __EBGF_ResourceHashTable[Hash];
	__EBGF_ResourceHashTable[Hash] = res;
}

CResource *__EBGF_FindPtr;

CResource *__EBGF_FindNextResource(const char *name)
{
	while(__EBGF_FindPtr)
	{
		if(!strcmp(name, __EBGF_FindPtr->Filename))
		{
			CResource *ret = __EBGF_FindPtr;
			__EBGF_FindPtr = __EBGF_FindPtr->HashNext;
			return ret;
		}
		__EBGF_FindPtr = __EBGF_FindPtr->HashNext;
	}
	return NULL;
}

CResource *__EBGF_FindFirstResource(const char *name)
{
	Uint16 Hash = __EBGF_GetHash(name);
	__EBGF_FindPtr = __EBGF_ResourceHashTable[Hash];
	return __EBGF_FindNextResource(name);
}

void __EBGF_UnlinkResource(CResource *Res)
{
	/* unlink CResource from hash list */
	Uint16 Hash = __EBGF_GetHash(Res->Filename);
	CResource **Nde = &__EBGF_ResourceHashTable[Hash];
	while(*Nde && (*Nde) != Res)
		Nde = &(*Nde)->HashNext;
	if(!(*Nde)) return;

	*Nde = (*Nde)->HashNext;
}

void EBGF_ReturnResource(CResource *r, bool dealloc)
{
	if(!r) return;
	if(!--r->RefCount)
	{
		__EBGF_UnlinkResource(r);
		if(dealloc) delete r;
	}
}

void __EBGF_CreateResourceList()
{
	int c = 65536;
	while(c--)
		__EBGF_ResourceHashTable[c] = NULL;
}

/* called at shutdown, kill entire list */
void __EBGF_DestroyResourceList()
{
	int c = 65536;
	while(c--)
	{
		while(__EBGF_ResourceHashTable[c])
		{
			CResource *Nxt = __EBGF_ResourceHashTable[c]->HashNext;
			delete __EBGF_ResourceHashTable[c];
			__EBGF_ResourceHashTable[c] = Nxt;
		}
	}
}

void __EBGF_BackupResources()
{
	int c = 65536;
	while(c--)
	{
		CResource *N = __EBGF_ResourceHashTable[c];
		while(N)
		{
			N->Backup();
			N = N->HashNext;
		}
	}
}

void __EBGF_RestoreResources()
{
	int c = 65536;
	while(c--)
	{
		CResource *N = __EBGF_ResourceHashTable[c];
		while(N)
		{
			N->Restore();
			N = N->HashNext;
		}
	}
}
