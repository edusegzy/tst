/*
    Name    : LCD Button Shield Menu
    Original version credits: Paul Siewert
    Modified by: Giuseppe Torino
    Created : June 19, 2016
    Last Modified: October 17, 2016
    Version : 1.0
    Notes   : A smart timer for TV time accounting.
    License : See original licence http://www.instructables.com/id/Arduino-Uno-Menu-Template/
 ***************************************************************************************/
 */

                        
// This function will generate the 2 menu items that can fit on the screen. 
// They will change as you scroll through your menu. Up and down arrows will 
// indicate your current menu position.
void mainMenuDraw() {
#if defined(DO_DEBUG)  
  Serial.print(menuPage);
#endif

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}


// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor() {
  for (int x = 0; x < 2; x++) {     // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }

  // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
  // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}


void operateMainMenu() {
  int button;
  int buttonPressed = 0;
  
  while (buttonPressed == 0) {
      if (!auth_key_on()) {   //getting out from menus 
        lcd.clear();          //without saving anything  
        GoMenu=false;
        return;
      }

    button =read_LCD_buttons();
    switch (button) {
      case btnNONE: // When button returns as 0 there is no action taken
          break;
      case btnRIGHT:  // This case will execute if the "forward" button is pressed
        button = btnNONE;
        switch (cursorPosition) { 
        // The case that is selected here is dependent on which menu page you are on and where the cursor is.
          case 0:
            menuItem1();
            break;
          case 1:
            menuItem2();
            break;
          case 2:
            menuItem3();
            break;
          case 3:
            menuItem4();
            break;
        }
        buttonPressed = 1;
        if (GoMenu){
          mainMenuDraw();
          drawCursor();
        }
        break;
      case btnUP:
        button = btnNONE;
        if (menuPage == 0) {
          cursorPosition = cursorPosition - 1;
          cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        }
        if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

        mainMenuDraw();
        drawCursor();
        buttonPressed = 1;
        break;
      case btnDOWN:
        button = btnNONE;
        if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages-1);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages-1);
        }

        cursorPosition = cursorPosition + 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        mainMenuDraw();
        drawCursor();
        buttonPressed = 1;
        break;
   case btnLEFT:  //getting out from menu
        button = btnNONE;
        lcd.clear();
        GoMenu=false;
        buttonPressed = 1;
        break;        
    }
  }
}


// -- Clock -- set the clock
void menuItem1() { 
  DateTime now = RTC.now();
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Clock");
  setMode(now);

   //adjusting global variables
   hours = data[0]/60;
   minutes = data[0]%60;
   seconds = 0;
   digitalWrite(rele, HIGH);
   //update max amount in ram
   RTC.writeBytesInRam(MaxTimeAddr, 1, data);
   //update current value in ram
   RTC.writeBytesInRam(CurTimeAddr, 1, data);
*/   
}


// -- Timer --  set the maximum timer duration
void menuItem2() { 
  int exitPressed = 0;
  int button;
  char buffer[4];

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Max Timer (min.)");

  uint8_t data[1] = { 0 };
  RTC.readBytesInRam(MaxTimeAddr, 1, data);
  
  while (exitPressed == 0) {
    if (!auth_key_on()) {   //getting out from menus
      lcd.clear();          //without saving anything  
      GoMenu=false;
      return;
    }
  
    lcd.setCursor(0, 1);
    sprintf(buffer, "%3d", data[0]);
    lcd.print(buffer);  
  
    button =read_LCD_buttons();
    
    switch (button) {
      case btnLEFT:  // This case will execute if the "back" button is pressed
        button = 0;
        exitPressed = 1;
        break;
      case btnUP:  // Increase the timer 
        button = 0;
        if (data[0]<255) data[0]++;
        break;
      case btnDOWN:  // Decrease the timer
        button = 0;
        if (data[0]>0) data[0]--;
        break;
    }
}
   //adjusting global variables
   hours = data[0]/60;
   minutes = data[0]%60;
   seconds = 0;
   digitalWrite(rele, HIGH);
   //update max amount in ram
   RTC.writeBytesInRam(MaxTimeAddr, 1, data);
   //update current value in ram
   RTC.writeBytesInRam(CurTimeAddr, 1, data);
}

/* Reset actual timer   */
void menuItem3() {
  //Immediate action, no submenu 
  uint8_t curr_time_value[1]; 
  lcd.clear();
  //Set the timer to its maximum value
  RTC.readBytesInRam(MaxTimeAddr, 1, curr_time_value);
  
  //adjusting global variables
  hours = curr_time_value[0]/60;
  minutes = curr_time_value[0]%60;
  seconds = 0;
  //aggiorno il valore corrente in ram
  RTC.writeBytesInRam(CurTimeAddr, 1, curr_time_value);
  //powering on the appliance
  digitalWrite(rele, HIGH);
  //exit from menus
  GoMenu=false;
}

void menuItem4() { // Currently unused
  int activeButton = 0;
  int button;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 4");

  button =read_LCD_buttons();

  switch (button) {
      case btnLEFT:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
  }
}


/* Submenu to edit the RTC*/
void setMode(DateTime now) {
    int button;
    
    boolean setMode = true;
    int setModeLevel = 0;
 
    int _day = now.day();
    int _month = now.month();
    int _year = now.year();
    int _hour = now.hour();
    int _min = now.minute();
    int _sec = now.second();
    char buffer[10];
    unsigned long timeSet=0;   
    int  setModeTime=10000;   
 
    lcd.clear();
    lcd.setCursor(0,0);
    sprintf(buffer,  "%s: %02d", "Day", _day);
    timeSet = millis();
 
    while ( setMode ) {
      if (!auth_key_on()) {   //getting out from menus
        lcd.clear();          //without saving anything  
        GoMenu=false;
        return;
      }
    
      button =read_LCD_buttons();

      if ( button == btnLEFT || button == btnUP || button == btnRIGHT ) { timeSet = millis(); }
 
      lcd.setCursor(0,0);

      switch (setModeLevel) {
        case 0:    // Set Day
          if ( button == btnUP && _day < 31) { _day++; }
          if ( button == btnDOWN && _day > 1) { _day--; }
          sprintf(buffer,  "%s: %02d", "Day", _day);
          break;
        case 1:    // Set Month
          if ( button == btnUP && _month < 12) { _month++; }
          if ( button == btnDOWN && _month > 1) { _month--; }
          sprintf(buffer,  "%s: %02d", "Month", _month);
          break;
        case 2:   // Set Year
          if ( button == btnUP && _year < 9999) { _year++; }
          if ( button == btnDOWN && _year > 1900) { _year--; }
          sprintf(buffer,  "%s: %02d", "Year", _year);
          break;
        case 3:  // Set Hour
          if ( button == btnUP && _hour < 24) { _hour++; }
          if ( button == btnDOWN && _hour > 1) { _hour--; }
          sprintf(buffer,  "%s: %02d", "Hours", _hour);
          break;
        case 4:  // Set Minute
          if ( button == btnUP && _min < 60) { _min++; }
          if ( button == btnDOWN && _min > 1) { _min--; }
          sprintf(buffer,  "%s: %02d", "Minutes", _min);
          break;
        case 5:   // Set Second
          if ( button == btnUP && _sec < 60) { _sec++; }
          if ( button == btnDOWN && _sec > 0) { _sec--; }
          sprintf(buffer,  "%s: %02d", "Seconds", _sec);
          break;
      } 
      lcd.print( buffer );
      if ( button == btnRIGHT ) { lcd.clear(); setModeLevel++;  }
      if ( button == btnLEFT ) { setMode = false; }  //Pressing LEFT will exit without saving
      //exit if timeouts or if there are just seconds left
      if ((timeSet > 0 && (setModeTime*2) < (millis() - timeSet) )|| (setModeLevel > 5)) {
         RTC.adjust(DateTime(_year, _month, _day, _hour, _min, _sec));
         setMode = false;
      }
    }
}



