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

#include <GUIButton.h>
#include <GUIStyle.h>
#include <Toolkit.h>
#include <assert.h>
#include <GraphicContext.h>

using namespace GAGCore;

namespace GAGGUI
{
	Button::Button(int x, int y, int w, int h, Uint32 hAlign, Uint32 vAlign, const char *sprite, int standardId, int highlightID, int returnCode, Uint16 unicodeShortcut)
	:HighlightableWidget(returnCode)
	{
		this->x=x;
		this->y=y;
		this->w=w;
		this->h=h;
		this->hAlignFlag=hAlign;
		this->vAlignFlag=vAlign;
	
		this->unicodeShortcut=unicodeShortcut;
	}
	
	
	void Button::onSDLEvent(SDL_Event *event)
	{
		int x, y, w, h;
		getScreenPos(&x, &y, &w, &h);
	
		HighlightableWidget::onSDLEvent(event);
	
		if (event->type==SDL_KEYDOWN)
		{
			Uint16 typedUnicode=event->key.keysym.unicode;
			if ((unicodeShortcut)&&(typedUnicode==unicodeShortcut))
				parent->onAction(this, BUTTON_SHORTCUT, returnCode, unicodeShortcut);
		}
		else if (event->type==SDL_MOUSEBUTTONDOWN)
		{
			if (isPtInRect(event->button.x, event->button.y, x, y, w, h) &&
				(event->button.button == SDL_BUTTON_LEFT))
				parent->onAction(this, BUTTON_PRESSED, returnCode, 0);
		}
		else if (event->type==SDL_MOUSEBUTTONUP)
		{
			if (isPtInRect(event->button.x, event->button.y, x, y, w, h) &&
				(event->button.button == SDL_BUTTON_LEFT))
				parent->onAction(this, BUTTON_RELEASED, returnCode, 0);
		}
	}
	
	
	TextButton::TextButton(int x, int y, int w, int h, Uint32 hAlign, Uint32 vAlign, const char *sprite, int standardId, int highlightID, const char *font, const char *text, int returnCode, Uint16 unicode)
	:Button(x, y, w, h, hAlign, vAlign, sprite, standardId, highlightID, returnCode, unicode)
	{
		assert(font);
		assert(text);
		this->font=font;
		this->text=text;
		fontPtr=NULL;
	}
	
	void TextButton::init(void)
	{
		Button::init();
		fontPtr = Toolkit::getFont(font.c_str());
		assert(fontPtr);
	}
	
	void TextButton::paint()
	{
		int x, y, w, h;
		getScreenPos(&x, &y, &w, &h);
		
		assert(parent);
		assert(parent->getSurface());
		
		Style::style->drawTextButtonBackground(parent->getSurface(), x, y, w, h, getNextHighlightValue());
		
		int decX=(w-fontPtr->getStringWidth(this->text.c_str()))>>1;
		int decY=(h-fontPtr->getStringHeight(this->text.c_str()))>>1;
	
		parent->getSurface()->drawString(x+decX, y+decY, fontPtr, text.c_str());
	}
	
	void TextButton::setText(const char *text)
	{
		assert(text);
		this->text=text;
	}
	
	
	OnOffButton::OnOffButton(int x, int y, int w, int h, Uint32 hAlign, Uint32 vAlign, bool startState, int returnCode)
	:HighlightableWidget(returnCode)
	{
		this->x=x;
		this->y=y;
		this->w=w;
		this->h=h;
		this->hAlignFlag=hAlign;
		this->vAlignFlag=vAlign;
	
		this->state=startState;
	}
	
	void OnOffButton::onSDLEvent(SDL_Event *event)
	{
		int x, y, w, h;
		getScreenPos(&x, &y, &w, &h);
	
		HighlightableWidget::onSDLEvent(event);
	
		if (event->type==SDL_MOUSEBUTTONDOWN)
		{
			if (isPtInRect(event->button.x, event->button.y, x, y, w, h) &&
				(event->button.button == SDL_BUTTON_LEFT))
			{
				state=!state;
				parent->onAction(this, BUTTON_PRESSED, returnCode, 0);
				parent->onAction(this, BUTTON_STATE_CHANGED, returnCode, state == true ? 1 : 0);
			}
		}
		else if (event->type==SDL_MOUSEBUTTONUP)
		{
			if (isPtInRect(event->button.x, event->button.y, x, y, w, h))
				parent->onAction(this, BUTTON_RELEASED, returnCode, 0);
		}
	}
	
	void OnOffButton::paint()
	{
		int x, y, w, h;
		getScreenPos(&x, &y, &w, &h);
		
		assert(parent);
		assert(parent->getSurface());
		
		Style::style->drawOnOffButton(parent->getSurface(), x, y, w, h, getNextHighlightValue(), state);
	}
	
	void OnOffButton::setState(bool newState)
	{
		if (newState!=state)
		{
			state=newState;
		}
	}
	
	ColorButton::ColorButton(int x, int y, int w, int h, Uint32 hAlign, Uint32 vAlign, int returnCode)
	:HighlightableWidget(returnCode)
	{
		this->x=x;
		this->y=y;
		this->w=w;
		this->h=h;
		this->hAlignFlag=hAlign;
		this->vAlignFlag=vAlign;
	
		selColor=0;
	}
	
	void ColorButton::onSDLEvent(SDL_Event *event)
	{
		int x, y, w, h;
		getScreenPos(&x, &y, &w, &h);
	
		HighlightableWidget::onSDLEvent(event);
	
		if (event->type==SDL_MOUSEBUTTONDOWN)
		{
			if (isPtInRect(event->button.x, event->button.y, x, y, w, h) && v.size())
			{
				if (event->button.button == SDL_BUTTON_LEFT)
				{
					selColor++;
					if (selColor>=(signed)v.size())
						selColor=0;
	
					parent->onAction(this, BUTTON_STATE_CHANGED, returnCode, selColor);
					parent->onAction(this, BUTTON_PRESSED, returnCode, 0);
				}
				else if (event->button.button == SDL_BUTTON_RIGHT)
				{
					selColor--;
					if (selColor<0)
						selColor=(signed)v.size()-1;
					
					parent->onAction(this, BUTTON_STATE_CHANGED, returnCode, selColor);
					parent->onAction(this, BUTTON_PRESSED, returnCode, 0);
				}
			}
		}
		else if (event->type==SDL_MOUSEBUTTONUP)
		{
			if (isPtInRect(event->button.x, event->button.y, x, y, w, h) &&
				(event->button.button == SDL_BUTTON_LEFT))
			{
				parent->onAction(this, BUTTON_RELEASED, returnCode, 0);
			}
		}
	}
	
	void ColorButton::paint()
	{
		int x, y, w, h;
		getScreenPos(&x, &y, &w, &h);
		
		assert(parent);
		assert(parent->getSurface());
		
		if (v.size())
			parent->getSurface()->drawFilledRect(x+1, y+1, w-2, h-2, v[selColor].r, v[selColor].g, v[selColor].b);
		HighlightableWidget::paint();
	}
	
	MultiTextButton::MultiTextButton(int x, int y, int w, int h, Uint32 hAlign, Uint32 vAlign, const char *sprite, int standardId, int highlightID, const char *font, const char *text, int returnCode, Uint16 unicode)
	:TextButton(x, y, w, h, hAlign, vAlign, sprite, standardId, highlightID, font, text, returnCode, unicode)
	{
		textIndex=0;
	}
	
	void MultiTextButton::onSDLEvent(SDL_Event *event)
	{
		int x, y, w, h;
		getScreenPos(&x, &y, &w, &h);
	
		HighlightableWidget::onSDLEvent(event);
	
		if (event->type==SDL_MOUSEBUTTONDOWN)
		{
			if (isPtInRect(event->button.x, event->button.y, x, y, w, h) && texts.size())
			{
				if (event->button.button == SDL_BUTTON_LEFT)
				{
					textIndex++;
					if (textIndex >= texts.size())
						textIndex = 0;
					setText(texts.at(textIndex));
	
					parent->onAction(this, BUTTON_STATE_CHANGED, returnCode, textIndex);
					parent->onAction(this, BUTTON_PRESSED, returnCode, 0);
				}
				else if (event->button.button == SDL_BUTTON_RIGHT)
				{
					if (textIndex > 0)
						textIndex--;
					else
						textIndex = texts.size()-1;
					setText(texts.at(textIndex));
					
					parent->onAction(this, BUTTON_STATE_CHANGED, returnCode, textIndex);
					parent->onAction(this, BUTTON_PRESSED, returnCode, 0);
				}
			}
		}
		else if (event->type==SDL_MOUSEBUTTONUP)
		{
			if (isPtInRect(event->button.x, event->button.y, x, y, w, h) &&
				(event->button.button == SDL_BUTTON_LEFT))
			{
				parent->onAction(this, BUTTON_RELEASED, returnCode, 0);
			}
		}
	}
	
	void MultiTextButton::addText(const char *s)
	{
		texts.push_back(s);
	}
	
	void MultiTextButton::clearTexts(void)
	{
		texts.clear();
	}
	
	void MultiTextButton::setTextIndex(int i)
	{
		textIndex=i;
		setText(texts.at(textIndex));
	}
	
	void MultiTextButton::setFirstTextIndex(int i)
	{
		textIndex=i;
		text=texts.at(textIndex);
	}
}
