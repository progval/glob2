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

#include <FileManager.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <SDL_endian.h>
#include <iostream>

// here we handle compile time options
#ifdef HAVE_CONFIG_H
#  include <config.h>
#else
#	ifdef WIN32
#		define PACKAGE_DATA_DIR ".."
#		define PACKAGE_SOURCE_DIR "../.."
#	else
#		define PACKAGE_DATA_DIR ".."
#		define PACKAGE_SOURCE_DIR "../.."
#	endif
#endif

// include for directory listing
#ifdef WIN32
#	include <windows.h>
#	include <io.h>
#	include <direct.h>
#	include "win32_dirent.h"
#else
#	include <sys/types.h>
#	include <dirent.h>
#	include <sys/stat.h>
#endif

//! define this to have a verbose vPath error
//#define DBG_VPATH_OPEN
//#define DBG_VPATH_LIST


FileManager::FileManager(const char *gameName)
{
	#ifndef WIN32
	std::string gameLocal(getenv("HOME"));
	gameLocal += "/.";
	gameLocal += gameName;
	mkdir(gameLocal.c_str(), S_IRWXU);
	addDir(gameLocal.c_str());
	#endif
	addDir(".");
	addDir(PACKAGE_DATA_DIR);
	addDir(PACKAGE_SOURCE_DIR);
	fileListIndex = -1;
}

FileManager::~FileManager()
{
	dirList.clear();
	clearFileList();
}

void FileManager::clearFileList(void)
{
	fileList.clear();
	fileListIndex = -1;
}

void FileManager::addDir(const char *dir)
{
	dirList.push_back(dir);
}

void FileManager::addWriteSubdir(const char *subdir)
{
	for (size_t i = 0; i < dirList.size(); i++)
	{
		std::string toCreate(dirList[i]);
		toCreate += '/';
		toCreate += subdir;
		#ifdef WIN32
		int result = _mkdir(toCreate.c_str());
		#else
		int result = mkdir(toCreate.c_str(), S_IRWXU);
		#endif
		// NOTE : We only want to create the subdir for the first index
// 		if (result==0)
			break;
		if ((result==-1) && (errno==EEXIST))
			break;
	}
}

SDL_RWops *FileManager::openWithbackup(const char *filename, const char *mode)
{
	if (strchr(mode, 'w'))
	{
		std::string backupName(filename);
		backupName += '~';
		rename(filename, backupName.c_str());
	}
	return SDL_RWFromFile(filename, mode);
}

FILE *FileManager::openWithbackupFP(const char *filename, const char *mode)
{
	if (strchr(mode, 'w'))
	{
		std::string backupName(filename);
		backupName += '~';
		rename(filename, backupName.c_str());
	}
	return fopen(filename, mode);
}

SDL_RWops *FileManager::open(const char *filename, const char *mode)
{
	for (size_t i = 0; i < dirList.size(); ++i)
	{
		std::string path(dirList[i]);
		path += DIR_SEPARATOR;
		path += filename;

		SDL_RWops *fp =  openWithbackup(path.c_str(), mode);
		if (fp)
			return fp;
	}

	#ifdef DBG_VPATH_OPEN
	std::cerr << "GAG : File " << filename << " not found in mode " << mode << "\n";
	std::cerr << "Searched path :\n";
	for (size_t i = 0; i < dirList.size(); ++i)
		std::cerr << dirList[i] << "\n";
	std::cerr << std::endl;
	#endif

	return NULL;
}


FILE *FileManager::openFP(const char *filename, const char *mode)
{
	for (size_t i = 0; i < dirList.size(); ++i)
	{
		std::string path(dirList[i]);
		path += DIR_SEPARATOR;
		path += filename;
		
		FILE *fp =  openWithbackupFP(path.c_str(), mode);
		if (fp)
			return fp;
	}

	#ifdef DBG_VPATH_OPEN
	std::cerr << "GAG : File " << filename << " not found in mode " << mode << "\n";
	std::cerr << "Searched path :\n";
	for (size_t i = 0; i < dirList.size(); ++i)
		std::cerr << dirList[i] << "\n";
	std::cerr << std::endl;
	#endif

	return NULL;
}

std::ifstream *FileManager::openIFStream(const std::string &fileName)
{
	for (size_t i = 0; i < dirList.size(); ++i)
	{
		std::string path(dirList[i]);
		path += DIR_SEPARATOR;
		path += fileName;

		std::ifstream *fp = new std::ifstream(path.c_str());
		if (fp->good())
			return fp;
		else
			delete fp;
	}
	
	#ifdef DBG_VPATH_OPEN
	std::cerr << "GAG : File " << fileName << " not found by std::ifstream\n";
	std::cerr << "Searched path :\n";
	for (size_t i = 0; i < dirList.size(); ++i)
		std::cerr << dirList[i] << "\n";
	std::cerr << std::endl;
	#endif
	
	return NULL;
}

Uint32 FileManager::checksum(const char *filename)
{
	Uint32 cs = 0;
	SDL_RWops *stream = open(filename);
	if (stream)
	{
		int length = SDL_RWseek(stream, 0, SEEK_END);
		SDL_RWseek(stream, 0, SEEK_SET);
		
		int lengthBlock = length & (~0x3);
		for (int i=0; i<(lengthBlock>>2); i++)
		{
			cs ^= SDL_ReadBE32(stream);
			cs = (cs<<31)|(cs>>1);
		}
		int lengthRest = length & 0x3;
		for (int i=0; i<lengthRest; i++)
		{
			unsigned char c;
			SDL_RWread(stream, &c, 1, 1);
			cs ^= (static_cast<Uint32>(c))<<(8*i);
		}
		SDL_RWclose(stream);
	}
	return cs;
}

void FileManager::remove(const char *filename)
{
	for (size_t i = 0; i < dirList.size(); ++i)
	{
		std::string path(dirList[i]);
		path += DIR_SEPARATOR;
		path += filename;
		std::remove(path.c_str());
	}
}

bool FileManager::isDir(const char *filename)
{
	#ifdef WIN32
	struct _stat s;
	#else
	struct stat s;
	#endif
	s.st_mode = 0;
	int serr = 1;
	for (size_t i = 0; (serr != 0) && (i < dirList.size()); ++i)
	{
		std::string path(dirList[i]);
		path += DIR_SEPARATOR;
		path += filename;
		#ifdef WIN32
		serr = ::_stat(path.c_str(), &s);
		#else
		serr = stat(path.c_str(), &s);
		#endif
	}
	return (s.st_mode & S_IFDIR) != 0;
}

bool FileManager::addListingForDir(const char *realDir, const char *extension, const bool dirs)
{
	DIR *dir = opendir(realDir);
	struct dirent *dirEntry;

	if (!dir)
	{
		#ifdef DBG_VPATH_LIST
		std::cerr << "GAG : Open dir failed for dir " << realDir << std::endl;
		#endif
		return false;
	}

	while ((dirEntry = readdir(dir))!=NULL)
	{
		#ifdef DBG_VPATH_LIST
		std::cerr << realDir << std::endl;
		#endif
		
		// there might be a way to optimize the decision of the ok
		bool ok = true;
		// hide hidden stuff
		if (dirEntry->d_name[0] == '.')
		{
			ok = false;
		}
		// take directories if asked
		else if (dirEntry->d_type == DT_DIR)
		{
			ok = dirs;
		}
		// check extension if provided
		else if (extension) 
		{
			size_t l, nl;
			l=strlen(extension);
			nl=strlen(dirEntry->d_name);
			ok = ((nl>l) &&
			      (dirEntry->d_name[nl-l-1]=='.') &&
			      (strcmp(extension,dirEntry->d_name+(nl-l))==0));
		}
		if (ok)
		{
			// test if name already exists in vector
			bool alreadyIn = false;
			for (size_t i = 0; i < fileList.size(); ++i)
			{
				if (fileList[i] == dirEntry->d_name)
				{
					alreadyIn = true;
					break;
				}
			}
			if (!alreadyIn)
			{
				fileList.push_back(dirEntry->d_name);
			}
		}
	}

	closedir(dir);
	return true;
}

bool FileManager::initDirectoryListing(const char *virtualDir, const char *extension, const bool dirs)
{
	bool result = false;
	clearFileList();
	for (size_t i = 0; i < dirList.size(); ++i)
	{
		std::string path(dirList[i]);
		path += DIR_SEPARATOR;
		path += virtualDir;
		#ifdef DBG_VPATH_LIST
		std::cerr << "GAG : Listing from dir " << path << std::endl;
		#endif
		result = addListingForDir(path.c_str(), extension, dirs) || result;
	}
	if (result)
		fileListIndex=0;
	else
		fileListIndex=-1;
	return result;
}

const char *FileManager::getNextDirectoryEntry(void)
{
	if ((fileListIndex >= 0) && (fileListIndex < (int)fileList.size()))
	{
		return fileList[fileListIndex++].c_str();
	}
	return NULL;
}

