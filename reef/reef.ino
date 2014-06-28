//Sistem 0.3
#include <UTFT.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <pt.h>
//#include <DS1307new.h>

static struct pt pt1, pt2;

byte aktuatori = 119;

//ovdje deklariram slimbe
//extern prog_uint16_t thermo[0x2400],
//                     ph[0x2400];

//Sensable pinovi multiplexor
int inh = A0;
int A = A3;
int B = A2;
int C = A1;

int CLOCK = A7;
int LATCH = A6;
int DATA = A5;

//parametri
float PH=0;
int ORP=0;
unsigned int EC=0;
unsigned int PPM=0;
float AH=0;
float AT=0;
float WT1=0;
float WT2=0;
float AWT=0;

//Atlas sajentifik data reading
String dataString="";
byte holding;

//kanali na multiplexoru do 8;
int AHT = 0;
int gPH = 1;
int gORP = 2;
int gEC = 3;
int DO = 4;
int DT = 0;

//set za lcd
UTFT myGLCD(SSD1963_800,38,39,40,41);

//fontovi
extern uint8_t DotMatrix_M[];
extern uint8_t SevenSegmentFull[];
extern uint8_t SmallFont[];


void getProbe(int probe) {
//funkcija koja čita data sa atlas sajentifik sranja
  Serial3.begin(38400);
  dataString = "";
    openProbeChannel(probe);
    delay(10);
    if (probe==gEC) {
      Serial3.print("P,3\r");
      Serial3.print("Z62\r");
    }
    if (probe==gPH) {
      Serial3.print(AWT,1);
      Serial3.print("\r");
    }
    Serial3.print("R\r");
//    Serial.println(probe);
    if(probe==gEC) delay(800);
    else delay(200);
    holding = 0;
    if(Serial3.available() > 1) {
      holding=Serial3.available();
      
      for(int i=0; i<holding; i++)
        dataString += char(Serial3.read());
          
//      Serial.print(probe);
//      Serial.print(": ");
//      Serial.println(dataString);
          
      if(probe==gPH){
        char dataChar[6];
        dataString.toCharArray(dataChar, 6);
        PH = atof(dataChar);
       
      }
      if(probe==gEC){
        char dataChar[6];
        String subString="";
        
        subString = dataString.substring(0,dataString.indexOf(','));
        subString.toCharArray(dataChar, 6);
        EC=atoi(dataChar);
        
        subString = dataString.substring(dataString.indexOf(',')+1,dataString.lastIndexOf(','));
        subString.toCharArray(dataChar, 6);
        PPM=atoi(dataChar);
        
      }
      if(probe==gORP) {
        char dataChar[4];
        if(dataString.startsWith("ÿ"))
          dataString = dataString.substring(1,5);
        else  dataString = dataString.substring(0,4);
        dataString.toCharArray(dataChar, 4);
        ORP = atoi(dataChar);
      }
    }
    else {
      //error probe treba dopisat
    }  
  Serial3.end();  
}

void getWaterTemp(int probe){
  //funkcija koja očitava temperaturu s one wire senzora
  openProbeChannel(probe);
  delay(10);  
  OneWire oneWire(15);
  DallasTemperature sensors(&oneWire);
  int numberOfDevices; 
  DeviceAddress tempDeviceAddress;
  sensors.begin();
  delay(10);
  numberOfDevices = sensors.getDeviceCount();

  
  sensors.requestTemperatures();
  for(int i=0;i<numberOfDevices; i++){
    if(sensors.getAddress(tempDeviceAddress, i)){
      float tempC = sensors.getTempC(tempDeviceAddress);
      if(i == 0){
        WT1=tempC;
      }
      else if (i == 1){
        WT2=tempC;
      }
      AWT=(WT1+WT2)/2;
      delay(100);
    }
  }
}

void getAirHumTemp(int probe){
  //temperatura i vlažnost zraka
  DHT dht(14, DHT22);
  dht.begin();
  openProbeChannel(probe);
  delay(10);
  AH = dht.readHumidity();
  AT = dht.readTemperature();
  
  if (isnan(AT) || isnan(AH)) {}
}

void openProbeChannel(int channel){
  //otvara portove na multiplekseru
  digitalWrite(inh, HIGH);
  delay(5);
  switch (channel) {
    case 0:                        //open channel Y0
      digitalWrite(A, LOW);        //S0=0
      digitalWrite(B, LOW);        //S1=0
      digitalWrite(C, LOW);        //S2=0
    break;

    case 1:                        //open channel Y1
      digitalWrite(A, HIGH);       //S0=1    
      digitalWrite(B, LOW);        //S1=0
      digitalWrite(C, LOW);        //S2=0
    break;

    case 2:                        //open channel Y2   
      digitalWrite(A, LOW);        //S0=0   
      digitalWrite(B, HIGH);       //S1=1
      digitalWrite(C, LOW);        //S2=0
    break;

    case 3:                        //open channel Y3
      digitalWrite(A, HIGH);       //S0=1    
      digitalWrite(B, HIGH);       //S1=1
      digitalWrite(C, LOW);        //S2=0
    break;
    case 4:                        //open channel Y4
      digitalWrite(A, LOW);        //S0=0  
      digitalWrite(B, LOW);        //S1=0
      digitalWrite(C, HIGH);       //S2=1
    break;

    case 5:                        //open channel Y5
      digitalWrite(A, HIGH);       //S0=1    
      digitalWrite(B, LOW);        //S1=0
      digitalWrite(C, HIGH);       //S2=0
    break;

    case 6:                        //open channel Y6   
      digitalWrite(A, LOW);        //S0=0    
      digitalWrite(B, HIGH);       //S1=1
      digitalWrite(C, HIGH);       //S2=0
    break;

    case 7:                        //open channel Y7
      digitalWrite(A, HIGH);       //S0=1    
      digitalWrite(B, HIGH);       //S1=1
      digitalWrite(C, HIGH);       //S2=0
    break;
    default: break;
  }
  digitalWrite(inh, LOW);
  delay(5);
  Serial3.print("\r");  //Get rid of errant data
  delay(5);
}

void data(){
  myGLCD.setFont(DotMatrix_M);
  myGLCD.print("*____",550,370);
  
  getAirHumTemp(AHT);
  
  myGLCD.print("AIR:",20,75);
  myGLCD.printNumF(AT,1,100,60);
  myGLCD.print("C",175,60);
  myGLCD.printNumF(AH,1,100,90);
  myGLCD.print("%",175,90);
  
  myGLCD.print("**___",550,370);
  getWaterTemp(DT);
  myGLCD.print("WATER TEMP:",20,150);
  myGLCD.setFont(SevenSegmentFull);
  myGLCD.printNumF(AWT,1,20,185);
  myGLCD.print("C",160,185);
  myGLCD.setFont(DotMatrix_M);
  myGLCD.print("T1:",20,240);
  myGLCD.printNumF(WT1,2,70,240);
  myGLCD.print("T2:",20,265);
  myGLCD.printNumF(WT2,2,70,265);
  myGLCD.setFont(SevenSegmentFull);
  myGLCD.print("C",160,240);
  
  myGLCD.setFont(DotMatrix_M);
  myGLCD.print("***__",550,370);
  getProbe(gPH);
  PH=PH+(PH*0.09);
  myGLCD.print("WATER pH:",20,320);
  myGLCD.setFont(SevenSegmentFull);
  myGLCD.print("     ",20,355);
  myGLCD.printNumF(PH,2,20,355);
  
  myGLCD.setFont(DotMatrix_M);
  myGLCD.print("***__",550,370);
  getProbe(gEC);
  myGLCD.print("CONDUCTIVITY:",250,80);
  myGLCD.setFont(SevenSegmentFull);
  myGLCD.print("      ",250,115);
  float floatEC=float(EC)/1000;
  myGLCD.printNumF(floatEC,1,250,115);
  
  myGLCD.setFont(DotMatrix_M);
  myGLCD.print("SALINITY:",250,200);
  myGLCD.print("       ",250,235);
  myGLCD.setFont(SevenSegmentFull);
  float floatPPM=float(PPM)/1000;
  myGLCD.printNumF(floatPPM,1,250,235);
  
  myGLCD.setFont(DotMatrix_M);
  myGLCD.print("****_",550,370);
  getProbe(gORP);
  myGLCD.print("WATER ORP:",250,320);
  myGLCD.setFont(SevenSegmentFull);
  myGLCD.print("     ",250,355);
  myGLCD.printNumI(ORP,250,355);
  
  myGLCD.setFont(DotMatrix_M);
  
}

void actuators(){
  myGLCD.setFont(DotMatrix_M);
  myGLCD.print("*****",550,370);
  
  if(AH >= 75) {bitClear(aktuatori,2);}
  else bitSet(aktuatori,2);
  
  if(AWT >= 27.5) {bitClear(aktuatori,1);}
  else bitSet(aktuatori,1);
  
  myGLCD.print("      ",520,80);
  myGLCD.print("HEATER",520,80);
  myGLCD.print(" ",680,80);
  myGLCD.printNumI(bitRead(aktuatori, 6),680,80);
  myGLCD.print("       ",520,120);
  myGLCD.print("Hum.FAN",520,120);
  myGLCD.print(" ",680,120);
  myGLCD.printNumI(!bitRead(aktuatori, 2),680,120);
  myGLCD.print("       ",520,160);
  myGLCD.print("Tem.FAN",520,160);
  myGLCD.print(" ",680,160);
  myGLCD.printNumI(!bitRead(aktuatori, 1),680,160);
  myGLCD.print("       ",520,200);
  myGLCD.print("LED.FAN",520,200);
  myGLCD.print(" ",680,200);
  myGLCD.printNumI(!bitRead(aktuatori, 3),680,200);
  myGLCD.print("    ",520,240);
  myGLCD.print("PUMP",520,240);
  myGLCD.print(" ",680,240);
  myGLCD.printNumI(bitRead(aktuatori, 5),680,240);
  myGLCD.print("   ",520,280);
  myGLCD.print("LED",520,280);
  myGLCD.print(" ",680,280);
  myGLCD.printNumI(!bitRead(aktuatori, 7),680,280);
  
  digitalWrite(LATCH, LOW);
  shiftOut(DATA, CLOCK, MSBFIRST, aktuatori);
  digitalWrite(LATCH, HIGH);
  
  myGLCD.print("_____",550,370);
}

void printData(){
  
  //mjenjaj koordinate
  //a ovdje treba mjenjat koordinate
  //ovdje ispišeš kod toga
  
  //niski napon 0-3
  //visoki 4-7
  
  /*
0=null
1=mali down
2=mali up
3=veliki
4=null
5=pumpa
6=grijač
7=led
*/
  /*
  myGLCD.setColor(VGA_WHITE);
  
  myGLCD.setFont(SevenSegmentFull);
  //u ovoj funkciji ispisuješ sve 7segmentnim fontom
  myGLCD.printNumF(PH,2,100,160);
  myGLCD.printNumI(EC,300,160);
  
  if(AH > 90) myGLCD.setColor(VGA_YELLOW);
  myGLCD.printNumF(AH,1,75,50);
  myGLCD.setColor(VGA_WHITE);
  
  if(AT > 30) myGLCD.setColor(VGA_YELLOW);
  myGLCD.printNumF(AT,1,330,50);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("C",470,50);
  
  if(WT1 > 27){
    myGLCD.setColor(VGA_YELLOW);
    bitClear(aktuatori, 1);//pali male vent
    bitClear(aktuatori, 2);
  }
  
  if(WT1 > 22 && WT1 < 27){
    bitSet(aktuatori, 1);//gasi
    bitSet(aktuatori, 2);
    bitSet(aktuatori, 6);//gasi grijač
  }
  if(WT1 < 21){
    myGLCD.setColor(VGA_BLUE);
    bitClear(aktuatori, 6);// pali grijač
  }
    
  myGLCD.printNumF(WT1,2,190,390);
  myGLCD.setColor(VGA_WHITE);
  
  
  if(WT2 > 27){
    myGLCD.setColor(VGA_YELLOW);
    bitClear(aktuatori, 1);//pali male vent
    bitClear(aktuatori, 2);
  }
  
  if(WT2 > 22 && WT2 < 27){
    bitSet(aktuatori, 1);//gasi
    bitSet(aktuatori, 2);
    bitSet(aktuatori, 6);//gasi grijač
  }
  if(WT2 < 21){
    myGLCD.setColor(VGA_BLUE);
    bitClear(aktuatori, 6);
  }
  myGLCD.printNumF(WT2,2,360,390);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("C",530,390);
  
  myGLCD.printNumI(ORP,100,280);
  myGLCD.printNumI(PPM,300,280);
  
  digitalWrite(LATCH, LOW);
  shiftOut(DATA, CLOCK, MSBFIRST, aktuatori);
  digitalWrite(LATCH, HIGH);
  /**/
}

void mainScreen(){
  
  myGLCD.clrScr();
  //myGLCD.drawRect(5,5,795,475);
  myGLCD.setFont(DotMatrix_M);
  myGLCD.setColor(VGA_BLUE);
  myGLCD.print("#LabOS Reef Controler",5,5);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("ETFOS",715,5);
  myGLCD.setColor(VGA_BLUE);
  myGLCD.print("VERSION 0.97",600,450);
  /*
  //myGLCD.drawBitmap(10,10,96,96,humidity);
  myGLCD.drawRect(5,5,250,110);
  myGLCD.print("HUMIDITY",75,20); 
  myGLCD.print("%",210,70);
  
//  myGLCD.drawBitmap(5,130,96,96,ph);
  myGLCD.drawRect (5,115,250,230);
  myGLCD.print("pH Value",80,120);
  
  //myGLCD.drawBitmap(10,240,96,96,orp1);
  myGLCD.drawRect (5,235,250,350);
  myGLCD.print("ORP",110,250);
    
  //drawBitmapFnc(270,10,96,96,Wtemp);
  myGLCD.drawRect(255,5,510,110);
  myGLCD.print("ROOM TEMP. ",320,20); 
 
// drawBitmapFnc(10,370,96,96,thermo);
 myGLCD.drawRect(5,355,570,475);
 myGLCD.print("WATER",90,390);
 myGLCD.print("TEMP.",90,430);
 myGLCD.print("T1",270,360);
 myGLCD.print("T2",430,360);
 myGLCD.drawLine(355,375,355,455);
 
  
  */
  // u ovoj dotmatrix fontom
  //ovdje dizajniram matrixa
  //tu negdje
  //i tu pišeš vie variabli
  //npr: PH: buffalo soliđer!
  //i tu ćeš dodavati sličice termometra i tako to...
  //ali najprije
  //treba provjerit librarije
  myGLCD.setColor(VGA_WHITE);
}

void enterScreen(){
  myGLCD.clrScr();
  //myGLCD.drawRect(5,5,795,475);
  myGLCD.setFont(DotMatrix_M);
  myGLCD.setColor(VGA_BLUE);
  myGLCD.print("#LabOS Reef Controler",CENTER,150);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("ETFOS",CENTER,200);
  myGLCD.setColor(VGA_BLUE);
  myGLCD.print("VERSION 0.97 ",CENTER,250);
}

static int protothread1(struct pt *pt, int interval) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) { // never stop 
    /* each time the function is called the second boolean
    *  argument "millis() - timestamp > interval" is re-evaluated
    *  and if false the function exits after that. */
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis(); // take a new timestamp
    data();
    actuators();
  }
  PT_END(pt);
}

static int protothread2(struct pt *pt, int interval) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) { // never stop 
    /* each time the function is called the second boolean
    *  argument "millis() - timestamp > interval" is re-evaluated
    *  and if false the function exits after that. */
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis(); // take a new timestamp
    bitClear(aktuatori, 7);
    actuators();
  }
  PT_END(pt);
}

/*
void drawBitmapFnc(int x, int y, int sx, int sy, prog_uint16_t* name){
  myGLCD.drawBitmap(x,y,sx,sy,name);
}*/
void setup(){
  PT_INIT(&pt1);
  pinMode(LATCH, OUTPUT);
  digitalWrite(LATCH, HIGH);
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  
  digitalWrite(LATCH, LOW);
  shiftOut(DATA, CLOCK, MSBFIRST, aktuatori);
  digitalWrite(LATCH, HIGH);
  
  Serial.begin(9600);
  myGLCD.InitLCD();
  myGLCD.clrScr();
  
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(inh, OUTPUT); 
  digitalWrite(inh, LOW);
  
  enterScreen();
  delay(2000);
  mainScreen();
  data();
  actuators();
  data();
  actuators();
}

void loop(){
  //printData();
  protothread1(&pt1, 20000);
}

/*
0=null
1=mali down
2=mali up
3=veliki
4=null
5=pumpa
6=grijač
7=led
*/
void serialEvent(){
  String dataString="";
  if(Serial.available() > 0) dataString=Serial.readStringUntil(';');
  if(dataString.startsWith("p")){
    dataString=dataString.substring(dataString.indexOf(":")+1,dataString.length());
    bitClear(aktuatori, dataString.toInt());
  }
  
  else if(dataString.startsWith("g")){
    dataString=dataString.substring(dataString.indexOf(":")+1,dataString.length());
    bitSet(aktuatori, dataString.toInt());
  }
  
  else if(dataString.startsWith("ledON")){
    bitClear(aktuatori, 7);
    bitClear(aktuatori, 3);
  }
  
  else if(dataString.startsWith("ledOFF")){
    bitSet(aktuatori, 7);
    bitSet(aktuatori, 3);
  }
  
  else if(dataString.startsWith("pump")){
    bitClear(aktuatori, 5);
  }
  
  digitalWrite(LATCH, LOW);
  shiftOut(DATA, CLOCK, MSBFIRST, aktuatori);
  digitalWrite(LATCH, HIGH);
  //mainScreen();
  
  actuators();

}
