/*////////////////////////////////////////////////////////////////
Creative Commons Attrib Share-Alike License
You are free to use/extend this library but please abide with the CCSA license:
http://creativecommons.org/licenses/by-sa/3.0/
2013-09-22 (C) k2ox@exothink.com

									john k2ox 9/22/2013

accepts serial input of the form abcdef,xyz,123,zzzzzzz, ...
and returns arg[0] = abcdef, arg[0] = xyz, ...

will also check if arg[0] maches a literal string.

/////////////////////////////////////////////////////////////////
*/

#ifndef CMD_LINE_h
#define CMD_LINE_h

#include <Arduino.h>

#define MAXSTRLEN  50   //total # of chars that will be accepted
#define MAXARGS 6       //max # of args accepted


class CMD_LINE {
  public:    
	char* arg[MAXARGS];
	char inStr[MAXSTRLEN];
	//char rawStr[MAXSTRLEN];  // 12/1/13
	int charCnt;
	
	byte getSerialInput();
	byte parse(char* inStr);
	boolean isCommand(char const* q);
	boolean isCommand(char q);
	boolean isNotCommand();
	void printArgs(byte cnt);
	void printBuffer(char* buffer, int length);
	
	private:
	bool isCmd;
  
};
#endif