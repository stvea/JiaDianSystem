#include <Keypad.h>
#include <String.h>
#include <avr/wdt.h>
#define TOPTIME 30000
#define TIMEOUT WDTO_8S
const byte ROWS = 4; 
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
    {'D', '#', '0', '*'},
    {'C', '9', '8', '7'},
    {'B', '6', '5', '4'},
    {'A', '3', '2', '1'}
};
byte rowPins[ROWS] = {45, 43, 41, 39}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {53, 51, 49, 47};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
char phoneNumber[12] = "";
unsigned long startTime;
int number_i = 0;
bool Index = true;//首页显示
bool Introduce = false;//首页显示
bool Choose = false;
bool Phone = false;
bool isElectrical = true;
bool isMobile = true;
bool initiaIndex = true;//首页显示
bool initiaIntroduce = false;
bool initiaChoose = false;
bool initiaPhone = false;
bool backIndex = false;

void controlHMI(char a[]){
  Serial2.write(a);
  Serial.println(a);
  Serial2.write(0XFF);
  Serial2.write(0XFF);
  Serial2.write(0XFF);
}

bool readSerial(){
  //Serial.println();
  int i=1;
  while(Serial1.available()>0){
    char inChar = Serial1.read();
    Serial.print(inChar); 
    if(inChar=='\n'){
      Serial1.flush();
    }else if(inChar!='\r'){
      //Serial.print(inChar); 
      if(inChar=='K'){
        //return true;
      }else if(inChar=='R'){
        return false;
      }
    }
  }
  return true;
}


bool sendMsg(char num[]){
  Serial1.println("AT+CMGF=1");
  delay(100);
  readSerial();
  Serial1.println("AT+CSCS=\"GSM\"");
  delay(100);
  readSerial();
  Serial1.println("AT+CMGS=\"13951807086\"");
  delay(500);
  readSerial();
  if(isElectrical){
    Serial1.print("JD:");
  }else{
    Serial1.print("JZ:");
  }
  Serial.println(num);
  Serial1.print(num);
  delay(100);
  Serial1.write(0x1A);
  //delay(2000);
  //Serial3.println();
  if(readSerial()){
    return true;
  }else{
    return false; 
  }
}

void setup() {
  // put your setup code here, to run once:
 Serial2.begin(9600);
  Serial.begin(9600);//显示屏串口
  Serial1.begin(115200);//短信模块
  controlHMI("page index");
  wdt_enable(TIMEOUT);

}

void loop() {
  //controlHMI("get 123");
  char customKey = customKeypad.getKey();
  if(customKey){
    Serial.println(customKey);
  }
  if(Index){
    if(initiaIndex){
      controlHMI("page index");
      initiaIndex = false;
    }else if(customKey){
      Introduce = true;
      Index = false;
      initiaIntroduce = true;
    }
  }else if(Introduce){
    if(initiaIntroduce){
      startTime = millis();
      controlHMI("page introduce");
      initiaIntroduce = false;
    }
    if(customKey){
      if(customKey == 'A'){
        isElectrical = true;
        Choose = true;
        Introduce = false;
        initiaChoose = true;
      }else if(customKey == 'B'){
        isElectrical = false;
        Choose = true;
        initiaChoose = true;
        Introduce = false;
      }
    }
  }else if(Choose){
    if(initiaChoose){
      if(isElectrical){
        controlHMI("page chooseD");
      }else{
        controlHMI("page chooseZ");
      }
      startTime = millis();
      initiaChoose = false;
    }
    if(customKey){
       if(customKey == 'C'){
         isMobile = true;
         Choose = false;
         Phone = true;
         initiaPhone = true;
       }else if(customKey == 'D'){
         isMobile = false;
         Choose = false;
         Phone = true;
         initiaPhone = true;
       }
    }
  }else if(Phone){
    if(initiaPhone){
      startTime = millis();
      controlHMI("page phone");
      if(!isMobile){
        phoneNumber[0]='0';
        phoneNumber[1]='2';
        phoneNumber[2]='5';
        number_i = 3;
      }
      char asd[] = "t0.txt=\"           \"";
      int j=0;
      for(int i=8;i<19;i++){
        if(phoneNumber[j]!=0)
          asd[i] = phoneNumber[j];
        j++;
      }
      controlHMI(asd);
      initiaPhone = false;
    }
    if(number_i==11){
      if(sendMsg(phoneNumber)){
        controlHMI("page success");
        delay(3000);
        backIndex = true;
      }else{
        controlHMI("page fail");
        delay(3000);
        backIndex = true;
      }
      
    }
    if(customKey){
      if(customKey!='A'&&customKey!='B'&&customKey!='C'&&customKey!='D'&&customKey!='#'){
        phoneNumber[number_i] = customKey;
        number_i++;
      }else if(customKey == '#'){
        //number_i--;
        phoneNumber[--number_i] = ' ';
      }
      char asd[] = "t0.txt=\"           \"";
      int j=0;
      for(int i=8;i<19;i++){
        if(phoneNumber[j]!=0)
          asd[i] = phoneNumber[j];
        j++;
      }
      Serial.println(asd);
      controlHMI(asd);
    }

  }
  if(millis()-startTime>TOPTIME){
    startTime=millis();
    backIndex = true;
    }
    
  if(!Phone){
    for(int i=0;i<12;i++)
      phoneNumber[i] = 0;
    number_i = 0;
  }
  if(customKey=='*'||backIndex){
    Index = true;
    initiaIndex = true;
    initiaChoose = initiaIntroduce = initiaPhone = false;
    Choose = Introduce = Phone = backIndex = false;
  }
  wdt_reset();
}
