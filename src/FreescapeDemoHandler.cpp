#include "FreescapeDemoHandler.h"

void CFSDemoHandler::DrawHud(CFont *Font, Sint32 *Variables, float Width, float Height)
{
	glPushMatrix();
		/*glBegin(GL_LINE_LOOP);
			glVertex2f(0, 0);
			glVertex2f(Width, 0);
			glVertex2f(Width, Height);
			glVertex2f(0, Height);
		glEnd();*/

		glTranslatef(0, Height-1, 0);
		glColor3f(1, 1, 0);
		Font->Print("Gold: %d", Variables[33]);

		glPushMatrix();
			glTranslatef(((Width * 0.5f) - (Font->GetWidth("Score: %d", Variables[34]) * 0.5)), 0, 0);
			glColor3f(1, 1, 1);
			Font->Print("Score: %d", Variables[34]);
		glPopMatrix();

		glPushMatrix();
			glTranslatef(Width - Font->GetWidth("Health: %d", 100-Variables[11]), 0, 0);
			glColor3f(1, 0, 0);
			Font->Print("Health: %d", 100-Variables[11]);
		glPopMatrix();

		glTranslatef(0, -1, 0);
		glPushMatrix();
			glTranslatef(((Width * 0.5f) - (Font->GetWidth("Time: %d", Variables[31]) * 0.5)), 0, 0);
			glColor3f(1, 1, 1);
			Font->Print("Time: %d", Variables[31]);
		glPopMatrix();
	glPopMatrix();
}
