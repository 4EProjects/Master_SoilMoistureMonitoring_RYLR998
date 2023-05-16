/* 
  **** Master/Sender ****
  Baudrate  : 9600 - AT+IPR=9600
  Address   : 1    - AT+ADDRESS=1
  NETWORKID : 18   - AT+NETWORKID=18
  
  *** Library *****
- Adafruit_GFX    : https://github.com/adafruit/Adafruit-GFX-Library
- Adafruit_ST7735 : https://github.com/adafruit/Adafruit-ST7735-Library
- RTClib          : https://github.com/adafruit/RTClib

*/

//==================Including the libraries==================
#include <Arduino.h>                   
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <icon.h>
#include "RTClib.h"
#include <EEPROM.h>
#define ONBuzzer   1
#define OFFBuzzer  0
// Information
#define Model          "Model: 4E0523GSDA   "
#define SeriaNumber    "S/N  : SN01         "
#define FirmwareVision "FW   : V1.0 328P    "
#define Author         "      4E Projects   "
#define SDT            "     036.788.0317   "
#define Origin         "   Made in VietNam  "

// Defines the slave/destination address.
#define slave_Address   02
#define master_Address  01

// Initialize "SoftwareSerial" and set the PIN used for RX and TX.
SoftwareSerial ReyaxLoRa(5, 4); //--> RX, TX
#define Down_Pin A0
#define Enter_Pin A1
#define Up_Pin A2
#define Buzzer_Pin A3

const unsigned long TIMEOUT = 20000;    // Thời gian chờ tối đa (20 giây)
unsigned long lastSerialTime = -20000;    // Lưu thời gian gửi tin nhắn gần nhất
bool buzzerState = false;    // Lưu trạng thái hiện tại của LED
unsigned long lastBlinkTime = 0; // Lưu thời gian nhấp nháy gần nhất

// initialize RTC library
RTC_DS1307 rtc;
DateTime   now;

// For LCD 1.8inch ST7735
#define TFT_CS        10
#define TFT_RST        8 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         9
// LED-3.3V ; SCK-D13 ; SDA-D11

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
// color definitions
const uint16_t  Display_Color_Black            = 0x0000;
const uint16_t  Display_Color_Backgroup        = 0x0000; // Black
const uint16_t  Display_Color_Blue             = 0x023e;
const uint16_t  Display_Color_Red              = 0xF800;
const uint16_t  Display_Color_Green            = 0x07E0;
const uint16_t  Display_Color_Cyan             = 0x07FF;
const uint16_t  Display_Color_Magenta          = 0xF81F;
const uint16_t  Display_Color_Yellow           = 0xFFE0;
const uint16_t  Display_Color_White            = 0xFFFF;

String TemperatureValue;
String HumidityValue;
String MoistureValue;
String BateryPercent;
int w_BateryPercent = 1;  // 1->22 px
uint16_t BatteryColor;

unsigned long previousMillisDisplay = -20000;  
//unsigned long currentMillisDisplay  = 0;
const uint16_t timeDisplay = 20000;   

int8_t BuzzerState = 0;
String _rssi;

int16_t
state = 0,   
pre_state = 1;

// Vị trí Text TimeSet DaySet |  Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc   11-22-2023
uint8_t day_x_pos         = 64;
uint8_t month_x_pos       = day_x_pos + 6*3;
uint8_t year_x_pos        = day_x_pos + 6*8;
uint8_t hour_x_pos        = 70;
uint8_t minute_x_pos      = hour_x_pos + 6*3;
uint8_t buzzer_x_pos      = 64;
uint8_t date_y_pos        = 30;
uint8_t time_y_pos        = 40;
uint8_t buzzer_y_pos      = 50;
uint8_t addr_slave_y_pos  = 60;
uint8_t addr_master_y_pos = 70;

void EEPROM_SaveSetting()
{
  EEPROM.update(0, BuzzerState);

}
void EEPROM_LoadSetting()
{
  EEPROM.get(0, BuzzerState);
}

void RTC_display()
{   
   // DateTime now = rtc.now();
    char _buffer[11];
    char dow_matrix[7][10] = {" SUNDAY  ", " MONDAY  ", " TUESDAY ", "WEDNESDAY", " THURSDAY", " FRIDAY  ", " SATURDAY"};
    //static byte previous_dow = 8;

    // print time
    tft.setTextSize(2);
    sprintf( _buffer, "%02u:%02u:%02u", now.hour(), now.minute(), now.second() );
    tft.setCursor(0, 7);
    tft.setTextColor(ST7735_RED, Display_Color_Backgroup);
    tft.print(_buffer);
    // print date
    sprintf( _buffer, "%02u/%02u/%04u", now.day(), now.month(), now.year() );
    tft.setTextSize(1);
    tft.setFont();
    tft.setTextColor(ST7735_YELLOW, Display_Color_Backgroup);
    tft.setCursor(99, 14);
    tft.print(_buffer);
    // print day of the week
    // tft.setCursor(102, 4);
    // if( previous_dow != now.dayOfTheWeek() )
    // {
    //   previous_dow = now.dayOfTheWeek();
      tft.setCursor(102, 4);
      tft.print( dow_matrix[now.dayOfTheWeek()] );
    // }
}

void Screen1Data()
{ 
  RTC_display();  // Hiển thị thời gian, ngày tháng - display time, date
  tft.setTextSize(1); 
  tft.setTextColor(ST7735_WHITE, Display_Color_Blue);
  // Giá trị nhiệt độ môi trường
  tft.setCursor(14, 47);
  tft.print(TemperatureValue);
  // Giá trị độ ẩm môi trường
  tft.setCursor(87, 47);
  tft.print(HumidityValue); tft.print("% ");
  // Giá trị độ ẩm đất
  tft.setCursor(14, 85);
  tft.print(MoistureValue); tft.print("% ");
  // Giá trị phần trăm pin
  tft.setCursor(87, 85);
  tft.print(BateryPercent); tft.print("% ");
  w_BateryPercent = map(BateryPercent.toInt(), 0, 100, 1, 22);
  if(w_BateryPercent < 1) w_BateryPercent = 1;
  if(w_BateryPercent > 22) w_BateryPercent = 22;
  tft.fillRect(121, 81, w_BateryPercent, 13, BatteryColor);
  if(BateryPercent.toInt() > 70) BatteryColor = ST7735_GREEN;
  else if(BateryPercent.toInt() <= 70 && BateryPercent.toInt() > 30 ) BatteryColor = ST7735_YELLOW;
  else if(BateryPercent.toInt() <= 30) BatteryColor = ST7735_RED;
  // RSSI
  tft.setTextColor(ST7735_GREEN, Display_Color_Backgroup);
  tft.setCursor(127, 113);
  tft.print(":"); tft.print(_rssi);
}

void Screen1_display()
{ 
  if(state != pre_state) 
  { 
    tft.fillScreen(Display_Color_Backgroup);
    pre_state = state;
    tft.drawLine(0,27,160,27,ST7735_WHITE);
    // Nhiệt độ
    tft.fillRect(10, 33, 69, 35, Display_Color_Blue);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(14, 37);
    tft.print("Temp");
    tft.drawBitmap(55, 35, TempIcon, 16, 28, ST7735_WHITE);
    // Độ ẩm Không khí
    tft.fillRect(82, 33, 69, 35, Display_Color_Blue);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(87, 37);
    tft.print("Hum");
    tft.drawBitmap(126, 35, HumIcon, 18, 25, ST7735_WHITE);
    // Độ ẩm Đất
    tft.fillRect(10, 71, 69, 35, Display_Color_Blue);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(14, 75);
    tft.print("Soil");
    tft.drawBitmap(48, 73, SoilIcon, 26, 26, ST7735_WHITE);
    // Phần trăm pin
    tft.fillRect(82, 71, 69, 35, Display_Color_Blue);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(87, 75);
    tft.print("Bat");
    tft.drawBitmap(118, 78, BatteryIcon, 30, 19, ST7735_WHITE);
    // Cường độ tín hiệu
    tft.drawBitmap(105, 110, SignalIcon, 18, 13, ST7735_GREEN);
  } 
  Screen1Data();
}

// a small function for button1 (B1) debounce
bool debounce ()
{
  byte count = 0;
  for(byte i = 0; i < 6; i++)
  {
    if ( !digitalRead(Enter_Pin) )
      count++;
    delay(10);
  }
  if(count > 2)  return 1;
  else           return 0;
}

byte edit(int8_t parameter)
{
  static byte i = 0, y_pos,
              x_pos[6] = {day_x_pos, month_x_pos, year_x_pos, hour_x_pos, minute_x_pos, buzzer_x_pos };
  char text[3];
  sprintf(text,"%02u", parameter);

  if(i < 3) {
    tft.setTextColor(ST7735_YELLOW, ST7735_BLUE);     // set text color to green and black background
    y_pos = date_y_pos;
  }
  if(i > 2  && i < 4){
    tft.setTextColor(ST7735_YELLOW, ST7735_BLUE);     // set text color to yellow and black background
    y_pos = time_y_pos;
  }
  if(i > 4 ){
    tft.setTextColor(ST7735_YELLOW, ST7735_BLUE);     // set text color to yellow and black background
    y_pos = buzzer_y_pos;
  }

  while( debounce() );   // call debounce function (wait for B1 to be released)

  while(true) {
    while( !digitalRead(Up_Pin) && digitalRead(Down_Pin) ) {  // while B2 is pressed
      parameter++;
      if(i == 0 && parameter > 31)    // if day > 31 ==> day = 1
        parameter = 1;
      if(i == 1 && parameter > 12)    // If month > 12 ==> month = 1
        parameter = 1;
      if(i == 2 && parameter > 99)    // If year > 99 ==> year = 0
        parameter = 0;
      if(i == 3 && parameter > 23)    // if hours > 23 ==> hours = 0
        parameter = 0;
      if(i == 4 && parameter > 59)    // if minutes > 59 ==> minutes = 0
        parameter = 0;
      if(i == 5 && parameter > 1)    // if minutes > 59 ==> minutes = 0
        parameter = 0;  

      sprintf(text,"%02u", parameter);
      tft.setCursor(x_pos[i], y_pos);
      tft.print(text);
      delay(200);       // wait 200ms
    }
    while( !digitalRead(Down_Pin) && digitalRead(Up_Pin) ) {  // while B2 is pressed
      parameter--;
      if(i == 0 && parameter < 01)    // if day <  01 ==> day = 31
        parameter = 31;
      if(i == 1 && parameter < 01)    // If month < 01 ==> month = 12
        parameter = 12;
      if(i == 2 && parameter < 00)    // If year < 00 > ==> year = 99
        parameter = 99;
      if(i == 3 && parameter < 00)    // if hours < 00 ==> hours = 23
        parameter = 23;
      if(i == 4 && parameter < 00)    // if minutes < 00 ==> minutes = 59
        parameter = 59;
      if(i == 5 && parameter < 00)    // if minutes < 00 ==> minutes = 59
        parameter = 1;  

      sprintf(text,"%02u", parameter);
      tft.setCursor(x_pos[i], y_pos);
      tft.print(text);
      delay(200);       // wait 200ms
    }
    

    tft.fillRect(x_pos[i], y_pos, 12, 9, ST7735_BLUE);  // fill nhấp nháy ô được chọn
    unsigned long previous_m = millis();
    while( (millis() - previous_m < 250) && digitalRead(Enter_Pin) && digitalRead(Up_Pin) && digitalRead(Down_Pin)) ;
    tft.setCursor(x_pos[i], y_pos);
    tft.print(text);
    previous_m = millis();
    while( (millis() - previous_m < 250) && digitalRead(Enter_Pin) && digitalRead(Up_Pin) && digitalRead(Down_Pin)) ;

    if(!digitalRead(Enter_Pin))
    {                     // if button B1 is pressed
      i = (i + 1) % 6;    // increment 'i' for the next parameter
      return parameter;   // return parameter value and exit
    }
  }
}
void RTC_Setting_display()
{ 
  char _buffer[11];
  // print date
  sprintf( _buffer, "%02u-%02u-%04u", now.day(), now.month(), now.year() );
  tft.setCursor(4, date_y_pos);
  tft.setTextColor(ST7735_YELLOW, ST7735_BLUE);     // set text color to yellow and black background
  tft.print("> Set Day:");
  tft.print(_buffer);
  // print time
  sprintf( _buffer, "%02u:%02u:%02u", now.hour(), now.minute(), now.second() );
  tft.setCursor(4, time_y_pos);
  tft.setTextColor(ST7735_YELLOW, ST7735_BLUE);     // set text color to green and black background
  tft.print("> Set Time:");
  tft.print(_buffer);
   // 
  sprintf( _buffer, "%02u", BuzzerState );
  tft.setCursor(4, buzzer_y_pos );
  tft.setTextColor(ST7735_YELLOW, ST7735_BLUE);     // set text color to green and black background
  tft.print("> Buzzer: ");
  tft.print(_buffer);
  // 
  tft.setCursor(4, addr_slave_y_pos);
  tft.setTextColor(ST7735_YELLOW, ST7735_BLUE);     // set text color to green and black background
  tft.print("> Slave Address: ");
  tft.print(slave_Address);
  tft.setCursor(4, addr_master_y_pos);
  tft.setTextColor(ST7735_YELLOW, ST7735_BLUE);     // set text color to green and black background
  tft.print("> Master Address: ");
  tft.print(master_Address);
  // 
  sprintf( _buffer, "%02u", BuzzerState );
  tft.setCursor(4, buzzer_y_pos );
  tft.setTextColor(ST7735_YELLOW, ST7735_BLUE);     // set text color to green and black background
  tft.print("> Buzzer: ");
  tft.print(_buffer);
}
void Screen2_display()
{ 
  digitalWrite(Buzzer_Pin, LOW); 
  if(state != pre_state) 
  { 
    pre_state = state;
    tft.fillScreen(ST7735_BLUE);
    tft.drawLine(0, 25, 160, 25,ST7735_WHITE);
    tft.setTextSize(2);
    tft.setTextColor(ST7735_YELLOW);
    tft.setCursor(13, 4);
    tft.print("Information");
    tft.fillRect(0, 88, 160, 40, ST7735_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_BLACK);
    tft.setCursor(4, 90);
    tft.print(Model);
    tft.setCursor(4, 99);
    tft.print(SeriaNumber);
    tft.setCursor(4, 108);
    tft.print(FirmwareVision);
    tft.setCursor(4, 117);
    tft.setTextColor(ST7735_RED);
    tft.print(Author);
  }
  
  tft.setTextSize(1);
  if( !digitalRead(Enter_Pin) )  // if B1 is pressed
  if( debounce() )             // call debounce function (make sure B1 is pressed)
  {
    while( debounce() );  // call debounce function (wait for B1 to be released)
    //DateTime now = rtc.now();
    byte day    = edit( now.day() );          // edit date
    byte month  = edit( now.month() );        // edit month
    byte year   = edit( now.year() - 2000 );  // edit year
    byte hour   = edit( now.hour() );         // edit hours
    byte minute = edit( now.minute() );       // edit minutes
    BuzzerState = edit(BuzzerState);
    EEPROM_SaveSetting();
    // write time & date data to the RTC chip
    rtc.adjust(DateTime(2000 + year, month, day, hour, minute, 0));

    while(debounce());  // call debounce function (wait for button B1 to be released)
  }

  RTC_Setting_display();
}
void ButtonHandle()
{
  if (!digitalRead(Down_Pin)) 
  { 
    if (state == 2) state = 0;
    else state += 1;
    delay(200);
  }
}
//___________ getValue() ___________ 
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//___________ ReyaxLoRa_Send() ___________
void ReyaxLoRa_Send(int addr, String data_send) {
  String str_Send;
  // AT commands to transmit data. 
  // For more details see the document "LoRa_AT_Command_RYLR998_RYLR498_EN.pdf" in the "AT+SEND" section.
  str_Send = "AT+SEND=" + String(addr) + "," + String(data_send.length()) + "," + data_send + "\r\n";
  ReyaxLoRa.print(str_Send);
  Serial.println();
  Serial.print("Send to Slave : ");
  Serial.print(str_Send);
  Serial.flush();
} 

//___________ ReyaxLoRa_Receive() ___________
void ReyaxLoRa_Receive() {
  if (ReyaxLoRa.available() > 0 ) {
    lastSerialTime = millis();    // Lưu thời gian gần nhất nhận được tín hiệu
    String rcv_Data_String = ReyaxLoRa.readString();
    if(rcv_Data_String.indexOf("OK") > 0 || rcv_Data_String.indexOf("ERR") > 0) {
      Serial.println();
      Serial.print(F("LoRa Reyax Module Response : "));
      Serial.println(rcv_Data_String);
      Serial.flush();
      return;
    } else {
      // Print received data or messages.
      Serial.println(F("Received from Sender : "));
      Serial.println(rcv_Data_String);
      Serial.flush();

      //---------------------------------------- Process incoming data.
      // For more details see the document "LoRa_AT_Command_RYLR998_RYLR498_EN.pdf" in the section "+RCV".
      
      //String _addr    = getValue(rcv_Data_String, ',', 0);    //--> address
      //_addr = _addr.substring(5);
      //String _length  = getValue(rcv_Data_String, ',', 1);    //--> data length
      String _message = getValue(rcv_Data_String, ',', 2);    //--> data/message
      _rssi    = getValue(rcv_Data_String, ',', 3);    //--> RSSI
      Serial.print(F("RSSI : "));
      Serial.println(_rssi);
      //String _snr     = getValue(rcv_Data_String, ',', 4);    //--> SNR
  
      //Serial.println();
      //Serial.println(F("Received from Sender."));
      //Serial.print(F("-Addr     : "));
      //Serial.println(_addr);
      //Serial.print(F("-Length   : "));
      //Serial.println(_length);
      //Serial.print(F("-Message  : "));
      //Serial.println(_message);
      //Serial.print(F("-RSSI     : "));
      //Serial.println(_rssi);
      //Serial.print(F("-SNR      : "));
      //Serial.println(_snr);
      //Serial.flush();.
      TemperatureValue = getValue(_message, '|', 0);
      HumidityValue = getValue(_message, '|', 1);
      MoistureValue = getValue(_message, '|', 2);
      BateryPercent = getValue(_message, '|', 3);
    }
  }
}
void AllowBuzzer(uint8_t BuzzerValue, uint8_t value)
{ 
  if(value) digitalWrite(Buzzer_Pin, BuzzerValue); 
  else digitalWrite(Buzzer_Pin, LOW); 
  
}
// Thông báo mất tín hiệu - Signal loss notification
void SignalLoss()
{
  tft.setTextSize(1);
  tft.setCursor(10, 113);
  tft.setTextColor(ST7735_WHITE, Display_Color_Backgroup);
  if (millis() - lastSerialTime > TIMEOUT) {    // Nếu thời gian chờ tối đa đã vượt quá
    if (millis() - lastBlinkTime > 1000) { // Nếu đã đủ thời gian để nhấp nháy LED
      buzzerState = !buzzerState;    // Đảo trạng thái của LED
      //digitalWrite(Buzzer, ledState);    // Cập nhật trạng thái của LED
      AllowBuzzer(buzzerState, BuzzerState);
      if(buzzerState == 0) {
        tft.setTextColor(ST7735_WHITE, ST7735_RED);
        tft.print("Signal Loss");
      }
      if(buzzerState == 1) {
        tft.setTextColor(ST7735_WHITE, Display_Color_Backgroup);
        tft.print("           ");
      }
      lastBlinkTime = millis(); // Lưu thời gian nhấp nháy gần nhất
    }
  } 
  else {
  tft.print("           "); 
  digitalWrite(Buzzer_Pin, LOW);    // Tắt đèn LED nếu còn nhận được tín hiệu
  lastBlinkTime = millis(); // Lưu thời gian nhấp nháy gần nhất
  }
}
//_____________ VOID SETUP() _____________
void setup() {
  pinMode(Buzzer_Pin, OUTPUT);
  pinMode(Up_Pin, INPUT_PULLUP);
  pinMode(Enter_Pin, INPUT_PULLUP);
  pinMode(Down_Pin, INPUT_PULLUP);
  Serial.begin(9600);
  // set the data rate for the SoftwareSerial port.
  ReyaxLoRa.begin(9600);
  // initialize RTC chip
  rtc.begin();   
  //rtc.adjust(DateTime(2023, 5, 14, 12, 23, 0));
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setRotation(1);  // 0->3
 // tft.fillScreen(ST7735_BLACK);  // fill screen with black color
  tft.fillScreen(Display_Color_Backgroup);
  EEPROM_LoadSetting();
} 

//________________VOID LOOP()_______________
void loop() { 
  ButtonHandle();
  now = rtc.now();  // read current time and date from the RTC chip

  switch (state) {
    case 0:
      Screen1_display();
      SignalLoss();
      ReyaxLoRa_Receive();
      break;
    case 1:
      Screen2_display();
      break;
  }
  //Serial.println(debounce());
}