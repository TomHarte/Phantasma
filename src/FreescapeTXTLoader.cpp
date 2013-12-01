#include "Freescape.h"
#include <stdio.h>

/*
	Basic navigation things. Not too yuck, but quite yuck.
*/

char CFreescapeGame::GetC()
{
	if(charptr == 2)
	{
		char n = fgetc(input);
		if(n == '\r' && lastn == '\n')
			n = fgetc(input);

		n = tolower(n);
		charin[0] = charin[1]; charin[1] = n;
		charptr--;
	}
	lastn = charin[charptr]; charptr++;
	return lastn;
}

void CFreescapeGame::UnGetC()
{
	charptr--;
}

const char *CFreescapeGame::GetLine()
{
	static char LineBuffer[256];
	char *Ptr = LineBuffer;

	while(1)
	{
		char Newc = GetC();
		if(Newc == '\n') break;
		*Ptr++ = Newc;
	}
	*Ptr = '\0';

	return LineBuffer;
}

int CFreescapeGame::GetInt()
{
	char c;
	int Number = 0;
	int Multiplier = 1;
	GIBase = 10;

	// skip white space
	while(1)
	{
		c = GetC();
		if(!((c == ' ') || (c == '\t')))
			break;
	}

	// check for negative
	if(c == '-')
	{
		Multiplier = -1;
		c = GetC();
	}

	// compile number
	while(1)
	{
		Number = (Number * 10) + (c - '0');
		c = GetC();
		if(c < '0' || c > '9') break;
		GIBase *= 10;
	}
	UnGetC();
	return Number*Multiplier;
}

float CFreescapeGame::GetFloat()
{
	float R = (float)GetInt();
	char c = GetC();
	if(c != '.')
	{
		UnGetC();
		return R;
	}
	int Sub = GetInt();
	R += (float)Sub / (float)GIBase;
	return R;
}

bool CFreescapeGame::Expect(char *string)
{
	while(*string)
	{
		if(*string != GetC()) return false;
		string++;
	}
	return true;
}

void CFreescapeGame::SkipWhiteSpace(bool SkipNewLines)
{
	while(1)
	{
		char c = GetC();
		if(!((c == ' ' ) || (c == '\t') || (c == '\n' && SkipNewLines))) break;
	}
	UnGetC();
}

void CFreescapeGame::SkipLine()
{
	while(1)
	{
		char newc = GetC();
		if(newc == '\n') break;
	}
}

CFreescapeGame::CColour CFreescapeGame::GetColour()
{
	CColour col;
	col.Entry = GetInt() << 4;
	char c = GetC();
	if(c == 'x')
	{
		col.Entry |= GetInt();
	}
	else
		UnGetC();
	MapColour(&col);
	return col;
}

bool CFreescapeGame::Find(char *string)
{
	while(!feof(input))
	{
		char *t = string;
		while(*t != GetC());
		while(1)
		{
			t++;
			if(!*t) return true;
			char newc = GetC();
			if(*t != newc) break;
		}
		UnGetC();
	}
//	printf("\n");
	return false;
}

/*
	YUCK!
*/

bool CFreescapeGame::OpenTXT(char *name)
{
	SetVariableMode(false);

	input = fopen(name, "rt");
	if(!input) return false;
	
	lastn = 0; charptr = 2;

	if(!Expect("-------------------------------------------------------------------------------\n\n"))
	{
		fclose(input);
		return false;
	}

	while(!feof(input))
	{
		// Find start of next section
		Find("\n------ ");
		if(feof(input)) break;
		char Buffer[5];
		Buffer[0] = GetC(); Buffer[1] = GetC(); Buffer[2] = GetC(); Buffer[3] = GetC(); Buffer[4] = '\0';
		
		if(!strcmp(Buffer, "data"))		// DATA FILE HEADER
		{
			SkipLine();
			SkipLine();
			// don't care about window and scale
			Find("at start:");
			Find("area"); Start.Area = GetInt();
			Find("entrance"); Start.Entrance = GetInt();
			Find("step"); Start.Step = GetInt();
			Find("angle"); Start.Angle = GetInt();
			Find("vehicle"); Start.Vehicle = GetInt();
			Find("height"); Start.Height = GetInt();

			Find("execute global condition"); Start.ResetCondition = GetInt();

			Find("every"); Timer = GetFloat();
			
			Find("max activate distance"); MaxActivateDistance = GetInt();
			Find("max fall ability"); MaxFall = GetInt();
			Find("max climbing ability"); MaxClimb = GetInt();

			Find("areas:"); NumAreas = GetInt();
		}
		if(!strcmp(Buffer, "key "))		// KEY AND ICON DATA
		{
			SkipLine();
			SkipLine();
//			printf("KEY AND ICON DATA ignored\n");
			// ignored
		}
		if(!strcmp(Buffer, "instr"))	// INSTRUMENTS
		{
			SkipLine();
			SkipLine();
//			printf("INSTRUMENTS ignored\n");
		}
		if(!strcmp(Buffer, "glob"))		// GLOBAL CONDITIONS
		{
			SkipLine();
			SkipLine();

			int ProgNum = GetInt();
			while(1)
			{
				CCondition *NewCondition = new CCondition;

				// don't care about name
				SkipLine();
				char newc;

				while(1)
				{
					SkipWhiteSpace(false);
					newc = GetC();
					UnGetC();
					if(newc < 'a' || newc > 'z')
					{
						break;
					}
					const char *Buf = GetLine();
					NewCondition->AddInstruction(Buf);
				}
				NewCondition->Compile(false, this);
				GlobalArea->AddCondition(ProgNum, NewCondition);
				if(newc < '0' || newc > '9')
				{
					break;
				}
				ProgNum = GetInt();
			}
		}
		if(!strcmp(Buffer, "area"))		//AN AREA
		{
			int AreaNum;
			Find("number"); AreaNum = GetInt();
			SkipLine();
			SkipLine();

			delete Areas[AreaNum];
			CArea *CurArea = Areas[AreaNum] = new CArea(this);
//			printf("AREA %d\n", AreaNum);

			Find("scale:"); float Scale = GetInt(); CurArea->SetScale(Scale);

			CColour cols[2];
			Find("horizon colours:");
			cols[0] = GetColour(); GetC(); 
			cols[1] = GetColour();
			CurArea->SetColours(cols[0], cols[1]);
			
			while(1)
			{
				bool Handled = false;
				SkipWhiteSpace();
				char Buffer[5];
				Buffer[0] = GetC();
				if(Buffer[0] == '-')
				{
					UnGetC(); UnGetC();
					break;
				}
				Buffer[1] = GetC();
				Buffer[2] = GetC();
				Buffer[3] = GetC();
				Buffer[4] ='\0';
				
				if(!strcmp(Buffer, "loca"))
				{
//					printf("\tLOCAL CONDITIONS\n");
					Handled = true;
//					printf("\tLOCAL CONDITIONS\n");
//					SkipLine();
					SkipLine();
					SkipLine();
						char c = GetC();
						if(c == '\n') break;
						SkipWhiteSpace();

						int ProgNum = GetInt();
					while(1)
					{
//						printf("\t\t%d\n", ProgNum);
						// don't care about name
						SkipLine();

						// an empty animator. Useful.
						char newc;
						newc = GetC();
						if(newc == '\n')
						{
							UnGetC();
							break;
						}

						CCondition *NewCondition = new CCondition;
						while(1)
						{
							SkipWhiteSpace(false);
							newc = GetC();
							UnGetC();
							if(newc < 'a' || newc > 'z')
							{
								break;
							}
							const char *Buf = GetLine();
							NewCondition->AddInstruction(Buf);
						}
						NewCondition->Compile(false, this);
						CurArea->AddCondition(ProgNum, NewCondition);
						if(newc < '0' || newc > '9')
						{
							break;
						}
						ProgNum = GetInt();
					}
				}
				if(!strcmp(Buffer, "anim"))
				{
					Handled = true;
//					printf("\tANIMATORS\n");
					SkipLine();
					SkipLine();
					SkipLine();
					char c = GetC();
					if(c == '\n')
					{
						UnGetC();
						break;
					}
					SkipWhiteSpace();

					int AnimatorNum = GetInt();
					while(1)
					{
//						printf("\t\t%d\n", AnimatorNum);
						// don't care about name
						SkipLine();

						// an empty animator. Useful.
						char newc;
						newc = GetC();
						if(newc == '\n')
						{
							UnGetC();
							break;
						}
						
						CCondition *NewAnimator = new CCondition;
						while(1)
						{
							SkipWhiteSpace(false);
							newc = GetC();
							UnGetC();
							if(newc < 'a' || newc > 'z')
							{
								break;
							}
							const char *Buf = GetLine();
							
							if(!strncmp(Buf, "included objects:", strlen("included objects:")))
							{
//								printf("!!!!");
								do
									Buf = GetLine();
								while(!strncmp(Buf, "          ", strlen("          ")));
							}
							
							NewAnimator->AddInstruction(Buf);
						}
						NewAnimator->Compile(true, this);
						CurArea->AddAnimator(AnimatorNum, NewAnimator);
						if(newc < '0' || newc > '9')
						{
							break;
						}
						AnimatorNum = GetInt();
					}
//					UnGetC();
				}
				if(!strcmp(Buffer, "entr"))
				{
					Handled = true;
//					printf("\tENTRANCES\n");
					SkipLine();
					SkipLine();
					SkipLine();
					while(1)
					{
						char c = GetC();
						if(c == '\n') break;
						SkipWhiteSpace();
						int EntranceNum = GetInt();
						CArea::Entrance *NewE = new CArea::Entrance;

						Find("position:");
						NewE->Pos[0] = GetInt();		NewE->Pos[1] = GetInt();		NewE->Pos[2] = GetInt();
						Find("rotations");
						NewE->Rotation[0] = GetInt();	NewE->Rotation[1] = GetInt();	NewE->Rotation[2] = GetInt();

						CurArea->SetEntrance(EntranceNum, NewE);
						SkipLine();
					}
					UnGetC();
				}
				if(!strcmp(Buffer, "obje"))
				{
					Handled = true;
//					printf("\tOBJECTS\n");
					SkipLine();
					SkipLine();
					SkipWhiteSpace();
					int ObjNum = GetInt();
					while(1)
					{
						CObject *NewObj = new CObject(this);
						CurArea->AddObject(ObjNum, NewObj);

//						printf("\t\tobj %d:\n", ObjNum);
						Find(". ");
						char Buf[4];
						Buf[0] = GetC();	Buf[1] = GetC();	Buf[2] = GetC();	Buf[3] = '\0';

						if(!strcmp(Buf, "gro")) NewObj->SetType(GROUP);
						if(!strcmp(Buf, "sen")) NewObj->SetType(SENSOR);
						if(!strcmp(Buf, "rec")) NewObj->SetType(RECTANGLE);
						if(!strcmp(Buf, "lin")) {NewObj->SetType(PLANAR); NewObj->SetNumSides(2);}
						if(!strcmp(Buf, "tri")) {NewObj->SetType(PLANAR); NewObj->SetNumSides(3);}
						if(!strcmp(Buf, "qua")) {NewObj->SetType(PLANAR); NewObj->SetNumSides(4);}
						if(!strcmp(Buf, "pen")) {NewObj->SetType(PLANAR); NewObj->SetNumSides(5);}
						if(!strcmp(Buf, "hex")) {NewObj->SetType(PLANAR); NewObj->SetNumSides(6);}
						if(!strcmp(Buf, "cub")) NewObj->SetType(CUBOID);

						// there's one extra piece of information for a pyramid...
						if(!strcmp(Buf, "pyr"))
						{
							char PyramidBuf[4];
							NewObj->SetType(PYRAMID);
							Find(" ");
							int c;
							for(c = 0; c < 3; c++)
							{
								PyramidBuf[c] = GetC();
								if(PyramidBuf[c] == '.')
								{
									UnGetC();
									break;
								}
							}
							PyramidBuf[c] = '\0';

							if(!strcmp(PyramidBuf, "i"))	NewObj->SetPyramidType(1);
							if(!strcmp(PyramidBuf, "ii"))	NewObj->SetPyramidType(2);
							if(!strcmp(PyramidBuf, "iii"))	NewObj->SetPyramidType(3);
							if(!strcmp(PyramidBuf, "iv"))	NewObj->SetPyramidType(4);
							if(!strcmp(PyramidBuf, "v"))	NewObj->SetPyramidType(5);
							if(!strcmp(PyramidBuf, "vi"))	NewObj->SetPyramidType(6);
						}
						
						Find(". ");
						
						char nextc = GetC();
						if(nextc == 'm')
						{
							NewObj->SetMoveable(true);
							Find(" ");
						}
						else
						{
							NewObj->SetMoveable(false);
							UnGetC();
						}

						if(GetC() == 'n')
						{
							NewObj->SetDefaultVisible(true);
						}
						else
						{
							Find(" ");
							if(GetC() == 'i')
								NewObj->SetDefaultVisible(false);
							else
								NewObj->SetDefaultVisible(true);
						}

						Find("position: ");
						float Pos[3], Size[3];
						Pos[0] = GetInt();		Pos[1] = GetInt();		Pos[2] = GetInt();
						Find("size ");
						Size[0] = GetInt();		Size[1] = GetInt();		Size[2] = GetInt();
						NewObj->SetLocation(Pos, Size);
						
						bool EQuit = false;

						switch(NewObj->GetType())
						{
							default: printf("\t\tUnhandled object type %s\n", Buf); EQuit = true; break;
							case CUBOID:
							case RECTANGLE:
							{
								Find("colours: ");
//								printf("\t\tcuboid or rectangle ");
								for(int col = 0; col < NewObj->GetNumSides(); col++)
								{
									CColour colst = GetColour();
									NewObj->SetColour(col, colst);
									GetC();
								}
								UnGetC();
//								printf("\n");									
								SkipLine();
							}
							break;
							case PYRAMID:
							{
								Find("colours: ");
//								printf("\t\tpyramid ");
								for(int col = 0; col < 6; col++)
								{
									CColour colst = GetColour();
									NewObj->SetColour(col, colst);
									GetC();
								}
//								printf("\n");

								Find("apex:");
								float ApexStart[2], ApexEnd[2];
								ApexStart[0] = GetInt();
								ApexStart[1] = GetInt();
								Find("to");
								ApexEnd[0] = GetInt();
								ApexEnd[1] = GetInt();
								NewObj->SetApex(ApexStart, ApexEnd);
								
//								printf("\t\t\tapex %d, %d to %d, %d ", Areas[AreaNum]->Objects[ObjNum]->Data.Pyramid.ApexStart[0], Areas[AreaNum]->Objects[ObjNum]->Data.Pyramid.ApexStart[1], Areas[AreaNum]->Objects[ObjNum]->Data.Pyramid.ApexEnd[0], Areas[AreaNum]->Objects[ObjNum]->Data.Pyramid.ApexEnd[1]);

//								printf("\n");
								SkipLine();
							}
							break;
							case GROUP:
							{
								Find("included objects");
//								printf("\t\tgroup: ");
								char newc = GetC();
								if(newc != ':')
								{
//									printf("no objects");
								}
								else
								{
									int GroupObjNum = GetInt();
									while(1)
									{
										// add ObjNum to group
										NewObj->AddObject(GroupObjNum);
//										Areas[AreaNum]->Objects[ObjNum]->Data.Group.Objects[Areas[AreaNum]->Objects[ObjNum]->Data.Group.NumObjects++] = GroupObjNum;
//										printf("%d ", GroupObjNum);

										SkipLine();
										int c = 4;
										while(c--)
										{
											char c = GetC();
											if(c != ' ') break;
										}
										if(c >= 0)
										{
											UnGetC();
											break;
										}
										GroupObjNum = GetInt();
									}
								}
//								printf("\n", ObjNum);
							}
							break;
							case SENSOR:
							{
								Find("colour: ");
//								printf("\t\tsensor ");
								for(int col = 0; col < 2; col++)
								{
									CColour colst = GetColour();
									NewObj->SetColour(col, colst);
									GetC();
									GetC();
								}
								UnGetC();
								
								Find("speed");
								int Speed, Range, DirFlags = 0;
								Speed = GetInt();
								Find("range");
								Range = GetInt();
								
								Find("shoots");
								while(1)
								{
									SkipWhiteSpace(false);
									char newc = GetC();
									if(newc == '.' || newc == '\n') break;
									switch(newc)
									{
										case 'u': DirFlags |= SENSORFLAG_UP; break;
										case 'd': DirFlags |= SENSORFLAG_DOWN; GetC(); GetC(); break;
										case 'n': DirFlags |= SENSORFLAG_NORTH; GetC(); GetC(); GetC(); break;
										case 's': DirFlags |= SENSORFLAG_SOUTH; GetC(); GetC(); GetC(); break;
										case 'e': DirFlags |= SENSORFLAG_EAST; GetC(); GetC(); break;
										case 'w': DirFlags |= SENSORFLAG_EAST; GetC(); GetC(); break;
									}
									GetC(); GetC();
								}
								UnGetC();
//								printf("diflags: %02x ", DirFlags);
								
								NewObj->SetSensorStats(Speed, Range, DirFlags, ObjNum); // TODO: get this properly
								//printf("speed %d, range %d ", Areas[AreaNum]->Objects[ObjNum]->Data.Sensor.Speed, Areas[AreaNum]->Objects[ObjNum]->Data.Sensor.Range);
								
//								Find("shoots");
								// bored now
//								printf("\n");
								SkipLine();
							}
							break;

							case PLANAR:
								Find("colours: ");
//								printf("\t\tline ");
								for(int col = 0; col < 2; col++)
								{
									CColour colst = GetColour();
									NewObj->SetColour(col, colst);
									GetC();
								}
								UnGetC();

								for(int vnum = 0; vnum < NewObj->GetNumSides(); vnum++)
								{
									char name[20];
									sprintf(name, "vertex %d:", vnum+1);
									Find(name);
									float Vert[3];
									Vert[0] = GetInt();
									Vert[1] = GetInt();
									Vert[2] = GetInt();
									NewObj->SetVertex(vnum, Vert);
//									printf("[%d %d %d] ", Areas[AreaNum]->Objects[ObjNum]->Data.Line.Vert[vnum][0], Areas[AreaNum]->Objects[ObjNum]->Data.Line.Vert[vnum][1], Areas[AreaNum]->Objects[ObjNum]->Data.Line.Vert[vnum][2]);
								}

//								printf("\n");
								SkipLine();
							break;
						}
						if(EQuit) break;
						char newc;

						// if movable, need to get start position
						if(NewObj->GetMoveable())
						{
							Find("start position:");
							float StartPos[3];
							StartPos[0] = GetInt();
							StartPos[1] = GetInt();
							StartPos[2] = GetInt();
							NewObj->SetStartPos(StartPos);
							SkipLine();
						}

						// check if we've stumbled upon a condition
						SkipWhiteSpace(false);
						newc = GetC();
						if(newc >= 'a' && newc <= 'z')
						{
							UnGetC();
							CCondition *NewC = new CCondition;
							while(1)
							{
								SkipWhiteSpace(false);
								newc = GetC();
								UnGetC();
								if(newc < 'a' || newc > 'z')
									break;
								const char *Buf = GetLine();
								NewC->AddInstruction(Buf);
							}
							NewC->Compile(false, this);
							NewObj->SetCondition(NewC);
						}
						else
							UnGetC();

						// check if we're at the end of the object list
						newc = GetC();
						if(newc == '\n')
							break;
						UnGetC();
						
						ObjNum = GetInt();
					}
				}

				if(!Handled)
				while(1)
				{
					SkipLine();
					char newc = GetC();
					if(newc == '\n') break;
				}
			}
		}
	}

	CrouchHeight = 64;
	Assemble();

	fclose(input);
	return true;
}
