
#include <EEPROM.h>
#include <RFM69NEW.h>
#include <RFM69registers.h>
#include <SPI.h>
#include <CMD_LINE.h>

///////////////////////////////////////////////////////////////////
//               MoteInit.ino      (c) john eckert k2ox 2013
// this utility sets the RFM69 radio parameters in non-volatile ram.
// as of 10/12/13 it uses the first 31 mem locations
// they contain: radio ID, options(pwr range, ext ram,...), freq
// offset correction, date, and sn info.
// Enter 'q' for info 
//                      change fcorr from MSB to LSB first  9/12/13
// update for temp coef, 20 new reserved bytes, new   ver  10/11/13
// users can use any locations from 31 up.  The higher the better! 
// int  nvTCOR;   //int changed to char as of              12/1/13 
// check char at loc 31                                    12/9/13
// added 3 byte radio freq                                 12/12/13
///////////////////////////////////////////////////////////////////

#define VERSION       "v.121213"

#define NV_START_ADR      0
#define NV_ID_ADR         nvStartADR
#define NV_OPT_ADR        nvStartADR + 1
#define NV_FCOR_LSB_ADR   nvStartADR + 2
#define NV_FCOR_MSB_ADR   nvStartADR + 3


#define NV_DY_ADR         nvStartADR + 4
#define NV_DM_ADR         nvStartADR + 5
#define NV_DD_ADR         nvStartADR + 6
#define NV_PN_ADR         nvStartADR + 7
#define NV_TCOR           nvStartADR + 8
#define NV_FRF_MSB        nvStartADR + 9   //radio TX freq  //added 12/12/13
#define NV_FRF_MS2        nvStartADR + 10  //unsigned long Hx
#define NV_FRF_MS1        nvStartADR + 11
#define NV_FRF_LSB        nvStartADR + 12
#define NV_CHECK_CHAR     nvStartADR + 31
#define NV_END_ADR        nvStartADR + 32

#define FSTEP             61.03515625

char inStr[20];
int  nvStartADR = NV_START_ADR;
byte adr;
byte val;
int iVal;

RFM69NEW::nvRamMap state = {};
RFM69NEW radio;
CMD_LINE my;

void setup() 
{
  Serial.begin(115200);
  Serial.print("  Moteino EEPROM Setup    ");
  Serial.println(VERSION);
  Serial.println("\n\n>[enter '?' for help]");  
}

void loop()
{
  byte argCnt = my.getSerialInput();   //check for an entry
  if (argCnt  > 0)
  {
    if (my.isCommand('?')) //query register values
    {
      Serial.println("\n**********************************************************************");
      Serial.println("\nEach Moteino is unique and must be initialized as such.");
      Serial.println("'MoteInit' lets you customize your Moteino radio for operation.");
      Serial.println("Values for frequency and temperature correction are examples of this.");
      Serial.println("\nUse 'q'(query) to display the state registers. The 'q' display also");
      Serial.println("shows individual register cmds.");
      Serial.println("\n  Ex. entering: 'f,-1230' will set the freq correction coef. to -1230 Hz");
      Serial.println("                'i,25' sets the radio's ID to 25");
      Serial.println("                'h,1' signifies a high pwr radio");
      Serial.println("\n'Clear nvRam' sets all state registers(EEPROM) to zero. Be Careful.");
      Serial.println("\n'reg,n,x' sets any register 'n' to the value 'x'");
      Serial.println("\n**********************************************************************");
    } 
 
    
    if (my.isCommand('q')) //query register values
    {
      Serial.println("\n*****************************************");
      nvReadAll();
    }
    
    if (my.isCommand('i')) // radio ID#
    {
      adr = NV_ID_ADR;
      val = atoi(my.arg[1]);
      Serial.print("> nv ID ADR ");
      Serial.print(adr); 
      Serial.print("   ID# ");
      Serial.println(val, DEC);
      state.nvID = val;
      EEPROM.write(adr, val);
      //radio.printState(0, state);
    }
    
   if (my.isCommand("Clear nvRam")) // init ram to 0x00
    {
    for ( int reg = NV_START_ADR; reg != NV_END_ADR; reg++)
      EEPROM.write(reg, 0x00);
    Serial.println("> State EEPROM Cleared!");
    }
    
   if (my.isCommand("reg")) // init ram to 0x00
    {
      int reg = atoi(my.arg[1]);
      byte val = atoi(my.arg[2]);
      EEPROM.write(reg, val);
      Serial.print("> reg ");
      Serial.print(reg);
      Serial.print(" = 0x");
      if(val < 0x10)
        Serial.print('0');
      Serial.print(val, HEX);
      Serial.print("\t");
      Serial.print(val);
     }
    
    if (my.isCommand('c')) // nv check character
    {
      adr = NV_CHECK_CHAR;
      Serial.print("> nv CHECK ADR ");
      Serial.print(adr); 
      Serial.print("   Check Char ");
      Serial.println(my.arg[1]);
      state.nvCHECK = *my.arg[1];
      EEPROM.write(adr, *my.arg[1]);
     // radio.printState(0, state);
    }
    
    if (my.isCommand('h')) //  set high or low pwr radio
    {
      adr = NV_OPT_ADR;
      val = atoi(my.arg[1]);
      Serial.print("> nv OPT ADR ");
      Serial.print(adr);
      if (val == 1)
      { 
        Serial.println("   HIGH Power Tx ");
        EEPROM.write(adr, EEPROM.read(adr) | 0x01);
        state.nvOPT = state.nvOPT | 0x01;
      } 
      else if (val == 0)
      {
        Serial.println("   LOW Power Tx ");
        state.nvOPT = state.nvOPT  ^ 0x01;
        EEPROM.write(adr, EEPROM.read(adr) ^ 0x01);
      }
      //radio.printState(0, state);      
    }    
    
    if (my.isCommand("fc")) //  freq correction int
    {
      adr = NV_FCOR_MSB_ADR;
      iVal = atoi(my.arg[1]);
      Serial.print("> nv Fcorr Adr ");
      Serial.print(adr); 
      Serial.print("   Fcorr ");
      Serial.println(iVal, DEC);
      state.nvFCOR = iVal;
      EEPROM.write(NV_FCOR_LSB_ADR, iVal);
      EEPROM.write(NV_FCOR_LSB_ADR + 1, iVal >> 8);
     // radio.printState(0, state);      
    }
    
    if (my.isCommand("fr")) //   radio tx freq Hz    unsigned long int
    {
      unsigned long int freq = strtol(my.arg[1], '\0', 0);
      Serial.print("> nv Radio Freq ");
      Serial.println(freq);
      Serial.println(freq, HEX);
      EEPROM.write(NV_FRF_LSB, (freq >> 24) & 0xff);
      EEPROM.write(NV_FRF_MS1, (freq >> 16) & 0xff);
      EEPROM.write(NV_FRF_MS2, (freq >> 8) & 0xff);
      EEPROM.write(NV_FRF_MSB, freq & 0xff );
    }  
      
    if (my.isCommand('y')) //  year
    {
      adr = NV_DY_ADR;
      val = atoi(my.arg[1]);
      Serial.print("> nv DY Adr ");
      Serial.print(adr); 
      Serial.print("   Year ");
      Serial.println(val, DEC);
      EEPROM.write(adr, val);
      state.nvDY = val;
    //  radio.printState(0, state);      
    }

    if (my.isCommand('m')) //  month
       {
        val = atoi(my.arg[1]);
        adr = NV_DM_ADR;
        Serial.print("> nv DM Adr ");
        Serial.print(adr); 
        Serial.print("   Month ");
        Serial.println(val, DEC);     
        state.nvDM = val;
        EEPROM.write(adr, val);
     //   radio.printState(0, state);      
       }
     if (my.isCommand('d')) //  day
     {
      val = atoi(my.arg[1]);
      adr = NV_DD_ADR;
      Serial.print("> nv DD Adr ");
      Serial.print(adr); 
      Serial.print("   Day ");
      Serial.println(val, DEC);   
      state.nvDD = val;
      EEPROM.write(adr, val);
     // radio.printState(0, state);      
     }
 
     if (my.isCommand('p')) //  sn
       {
        val = atoi(my.arg[1]);
        adr = NV_PN_ADR;
        Serial.print("> nv PN Adr ");
        Serial.print(adr); 
        Serial.print("   prod # ");
        Serial.println(val, DEC);     
        state.nvPN = val;
        EEPROM.write(adr, val);
      //  radio.printState(0, state);      
       }
       
    if (my.isCommand('r')) //  added ram
       {
        adr = NV_OPT_ADR;
        val = atoi(my.arg[1]);
        Serial.print("> nv OPT ADR ");
        Serial.print(adr);
        if (val == 1)
        { 
          Serial.println("   Has External nvRam ");
          EEPROM.write(adr, EEPROM.read(adr) | 0x02);
          state.nvOPT = state.nvOPT | 0x02;
        } 
        else if (val == 0)
        {
          Serial.println("   No External nvRam ");
          EEPROM.write(adr, EEPROM.read(adr) ^ 0x02);
          state.nvOPT = state.nvOPT ^ 0x02;
        }     
       // radio.printState(0, state);      
       }
    if (my.isCommand('t')) //  temperature coef
       {
        adr = NV_TCOR;
        iVal = atoi(my.arg[1]);
        Serial.print("> nv TCOR Adr ");
        Serial.print(adr); 
        Serial.print("   Temperature Correction ");
        Serial.println(iVal);
        EEPROM.write(adr, iVal);
       // EEPROM.write(adr + 1, iVal >> 8);     
        state.nvTCOR = iVal;
      //  radio.printState(0, state);      
       }       
  }  
}

void nvReadAll()                              //recalls values to non volatile memory
{
  for ( int reg = NV_START_ADR; reg != NV_END_ADR; reg++)
  {
    byte val = EEPROM.read(reg);
    Serial.print("reg ");
    if(reg < 10)
      Serial.print(' ');
    Serial.print(reg);
    Serial.print(" = 0x");
    if(val < 0x10)
      Serial.print('0');
    Serial.print(val, HEX);
    Serial.print("\t");
    Serial.print(val);
    switch (reg) 
    {
    case 0:  //ID
      Serial.print("\t(i) Radio ID = ");
      Serial.print(val);
      break;
    case 1:  //radio options
      Serial.print("\t    Options = ");
      if (val & 0x01)
        Serial.print("(h,1) High Power Tx");
      else
        Serial.print("(h,0) Low Power Tx");
      if (val & 0x02)
        Serial.print(", (r,1) Has External nvRam");
      else
        Serial.print(", (r,0) No External Ram");
      break;
    case 2:
      Serial.print("\t(fc) (+/-32767)  F Corr LSB");
      break;      
    case 3:
      Serial.print("\t                 F Corr MSB, ");
      Serial.print(" F Corr = ");
      Serial.print(256 * val + EEPROM.read(reg - 1));
      Serial.print(" Hz");
      break;   
    case 4:
      Serial.print("\t(y) Year = ");
      Serial.print(val);
      break;
    case 5:
      Serial.print("\t(m) Month = ");
      Serial.print(val);
      break;
    case 6:
      Serial.print("\t(d) Day = ");
      Serial.print(val);
      break;
    case 7:
      Serial.print("\t(p) Prod # = ");
      Serial.print(val);
      break;
    case 8:
      Serial.print("\t(t) Temp Corr = ");
      Serial.print((char)val, DEC);  //was int now char 12/2/13
      break;
    case 9:
      Serial.print("\t(fr)Frequency = ");
      unsigned long freq;
      freq =  0x1000000UL * EEPROM.read(reg + 3);
      freq += 0x10000UL * EEPROM.read(reg + 2);
      freq += 0x100UL * EEPROM.read(reg + 1);
      freq += val;
      Serial.print(freq);
      break; 
    case 13:
      Serial.print("\tStart of TBD Reg");
      break;
    case 30:
      Serial.print("\tEnd of TBD Reg");
      break;
    case 31:
      Serial.print("\t(c) Check Char = ");
      Serial.print((char)val);
      break;
    }
    Serial.println();
  } 
}
  
//  nvRamMap 
//0      byte nvID;     //ID of this radio
//1      byte nvOPT;    //installed options, bo = hipwr, b1 = ext serial ram
//2,3    int  nvFCOR;   //freq correction val
//4      byte nvDY;     //date code
//5      byte nvDM;
//6      byte nvDD;
//7      byte nvPN;     //production number per date code
//8      char nvTCOR;   //temperature cal. coef  ***char as of 12/1/13
//9      byte nvFrfMSB  //radio TX freq,  unsigned long int //added 12/12/13
//10     byte nvFrfMS2  //radio TX freq   
//11     byte nvFrfMS1  //radio TX freq
//12     byte nvFrfLSB  //radio TX freq
//13-30  byte reserved[18] = {};
//31     char nvCHECK;   //checks nv state, if struct changes size this will re-init it 

/*
    if (my.isCommand('')) //  year
       {
        val = atoi(my.arg[1]);
     
        state.nv = val;
        radio.printState(0, state);      
       }
*/
