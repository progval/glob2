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

#include <GUIBase.h>
#include <assert.h>
#include <GraphicContext.h>

// this function support base unicode (UCS16)
void UCS16toUTF8(Uint16 ucs16, char utf8[4])
{
	if (ucs16<0x80)
	{
		utf8[0]=static_cast<Uint8>(ucs16);
		utf8[1]=0;
	}
	else if (ucs16<0x800)
	{
		utf8[0]=static_cast<Uint8>(((ucs16>>6)&0x1F)|0xC0);
		utf8[1]=static_cast<Uint8>((ucs16&0x3F)|0x80);
		utf8[2]=0;
	}
	else if (ucs16<0xd800)
	{
		utf8[0]=static_cast<Uint8>(((ucs16>>12)&0x0F)|0xE0);
		utf8[1]=static_cast<Uint8>(((ucs16>>6)&0x3F)|0x80);
		utf8[2]=static_cast<Uint8>((ucs16&0x3F)|0x80);
		utf8[3]=0;
	}
	else
	{
		utf8[0]=0;
		fprintf(stderr, "GAG : UCS16toUTF8 : Error, can handle UTF16 characters\n");
	}
}

// this function support full unicode (UCS32)
unsigned getNextUTF8Char(unsigned char c)
{
	if (c>0xFC)
	{
		return 6;
	}
	else if (c>0xF8)
	{
		return 5;
	}
	else if (c>0xF0)
	{
		return 4;
	}
	else if (c>0xE0)
	{
		return 3;
	}
	else if (c>0xC0)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

unsigned getNextUTF8Char(const char *text, unsigned pos)
{
	unsigned next=pos+getNextUTF8Char(text[pos]);
	assert(next<=strlen(text));
	return next;
}

unsigned getPrevUTF8Char(const char *text, unsigned pos)
{
	// TODO : have a more efficient algo
	unsigned last=0, i=0;
	while (i<(unsigned)pos)
	{
		last=i;
		i+=getNextUTF8Char(text[i]);
	}
	return last;
}

Widget::Widget()
{
	visible=true;
	parent=NULL;
}

Widget::~Widget()
{
	
}


RectangularWidget::RectangularWidget()
{
	x=y=w=h=hAlignFlag=vAlignFlag=0;
}

void RectangularWidget::show(void)
{
	if (!visible)
		parent->toShow.push_back(this);
}

void RectangularWidget::hide(void)
{
	if (visible)
		parent->toHide.push_back(this);
}

void RectangularWidget::doHide(void)
{
	assert(parent);
	visible=false;

	int x, y, w, h;
	getScreenPos(&x, &y, &w, &h);
	parent->paint(x, y, w, h);
	parent->addUpdateRect(x, y, w, h);
}

void RectangularWidget::doShow(void)
{
	assert(parent);
	visible=true;

	int x, y, w, h;
	getScreenPos(&x, &y, &w, &h);
	paint();
	parent->addUpdateRect(x, y, w, h);
}

void RectangularWidget::setVisible(bool newState)
{
	if (newState)
		show();
	else
		hide();
}

void RectangularWidget::paint(void)
{
	int x, y, w, h;
	getScreenPos(&x, &y, &w, &h);

	internalInit(x, y, w, h);
	internalRepaint(x, y, w, h);
}

void RectangularWidget::repaint(void)
{
	if (visible)
	{
		int x, y, w, h;
		getScreenPos(&x, &y, &w, &h);

		parent->paint(x, y, w, h);
		internalRepaint(x, y, w, h);
		parent->addUpdateRect(x, y, w, h);
	}
}

void RectangularWidget::getScreenPos(int *sx, int *sy, int *sw, int *sh)
{
	assert(sx);
	assert(sy);
	assert(sw);
	assert(sh);
	assert(parent);
	assert(parent->getSurface());

	int screenw = parent->getSurface()->getW();
	int screenh = parent->getSurface()->getH();

	switch (hAlignFlag)
	{
		case ALIGN_LEFT:
			*sx=x;
			*sw=w;
			break;

		case ALIGN_RIGHT:
			*sx=screenw-w-x;
			*sw=w;
			break;

		case ALIGN_FILL:
			*sx=x;
			*sw=screenw-w-x;
			break;
			
		case ALIGN_SCREEN_CENTERED:
			*sx=x+((screenw-640)>>1);
			*sw=w;
			break;

		default:
			assert(false);
	}

	switch (vAlignFlag)
	{
		case ALIGN_LEFT:
			*sy=y;
			*sh=h;
			break;

		case ALIGN_RIGHT:
			*sy=screenh-h-y;
			*sh=h;
			break;

		case ALIGN_FILL:
			*sy=y;
			*sh=screenh-h-y;
			break;
			
		case ALIGN_SCREEN_CENTERED:
			*sy=y+((screenh-480)>>1);
			*sh=h;
			break;

		default:
			assert(false);
	}
}

void HighlightableWidget::onSDLEvent(SDL_Event *event)
{
	int x, y, w, h;
	getScreenPos(&x, &y, &w, &h);
	
	if (event->type==SDL_MOUSEMOTION)
	{
		if (isPtInRect(event->motion.x, event->motion.y, x, y, w, h))
		{
			if (!highlighted)
			{
				highlighted=true;
				repaint();
				parent->onAction(this, BUTTON_GOT_MOUSEOVER, returnCode, 0);
			}
		}
		else
		{
			if (highlighted)
			{
				highlighted=false;
				repaint();
				parent->onAction(this, BUTTON_LOST_MOUSEOVER, returnCode, 0);
			}
		}
	}
}

Screen::Screen()
{
	gfxCtx = NULL;
	returnCode = 0;
	run = false;
}

Screen::~Screen()
{
	for (std::set<Widget *>::iterator it=widgets.begin(); it!=widgets.end(); ++it)
	{
		delete (*it);
	}
}

void Screen::paint()
{
	assert(gfxCtx);
	paint(0, 0, gfxCtx->getW(), gfxCtx->getH());
}

int Screen::execute(DrawableSurface *gfx, int stepLength)
{
	Uint32 frameStartTime;
	Sint32 frameWaitTime;

	dispatchPaint(gfx);
	addUpdateRect();
	repaint(gfx);
	run=true;
	onAction(NULL, SCREEN_CREATED, 0, 0);
	
	while (run)
	{
		// get first timer
		frameStartTime=SDL_GetTicks();

		// send timer
		dispatchTimer(frameStartTime);

		// send events
		SDL_Event lastMouseMotion, windowEvent, event;
		bool hadLastMouseMotion=false;
		bool wasWindowEvent=false;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
				{
					run=false;
					returnCode=-1;
					break;
				}
				break;
				case SDL_MOUSEMOTION:
				{
					hadLastMouseMotion=true;
					lastMouseMotion=event;
				}
				break;
				case SDL_ACTIVEEVENT:
				{
					windowEvent=event;
					wasWindowEvent=true;
				}
				break;
				case SDL_VIDEORESIZE:
				{
					gfx->setRes(event.resize.w, event.resize.h, gfx->getDepth(), gfx->getFlags());
					dispatchPaint(gfx);
					addUpdateRect();
					repaint(gfx);
					onAction(NULL, SCREEN_RESIZED, gfx->getW(), gfx->getH());
				}
				break;
				default:
				{
					dispatchEvents(&event);
				}
				break;
			}
		}
		if (hadLastMouseMotion)
			dispatchEvents(&lastMouseMotion);
		if (wasWindowEvent)
			dispatchEvents(&windowEvent);
			
		// process to hide/ to show requests
		for (unsigned i=0; i<toHide.size(); i++)
			toHide[i]->doHide();
		toHide.clear();
			
		for (unsigned i=0; i<toShow.size(); i++)
			toShow[i]->doShow();
		toShow.clear();

		// redraw
		repaint(gfx);

		// wait timer
		frameWaitTime=SDL_GetTicks()-frameStartTime;
		frameWaitTime=stepLength-frameWaitTime;
		if (frameWaitTime>0)
			SDL_Delay(frameWaitTime);
	}
	onAction(NULL, SCREEN_DESTROYED, 0, 0);

	return returnCode;
}

void Screen::endExecute(int returnCode)
{
	run=false;
	this->returnCode=returnCode;
}

void Screen::addUpdateRect()
{
	assert(gfxCtx);
	updateRects.clear();
	addUpdateRect(0, 0, gfxCtx->getW(), gfxCtx->getH());
}

void Screen::addUpdateRect(int x, int y, int w, int h)
{
	SDL_Rect r;
	r.x=static_cast<Sint16>(x);
	r.y=static_cast<Sint16>(y);
	r.w=static_cast<Uint16>(w);
	r.h=static_cast<Uint16>(h);
	updateRects.push_back(r);
}

void Screen::addWidget(Widget* widget)
{
	assert(widget);
	widget->parent=this;
	// this option enable or disable the multiple add check
	widgets.insert(widget);
}

void Screen::removeWidget(Widget* widget)
{
	assert(widget);
	assert(widget->parent==this);
	widgets.erase(widget);
}

void Screen::dispatchEvents(SDL_Event *event)
{
	onSDLEvent(event);
	for (std::set<Widget *>::iterator it=widgets.begin(); it!=widgets.end(); ++it)
	{
		if ((*it)->visible)
			(*it)->onSDLEvent(event);
	}
}

void Screen::dispatchTimer(Uint32 tick)
{
	onTimer(tick);
	for (std::set<Widget *>::iterator it=widgets.begin(); it!=widgets.end(); ++it)
	{
		if ((*it)->visible)
			(*it)->onTimer(tick);
	}
}

void Screen::dispatchPaint(DrawableSurface *gfx)
{
	assert(gfx);
	gfxCtx=gfx;
	gfxCtx->setClipRect();
	paint();
	for (std::set<Widget *>::iterator it=widgets.begin(); it!=widgets.end(); ++it)
	{
		if ((*it)->visible)
			(*it)->paint();
	}
}

void Screen::repaint(DrawableSurface *gfx)
{
	if (updateRects.size()>0)
	{
		SDL_Rect *rects=new SDL_Rect[updateRects.size()];
		
		for (size_t i=0; i<updateRects.size(); i++)
			rects[i]=updateRects[i];
			
		gfx->updateRects(rects, updateRects.size());
		
		delete[] rects;
		updateRects.clear();
	}
}

void Screen::paint(int x, int y, int w, int h)
{
	gfxCtx->drawFilledRect(x, y, w, h, 0, 0, 0);
}

int Screen::getW(void)
{
	if (gfxCtx)
		return gfxCtx->getW();
	else
		return 0;

}

int Screen::getH(void)
{
	if (gfxCtx)
		return gfxCtx->getH();
	else
		return 0;
}

// Overlay screen, used for non full frame dialog

OverlayScreen::OverlayScreen(GraphicContext *parentCtx, unsigned w, unsigned h)
{
	gfxCtx=new DrawableSurface();
	gfxCtx->setRes(w, h);
	gfxCtx->setAlpha(false, 180);
	decX=(parentCtx->getW()-w)>>1;
	decY=(parentCtx->getH()-h)>>1;
	endValue=-1;
}

OverlayScreen::~OverlayScreen()
{
	delete gfxCtx;
}

void OverlayScreen::translateAndProcessEvent(SDL_Event *event)
{
	SDL_Event ev=*event;
	switch (ev.type)
	{
		case SDL_MOUSEMOTION:
			ev.motion.x-=static_cast<Uint16>(decX);
			ev.motion.y-=static_cast<Uint16>(decY);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			ev.button.x-=static_cast<Uint16>(decX);
			ev.button.y-=static_cast<Uint16>(decY);
			break;
		default:
			break;
	}
	dispatchEvents(&ev);
}

void OverlayScreen::paint(int x, int y, int w, int h)
{
	//gfxCtx->drawFilledRect(x, y, w, h, 15, 44, 79);
	gfxCtx->drawFilledRect(x, y, w, h, 0, 0, 40);
}


