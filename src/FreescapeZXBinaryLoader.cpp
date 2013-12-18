#include "Freescape.h"

#define Get8(v) v = *DPtr; DPtr++;
#define Get16(v) v = *DPtr; DPtr++; v |= (*DPtr) << 8; DPtr++;

bool CFreescapeGame::OpenZXBinary(char *name, int Offset)
{
	float SpecCols[16][3] = {
		{0, 0, 0}, {0, 0, 0.75}, {0.75, 0, 0}, {0.75, 0, 0.75}, {0, 0.75, 0}, {0, 0.75, 0.75}, {0.75, 0.75, 0}, {0.75, 0.75, 0.75},
		{0, 0, 0}, {0, 0, 1}, {1, 0, 0}, {1, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 1, 0}, {1, 1, 1}
	};

	SetVariableMode(true);

	FILE *b = fopen(name, "rb");
	if(!b) return false;

	Uint8 Buffer[49152];
	fread(Buffer, 1, 49152, b);
	fclose(b);

	Uint8 *Base = &Buffer[Offset - 16384];

	Uint8 NumRooms = Base[0];

	Start.Area = Base[3];
	Start.Entrance = Base[4];

//	Uint8 InitJetEnergy, InitJetShield, InitProbeEnergy, InitProbeShield;
//	Get8(InitJetEnergy); Get8(InitJetShield);
//	Get8(InitProbeEnergy); Get8(InitProbeShield);

	Uint16 GlobalByteCodeTable;
	GlobalByteCodeTable = Base[0x48] | (Base[0x49] << 8);

	printf("GBCT: %d\n", GlobalByteCodeTable);

	Uint8 *ConditionPointer = &Base[GlobalByteCodeTable];
	int NumConditions = ConditionPointer[0]; ConditionPointer++;
	printf("%d global conditions\n", NumConditions);
	while(NumConditions--)
	{
		CCondition *NewCon = GetConditionZXBinary(&ConditionPointer[1], ConditionPointer[0], true);
		GlobalArea->AddCondition(NumConditions, NewCon);
		printf("\t%d bytes\n", *ConditionPointer);
		ConditionPointer += *ConditionPointer;
	}

	Uint16 Offsets[35];
	memset(Offsets, 0, sizeof(Uint16)*35);

	for(int c = 0; c < NumRooms; c++)
	{
		Offsets[c] = Base[0xc8 + (c*2)] | (Base[0xc9 + (c*2)] << 8);
	}

	for(int c = 0; c < 35; c++)
	{
		if(Offsets[c])
		{
			Uint8 *IPtr = &Base[Offsets[c]];
			Uint16 BytePointer;

			int AreaId = IPtr[2];
			CArea *CurArea = Areas[AreaId] = new CArea(this);
			BytePointer = IPtr[3] | IPtr[4] << 8;
			CurArea->SetScale(IPtr[5]);

			Set16PaletteGradient(SpecCols[IPtr[8]&15], SpecCols[IPtr[9]&15]);

			/* get area conditions */
			Uint8 *ConditionPointer = &IPtr[BytePointer];
			int NumConditions = ConditionPointer[0]; ConditionPointer++;
			while(NumConditions--)
			{
				CCondition *NewCon = GetConditionZXBinary(&ConditionPointer[1], ConditionPointer[0], true);
				CurArea->AddCondition(NumConditions, NewCon);
				ConditionPointer += *ConditionPointer;
			}

			/* get objects */
			int NumObjects = IPtr[1];
			Uint8 *ObjPtr = &IPtr[(Offset == OFFSET_TOTALECLIPSE) ? 15 : 25];
			while(NumObjects--)
			{
				Uint8 ObjType = ObjPtr[0]&0x1f, ObjLen, ObjId = ObjPtr[7];
				ObjLen = ObjPtr[8];

				if(ObjType == 0)
				{
					// entrance, not object
					CArea::Entrance *NewE = new CArea::Entrance;

					NewE->Pos[0] = ObjPtr[1]*32;
					NewE->Pos[1] = ObjPtr[2]*32;
					NewE->Pos[2] = ObjPtr[3]*32;
					NewE->Rotation[0] = ObjPtr[4]*5;
					NewE->Rotation[1] = ObjPtr[5]*5;
					NewE->Rotation[2] = ObjPtr[6]*5;
					ObjPtr += ObjLen;

					CurArea->SetEntrance(ObjId, NewE);
				}
				else
				{
					CObject *NewObj = new CObject(this);
					float Pos[3], Size[3];
					int EndPointer;

					Pos[0] = ObjPtr[1]*32; Pos[1] = ObjPtr[2]*32; Pos[2] = ObjPtr[3]*32;
					Size[0] = ObjPtr[4]*32; Size[1] = ObjPtr[5]*32; Size[2] = ObjPtr[6]*32;

					switch(ObjType)
					{
						default: delete NewObj; NewObj = NULL; break;	//unhandled
						case 1:
						{
							NewObj->SetType(CUBOID);

							CColour col;
							col.Entry = ObjPtr[9]>> 4; MapColour(&col); NewObj->SetColour(1, col);
							col.Entry = ObjPtr[9]&0x0f; MapColour(&col); NewObj->SetColour(0, col);
							col.Entry = ObjPtr[10] >> 4; MapColour(&col); NewObj->SetColour(3, col);
							col.Entry = ObjPtr[10]&0x0f; MapColour(&col); NewObj->SetColour(2, col);
							col.Entry = ObjPtr[11]>> 4; MapColour(&col); NewObj->SetColour(5, col);
							col.Entry = ObjPtr[11]&0x0f; MapColour(&col); NewObj->SetColour(4, col);
							EndPointer = 12;
						}
						break;	//cube, colours

						case 3:
						{
							NewObj->SetType(RECTANGLE);

							CColour col;
							col.Entry = ObjPtr[9]>> 4; MapColour(&col); NewObj->SetColour(1, col);
							col.Entry = ObjPtr[9]&0x0f; MapColour(&col); NewObj->SetColour(0, col);
							EndPointer = 10;
						}
						break;
						case 4:
						case 5:
						case 6:
						case 7:
						case 8:
						case 9:
						{
							NewObj->SetType(PYRAMID);
							NewObj->SetPyramidType(ObjType-3);

							CColour col;
							col.Entry = ObjPtr[9]>> 4; MapColour(&col); NewObj->SetColour(1, col);
							col.Entry = ObjPtr[9]&0x0f; MapColour(&col); NewObj->SetColour(0, col);
							col.Entry = ObjPtr[10] >> 4; MapColour(&col); NewObj->SetColour(3, col);
							col.Entry = ObjPtr[10]&0x0f; MapColour(&col); NewObj->SetColour(2, col);
							col.Entry = ObjPtr[11]>> 4; MapColour(&col); NewObj->SetColour(5, col);
							col.Entry = ObjPtr[11]&0x0f; MapColour(&col); NewObj->SetColour(4, col);

							float ApexStart[2], ApexEnd[2];
							ApexStart[0] = ObjPtr[12]*32; ApexStart[1] = ObjPtr[13]*32;
							ApexEnd[0] = ObjPtr[14]*32; ApexEnd[1] = ObjPtr[15]*32;
							NewObj->SetApex(ApexStart, ApexEnd);
							EndPointer = 16;
						}
						break;
						case 10:
						case 11: 
						case 12:
						case 13:
						case 14:
							NewObj->SetType(PLANAR);
							NewObj->SetNumSides(ObjType - 8);
						break;
						case 2:
							NewObj->SetType(SENSOR);
							NewObj->SetSensorStats(ObjPtr[9] | (ObjPtr[10] << 8), ObjPtr[11] | (ObjPtr[12] << 8), ObjPtr[13], ObjId);
							printf("Sensor, speed %d range %d flags %02x\n", ObjPtr[9] | (ObjPtr[10] << 8), ObjPtr[11] | (ObjPtr[12] << 8), ObjPtr[13]);
							EndPointer = 14;
						break;
					}

					if(NewObj)
					{
						if(ObjPtr[0]&0xd0) NewObj->SetDefaultVisible(false);

						if(NewObj->GetType() == PLANAR)
						{
							NewObj->SetDefaultVisible(true);
							CColour col;
							col.Entry = ObjPtr[9]>> 4; MapColour(&col); NewObj->SetColour(1, col);
							col.Entry = ObjPtr[9]&0x0f; MapColour(&col); NewObj->SetColour(0, col);

							float Low[3], High[3];

							for(int c = 0; c < NewObj->GetNumSides(); c++)
							{
								float Pos[3];
								Pos[0] = ObjPtr[10 + (c*3)] *32; 
								Pos[1] = ObjPtr[11 + (c*3)] *32; 
								Pos[2] = ObjPtr[12 + (c*3)] *32;
								if(!c || Pos[0] < Low[0]) Low[0] = Pos[0];
								if(!c || Pos[0] > High[0]) High[0] = Pos[0];
								if(!c || Pos[1] < Low[1]) Low[1] = Pos[1];
								if(!c || Pos[1] > High[1]) High[1] = Pos[1];
								if(!c || Pos[2] < Low[2]) Low[2] = Pos[2];
								if(!c || Pos[2] > High[2]) High[2] = Pos[2];
							}
							for(int c = 0; c < NewObj->GetNumSides(); c++)
							{
								float Pos[3];
								Pos[0] = (ObjPtr[10 + (c*3)] *32) - Low[0];
								Pos[1] = (ObjPtr[11 + (c*3)] *32) - Low[1];
								Pos[2] = (ObjPtr[12 + (c*3)] *32) - Low[2];
								NewObj->SetVertex(c, Pos);
							}
							EndPointer = 10 + NewObj->GetNumSides()*3;

							Pos[0] = Low[0]; Pos[1] = Low[1]; Pos[2] = Low[2];
							Size[0] = High[0] - Low[0]; Size[1] = High[1] - Low[1]; Size[2] = High[2] - Low[2];
						}

						NewObj->SetLocation(Pos, Size);
						CurArea->AddObject(ObjId, NewObj);

						if(EndPointer != ObjLen)
						{
							CCondition *C = GetConditionZXBinary(&ObjPtr[EndPointer], ObjLen - EndPointer, AreaId == 24);
							NewObj->SetCondition(C);
						}
					}

					ObjPtr += ObjLen;
				}
			}
		}
	}

	CrouchHeight = 16;
	MaxClimb = 32;
	Start.Height = 64;//Areas[Start.Area]->GetEntrance(Start.Entrance)->Pos[1];
	Start.ResetCondition = 0;

	Assemble();
	return true;
}
