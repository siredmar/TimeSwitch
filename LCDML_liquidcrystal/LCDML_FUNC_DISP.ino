#include "EmonLib.h"
#include <EEPROM.h>

#define RELAIS1 15
#define RELAIS2 16
extern unsigned int ThresholdWatts;
extern unsigned int ThresholdTime;
extern unsigned int AdcCalib;
extern void initdisplay();
extern EnergyMonitor emon1;   

#define EEPROM_ADDRESS_THRESHOLD_W 0
#define EEPROM_ADDRESS_THRESHOLD_T 10 
#define EEPROM_ADDRESS_CALIB       20

void EEPROMWriteInt(int p_address, int p_value)
{
     byte lowByte = ((p_value >> 0) & 0xFF);
     byte highByte = ((p_value >> 8) & 0xFF);

     EEPROM.update(p_address, lowByte);
     EEPROM.update(p_address + 1, highByte);
}

int EEPROMReadInt(int p_address)
{
     byte lowByte = EEPROM.read(p_address);
     byte highByte = EEPROM.read(p_address + 1);

     return (int)((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void writeEeprom()
{
  EEPROMWriteInt(EEPROM_ADDRESS_THRESHOLD_W, ThresholdWatts);
  EEPROMWriteInt(EEPROM_ADDRESS_THRESHOLD_T, ThresholdTime);
  EEPROMWriteInt(EEPROM_ADDRESS_CALIB, AdcCalib);
}

void readEeprom()
{
  ThresholdWatts = EEPROMReadInt(EEPROM_ADDRESS_THRESHOLD_W);
  ThresholdTime = EEPROMReadInt(EEPROM_ADDRESS_THRESHOLD_T);
  AdcCalib = EEPROMReadInt(EEPROM_ADDRESS_CALIB);
}

extern void ActivatePowerLineRelais();
extern void DeactivatePowerLineRelais();
extern void InitRelais();

int g_func_timer_info = 0;  // time counter (global variable)
unsigned long g_timer_1 = 0;    // timer variable (globale variable)
// *********************************************************************
void LCDML_DISP_setup(LCDML_FUNC_mainView)
// *********************************************************************
{
  readEeprom();
}

void LCDML_DISP_loop(LCDML_FUNC_mainView)
{
  static int timeoutStarted = 0;
  static int Timeout_s = 0;
  double Irms = emon1.calcIrms(1480) / 4;  // Calculate Irms only
  double Watts = Irms*230.0;
  lcd.setCursor(0, 0);
  lcd.print("Watts: ");
  lcd.setCursor(10, 0);
  lcd.print(Watts);
  lcd.setCursor(0, 1);
  lcd.print("Treshold [W]: ");
  lcd.setCursor(15, 1);
  lcd.print(ThresholdWatts);
  lcd.setCursor(0, 2);
  lcd.print("Treshold [min]: ");
  lcd.setCursor(16, 2);
  lcd.print(ThresholdTime);

  if(LCDML_BUTTON_checkEnter()  /* || watts < thresh */)
  {
    ActivatePowerLineRelais();
    timeoutStarted = 1;
    Timeout_s = 60 * ThresholdTime;
  }

  if(timeoutStarted == 1)
  {
    if((millis() - g_timer_1) >= 500) 
    {
      g_timer_1 = millis();   
      g_func_timer_info--;                // increment the value every secound
      lcd.setCursor(0, 3);                // set cursor pos
      lcd.print(g_func_timer_info);       // print the time counter value
      if(g_func_timer_info < 0)
      {
        g_func_timer_info = 0;
        DeactivatePowerLineRelais();
      }
    }
  }
  
  if(LCDML_BUTTON_checkLeft())
  {
    LCDML_DISP_funcend();
  } 
}

void LCDML_DISP_loop_end(LCDML_FUNC_mainView)
{
  LCDML_DISP_resetIsTimer();
}  

void LCDML_DISP_setup(LCDML_FUNC_threshold_w) 
{
  readEeprom();
}
void LCDML_DISP_loop_end(LCDML_FUNC_threshold_w) 
{
  writeEeprom();
}
void LCDML_DISP_loop(LCDML_FUNC_threshold_w)   
{
  static int Mult = 1;
  if(LCDML_BUTTON_checkEnter())
  {
    LCDML_BUTTON_resetAll();
    if(Mult % 1000 != 0)
      Mult *= 10;
    else
      Mult = 1;
  }
  lcd.setCursor(10, 4);
  lcd.print(F("Mult:          "));
  lcd.setCursor(16, 4);
  lcd.print(Mult);

  if(LCDML_BUTTON_checkUp())
  {
    LCDML_BUTTON_resetAll();
    ThresholdWatts += Mult;
  }
  else if(LCDML_BUTTON_checkDown())
  {
    LCDML_BUTTON_resetAll();
    ThresholdWatts -= Mult;
  }

  if(ThresholdWatts > 3680)
    ThresholdWatts = 0; 
  
  lcd.setCursor(0, 0);
  lcd.print(F("Threshold [W]:       ")); 
  lcd.setCursor(15, 0);
  lcd.print(ThresholdWatts);
}

void LCDML_DISP_setup(LCDML_FUNC_threshold_t) 
{
  readEeprom();
}
void LCDML_DISP_loop_end(LCDML_FUNC_threshold_t) 
{
  writeEeprom();
}
void LCDML_DISP_loop(LCDML_FUNC_threshold_t)   
{
  static int Mult = 1;
  if(LCDML_BUTTON_checkEnter())
  {
    LCDML_BUTTON_resetAll();
    if(Mult % 100 != 0)
      Mult *= 10;
    else
      Mult = 1;
  }
  lcd.setCursor(10, 4);
  lcd.print(F("Mult:          "));
  lcd.setCursor(16, 4);
  lcd.print(Mult);

  if(LCDML_BUTTON_checkUp())
  {
    LCDML_BUTTON_resetAll();
    ThresholdTime += Mult;
  }
  else if(LCDML_BUTTON_checkDown())
  {
    LCDML_BUTTON_resetAll();
    ThresholdTime -= Mult;
  }
  
  if(ThresholdTime > 999)
    ThresholdTime = 0;

  lcd.setCursor(0, 0);
  lcd.print(F("Threshold [min]:       ")); 
  lcd.setCursor(17, 0);
  lcd.print(ThresholdTime);
}

void LCDML_DISP_setup(LCDML_FUNC_calibration) 
{
  readEeprom();
}
void LCDML_DISP_loop(LCDML_FUNC_calibration)     
{
  lcd.setCursor(0, 0);
  lcd.print(F("Unplug connectors"));
  lcd.setCursor(0, 1);
  lcd.print(F("and exit this menu"));
  int adcValue = analogRead(A0);
  lcd.setCursor(0, 3);
  lcd.print(F("Calib value:    "));
  lcd.setCursor(13, 3);
  lcd.print(adcValue);
  if(LCDML_BUTTON_checkLeft())
  {
    LCDML_DISP_funcend();
  } 
} 
void LCDML_DISP_loop_end(LCDML_FUNC_calibration)
{
  writeEeprom();
}

void LCDML_DISP_setup(LCDML_FUNC_test)   
{
  InitRelais();
} 
  
void LCDML_DISP_loop(LCDML_FUNC_test)
{
  static int ButtonPressed = 0;
  static int RelaisSwitch = 0;
  lcd.setCursor(0, 0);
  if(ButtonPressed == 0)
  {
     lcd.print(F("Relais turned on "));
  }
  if(LCDML_BUTTON_checkEnter())
  {
    ButtonPressed = 1;
    LCDML_BUTTON_resetAll();
    if(RelaisSwitch == 0)
    {
      DeactivatePowerLineRelais();
      RelaisSwitch = 1;
      initdisplay();
      lcd.setCursor(0, 0);
      lcd.print(F("Relais turned off "));
    }
    else
    {
      ActivatePowerLineRelais();
      RelaisSwitch = 0;
      initdisplay();
      lcd.setCursor(0, 0);
      lcd.print(F("Relais turned on "));
      
    }
  }
  
}
void LCDML_DISP_loop_end(LCDML_FUNC_test) 
{
  DeactivatePowerLineRelais();
  initdisplay();
}
