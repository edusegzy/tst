
// You can have up to 10 menu items in the menuItems[] array below without having to change the base programming at all. Name them however you'd like. Beyond 10 items, you will have to add additional "cases" in the switch/case
// section of the operateMainMenu() function below. You will also have to add additional void functions (i.e. menuItem11, menuItem12, etc.) to the program.
String menuItems[] = {"Clock", "Timer", "Reset timer"};

// Menu control variables
bool GoMenu=false;
int menuPage = 0;
int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
int cursorPosition = 0;

// Creates 3 custom characters for the menu display
byte downArrow[8] = {
  B00100, //   *
  B00100, //   *
  B00100, //   *
  B00100, //   *
  B00100, //   *
  B10101, // * * *
  B01110, //  ***
  B00100  //   *
};

byte upArrow[8] = {
  B00100, //   *
  B01110, //  ***
  B10101, // * * *
  B00100, //   *
  B00100, //   *
  B00100, //   *
  B00100, //   *
  B00100  //   *
};

byte menuCursor[8] = {
  B01000, //  *
  B00100, //   *
  B00010, //    *
  B00001, //     *
  B00010, //    *
  B00100, //   *
  B01000, //  *
  B00000  //
};
