

/*////////////////////////////////////////////////////////////////

Creative Commons Attrib Share-Alike License
You are free to use/extend this library but please abide with the CCSA license:
http://creativecommons.org/licenses/by-sa/3.0/
2013-09-22 (C) k2ox@exothink.com

									john k2ox 9/22/2013

accepts serial input of the form abcdef,xyz,123,zzzzzzz, ...
and returns arg[0] = abcdef, arg[0] = xyz, ...

also checks if arg[0] maches a literal string or a char.

added "literal" args. ex  r,1,"2,3,4,5",67,"8,9,0",100
						arg[0] = r
						arg[1] = 1
						arg[3] = 2,3,4,5
						arg[4] = 67
						arg[5] = 8,9,0
						arg[6] = 100         12/01/13

/////////////////////////////////////////////////////////////////
*/
#include <CMD_LINE.h>



byte CMD_LINE::getSerialInput()  //works 9/21/13
{
 if (Serial.available())
  {
    for(int i = 0; i <= MAXSTRLEN; i++)   //clear mem
	  inStr[i] = '\0';
    Serial.readBytesUntil('\0', inStr, MAXSTRLEN);
  }
  else
    return 0;
return parse(inStr);
}


byte CMD_LINE::parse(char* inStr)
{
 byte argNumber = 1;
 boolean lit = false;
  for(charCnt = 0; inStr[charCnt] != '\0'; charCnt++)
  {
    if(inStr[charCnt] == '"')   //check for double quotes
	{
	if(lit)
		inStr[charCnt] = '\0';  // if closing quote terminate the literal		
	lit = !lit;                 // found a literal
	}
	
	if((inStr[charCnt] == ',') && (lit == false)) //separate at commas unless a literal
    {
    inStr[charCnt] = '\0';
    if(argNumber <= MAXARGS)
      {
		arg[argNumber] = &inStr[charCnt] + 1;
		if(inStr[charCnt + 1] == '"')  //check if char after a comma is a double quote
		  arg[argNumber]++;	            //if so arg literal arg starts after the quote
        argNumber++;                    //arg is complete, check the next		
      }
    }
  }
  arg[0] = inStr;	//arg[0] starts at pos = 0, ends where the first comma was
  isCmd = false;
  return argNumber;
}


boolean CMD_LINE::isCommand(char const* q)
{
  char* pinStr = arg[0];
  while (*pinStr != '\0' )
  {
    if( *pinStr != *q)
      return false;  
    *pinStr++;
    *q++;
  }
  if( *pinStr != *q)  //same length?  bug fixed 11/02/13
	return false;
  else
  {
    isCmd = true;
    return true; 
  }
}

boolean CMD_LINE::isCommand(char q)
{
  char* pinStr = arg[0];
  if( (*pinStr == q) && (*++pinStr == '\0'))
  {
	isCmd = true;
	return true;
  }
  else
  return false;
}

boolean CMD_LINE::isNotCommand()
{
  return !isCmd;
}

void CMD_LINE::printArgs(byte cnt)
{
  for( int i=0; i < cnt; i++)
  {
    Serial.print(" Arg(");
    Serial.print(i);
    Serial.print(") = "); 
    Serial.println(arg[i]);
  }
  Serial.println();
}


void CMD_LINE::printBuffer(char* buffer, int length)
{
  for (byte i = 0; i < length; i++)
   // Serial.print(buffer[i], HEX);
    Serial.print(buffer[i]);
  Serial.println();
}