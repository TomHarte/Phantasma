#include "Freescape.h"

#define Get8(v) v = *DPtr; DPtr++;
#define Get16(v) v = *DPtr; DPtr++; v |= (*DPtr) << 8; DPtr++;

CFreescapeGame::CCondition *CFreescapeGame::GetConditionZXBinary(Uint8 *Ptr, int Len, bool print )
{
	Uint8 *OPtr = Ptr;
	CCondition *NewC = new CCondition;
	bool Shot = (Ptr[0]&0x80) ? true : false;

	if(Shot)
	{
		NewC->AddInstruction("if shot? then ");
		if(print) printf("\tif shot? then\n");
	}
	else
	{
		NewC->AddInstruction("if collided? then ");
		if(print) printf("\tif collided? then\n");
	}

	int c = 0;
	while(c < Len)
	{
		bool NewShot = (Ptr[c]&0x80) ? true : false;

		if(Shot != NewShot)
		{
			NewC->AddInstruction("endif ");
			if(print) printf("\tendif\n");
			if(NewShot)
			{
				NewC->AddInstruction("if shot? then ");
				if(print) printf("\tif shot? then\n");
			}
			else
			{
				NewC->AddInstruction("if collided? then ");
				if(print) printf("\tif collided? then\n");
			}
		}
		Shot = NewShot;

		bool Understood = true;
		char NewCommand[256];
		Uint8 Command;
		Command = Ptr[c]&0x1f; c++;
		sprintf(NewCommand, "");
		switch(Command)
		{
			default:
			{
				printf("unhandled %d:\n\t", Command);
				for(int c = 0; c < Len; c++)
					printf("%d ", OPtr[c]&0x1f);
				printf("\n");
				Understood = false;
			}
			break;
			case 0:	//seems to be NOP?
			break;
			case 1:
			{
				int Val = Ptr[c];
				Val |= Ptr[c+1] << 8;
				Val |= Ptr[c+2] << 16; c += 3;
				sprintf(NewCommand, "addvar (%d, v%d) ", Val, VAR_SCORE);
			}
			break;
			case 2:
				sprintf(NewCommand, "addvar (%d, v%d) ", Ptr[c], VAR_ENERGY); c++;
			break;
			case 3:
				sprintf(NewCommand, "togvis (%d) ", Ptr[c]); c++;
			break;
			case 4:
				sprintf(NewCommand, "vis (%d) ", Ptr[c]); c++;
			break;
			case 5:
				sprintf(NewCommand, "invis (%d) ", Ptr[c]); c++;
			break;
			case 6:
				sprintf(NewCommand, "togvis (%d,%d) ", Ptr[c+1], Ptr[c]); c+=2;
			break;
			case 7:
				sprintf(NewCommand, "vis (%d,%d) ", Ptr[c+1], Ptr[c]); c+=2;
			break;
			case 8:
				sprintf(NewCommand, "invis (%d,%d) ", Ptr[c+1], Ptr[c]); c+=2;
			break;
			case 9:
				sprintf(NewCommand, "addvar (1,v%d) ", Ptr[c]); c++;
			break;
			case 10:
				sprintf(NewCommand, "subvar (1,v%d) ", Ptr[c]); c++;
			break;
			case 11:
				sprintf(NewCommand, "if var!=? (v%d,%d) then end endif ", Ptr[c], Ptr[c+1]); c+=2;
			break;
			case 12:
				sprintf(NewCommand, "setbit (%d) ", Ptr[c]); c++;
			break;
			case 13:
				sprintf(NewCommand, "clrbit (%d) ", Ptr[c]); c++;
			break;
			case 14:
				sprintf(NewCommand, "if bit!=? (%d, %d) then end endif ", Ptr[c], Ptr[c+1]); c+=2;
			break;
			case 15:
				sprintf(NewCommand, "sound (%d) ", Ptr[c]); c++;
			break;
			case 16:
				sprintf(NewCommand, "destroy (%d) ", Ptr[c]); c++;
			break;
			case 17:
				sprintf(NewCommand, "destroy (%d,%d) ", Ptr[c+1], Ptr[c]); c+=2;
			break;
			case 18:
				sprintf(NewCommand, "goto (%d,%d) ", Ptr[c+1], Ptr[c]); c+=2;
			break;
			case 19:
				sprintf(NewCommand, "addvar (%d,v%d) ", Ptr[c], VAR_SHIELD); c++;
			break;
			case 20:
				sprintf(NewCommand, "setvar (%d,v%d) ", Ptr[c+1], Ptr[c]); c+=2;
			break;
			case 21:
				sprintf(NewCommand, "swapjet "); 
			break;
			case 25:
				c++;	//toggle border colours...
			break;
			case 26:
				sprintf(NewCommand, "redraw ", Ptr[c]);
			break;
			case 27:
				sprintf(NewCommand, "delay (%d) ", Ptr[c]); c++;
			break;
			case 28:
				sprintf(NewCommand, "syncsnd (%d) ", Ptr[c]); c++;
			break;
			case 29:
				sprintf(NewCommand, "togbit (%d) ", Ptr[c]); c++;
			break;
			case 30:
				sprintf(NewCommand, "if invis? (%d) then end endif ", Ptr[c]); c++;
			break;
			case 31:
				sprintf(NewCommand, "if vis? (%d) then end endif ", Ptr[c]); c++;
			break;
		}
		if(Understood) NewC->AddInstruction(NewCommand); else break;
		if(print) printf("\t%s\n", NewCommand);
	}

	NewC->AddInstruction("endif ");
	if(print) printf("\tendif\n\n");
	NewC->Compile(false, this);
	return NewC;
}

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
