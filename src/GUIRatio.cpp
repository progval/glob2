/*
    Copyright (C) 2001, 2002 Stephane Magnenat & Luc-Olivier de Charriere
    for any question or comment contact us at nct@ysagoon.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "GUIRatio.h"
#include "GlobalContainer.h"

Ratio::Ratio(int x, int y, int w, int h, int size, int value)
{
	this->x=x;
	this->y=y;
	this->w=w;
	this->h=h;
	
	this->size=size;
	this->value=value;
	oldValue=value;
	
	max=w-size-1;
	assert(value<max);
	needRefresh=true;
	pressed=false;
}

Ratio::~Ratio()
{
	// Let's sing.
}

void Ratio::onTimer(Uint32 tick)
{
	if (needRefresh)
	{
		parent->onAction(this, RATIO_CHANGED, value, 0);
		repaint();
	}
}

void Ratio::onSDLEvent(SDL_Event *event)
{
	if (event->type==SDL_MOUSEBUTTONDOWN)
	{
		if (isPtInRect(event->button.x, event->button.y, x+value, y, x+value+size, h))
		{
			pressed=true;
			px=event->button.x;
			py=event->button.y;
			pValue=value;
			needRefresh=true;
		}
	}
	else if (event->type==SDL_MOUSEBUTTONUP)
	{
		needRefresh=pressed;
		pressed=false;
	}
	else  if ((event->type==SDL_MOUSEMOTION) && pressed )
	{
		int dx=event->motion.x-px;
		int dy=event->motion.y-py;
		if (abs(dy)>h)
			dx=0;
		value=pValue+dx;
		if (value<0)
			value=0;
		else if (value>max)
			value=max;
		if (oldValue!=value)
		{
			oldValue=value;
			needRefresh=true;
		}
	}
	
}

void Ratio::internalPaint(void)
{
	assert(parent);
	assert(parent->getSurface());
	parent->getSurface()->drawRect(x, y, w, h, 180, 180, 180);
	if (pressed)
	{
		parent->getSurface()->drawHorzLine(x+value+1, y+1, size-2, 170, 170, 240);
		parent->getSurface()->drawHorzLine(x+value+1, y+h-2, size-2, 170, 170, 240);
		
		parent->getSurface()->drawVertLine(x+value+1, y+1, h-2, 170, 170, 240);
		parent->getSurface()->drawVertLine(x+value+size-1, y+1, h-2, 170, 170, 240);
	}
	else
	{
		parent->getSurface()->drawHorzLine(x+value+1, y+1, size-2, 180, 180, 180);
		parent->getSurface()->drawHorzLine(x+value+1, y+h-2, size-2, 180, 180, 180);
		
		parent->getSurface()->drawVertLine(x+value+1, y+1, h-2, 180, 180, 180);
		parent->getSurface()->drawVertLine(x+value+size-1, y+1, h-2, 180, 180, 180);
	}
	needRefresh=false;
}

void Ratio::paint(void)
{
	if (visible)
		internalPaint();
}

void Ratio::repaint(void)
{
	assert(parent);
	parent->paint(x, y, w, h);
	if (visible)
		internalPaint();
	parent->addUpdateRect(x, y, w, h);
}

int Ratio::getMax(void)
{
	return max;
}

int Ratio::get(void)
{
	return value;
}
