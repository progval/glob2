/*
  Copyright (C) 2001, 2002, 2003 Stephane Magnenat, Luc-Olivier de Charrière
  and Martin S. Nyffenegger
  for any question or comment contact us at nct@ysagoon.com, nuage@ysagoon.com
  or barock@ysagoon.com

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

#ifndef SGSL_H
#define SGSL_H

#include <string>
#include <deque>
#include <vector>
#include <stdio.h>
#include "Marshaling.h"

struct Token
{
	enum TokenType
	{
		// Data Types
		NIL=0,
		INT,
		STRING,

		// Syntaxic token
		S_PAROPEN=20,
		S_PARCLOSE,
		S_SEMICOL,
		S_STORY,
		S_EOF,

		// Language keywords
		S_EQUAL=30,
		S_HIGHER,
		S_LOWER,
		S_NOT,

		// Functions
		S_WAIT=50,
		S_TIMER,
		S_SHOW,
		S_HIDE,
		S_ALLIANCE,
		S_GUIENABLE,
		S_GUIDISABLE,
		S_SUMMON,
		S_WIN,
		S_LOOSE,

		// Constants
		// Units
		S_WORKER=100,
		S_EXPLORER,
		S_WARRIOR,
		// Buildings
		S_SWARM_B,
		S_FOOD_B,
		S_HEALTH_B,
		S_WALKSPEED_B,
		S_SWIMSPEED_B,
		S_ATTACK_B,
		S_SCIENCE_B,
		S_DEFENCE_B,
		S_MARKET_B,
		S_WALL_B,
		// Flags
		S_EXPLOR_F,
		S_FIGHT_F,
		S_CLEARING_F,
		S_FORBIDDEN_F,
		// GUI elements that can be disabled or enabled
		S_BUILDINGTAB,
		S_FLAGTAB,
		S_TEXTSTATTAB,
		S_GFXSTATTAB,
		S_ALLIANCESCREEN,

		//SGSL
		S_FRIEND=310,
		S_YOU=315,

		S_ENEMY=311,
		S_ISDEAD=312,
		S_FLAG=314,
		S_NOENEMY=316,
		S_LABEL=321,
		S_JUMP=322,
		S_SETFLAG=323,
		S_ALLY=324,
	} type;

	struct TokenSymbolLookupTable
	{
		TokenType type;
		const char *name;
	};

	int value;
	std::string msg;

	//! Constructor, set logic default values
	Token() { type=NIL; value=0; }

	//! This table is a map table between token type and token names
	static TokenSymbolLookupTable table[];

	//! Returns the type of a given name (parsing phase)
	static TokenType getTypeByName(const char *name);

	//! Returns the name a of given type (debug & script recreation phase)
	static const char *getNameByType(TokenType type);
};

struct ErrorReport
{
	enum ErrorType
	{
		ET_OK=0,
		ET_INVALID_VALUE,
		ET_SYNTAX_ERROR,
		ET_INVALID_PLAYER,
		ET_NO_SUCH_FILE,
		ET_INVALID_FLAG_NAME,
		ET_DOUBLE_FLAG_NAME,
		ET_MISSING_PAROPEN,
		ET_MISSING_PARCLOSE,
		ET_MISSING_SEMICOL,
		ET_MISSING_ARGUMENT,
		ET_UNKNOWN,
		ET_NB_ET,
	} type;

	unsigned line;
	unsigned col;
	unsigned pos;

	ErrorReport() { type=ET_UNKNOWN; line=0; col=0; pos=0; }
	ErrorReport(ErrorType et) { type=et; line=0; col=0; pos=0; }

	const char *getErrorString(void);
};

//Text parser, returns tokens
class Aquisition
{
public:
	Aquisition(void);
	virtual ~Aquisition(void);

public:
	const Token *getToken() { return &token; }
	void nextToken();
	bool newFile(const char*);
	unsigned getLine(void) { return lastLine; }
	unsigned getCol(void) { return lastCol; }
	unsigned getPos(void) { return lastPos; }

	virtual int getChar(void) = 0;
	virtual int ungetChar(char c) = 0;

private:
	Token token;
	unsigned actLine, actCol, actPos, lastLine, lastCol, lastPos;
	bool newLine;
};

//File parser
class FileAquisition: public Aquisition
{
public:
	FileAquisition() { fp=NULL; }
	virtual ~FileAquisition() { if (fp) fclose(fp); }
	bool open(const char *filename);

	virtual int getChar(void) { return ::fgetc(fp); }
	virtual int ungetChar(char c) { return ::ungetc(c, fp); }

private:
	FILE *fp;
};

//String parser
class StringAquisition: public Aquisition
{
public:
	StringAquisition();
	virtual ~StringAquisition();
	void open(const char *text);

	virtual int getChar(void);
	virtual int ungetChar(char c);

private:
	char *buffer;
	int pos;
};

class Mapscript;

//Independant story line
class Story
{
public:
	Story(Mapscript *mapscript);
	virtual ~Story();

public:
	std::deque<Token> line;
	bool hasWon, hasLost;

	void step();
	Sint32 checkSum() { return lineSelector; }

private:
	bool conditionTester(int pc, bool l);
	bool testCondition();
	int valueOfVariable(Token::TokenType type, int playerNumber, int level);
	int lineSelector;
	Mapscript *mapscript;
	int internTimer;
};

class Game;

struct Flag
{
	int x, y, r;
	std::string name;
};

class Mapscript
{
public:
	Mapscript();
	~Mapscript();
	
public:
	ErrorReport compileScript(Game *game, const char *script);
	ErrorReport compileScript(Game *game);
	ErrorReport loadScript(const char *filename, Game *game);

	bool load(SDL_RWops *stream);
	void save(SDL_RWops *stream);
	void setSourceCode(const char *sourceCode);
	const char *getSourceCode(void) { return sourceCode; }
	
	void step();
	Sint32 checkSum();
	bool hasTeamWon(unsigned teamNumber);
	bool hasTeamLost(unsigned teamNumber);
	int getMainTimer(void) { return mainTimer; }

	void reset(void);
	bool isTextShown;
	std::string textShown;

private:
	friend class Story;

	ErrorReport parseScript(Aquisition *donnees, Game *game);
	bool testMainTimer(void);
	bool doesFlagExist(std::string name);
	bool getFlagPos(std::string name, int *x, int *y, int *r);
	
	int mainTimer;
	std::deque<Story> stories;
	std::vector<Flag> flags;
	Game *game;
	char *sourceCode;
};



#endif
