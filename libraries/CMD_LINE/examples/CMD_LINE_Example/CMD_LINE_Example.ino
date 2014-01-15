/* 
 ////////////////////////////////////////////////////////////////////////
												(c)john k2ox 9/22/13
 demo's the use of the CMD_LINE class  
 added isNot... 11/14/13 

accepts serial input of the form abcdef,xyz,123,zzzzzzz, ...
and returns arg[0] = abcdef, arg[0] = xyz, ...

also checks if arg[0] maches a literal string or a char.

added "literal" args. 

example:  	r,1,"2,3,4,5",67,"8,9,0",100

then:
					arg[0] = r
					arg[1] = 1
					arg[3] = 2,3,4,5
					arg[4] = 67
					arg[5] = 8,9,0
					arg[6] = 100         12/01/13

/////////////////////////////////////////////////////////////////////////
*/



#include <CMD_LINE.h>

CMD_LINE my;

void setup()
{
  Serial.begin(115200);
  Serial.println("Enter A Command And It's Args [Ex. print all,100,"tOut,23,a",36,8,... ]");
}
//&&&&&&&&&&&&&&&&&&&&&& loop &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
void loop()
{
  char** cmd = my.arg;
  byte argCnt = my.getSerialInput();   // check for an entry
  if (argCnt  > 0)
  {
    Serial.print(" You Entered the Command: ");
    Serial.println(my.arg[0]);

      if (my.isCommand("test")){         // does arg[0] == "test" ?
      int tst = atoi(my.arg[1]);
      Serial.println(tst);
    }       
    if (my.isCommand("print all"))
      my.printArgs(argCnt);

    if (my.isCommand("pi")){             // returns a float
      float tst = atof(my.arg[1]);
      Serial.println(tst,5);
    }
    if (my.isNotCommand()){              // default
      Serial.print(">>>>> "); 
      Serial.print(my.arg[0]);   
      Serial.println(" is Not Recognized");
    } 
  }
}





