/*
  Copyright (C) 2001-2004 Stephane Magnenat & Luc-Olivier de Charrière
  for any question or comment contact us at nct@ysagoon.com or nuage@ysagoon.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef __GUITEXTAREA_H
#define __GUITEXTAREA_H

#include "GUIBase.h"
#include <vector>

class Font;

class TextArea:public RectangularWidget
{
public:
	TextArea() { font=NULL; textBuffer=NULL; }
	TextArea(int x, int y, int w, int h, Uint32 hAlign, Uint32 vAlign, const char *font, bool readOnly=true, const char *text="");
	virtual ~TextArea();

	virtual void paint(void);
	virtual void onSDLEvent(SDL_Event *event);

	virtual void setText(const char *text);
	virtual const char *getText(void) { return textBuffer; }
	virtual void addText(const char *text);
	virtual void addChar(const char c);
	virtual void remText(unsigned pos, unsigned len);
	virtual void scrollDown(void);
	virtual void scrollUp(void);
	virtual void scrollToBottom(void);
	virtual void setCursorPos(unsigned pos);

protected:
	virtual void internalPaint(void);
	virtual void internalSetText(const char *text);
	virtual void repaint(void);
	//! we make sure the repaint will show something correct
	virtual void computeAndRepaint(void);

protected:
	bool readOnly;
	const Font *font;
	char *textBuffer;
	const char *initialText;
	unsigned int textBufferLength;
	unsigned int areaHeight;
	unsigned int areaPos;
	unsigned int charHeight;
	std::vector <unsigned int> lines;
	
	// edit mod variables
	// this one is the only one always valid, other are recomputed from it
	unsigned int cursorPos;
	// this one can be invalid, but must be within textBufferLength
	unsigned int cursorPosY;
	// this one can be anything
	unsigned int cursorScreenPosY;
};

#endif
