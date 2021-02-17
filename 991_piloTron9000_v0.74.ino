// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
  
  // piloTron 9000
  // kontrolní jednotka pásové pily
  // zapojení https://imgur.com/7aSZtCA
  // zapojení čidla https://i.imgur.com/q9Abr6r.png
  // datum 17.02.2021
  // verze
  String ver = "v0.74 beta";

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // PIN NUMBER SETUP
  
  // relay pin setup
  int relayUp = 12;
  int relayDown = 13;
  
  // buttons
  int butPin_up = 6; // nahoru / přidat
  int butPin_down = 7; // dolu / ubrat
  int butPin_stepSet = 8; // nastaveni kroku
  
  // end switches
  int butPin_lowerEndSwitch = 9; // doraz dolni
  int butPin_upperEndSwitch = 10; // doraz horni
  
  // encoder pins
  int encoderA = 2;
  int encoderB = 3;

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // GLOBAL VARIABLE SETUP
  
  // stepSet limit mm
  int stepLimit = 400;

  // default over cut value
  int defaultOvercut = 4;

  // endSwitch triggers
  volatile bool enableUp;
  volatile bool enableDown;

  // endSwitch states
  volatile int upState = LOW;
  volatile int downState = LOW;

  // step tolerance mm
  float tolerance = 0;

  // step upper bound init
  volatile int countLimit;

  //TEMPORARY - NEEDS FIXING:
  
  // conversion factor [impulses per milimeter]
  float convUp = 20.30;
  float convDown = 21.70;
  

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // LIBRARY LOAD UP

  // I2C knihovna Arduina
  #include <Wire.h>
  // i2c knihovna pro LCD
  #include <LiquidCrystal_I2C.h>

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // LCD INITIALIZATION

  // LCD Pinout
  const int en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;

  // LCD panel I2C address
  const int i2cAddress = 0x27;

  // LCD Display Object
  LiquidCrystal_I2C lcd(i2cAddress, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);

  // custom chars
  byte horniDoraz[8] = {
    B11111,
    B11111,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100
  };

  byte horniOK[8] = {
    B00100,
    B01110,
    B10101,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100
  };


  byte dolniDoraz[8] = {
    B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B11111,
    B11111
  };


  byte dolniOK[8] = {
    B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B10101,
    B01110,
    B00100
  };


  byte hore[8] = {
    B11011,
    B10001,
    B01010,
    B11011,
    B11011,
    B11011,
    B11011,
    B11011
  };


  byte dole[8] = {
    B11011,
    B11011,
    B11011,
    B11011,
    B11011,
    B01010,
    B10001,
    B11011
  };

  byte nastavit[8] = {
    B00100,
    B01110,
    B11111,
    B00100,
    B00100,
    B11111,
    B01110,
    B00100
  };

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // CONTROL BUTTON VARIABLES INITIALIZATION

  // reading variables
  int reading_up;
  int reading_down;
  int reading_stepSet;

  // init butt states
  int initButState_up = HIGH;
  int initButState_down = HIGH;
  int initButState_stepSet = HIGH;

  // actual butt states
  int butState_up = 0;
  int butState_down = 0;
  int butState_stepSet = 0;
  int butState_adjUp = 0;
  int butState_adjDown = 0;

  // init button debounce timestamps
  unsigned long timeStamp_up;
  unsigned long timeStamp_down;
  unsigned long timeStamp_adjUp;
  unsigned long timeStamp_adjDown;

  // fast input hold down delay
  int dly = 500;

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // ENDSWITCH VARIABLES INITIALIZATION

  // reading variables
  int reading_lowerEndSwitch;
  int reading_upperEndSwitch;

  // init switch states
  int initButState_lowerEndSwitch = HIGH;
  int initButState_upperEndSwitch = HIGH;

  // actual switch states
  int butState_lowerEndSwitch = 0;
  int butState_upperEndSwitch = 0;

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // COUNTERS INITIALIZATION

  int stepValue = 0;
  int overCutValue = defaultOvercut;


  // encoder impulse counter init
  volatile float count = 0;

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // LCD CHAR BUFFER

  char line0[13];
  char line1[13];

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

// VOID SETUP START

void setup() {

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // LCD SETUP

  // Initialize LCD as 16 characters by 2 rows
  lcd.begin(16,2);

  // flash screen
  lcd.print(ver);
  delay(1000);

  // lcd custom chars load
  lcd.createChar(0,horniOK);
  lcd.createChar(1,horniDoraz);
  lcd.createChar(2,dolniOK);
  lcd.createChar(3,dolniDoraz);
  lcd.createChar(4,hore);
  lcd.createChar(5,dole);
  lcd.createChar(6,nastavit);

// ******************************************************************************
// ******************************************************************************
// ****************************************************************************** 

  // SERIAL MONITOR BEGIN
  Serial.begin(9600);  

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // PIN MODES
  
  // button pinMode
  pinMode(butPin_up,INPUT_PULLUP);
  pinMode(butPin_down,INPUT_PULLUP);
  pinMode(butPin_stepSet,INPUT_PULLUP);

  // end switch pinMode
  pinMode(butPin_lowerEndSwitch,INPUT_PULLUP);
  pinMode(butPin_upperEndSwitch,INPUT_PULLUP);
  
  // relay pinMode
  pinMode(relayUp,OUTPUT);
  pinMode(relayDown,OUTPUT);

  // encoder pinMode
  pinMode(encoderA,INPUT);
  pinMode(encoderB,INPUT);

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // ENCODER INTERRUPT SETUP
  attachInterrupt(digitalPinToInterrupt(encoderA),encoderHandle, CHANGE);
  
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // RELAY INITIALIZATION
  digitalWrite(relayUp,LOW); 
  digitalWrite(relayDown,LOW); 

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // GET INITIAL ENDSWITCH STATES

  // upper end switch
  reading_upperEndSwitch = digitalRead(butPin_upperEndSwitch);
    if (reading_upperEndSwitch == initButState_upperEndSwitch){
      enableUp = true;
    } else {
      enableUp = false;
    }

  // lower end switch
  reading_lowerEndSwitch = digitalRead(butPin_lowerEndSwitch);
    if (reading_lowerEndSwitch == initButState_lowerEndSwitch){
      enableDown = true;
    } else {
      enableDown = false;
    }

  // end switch states serial print
  Serial.println(enableUp);
  Serial.println(enableDown);


// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

// VOID SETUP END

}

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

// VOID LOOP START

void loop() {

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // READING UPDATE

  // button reading
  reading_up = digitalRead(butPin_up);
  reading_down = digitalRead(butPin_down);
  reading_stepSet = digitalRead(butPin_stepSet);

  // end switch reading
  reading_upperEndSwitch = digitalRead(butPin_upperEndSwitch);
  reading_lowerEndSwitch = digitalRead(butPin_lowerEndSwitch);

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // STEP SETTING

  // step increase
  if (reading_up != initButState_up && 
      reading_stepSet != initButState_stepSet && 
      reading_down == initButState_down && 
      stepValue < stepLimit){      
        if (butState_up == 0){
            butState_up = 1;
            timeStamp_up = millis();
        }    
        
        if (millis() - timeStamp_up < dly){
            stepValue = stepValue + 1;
            delay(100);         
            Serial.println("si nastavil krok hore");
            Serial.println(stepValue);
        } else if (millis() - timeStamp_up > dly){
            stepValue = stepValue + 1;
            delay(10);      
            Serial.println("si nastavil krok hore jak cyp");
            Serial.println(stepValue);
        }        
  } else {
    // button not pressed
    butState_up = 0;
  }


  // step decrease
  if (reading_down != initButState_down &&
      reading_stepSet != initButState_stepSet &&
      reading_up == initButState_up &&
      stepValue > 0){
         if (butState_down == 0){
             butState_down = 1;
             timeStamp_down = millis();
         }    
         
         if (millis() - timeStamp_down < dly){
             stepValue = stepValue - 1;
             delay(100);           
             Serial.println("si nastavil krok dule");
             Serial.println(stepValue);
         } else if (millis() - timeStamp_down > dly){
             stepValue = stepValue - 1;
             delay(10);      
             Serial.println("si nastavil krok dule jak cyp");
             Serial.println(stepValue);
         }         
  } else {
    // button not pressed
    butState_down = 0;
  }

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // STEP ENGAGE

  // engage up
  if (reading_up != initButState_up &&
      reading_down == initButState_down &&
      reading_stepSet == initButState_stepSet &&
      reading_upperEndSwitch == initButState_upperEndSwitch &&
      stepValue != 0){

        Serial.println("START");

        count = 0; // count reset
        countLimit = (stepValue+overCutValue)*convUp + tolerance;

        enableUp = true;
        interrupts();

        while (enableUp == true){
          digitalWrite(relayUp, HIGH);

          if (digitalRead(butPin_upperEndSwitch) != initButState_upperEndSwitch){
            enableUp = false;      
          }
        }

      digitalWrite(relayUp, LOW);
      
      Serial.println(count);
      Serial.println("STOP-UP");
  }

  // engage down
  if (reading_down != initButState_down &&
      reading_up == initButState_up &&
      reading_stepSet == initButState_stepSet &&
      reading_lowerEndSwitch == initButState_lowerEndSwitch &&
      stepValue != 0){

        Serial.println("START");

        count = 0; // count reset
        countLimit = (stepValue+overCutValue)*convDown + tolerance;

        enableDown = true;
        interrupts();

        while (enableDown == true){
          digitalWrite(relayDown, HIGH);

          if (digitalRead(butPin_lowerEndSwitch) != initButState_lowerEndSwitch){
            enableDown = false;      
          }
        }

      digitalWrite(relayDown, LOW);
      
      Serial.println(count);
      Serial.println("STOP-DOWN");
  }

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // LCD DISPLAY OUTPUT

  // step value display
  sprintf(line0, "krok: %-4d mm", stepValue);
  lcd.setCursor(0,0);
  lcd.print(line0);

  // step setting indicator
  if (reading_stepSet != initButState_stepSet){
        lcd.setCursor(14,0);
        lcd.write(byte(6));
  } else {
        lcd.setCursor(14,0);
        lcd.print(" ");
  }

  // upper end switch released
  if (reading_upperEndSwitch == initButState_upperEndSwitch &&
      reading_up == initButState_up){
         lcd.setCursor(15,0);
         lcd.write(byte(0));
  }

  // upper end switch engaged
  if (reading_upperEndSwitch != initButState_upperEndSwitch){
      lcd.setCursor(15,0);
      lcd.write(byte(1));
      Serial.println("koncák hore je zamačklé");
  } 

  // lower end switch released
  if (reading_lowerEndSwitch == initButState_lowerEndSwitch &&
      reading_down == initButState_down){
         lcd.setCursor(15,1);
         lcd.write(byte(2));    
  }
  
  // lower end switch engaged
  if (reading_lowerEndSwitch != initButState_lowerEndSwitch){
      lcd.setCursor(15,1);
      lcd.write(byte(3));
      Serial.println("koncák dule je zamačklé");
  }
/*
  // step engage indication - UP direction
  if(enableUp = true){
    lcd.setCursor(15,0);
    lcd.write(byte(4));
  } 

  // step engage indication - down direction
  if(enableDown = true){
    lcd.setCursor(15,1);
    lcd.write(byte(5));
  } 
*/
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

// VOID LOOP END

}

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
// ******************************************************************************

  // ENCODER INTERRUPT HANDLER
  
  void encoderHandle()
  {
    count = count + 1;

    if (count >= countLimit){
      noInterrupts();
      enableDown = false;
      enableUp = false;
    }
            
  }

// ******************************************************************************
// ******************************************************************************
// ******************************************************************************
