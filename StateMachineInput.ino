// Keypad Input Validation Using State Machine Programming
// Author: Tony Keith
// Description: Input Validation Example

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>

// Comment out for Serial debug
#define SERIAL_DEBUG_ENABLED

// States
#define INITIAL       1
#define VALUE1        2
#define VALUE2        3
#define ATSIGN        4
#define HOUR1         5
#define HOUR2         6
#define COLON         7
#define MIN1          8
#define MIN2          9
#define EXECUTE       10
#define CLEAR         11

const byte ROWS = 4; // define four rows
const byte COLS = 4; // define four
char keys [ROWS] [COLS] = {
{'1', '2', '3','@'},
{'4', '5', '6','B'},
{'7', '8', '9','C'},
{':', '0', '#','D'}
};

// Alpha-Numeric Key Mappings:
// A = @ (at sign)
// B = NOT USED
// D = NOT USED
// C = Clear
// * = : (colon)
// # = Execute (accept or enter or execute) the command

// 4x4 membrane keypad
// Pin  R/C Port
// 8    C4  13
// 7    C3  12
// 6    C2  11
// 5    C1  10
// 4    R4  9
// 3    R3  8
// 2    R2  7
// 1    R1  6

// Connect 4 * 4 keypad row-bit port, the corresponding digital IO ports panel
byte rowPins [ROWS] = {6,7,8,9};

// Connect 4 * 4 buttons faithfully port, the corresponding digital IO ports panel
byte colPins [COLS] = {10,11,12,13};

// Call the function library function Keypad
Keypad keypad = Keypad (makeKeymap (keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x3F,20,4);

#define MAXCMDBUFFER  15

static char buffer[MAXCMDBUFFER];
static int index=0;
static int pos = 0;

static int state=INITIAL;

void setup () {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("State Machine ");
  lcd.setCursor(0,1);
  lcd.print("Enter Cmd:");
// Serial
#ifdef SERIAL_DEBUG_ENABLED
  Serial.begin (9600);
#endif

}

void loop () {
  
  char key = keypad.getKey ();
  if (key != NO_KEY) {
    // Clear
    if(key == 'C') {
      state = CLEAR;
    }
    displayState(state);
    switch(state) {
      case INITIAL:
        // X
        if(validKeys0to9(key)) {
          pos = displayKey(key, pos);
          storeKey(key);
          state = VALUE1;
        } else {
          invalidFormat();
        }
      break;
      case VALUE1:
        // XX
        if(validKeys0to9(key)) {
          pos = displayKey(key, pos);
          storeKey(key);
          state = VALUE2;
        } else if (validAtSign(key)) {
          // X@
          pos = displayKey(key, pos);
          state = ATSIGN;          
        } else {
          invalidFormat();
        }
      break;
      case VALUE2:
        // XX@
        if(validAtSign(key)) {
          pos = displayKey(key, pos);
          storeKey(key);
          state = ATSIGN;
        } else {
          invalidFormat();
        }
      break;
      case ATSIGN:
        // X@H, HH@H
        if(validKeys0to9(key)) {
          pos = displayKey(key, pos);
          storeKey(key);
          state = HOUR1;
        } else {
          invalidFormat();
        }      
      break;
      case HOUR1:
         // X@HH, XX@HH
        if(validKeys0to9(key)) {
          pos = displayKey(key, pos);
          storeKey(key);
          state = HOUR2;
        } else if (validColon(key)) {
          // X@HH:, XX@H:
          pos = displayKey(key, pos);
          state = COLON;          
        } else {
          invalidFormat();
        }
      break;
      case HOUR2:
         // X@HH:, XX@HH:
        if (validColon(key)) {
          // X@HH:, XX@H:
          pos = displayKey(key, pos);
          state = COLON;          
        } else {
          invalidFormat();
        }
      break;
      case COLON:     
        // X@HH:M, XX@HH:M
        if(validKeys0to5(key)) {
          pos = displayKey(key, pos);
          storeKey(key);
          state = MIN1;
        } else {
          invalidFormat();
        }
      break;
      case MIN1:
        // X@HH:MM, XX@HH:MM
        if(validKeys0to9(key)) {
          pos = displayKey(key, pos);
          storeKey(key);
          state = MIN2;
        } else {
          invalidFormat();
        }
      break;
      case MIN2:
        // X@HH:MM#, XX@HH:MM#
        if(validPoundSign(key)) {
          pos = displayKey(key, pos);
          storeKey(key);
          state = EXECUTE;
        } else {
          invalidFormat();
        }
      break;
      case CLEAR:
        clearAll();
      break;
      default:
      break;      
    } // switch
    
#ifdef SERIAL_DEBUG_ENABLED          
    Serial.print("Key:");
    Serial.print(key);
    Serial.print(" Idx:");
    Serial.println(index);
#endif   
    // EXECUTE
    if(state == EXECUTE) {
        executeCmd();   
        clearAll();
    }
  } // if 
} // loop    

// Clears resets everything
void clearAll(void) {
  buffer[0] = '\0';
  index=0;
  pos=0;
  state = INITIAL;
  clearDisplay();
}

// Clear the Cmd and execute area of the display
void clearDisplay(void) {
  lcd.setCursor(0,2);
  lcd.print("                    ");
  lcd.setCursor(0,3);
  lcd.print("                    ");
}

// Display format error
void invalidFormat(void) {
  lcd.setCursor(0,3);
  lcd.print("Invalid Format:");
  delay(1000);
  lcd.setCursor(0,3);
  lcd.print("XX@HH:MM#      ");
  delay(1000); 
  lcd.setCursor(0,3);
  lcd.print("               ");
}
// executeCmd
void executeCmd(void) {
  lcd.setCursor(0,3);
  lcd.print("Command Accepted");
  delay(1000);
  clearDisplay();
}
// Display key on Cmd line
int displayKey(char key, int pos) {
  lcd.setCursor(pos,2);
  lcd.print(key);
  pos++;
  return(pos);
}
// Store key in buffer
void storeKey(char key) {
  buffer[index] = key;
  index++;
}
// Clear buffer
void clearBuffer(void) {
  buffer[0] = '\0';
  index=0;
}

// Display state name
void displayState(int state) {
#ifdef SERIAL_DEBUG_ENABLED 
    char stateName[15];
    
    switch(state) {
      case INITIAL:
        strcpy(stateName, "INITIAL");
      break;
      case VALUE1:
        strcpy(stateName, "VALUE1");
      break;
      case VALUE2:
       strcpy(stateName, "VALUE2");
      break;
      case ATSIGN:
       strcpy(stateName, "ATSIGN");
      break;
      case HOUR1:
       strcpy(stateName, "HOUR1");
      break;
      case HOUR2:
       strcpy(stateName, "HOUR2");
      break;
      case COLON:
       strcpy(stateName, "COLON");
      break;
      case MIN1:
       strcpy(stateName, "MIN1");
      break;
      case MIN2:
       strcpy(stateName, "MIN2");
      break;
      case EXECUTE:
       strcpy(stateName, "EXECUTE");
      break;
      default:
        strcpy(stateName, "UNKNOWN");
      break;
  } // switch
  Serial.print("State:");
  Serial.println(stateName);
#endif
}

// Validate keys: : (colon)
// return true if valid, else false
int validColon(int key) {
  int valid=0;
  switch(key) {
    case ':':
      valid=1;
    break;
  }
  return valid;
}
// Validate keys: @ (at sign)
// return true if valid, else false
int validAtSign(int key) {
  int valid=0;
  switch(key) {
    case '@':
      valid=1;
    break;
  }
  return valid;
}
// Validate keys: # (pound sign)
// return true if valid, else false
int validPoundSign(int key) {
  int valid=0;
  switch(key) {
    case '#':
      valid=1;
    break;
  }
  return valid;
}
// Validate keys: 0-9
// return true if valid, else false
int validKeys0to9(int key) {
  int valid=0;
  switch(key) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      valid=1;
    break;
  }
  return valid;
}

// Validate keys: 0-5
// return true if valid, else false
int validKeys0to5(int key) {
  int valid=0;
  switch(key) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
      valid=1;
    break;
  }
  return valid;
}

// END OF FILE
