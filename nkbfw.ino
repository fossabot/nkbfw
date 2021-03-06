//Make sure to install all the library before uploading code.
//nut1414
#include <EEPROM.h>
#include <FastLED.h>
#include <Keyboard.h>
#include <Bounce2.h>

/*configuration*/

//#define HASLED 
#define LEDCHIPSET WS2812   //see FastLED list of supported chipset
#define LEDCLORDER GRB
#define LEDPIN 17
#define LEDCOUNT 1  //NEVER under any circumstance set led count to 0, this will crash the board and you have to ground the reset pin in order to upload any code again. (has led only)
#define DEFAULTR 255
#define DEFAULTG 192
#define DEFAULTB 203
#define TURNOFFINBUILTLED

#define EEPROMSAVE
#define EEPROMLK 100 //eeprom location for storing key byte
#define EEPROMCHK 1023 //eeprom location for checking first time write
#define EEPROMLED -90 //eeprom led offset

/*set your key matrix here*/
#define COLKEYMATRIX 4
#define ROWKEYMATRIX 1
#define KEYCOUNT 4

/*default pin of the switch*/
const uint8_t colpin[COLKEYMATRIX]=
  {
    9,8,10,16
  };
const uint8_t rowpin[ROWKEYMATRIX]=
  {
    NULL
  };

/*default key change your key binding here*/
uint8_t key[COLKEYMATRIX][ROWKEYMATRIX],defaultkey[COLKEYMATRIX][ROWKEYMATRIX]=
  {
    'z','x','c','v'
  };


//////////////////////////////////////////
#ifdef HASLED
#if LEDCOUNT <= 0
#define LEDCOUNT 1 //NEVER under any circumstance set led count to 0, this will crash the board and you have to ground the reset pin in order to upload any code again.
#endif
#else
#define LEDCOUNT 0
#endif
Bounce button[KEYCOUNT];
#ifdef HASLED 
CRGBArray<LEDCOUNT> leds;
const int drgb[]{DEFAULTR,DEFAULTG,DEFAULTB};
#endif
String serialstr;
char parsechar[24],cmd[5];
int i,j,count;

bool firstc = false;

void setup(){
  //writing EEPROM default if have not written before
  #ifdef EEPROMSAVE
if(EEPROM.read(EEPROMCHK) == 255)
  {
    EEPROM.write(EEPROMCHK, 127);
    firstc=true;
  }
#endif

//init led
 #ifdef HASLED 
  FastLED.addLeds<LEDCHIPSET,LEDPIN,LEDCLORDER>(leds,LEDCOUNT);
  
  for (int k = 0;k<LEDCOUNT;k++)
   {
   
    if (firstc==true)
    {
      for (int l= 0 ; l<3;l++)
      {
      if (EEPROM.read(EEPROMLK+EEPROMLED+l)!=drgb[l])
        {
        EEPROM.write(EEPROMLK+EEPROMLED+l,drgb[l]);
        }
        leds[k][l]=EEPROM.read(EEPROMLK+EEPROMLED+l);
      }
      
    }else{
    for (int l= 0 ; l<3;l++)
      {
        leds[k][l]=EEPROM.read(EEPROMLK+EEPROMLED+l);
      }
      }
    FastLED.show();
   }
  
 #endif

  
//begin keyboard
Keyboard.begin();
Serial.begin(9600);


//attach bounce
int buttoncount;

 for (int k = 0; k < ROWKEYMATRIX ; k++) 
 {
  pinMode(rowpin[k],OUTPUT);
  for (int l = 0; l < COLKEYMATRIX; l++) 
  {
    key[l][k] = defaultkey[l][k];
    button[buttoncount] = Bounce();                                     
    button[buttoncount].attach(colpin[l], INPUT_PULLUP); 
    button[buttoncount].interval(1);
    //do read/write eeproms
    //for the first boot
    #ifdef EEPROMSAVE
    if (firstc==true)
    {
      //if the key that is already been written isn't the same as the default, overwrite the key
      if (EEPROM.read(EEPROMLK+buttoncount)!=defaultkey[l][k])
      {
      EEPROM.write(EEPROMLK+buttoncount,defaultkey[l][k]);
      }
    }
    key[l][k] = EEPROM.read(EEPROMLK+buttoncount);
    #endif
    buttoncount++;
  }
 }


 firstc=false;

  
 //option to disable inbuilt led
 #ifdef TURNOFFINBUILTLED
  pinMode(LED_BUILTIN, INPUT);
  pinMode(LED_BUILTIN_TX,INPUT);
  pinMode(LED_BUILTIN_RX,INPUT);
 #endif

}


void loop() {
 count = 0;
 for (j = 0; j < ROWKEYMATRIX; j++) 
 {
  digitalWrite(rowpin[j],HIGH);
  
  for(i = 0; i < COLKEYMATRIX; i++)
    {
      
      button[count].update();
      
      if (button[count].fell()){
       Keyboard.press(key[i][j]);
       
     }
      if (button[count].rose()){
      Keyboard.release(key[i][j]);
     }
     count++;
    }
   digitalWrite(rowpin[j],LOW);
 }
 
 while (Serial.available() > 0) 
 {
    serialstr = Serial.readString();
    serialParser(serialstr);
 }
#ifdef HASLED
 FastLED.show();
#endif
}



//parsing the serial input
void serialParser(String input)
{
  int argi1,argi2,argi3,argi4,argi5,argi6;
  uint8_t argb1,argb2,argb3;
  //converting serial.tostring to char array
  input.toCharArray(parsechar,20);
  
  //scan for cmd
  sscanf(parsechar,"%s",cmd);
  
  
  if(strcmp(cmd,"CK")==0)
   {
    //change key
    //arg1 = X in the matrix, arg2 = Y in the matrix, argb1 is the letter you want to replace in hexadecimal (look up 'ASCII Table')
   sscanf(parsechar,"%s %d %d %x",cmd,&argi1,&argi2,&argb1);
   //making sure to not setting unknown key loc
   if((argi1<COLKEYMATRIX)||(argi2<ROWKEYMATRIX))
   {
   key[argi1][argi2] = argb1;
   Serial.println("OK");
   }else Serial.println("INVALID");
   
  }else if(strcmp(cmd,"DK")==0)
  {
    //default key
    for (int k = 0; k < ROWKEYMATRIX ; k++) 
      {
        
        for (int l = 0; l < COLKEYMATRIX; l++) 
          {
            
            key[l][k] = defaultkey[l][k];
            
          }
      }
      Serial.println("OK");
      
  }else if(strcmp(cmd,"SS")==0)
  {
    //save
    #ifdef EEPROMSAVE
    int b = 0;
   
    for (int k = 0; k < ROWKEYMATRIX ; k++) 
    {
     for (int l = 0; l < COLKEYMATRIX; l++) 
     {
      //if the key that is already been written isn't the same as the default, overwrite the key
      if (EEPROM.read(EEPROMLK+b)!=key[l][k])
      {
       EEPROM.write(EEPROMLK+b,key[l][k]);
       Serial.println("CHANGE");
       Serial.println(EEPROM.read(EEPROMLK+b));
       Serial.println("LOC");
       Serial.println(EEPROMLK+b);
      }
      b++;
     }
    
    }
  //led
  #ifdef HASLED
    for (int k = 0;k<LEDCOUNT;k++)
    {
      for (int l= 0 ; l<3;l++)
      {
      if (EEPROM.read(EEPROMLK+EEPROMLED+l)!=leds[k][l])
        {
        EEPROM.write(EEPROMLK+EEPROMLED+l,leds[k][l]);
        Serial.println("CHANGE");
        Serial.println(EEPROM.read(EEPROMLK+EEPROMLED+l));
        Serial.println("LOC");
        Serial.println(EEPROMLK+b);
        }
      }
    }
    #endif
    #endif 
    Serial.println("OK");
    
  }else if(strcmp(cmd,"LC")==0)
  {
    //LED Color Change
    #ifdef HASLED
    //argi1 = r, argi2 = g, argi3 = b 0-255
    sscanf(parsechar,"%s %d %d %d",cmd,&argi1,&argi2,&argi3);
    
    for(int startpos = 0; startpos < LEDCOUNT; startpos++)
      {   
       leds[startpos].r = argi1;
       leds[startpos].g = argi2;
       leds[startpos].b = argi3; 
    }
    #endif
    Serial.println("OK");
    
  }else if(strcmp(cmd,"DL")==0)
  {
    //default for led
    #ifdef HASLED
    for(int startpos = 0; startpos < LEDCOUNT; startpos++)
      {   
       leds[startpos].r = DEFAULTR;
       leds[startpos].g = DEFAULTG;
       leds[startpos].b = DEFAULTB; 
      }
    #endif
      Serial.println("OK");
      
  }else if(strcmp(cmd,"IN")==0)
  {
    Serial.print("KEY,");
    Serial.print("COL,");
    Serial.print("ROW,");
    Serial.print("LED,");
    Serial.println("RGB,");
    Serial.print(KEYCOUNT);
    Serial.print(",");
    Serial.print(COLKEYMATRIX);
    Serial.print(",");
    Serial.print(ROWKEYMATRIX);
    Serial.print(",");
    Serial.print(LEDCOUNT);
    Serial.print(",");
    #ifdef HASLED
    Serial.print(leds[0].r);
    Serial.print("|");
    Serial.print(leds[0].g);
    Serial.print("|");
    Serial.print(leds[0].B);
    Serial.println(",");
    #else
    Serial.print(0);
    Serial.print("|");
    Serial.print(0);
    Serial.print("|");
    Serial.print(0);
    Serial.println(",");
    #endif
    
  }else if(strcmp(cmd,"PG")==0)
  {
    Serial.println(1);
  }else Serial.println("INVALID");
  }
   
