/*
    Name    : TSST  TV Screen Smart Timer
    Author  : Giuseppe Torino
    Created : June 6, 2016
    Last Modified: October 17, 2016
    Version : 1.0
    Notes   : A smart timer for TV time accounting.
    License : This program is free software. You can redistribute it and/or modify
              it under the terms of the GNU General Public License as published by
              the Free Software Foundation, either version 3 of the License, or
              (at your option) any later version.
              This program is distributed in the hope that it will be useful,
              but WITHOUT ANY WARRANTY; without even the implied warranty of
              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
              GNU General Public License for more details.
 ***************************************************************************************/
 */
#include <Wire.h>
#include <RTClib.h>    //Jeelab RTC library https://github.com/jcw/rtclib
#include <LiquidCrystal.h>

#include "menu.h"

//#define DO_DEBUG  //With the directive uncommented all serial.print() are included in the code 
#define backlight 10  // Digital I / O pin that controls LCD backlight
#define rele 2      // pin I/O for relais
#define authKey 3   //Analog input used for the key

#define btnRIGHT  1
#define btnUP     2
#define btnDOWN   3
#define btnLEFT   4
#define btnSELECT 5
#define btnNONE   0
#define MaxTimeAddr  0x08     //RAM address for max timer
#define CurTimeAddr  0x09     //RAM address for current timer value
#define CurSecAddr   0x0A     //RAM address for remaining seconds
#define CurDayAddr   0x0B     //RAM address for current day
#define CurMonthAddr   0x0C     //RAM address for current month


/*****************************************/
/*          global vars                  */
/*****************************************/
RTC_DS1307 RTC;
 
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);           // select the pins used on the LCD panel

// usate da LCD e bottoni
int lcd_key     = 0;
int adc_key_in  = 0;
int elapsed = 0;   //time in seconds
int curr_secs = 0;
unsigned long next;
bool fading = false;
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// Navigation button variables
int readKey;

/* per il countdown */
int hours = 0; // start hours
int minutes = 0; //start min
int seconds = 0; //start seconds
bool paused = true;

//change key state event
bool change_k_state = false;
bool menu_enabled;   // key state

//data corrente 
uint8_t curr_day_value[1]; //vurrent day
uint8_t curr_month_value[1]; //current month

int read_LCD_buttons(){               // read the buttons
    adc_key_in = analogRead(0);       // read the value from the sensor 
    if (adc_key_in < 790) {           //debouncing routine 
      delay(100);
      adc_key_in = analogRead(0);
    }
    // my buttons when read are centered at these values: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    // We make this the 1st option for speed reasons since it will be the most likely result
 
    if (adc_key_in > 1000) 
        return btnNONE;
      else {  //any button pressed
        //timer managing LCD lighting is started
        curr_secs = millis()/1000;
        digitalWrite(backlight, HIGH);  //LCD is lighted on   
      }
 
    // For V1.1 us this threshold
    if (adc_key_in < 50)   return btnRIGHT;  
    if (adc_key_in < 250)  return btnUP; 
    if (adc_key_in < 450)  return btnDOWN; 
    if (adc_key_in < 650)  return btnLEFT; 
    if (adc_key_in < 850)  return btnSELECT;  
 
    return btnNONE;                // when all others fail, return this.
}

//return true if the key ois inserted
bool auth_key_on(){               // read the key
    int auth_val = analogRead(authKey);       // read the value from the analog input of the key 
   
    if (auth_val < 790) {           //debouncing routine 
      delay(100);
      auth_val = analogRead(authKey);
    }

    if ((auth_val >400) && (auth_val < 600)){
        return true;
    } else {
        return false;
    }
}


/* Step down timer of one second */
void stepDown() { 
  uint8_t curr_time_value[1]; 
  if (seconds > 0) {
    seconds -= 1;
  } else {
    if (minutes > 0) {
      seconds = 59;
      minutes -= 1;
    } else {
        if (hours > 0) {
          seconds = 59;
          minutes = 59;
          hours -= 1;
        } else {   
          hours=0; minutes=0; seconds=0;
          paused = true;     
          digitalWrite(rele, LOW);
        }
    }
  }
  //save the current hour in RAM
  curr_time_value[0] = hours*60+minutes;
  RTC.writeBytesInRam(CurTimeAddr, 1, curr_time_value);
  curr_time_value[0] = seconds;
  RTC.writeBytesInRam(CurSecAddr, 1, curr_time_value);
}
 
//timer reset 
void reset_time(){
  uint8_t a_time_value[1]; 
  
  DateTime now = RTC.now();
  curr_day_value[0] = now.day();
  curr_month_value[0] = now.month();
    
  RTC.writeBytesInRam(CurDayAddr, 1, curr_day_value);  
  RTC.writeBytesInRam(CurMonthAddr, 1, curr_month_value);  

  //set timer to the max value
  RTC.readBytesInRam(MaxTimeAddr, 1, a_time_value);
  hours= a_time_value[0]/60;
  minutes= a_time_value[0]%60;
  seconds= 0;
  //update time in ram
  RTC.writeBytesInRam(CurTimeAddr, 1, a_time_value);
  RTC.writeBytesInRam(CurSecAddr, 1, (uint8_t) 0);
#if defined(DO_DEBUG)
  Serial.println("Timer resettato per cambio giorno!");
#endif
}


/*****************************************/
/*               setup()                 */
/*****************************************/
void setup() {
  uint8_t curr_time_value[1]; 
#if defined(DO_DEBUG)
  Serial.begin(57600);
  Serial.print("Inizializing\n");
  char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};  
#endif

  lcd.begin(16, 2);               // start the library
  lcd.setCursor(0,0);             // set the LCD cursor position 
  pinMode(backlight, OUTPUT);      // sets the digital pin as output
  digitalWrite(backlight, HIGH);   // accende la luce di sfondo del display  

  //set the initial behavior when the key is inserted
  if(auth_key_on()) {
    menu_enabled = true;
    digitalWrite(rele, HIGH);       
  } else  {
    menu_enabled = false;
    digitalWrite(rele, LOW);       
  }
  pinMode(rele, OUTPUT);   

  next=0;
  
  //I2C e RTC inizialization
  Wire.begin();
  RTC.begin();
 
  if (! RTC.isrunning()) {
#if defined(DO_DEBUG)
    Serial.println("RTC is NOT running!");
#endif
    // following line sets the RTC to the date & time this sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  // Creates the byte for the 3 custom characters
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);

  RTC.readBytesInRam(CurDayAddr, 1, curr_day_value);
  RTC.readBytesInRam(CurMonthAddr, 1, curr_month_value);

  DateTime now = RTC.now();

#if defined(DO_DEBUG)
  Serial.print("RTC day: ");
  Serial.print(now.day());
  Serial.print(" -  mem day: ");
  Serial.print(curr_day_value[0]);
  Serial.print(" -  dayOfWeek: ");
  Serial.println(daysOfTheWeek[now.dayOfWeek()]);
#endif

  /*  in case of next day */
  if (curr_day_value[0] != now.day() || curr_month_value[0] != now.month()){   
    reset_time();
  } else {
    RTC.readBytesInRam(CurTimeAddr, 1, curr_time_value);
    hours= curr_time_value[0]/60;
    minutes= curr_time_value[0]%60;
    RTC.readBytesInRam(CurSecAddr, 1, curr_time_value);
    seconds= curr_time_value[0];
#if defined(DO_DEBUG)
    Serial.println("Timer al valore corrente!");
#endif
  }
 
}
/************ end setup() ******************************/

/*****************************************/
/*               loop()                  */
/*****************************************/
void loop() {
  char buffer[16];
  char message[10];
  uint8_t curr_time_value[1];

  lcd.setCursor(0,0);             // move to the begining of the second line
 
//event management for the key
  if (auth_key_on() != menu_enabled) {
    menu_enabled = auth_key_on();
    change_k_state = true;
  }
  
  if (change_k_state) {  
    switch (menu_enabled){ 
      case true:{           //key inserted
        digitalWrite(rele, HIGH); 
        digitalWrite(backlight, HIGH);   
        break;
      }
      case false:{           //key off
        digitalWrite(rele, LOW);  
        break;
      }
    }
      paused=true;       //timer paused
      change_k_state = false;  
  }

  lcd_key = read_LCD_buttons();   // read the buttons

  switch (lcd_key){    
    case btnSELECT:{  // goes to menu
        if (menu_enabled) GoMenu = true;
        break;
    }
    case btnUP:{    // manage timer pauses
        if (!menu_enabled && !GoMenu) {   //button disabled if the key is inserted 
          if (!paused){                   
            digitalWrite(rele, LOW);
            paused=true;
          } else {
            if (hours > 0 || minutes > 0 || seconds > 0){ 
              digitalWrite(rele, HIGH);
              paused=false;
            }
          }
          delay(400);
          break;
        }
    }
  } 

  // display dimming when there are less than 60 min. left
  if (hours == 0  and minutes <= 59 && !paused)
    fading = true;
  else
    fading = false;

  // set the brightness of pin 9:
  if (fading){
    analogWrite(backlight, brightness);
    // change the brightness for next time through the loop:
    brightness = brightness + fadeAmount;
    // reverse the direction of the fading at the ends of the fade:
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(30);
  }
  elapsed= millis()/1000 - curr_secs;
  //the LCD backlight switches off after 6 minutes
  if (!fading && !GoMenu && (elapsed > 360)) digitalWrite(backlight, LOW);

  if (GoMenu) {
    mainMenuDraw();
    drawCursor();
    operateMainMenu();
  }

/* procedure executed every second  */
  if (millis() >= next)  {

    if (!paused && !GoMenu) stepDown();  //timer update
    next = millis() + 1000;  

    DateTime now = RTC.now();

    if (!GoMenu){
      /* in case tomorrow is reached when TSST is on */
      if (curr_day_value[0] != now.day() || curr_month_value[0] != now.month())   
        reset_time();
      
      if (hours > 0 || minutes > 0 || seconds > 0 ) {
        if (menu_enabled){
          sprintf(message,   "*Unlocked* ");
        } else {
          if(paused)
            sprintf(message, "**Paused** " );
          else
            sprintf(message, "Time:      " );
        }
          sprintf(buffer,  "%s%02d:%02d", message, now.hour(), now.minute());
      } else {
        sprintf(buffer,  " * Out of time!*");
      }
 #if defined(DO_DEBUG)
  Serial.print("Current Timer: ");
  Serial.print( hours);
  Serial.print( ", ");
  Serial.println( minutes);
#endif

      lcd.setCursor(0,1);
      lcd.print( buffer );

      //time left
      lcd.setCursor(0,0);
      sprintf(buffer,  "Left:   %02d:%02d:%02d", hours, minutes, seconds);
      lcd.print( buffer );
    } 
  }
}
/* ----   loop() end  ---- */                                  


