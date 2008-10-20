/*
  Copyright (C) 2007 Bradley Arsenault

  Copyright (C) 2001-2004 Stephane Magnenat & Luc-Olivier de Charrière
  for any question or comment contact us at <stephane at magnenat dot net> or <NuageBleu at gmail dot com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "SettingsScreen.h"
#include "GlobalContainer.h"
#include <assert.h>
#include <sstream>
#include <GUIText.h>
#include <GUITextInput.h>
#include <GUIList.h>
#include <GUIButton.h>
#include <GUISelector.h>
#include <GUINumber.h>
#include <Toolkit.h>
#include <StringTable.h>
#include <GraphicContext.h>
#include "SoundMixer.h"
#include <ostream>
#include <algorithm>
#include "boost/lexical_cast.hpp"
#include "GameGUIKeyActions.h"
#include "MapEditKeyActions.h"

SettingsScreen::SettingsScreen()
 : mapeditKeyboardManager(MapEditShortcuts), guiKeyboardManager(GameGUIShortcuts)
{
	old_settings=globalContainer->settings;
	//following are standard choices for all screens
	//tab choices
	generalsettings=new TextButton( 10, 10, 200, 40, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "menu", Toolkit::getStringTable()->getString("[general settings]"), GENERALSETTINGS);
	addWidget(generalsettings);

	unitsettings=new TextButton( 220, 10, 200, 40, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "menu", Toolkit::getStringTable()->getString("[unit settings]"), UNITSETTINGS);
	addWidget(unitsettings);

	keyboardsettings=new TextButton( 430, 10, 200, 40, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "menu", Toolkit::getStringTable()->getString("[keyboard settings]"), KEYBOARDSETTINGS);
	addWidget(keyboardsettings);

	// Screen entry/quit part
	ok=new TextButton( 230, 420, 180, 40, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "menu", Toolkit::getStringTable()->getString("[ok]"), OK);
	addWidget(ok);
	cancel=new TextButton(440, 420, 180, 40, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "menu", Toolkit::getStringTable()->getString("[Cancel]"), CANCEL);
	addWidget(cancel);

	//following are all general settings
	// language part
	language=new Text(20, 60, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[language-tr]"));
	addWidget(language);
	languageList=new List(20, 90, 180, 200, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard");
	for (int i=0; i<Toolkit::getStringTable()->getNumberOfLanguage(); i++)
	{
		if(!Toolkit::getStringTable()->isLangComplete(i))
			languageList->addText(Toolkit::getStringTable()->getStringInLang("[language incomplete]", i));
		else
			languageList->addText(Toolkit::getStringTable()->getStringInLang("[language]", i));
	}
	addWidget(languageList);

	// graphics part
	display=new Text(230, 60, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[display]"));
	addWidget(display);
	actDisplay = new Text(440, 60, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", actDisplayModeToString().c_str());
	addWidget(actDisplay);
	modeList=new List(440, 90, 180, 200, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard");
	globalContainer->gfx->beginVideoModeListing();
	int w, h;
	while(globalContainer->gfx->getNextVideoMode(&w, &h))
	{
		std::ostringstream ost;
		ost << w << "x" << h;
		if (!modeList->isText(ost.str().c_str()))
			modeList->addText(ost.str().c_str());
	}
	addWidget(modeList);
	
	fullscreen=new OnOffButton(230, 90, 20, 20, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, globalContainer->settings.screenFlags & GraphicContext::FULLSCREEN, FULLSCREEN);
	addWidget(fullscreen);
	fullscreenText=new Text(260, 90, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[fullscreen]"), 180);
	addWidget(fullscreenText);
	
	#ifdef HAVE_OPENGL
	#endif
	usegpu=new OnOffButton(230, 90 + 30, 20, 20, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, globalContainer->settings.screenFlags & GraphicContext::USEGPU, USEGL);
	addWidget(usegpu);
	usegpuText=new Text(260, 90 + 30, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", "OpenGL", 180);
	addWidget(usegpuText);
	
	lowquality=new OnOffButton(230, 90 + 60, 20, 20, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, globalContainer->settings.optionFlags & GlobalContainer::OPTION_LOW_SPEED_GFX, LOWQUALITY);
	addWidget(lowquality);
	lowqualityText=new Text(260, 90 + 60, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[lowquality]"), 180);
	addWidget(lowqualityText);

	customcur=new OnOffButton(230, 90 + 90, 20, 20, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, globalContainer->settings.screenFlags & GraphicContext::CUSTOMCURSOR, CUSTOMCUR);
	addWidget(customcur);
	customcurText=new Text(260, 90 + 90, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[customcur]"), 180);
	addWidget(customcurText);
	rememberUnitButton=new OnOffButton(230, 90 + 120, 20, 20, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, globalContainer->settings.rememberUnit, REMEMBERUNIT);
	addWidget(rememberUnitButton);
	rememberUnitText=new Text(260, 90 + 120, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[remember unit]"), 180);
	addWidget(rememberUnitText);

	scrollwheel=new OnOffButton(230, 90 + 150, 20, 20, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, globalContainer->settings.scrollWheelEnabled, SCROLLWHEEL);
	addWidget(scrollwheel);
	scrollwheelText=new Text(260, 90 + 150, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[scroll wheel enabled]"), 180);
	addWidget(scrollwheelText);



	
	rebootWarning=new Text(0, 300, ALIGN_FILL, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[Warning, you need to reboot the game for changes to take effect]"));
	rebootWarning->setStyle(Font::Style(Font::STYLE_BOLD, 255, 60, 60));
	addWidget(rebootWarning);
	
	setVisibilityFromGraphicType();
	rebootWarning->visible=false;

	// Username part
	userName=new TextInput(20, 360, 180, 25, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", globalContainer->getUsername(), true, 32);
	addWidget(userName);
	usernameText=new Text(20, 330, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[username]"));
	addWidget(usernameText);

	// Audio part
	audio=new Text(230, 330, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[audio]"), 300);
	addWidget(audio);
	audioMute=new OnOffButton(230, 360, 20, 20, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, globalContainer->settings.mute, MUTE);
	addWidget(audioMute);
	audioMuteText=new Text(260, 360, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[mute]"), 200);
	addWidget(audioMuteText);
	musicVol=new Selector(320, 360, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, 180, globalContainer->settings.musicVolume, 256, true);
	addWidget(musicVol);
	musicVolText=new Text(320, 330, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[Music volume]"), 300);
	addWidget(musicVolText);
	setVisibilityFromAudioSettings();
	
	
	int first_group_row=0;
	int first_group_current_column_x=20;
	int first_group_widest_element=0;
	
	int second_group_row=1;
	int second_group_first_row_x=170;
	int second_group_current_column_x=170;
	int second_group_widest_element=0;
	
	for(int t=0; t<IntBuildingType::NB_BUILDING; ++t)
	{
		for(int l=0; l<6; ++l)
		{
			unitRatios[t][l]=NULL;
			unitRatioTexts[t][l]=NULL;
		}
	}
	
	///First group are completed buildings, Inn, Swarm and Defence tower
	for(int t=0; t<IntBuildingType::NB_BUILDING; ++t)
	{
		std::string name=IntBuildingType::typeFromShortNumber(t);
		for(int l=0; l<3; ++l)
		{
			if(globalContainer->buildingsTypes.getByType(name, l, false) != NULL && globalContainer->settings.defaultUnitsAssigned[t][l*2+1]>0)
			{
				int size = addDefaultUnitAssignmentWidget(t, l*2+1, first_group_current_column_x, 100 + 40*first_group_row);
				first_group_widest_element = std::max(first_group_widest_element, size);
				
				first_group_row += 1;
				if(first_group_row == 8)
				{
					first_group_row = 0;
					first_group_current_column_x += first_group_widest_element;
					first_group_widest_element = 0;
				}
			}
		}	
	}
	
	///Second group, at the top is Swarm, Wall, Market, the rest follow horizontally
	for(int l=0; l<3; ++l)
	{
		for(int t=0; t<IntBuildingType::NB_BUILDING; ++t)
		{
			std::string name=IntBuildingType::typeFromShortNumber(t);
			//Even numbers represent under-construction, whereas odd numbers represent completed buildings		
			if(globalContainer->buildingsTypes.getByType(name, l, true) != NULL && globalContainer->settings.defaultUnitsAssigned[t][l*2]>0)
			{
				int this_row = second_group_row;
				int this_x = second_group_current_column_x;
				if(t == IntBuildingType::SWARM_BUILDING || t == IntBuildingType::STONE_WALL || t == IntBuildingType::MARKET_BUILDING)
				{
					this_row = 0;
					this_x = second_group_first_row_x;
					second_group_first_row_x += 180;
				}
				else
				{
					second_group_row += 1;
				}
				
				int size = addDefaultUnitAssignmentWidget(t, l*2, this_x, 100 + 40*this_row);
				second_group_widest_element = std::max(second_group_widest_element, size);
				
				if(second_group_row == 8)
				{
					second_group_row = 1;
					second_group_current_column_x += 180;
					second_group_widest_element = 0;
				}
			}
		}
	}
	
	unitSettingsExplanation = new Text( 100, 60, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[unit settings explanation]"));
	unitSettingsHeading1 = new Text( 160, 80, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[construction and upgrades]"));
	unitSettingsHeading2 = new Text( 10, 80, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[constructed buildings]"));


	unitSettingsExplanation->visible = false;
	unitSettingsHeading1->visible = false;
	unitSettingsHeading2->visible = false;

	addWidget(unitSettingsExplanation);
	addWidget(unitSettingsHeading1);
	addWidget(unitSettingsHeading2);

	//shortcuts part
	game_shortcuts=new TextButton( 100, 60, 120, 20, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[game shortcuts]"), GAMESHORTCUTS);
	game_shortcuts->visible=false;

	editor_shortcuts=new TextButton( 230, 60, 120, 20, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[editor shortcuts]"), EDITORSHORTCUTS);
	editor_shortcuts->visible=false;

	shortcut_list = new List(20, 110, 325, 160, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard");
	action_list = new List(365, 110 , 265, 190, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard");
	select_key_1 = new KeySelector(20, 275, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", 100, 25);
	key_2_active = new OnOffButton(125, 275, 25, 25, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, false, SECONDKEY);
	select_key_2 = new KeySelector(155, 275, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", 100, 25);
	pressedUnpressedSelector = new MultiTextButton(260, 275, 80, 25, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", "", PRESSEDSELECTOR);
	add_shortcut = new TextButton(20, 305, 158, 40, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[add shortcut]"), ADDSHORTCUT);
	remove_shortcut = new TextButton(188, 305, 157, 40, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[remove shortcut]"), REMOVESHORTCUT);
	restore_default_shortcuts = new TextButton(365, 305, 265, 40, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString("[restore default shortcuts]"), RESTOREDEFAULTSHORTCUTS);

	pressedUnpressedSelector->clearTexts();
	pressedUnpressedSelector->addText(Toolkit::getStringTable()->getString("[on press]"));
	pressedUnpressedSelector->addText(Toolkit::getStringTable()->getString("[on unpress]"));

	
	game_shortcuts->visible = false;
	editor_shortcuts->visible = false;
	restore_default_shortcuts->visible = false;
	shortcut_list->visible = false;
	action_list->visible = false;
	select_key_1->visible = false;
	key_2_active->visible = false;
	select_key_2->visible = false;
	pressedUnpressedSelector->visible = false;
	add_shortcut->visible = false;
	remove_shortcut->visible = false;
	restore_default_shortcuts->visible=false;
		
	addWidget(game_shortcuts);
	addWidget(editor_shortcuts);
	addWidget(shortcut_list);
	addWidget(action_list);
	addWidget(select_key_1);
	addWidget(key_2_active);
	addWidget(select_key_2);
	addWidget(pressedUnpressedSelector);
	addWidget(add_shortcut);
	addWidget(remove_shortcut);
	addWidget(restore_default_shortcuts);

	currentMode = GameGUIShortcuts;

	gfxAltered = false;
}


void SettingsScreen::addNumbersFor(int low, int high, Number* widget)
{
	for(int i=low; i<=high; ++i)
	{
		widget->add(i);
	}
}


void SettingsScreen::onAction(Widget *source, Action action, int par1, int par2)
{
	if ((action==BUTTON_RELEASED) || (action==BUTTON_SHORTCUT))
	{
		if (par1==OK)
		{
			globalContainer->setUserName(userName->getText());
			globalContainer->settings.language = Toolkit::getStringTable()->getStringInLang("[language-code]", Toolkit::getStringTable()->getLang());
			globalContainer->settings.save();
			mapeditKeyboardManager.saveKeyboardLayout();
			guiKeyboardManager.saveKeyboardLayout();
			endExecute(par1);
		}
		else if (par1==CANCEL)
		{
			globalContainer->settings=old_settings;
			if (gfxAltered)
				updateGfxCtx();

			Toolkit::getStringTable()->setLang(Toolkit::getStringTable()->getLangCode(globalContainer->settings.language));

			///Send the old volume to the mixer
			globalContainer->mix->setVolume(globalContainer->settings.musicVolume, globalContainer->settings.mute);

			endExecute(par1);
		}
		else if (par1==GENERALSETTINGS)	
		{
			language->visible=true;
			languageList->visible=true;
			display->visible=true;
			actDisplay->visible=true;
			modeList->visible=true;
			fullscreen->visible=true;
			fullscreenText->visible=true;
			usegpu->visible=true;
			usegpuText->visible=true;
			lowquality->visible=true;
			lowqualityText->visible=true;
			customcur->visible=true;
			customcurText->visible=true;
			userName->visible=true;
			usernameText->visible=true;
			audio->visible=true;
			audioMuteText->visible=true;
			audioMute->visible=true;
			musicVol->visible=true;
			musicVolText->visible=true;
			rememberUnitButton->visible=true;
			rememberUnitText->visible=true;
			scrollwheel->visible=true;
			scrollwheelText->visible=true;
			
			
			for(int t=0; t<IntBuildingType::NB_BUILDING; ++t)
			{
				for(int l=0; l<6; ++l)
				{
					if(unitRatios[t][l])
					{
						unitRatios[t][l]->visible=false;
						unitRatioTexts[t][l]->visible=false;
					}
				}
			}
			
			
			unitSettingsExplanation->visible=false;
			unitSettingsHeading1->visible=false;
			unitSettingsHeading2->visible=false;

			game_shortcuts->visible=false;
			editor_shortcuts->visible=false;
			select_key_1->visible=false;
			key_2_active->visible=false;
			select_key_2->visible=false;
			pressedUnpressedSelector->visible=false;
			shortcut_list->visible=false;
			action_list->visible=false;
			add_shortcut->visible=false;
			remove_shortcut->visible=false;
			restore_default_shortcuts->visible=false;
			setVisibilityFromAudioSettings();
		}

		
		else if (par1==UNITSETTINGS)
		{
			language->visible=false;
			languageList->visible=false;
			display->visible=false;
			actDisplay->visible=false;
			modeList->visible=false;
			fullscreen->visible=false;
			fullscreenText->visible=false;
			usegpu->visible=false;
			usegpuText->visible=false;
			lowquality->visible=false;
			lowqualityText->visible=false;
			customcur->visible=false;
			customcurText->visible=false;
			userName->visible=false;
			usernameText->visible=false;
			audio->visible=false;
			audioMuteText->visible=false;
			audioMute->visible=false;
			musicVol->visible=false;
			musicVolText->visible=false;
			rememberUnitButton->visible=false;
			rememberUnitText->visible=false;
			scrollwheel->visible=false;
			scrollwheelText->visible=false;
			
			
			for(int t=0; t<IntBuildingType::NB_BUILDING; ++t)
			{
				for(int l=0; l<6; ++l)
				{
					if(unitRatios[t][l])
					{
						unitRatios[t][l]->visible=true;
						unitRatioTexts[t][l]->visible=true;
					}
				}
			}
			
			unitSettingsExplanation->visible=true;
			unitSettingsHeading1->visible=true;
			unitSettingsHeading2->visible=true;
	
			game_shortcuts->visible=false;
			editor_shortcuts->visible=false;
			select_key_1->visible=false;
			key_2_active->visible=false;
			select_key_2->visible=false;
			pressedUnpressedSelector->visible=false;
			shortcut_list->visible=false;
			action_list->visible=false;
			add_shortcut->visible=false;
			remove_shortcut->visible=false;
			restore_default_shortcuts->visible=false;
		}

		else if (par1==KEYBOARDSETTINGS)
		{
			language->visible=false;
			languageList->visible=false;
			display->visible=false;
			actDisplay->visible=false;
			modeList->visible=false;
			fullscreen->visible=false;
			fullscreenText->visible=false;
			usegpu->visible=false;
			usegpuText->visible=false;
			lowquality->visible=false;
			lowqualityText->visible=false;
			customcur->visible=false;
			customcurText->visible=false;
			userName->visible=false;
			usernameText->visible=false;
			audio->visible=false;
			audioMuteText->visible=false;
			audioMute->visible=false;
			musicVol->visible=false;
			musicVolText->visible=false;
			rememberUnitButton->visible=false;
			rememberUnitText->visible=false;
			scrollwheel->visible=false;
			scrollwheelText->visible=false;
			
			
			for(int t=0; t<IntBuildingType::NB_BUILDING; ++t)
			{
				for(int l=0; l<6; ++l)
				{
					if(unitRatios[t][l])
					{
						unitRatios[t][l]->visible=false;
						unitRatioTexts[t][l]->visible=false;
					}
				}
			}
			
			
			unitSettingsExplanation->visible=false;
			unitSettingsHeading1->visible=false;
			unitSettingsHeading2->visible=false;
	
			game_shortcuts->visible=true;
			editor_shortcuts->visible=true;
			select_key_1->visible=true;
			key_2_active->visible=true;
			select_key_2->visible=true;
			pressedUnpressedSelector->visible=true;
			shortcut_list->visible=true;
			action_list->visible=true;
			add_shortcut->visible=true;
			remove_shortcut->visible=true;
			restore_default_shortcuts->visible=true;
			currentMode = GameGUIShortcuts;
			updateShortcutList();
			if(shortcut_list->getCount() == 0)
				shortcut_list->setSelectionIndex(-1);
			else
				shortcut_list->setSelectionIndex(0);
			updateActionList();
			updateShortcutInfoFromSelection();
		}
		else if (par1==RESTOREDEFAULTSHORTCUTS)
		{
			loadDefaultKeyboardShortcuts();
		}
		else if(par1==GAMESHORTCUTS)
		{
			currentMode = GameGUIShortcuts;
			updateShortcutList();
			if(shortcut_list->getCount() == 0)
				shortcut_list->setSelectionIndex(-1);
			else
				shortcut_list->setSelectionIndex(0);
			updateActionList();
			updateShortcutInfoFromSelection();
		}
		else if(par1==EDITORSHORTCUTS)
		{
			currentMode = MapEditShortcuts;
			updateShortcutList();
			if(shortcut_list->getCount() == 0)
				shortcut_list->setSelectionIndex(-1);
			else
				shortcut_list->setSelectionIndex(0);
			updateActionList();
			updateShortcutInfoFromSelection();
		}
		else if(par1==PRESSEDSELECTOR)
		{
		}
		else if(par1==ADDSHORTCUT)
		{
			addNewShortcut();
		}
		else if(par1==REMOVESHORTCUT)
		{
			removeShortcut();
		}
	}
	else if (action==NUMBER_ELEMENT_SELECTED)
	{
		for(int t=0; t<IntBuildingType::NB_BUILDING; ++t)
		{
			for(int l=0; l<6; ++l)
			{
				if(unitRatios[t][l])
				{
					if(unitRatios[t][l]->getNth() == 0)
						unitRatios[t][l]->setNth(1);
					globalContainer->settings.defaultUnitsAssigned[t][l]=unitRatios[t][l]->getNth();
				}
			}
		}
	}
	else if (action==LIST_ELEMENT_SELECTED)
	{
		if (source==languageList)
		{
			Toolkit::getStringTable()->setLang(par1);
			ok->setText(Toolkit::getStringTable()->getString("[ok]"));
			cancel->setText(Toolkit::getStringTable()->getString("[Cancel]"));

//;			title->setText(Toolkit::getStringTable()->getString("[settings]"));
			language->setText(Toolkit::getStringTable()->getString("[language-tr]"));
			display->setText(Toolkit::getStringTable()->getString("[display]"));
			usernameText->setText(Toolkit::getStringTable()->getString("[username]"));
			audio->setText(Toolkit::getStringTable()->getString("[audio]"));

			generalsettings->setText(Toolkit::getStringTable()->getString("[general settings]"));
			unitsettings->setText(Toolkit::getStringTable()->getString("[unit settings]"));
			keyboardsettings->setText(Toolkit::getStringTable()->getString("[keyboard settings]"));

			fullscreenText->setText(Toolkit::getStringTable()->getString("[fullscreen]"));
			//usegpuText->setText(Toolkit::getStringTable()->getString("[opengl]"));
			lowqualityText->setText(Toolkit::getStringTable()->getString("[lowquality]"));
			customcurText->setText(Toolkit::getStringTable()->getString("[customcur]"));

			musicVolText->setText(Toolkit::getStringTable()->getString("[Music volume]"));
			audioMuteText->setText(Toolkit::getStringTable()->getString("[mute]"));
			
			rebootWarning->setText(Toolkit::getStringTable()->getString("[Warning, you need to reboot the game for changes to take effect]"));
		}
		else if (source==modeList)
		{
			int w, h;
			sscanf(modeList->getText(par1).c_str(), "%dx%d", &w, &h);
			globalContainer->settings.screenWidth=w;
			globalContainer->settings.screenHeight=h;
			updateGfxCtx();
		}
		else if (source == shortcut_list)
		{
			updateShortcutInfoFromSelection();
		}
		else if(source == action_list)
		{
			updateKeyboardManagerFromShortcutInfo();
		}
	}
	else if (action==VALUE_CHANGED)
	{
		globalContainer->settings.musicVolume = musicVol->getValue();
		globalContainer->mix->setVolume(globalContainer->settings.musicVolume, globalContainer->settings.mute);
	}
	else if (action==BUTTON_STATE_CHANGED)
	{
		if (source==rememberUnitButton)
		{
			globalContainer->settings.rememberUnit=rememberUnitButton->getState();
		}
		else if (source==scrollwheel)
		{
			globalContainer->settings.scrollWheelEnabled=scrollwheel->getState();
		}
		else if (source==lowquality)
		{
			globalContainer->settings.optionFlags=lowquality->getState() ? GlobalContainer::OPTION_LOW_SPEED_GFX : 0;
		}
		else if (source==fullscreen)
		{
			if (fullscreen->getState())
			{
				globalContainer->settings.screenFlags |= GraphicContext::FULLSCREEN;
				globalContainer->settings.screenFlags &= ~(GraphicContext::RESIZABLE);
			}
			else
			{
				globalContainer->settings.screenFlags &= ~(GraphicContext::FULLSCREEN);
				globalContainer->settings.screenFlags |= GraphicContext::RESIZABLE;
			}
			updateGfxCtx();
		}
		else if (source==usegpu)
		{
			if (usegpu->getState())
			{
				globalContainer->settings.screenFlags |= GraphicContext::USEGPU;
			}
			else
			{
				globalContainer->settings.screenFlags &= ~(GraphicContext::USEGPU);
			}
			updateGfxCtx();
		}
		else if (source==customcur)
		{
			if (customcur->getState())
			{
				globalContainer->settings.screenFlags |= GraphicContext::CUSTOMCURSOR;
			}
			else
			{
				globalContainer->settings.screenFlags &= ~(GraphicContext::CUSTOMCURSOR);
			}
			updateGfxCtx();
		}
		else if (source==audioMute)
		{
			globalContainer->settings.mute = audioMute->getState();
			globalContainer->mix->setVolume(globalContainer->settings.musicVolume, globalContainer->settings.mute);
			setVisibilityFromAudioSettings();
		}
		else if (source==key_2_active)
		{
			if(key_2_active->getState() == true)
			{
				select_key_2->setKey(KeyPress());
				select_key_2->visible=true;
			}
			else
			{
				select_key_2->visible=false;
			}
			updateKeyboardManagerFromShortcutInfo();
		}
	}
	else if (action==KEY_CHANGED)
	{
		updateKeyboardManagerFromShortcutInfo();
	}
}


void SettingsScreen::setVisibilityFromGraphicType(void)
{
	rebootWarning->visible = globalContainer->settings.screenFlags & GraphicContext::USEGPU;
}

void SettingsScreen::setVisibilityFromAudioSettings(void)
{
	musicVol->visible = !globalContainer->settings.mute;
	musicVolText->visible = !globalContainer->settings.mute;
}

void SettingsScreen::updateGfxCtx(void)
{
	if ((globalContainer->settings.screenFlags & GraphicContext::USEGPU) == 0)
		globalContainer->gfx->setRes(globalContainer->settings.screenWidth, globalContainer->settings.screenHeight, globalContainer->settings.screenFlags);
	setVisibilityFromGraphicType();
	actDisplay->setText(actDisplayModeToString().c_str());
	gfxAltered = true;
}

std::string SettingsScreen::actDisplayModeToString(void)
{
	std::ostringstream oss;
	oss << globalContainer->gfx->getW() << "x" << globalContainer->gfx->getH();
	if (globalContainer->gfx->getOptionFlags() & GraphicContext::USEGPU)
		oss << " GL";
	else
		oss << " SDL";
	return oss.str();
}



int SettingsScreen::addDefaultUnitAssignmentWidget(int type, int level, int x, int y)
{	
	std::string name=IntBuildingType::typeFromShortNumber(type);

	unitRatios[type][level] = new Number(x, y+20, 100, 18, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, 20, "menu");
	addNumbersFor(0, 20, unitRatios[type][level]);
	unitRatios[type][level]->setNth(globalContainer->settings.defaultUnitsAssigned[type][level]);
	unitRatios[type][level]->visible=false;
	addWidget(unitRatios[type][level]);

	std::string keyname="[";
	if((level+1)%2)
		keyname+="build ";
	keyname+=name + " level " + boost::lexical_cast<std::string>(level/2) + "]";
	unitRatioTexts[type][level]=new Text(x, y, ALIGN_SCREEN_CENTERED, ALIGN_SCREEN_CENTERED, "standard", Toolkit::getStringTable()->getString(keyname.c_str()));
	addWidget(unitRatioTexts[type][level]);
	unitRatioTexts[type][level]->visible=false;

	return std::max(unitRatioTexts[type][level]->getWidth(), unitRatios[type][level]->getWidth());
}



void SettingsScreen::updateShortcutList(int an)
{
	KeyboardManager* m = NULL;
	if(currentMode == GameGUIShortcuts)
		m = &guiKeyboardManager;
	else if(currentMode == MapEditShortcuts)
		m = &mapeditKeyboardManager;
	
	const std::list<KeyboardShortcut>& shortcuts = m->getKeyboardShortcuts();
	size_t n = 0;
	for(std::list<KeyboardShortcut>::const_iterator i = shortcuts.begin(); i!=shortcuts.end(); ++i)
	{
		if(an==-1 || int(n) == an)
		{
			std::string name = i->formatTranslated(currentMode);
			if(n >= shortcut_list->getCount())
				shortcut_list->addText(name);
			else if(shortcut_list->getText(n) != name)
				shortcut_list->setText(n, name);
		}
		n += 1;
	}
	//Remove entries that are off the end
	while(n < shortcut_list->getCount())
		shortcut_list->removeText(n);
}



void SettingsScreen::updateActionList()
{
	action_list->clear();
	if(shortcut_list->getSelectionIndex() != -1)
	{
		if(currentMode == GameGUIShortcuts)
		{
			for(int i=GameGUIKeyActions::DoNothing; i<GameGUIKeyActions::ActionSize; ++i)
			{
				std::string key = "[" + GameGUIKeyActions::getName(i) + "]";
				action_list->addText(Toolkit::getStringTable()->getString(key.c_str()));
			}
		}
		else if(currentMode == MapEditShortcuts)
		{
			for(int i=MapEditKeyActions::DoNothing; i<MapEditKeyActions::ActionSize; ++i)
			{
				std::string key = "[" + MapEditKeyActions::getName(i) + "]";
				action_list->addText(Toolkit::getStringTable()->getString(key.c_str()));
			}
		}
	}
}



void SettingsScreen::updateShortcutInfoFromSelection()
{
	KeyboardManager* m = NULL;
	if(currentMode == GameGUIShortcuts)
		m = &guiKeyboardManager;
	else if(currentMode == MapEditShortcuts)
		m = &mapeditKeyboardManager;

	const std::list<KeyboardShortcut>& shortcuts = m->getKeyboardShortcuts();
	int selection_n = shortcut_list->getSelectionIndex();

	if(selection_n == -1)
	{
		select_key_1->visible=false;
		key_2_active->visible=false;
		select_key_2->visible=false;
		action_list->visible=false;
	}
	else
	{
		std::list<KeyboardShortcut>::const_iterator i = shortcuts.begin();
		std::advance(i, selection_n);
		select_key_1->setKey(i->getKeyPress(0));
		if(i->getKeyPressCount() == 1)
		{
			key_2_active->setState(false);
			select_key_2->visible=false;
		}
		else
		{
			select_key_2->setKey(i->getKeyPress(1));
			key_2_active->setState(true);
			select_key_2->visible=true;
		}

		if(i->getKeyPress(0).getPressed())
			pressedUnpressedSelector->setIndex(0);
		else
			pressedUnpressedSelector->setIndex(1);

		action_list->setSelectionIndex(i->getAction());
		action_list->centerOnItem(action_list->getSelectionIndex());
	}
}



void SettingsScreen::updateKeyboardManagerFromShortcutInfo()
{
	KeyboardManager* m = NULL;
	if(currentMode == GameGUIShortcuts)
		m = &guiKeyboardManager;
	else if(currentMode == MapEditShortcuts)
		m = &mapeditKeyboardManager;

	std::list<KeyboardShortcut>& shortcuts = m->getKeyboardShortcuts();
	int selection_n = shortcut_list->getSelectionIndex();

	if(selection_n != -1)
	{
		std::list<KeyboardShortcut>::iterator i = shortcuts.begin();
		std::advance(i, selection_n);
		KeyboardShortcut new_shortcut;
		
		KeyPress first = KeyPress(select_key_1->getKey(), (pressedUnpressedSelector->getIndex() == 0 ? true : false));
		KeyPress second = KeyPress(select_key_2->getKey(), (pressedUnpressedSelector->getIndex() == 0 ? true : false));
		
		new_shortcut.addKeyPress(first);
		if(key_2_active->getState())
			new_shortcut.addKeyPress(second);
		new_shortcut.setAction(action_list->getSelectionIndex());
		(*i) = new_shortcut;
		updateShortcutList(selection_n);
	}
}



void SettingsScreen::loadDefaultKeyboardShortcuts()
{
	KeyboardManager* m = NULL;
	if(currentMode == GameGUIShortcuts)
		m = &guiKeyboardManager;
	else if(currentMode == MapEditShortcuts)
		m = &mapeditKeyboardManager;
	m->loadDefaultShortcuts();
	updateShortcutList();
	updateShortcutInfoFromSelection();
}



void SettingsScreen::addNewShortcut()
{
	KeyboardShortcut ks;
	ks.addKeyPress(KeyPress());
	if(currentMode == GameGUIShortcuts)
	{
		ks.setAction(GameGUIKeyActions::DoNothing);
		std::list<KeyboardShortcut>& shortcuts = guiKeyboardManager.getKeyboardShortcuts();
		shortcuts.push_back(ks);
	}
	else if(currentMode == MapEditShortcuts)
	{
		ks.setAction(MapEditKeyActions::DoNothing);
		std::list<KeyboardShortcut>& shortcuts = mapeditKeyboardManager.getKeyboardShortcuts();
		shortcuts.push_back(ks);
	}
	updateShortcutList(shortcut_list->getCount());
	shortcut_list->setSelectionIndex(shortcut_list->getCount()-1);
	shortcut_list->centerOnItem(shortcut_list->getCount()-1);
	updateShortcutInfoFromSelection();
}



void SettingsScreen::removeShortcut()
{
	int selection_n = shortcut_list->getSelectionIndex();
	if(currentMode == GameGUIShortcuts)
	{
		std::list<KeyboardShortcut>& shortcuts = guiKeyboardManager.getKeyboardShortcuts();
		std::list<KeyboardShortcut>::iterator i = shortcuts.begin();
		std::advance(i, selection_n);
		shortcuts.erase(i);
	}
	else if(currentMode == MapEditShortcuts)
	{
		std::list<KeyboardShortcut>& shortcuts = mapeditKeyboardManager.getKeyboardShortcuts();
		std::list<KeyboardShortcut>::iterator i = shortcuts.begin();
		std::advance(i, selection_n);
		shortcuts.erase(i);
	}
	shortcut_list->setSelectionIndex(std::max(0, selection_n-1));
	updateShortcutList();
	updateShortcutInfoFromSelection();
}



int SettingsScreen::menu(void)
{
	return SettingsScreen().execute(globalContainer->gfx, 30);
}
