#include "TEF6686.h"

const char  PTY_0 []    PROGMEM   = { "None PTY" };
const char  PTY_1 []    PROGMEM   = { "  News  " };
const char  PTY_2 []    PROGMEM   = { "Affairs " };
const char  PTY_3 []    PROGMEM   = { "  Info  " };
const char  PTY_4 []    PROGMEM   = { " Sport  " };
const char  PTY_5 []    PROGMEM   = { "Educate " };
const char  PTY_6 []    PROGMEM   = { " Drama  " };
const char  PTY_7 []    PROGMEM   = { "Culture " };
const char  PTY_8 []    PROGMEM   = { "Science " };
const char  PTY_9 []    PROGMEM   = { " Varied " };
const char  PTY_10[]    PROGMEM   = { " Pop M  " };
const char  PTY_11[]    PROGMEM   = { " Rock M " };
const char  PTY_12[]    PROGMEM   = { " Easy M " };
const char  PTY_13[]    PROGMEM   = { "Light M " };
const char  PTY_14[]    PROGMEM   = { "Classics" };
const char  PTY_15[]    PROGMEM   = { "Other M " };
const char  PTY_16[]    PROGMEM   = { "Weather " };
const char  PTY_17[]    PROGMEM   = { "Finance " };
const char  PTY_18[]    PROGMEM   = { "Children" };
const char  PTY_19[]    PROGMEM   = { " Social " };
const char  PTY_20[]    PROGMEM   = { "Religion" };
const char  PTY_21[]    PROGMEM   = { "Phone In" };
const char  PTY_22[]    PROGMEM   = { " Travel " };
const char  PTY_23[]    PROGMEM   = { "Leisure " };
const char  PTY_24[]    PROGMEM   = { "  Jazz  " };
const char  PTY_25[]    PROGMEM   = { "Country " };
const char  PTY_26[]    PROGMEM   = { "Nation M" };
const char  PTY_27[]    PROGMEM   = { " Oldies " };
const char  PTY_28[]    PROGMEM   = { " Folk M " };
const char  PTY_29[]    PROGMEM   = { "Document" };
const char  PTY_30[]    PROGMEM   = { "  TEST  " };
const char  PTY_31[]    PROGMEM   = { "Alarm ! " };

const char* const pty[] PROGMEM =
{
    PTY_0,  PTY_1,  PTY_2,  PTY_3,
    PTY_4,  PTY_5,  PTY_6,  PTY_7,
    PTY_8,  PTY_9,  PTY_10, PTY_11,
    PTY_12, PTY_13, PTY_14, PTY_15,
    PTY_16, PTY_17, PTY_18, PTY_19,
    PTY_20, PTY_21, PTY_22, PTY_23,
    PTY_24, PTY_25, PTY_26, PTY_27,
    PTY_28, PTY_29, PTY_30, PTY_31
};

TEF6686::TEF6686() {
}

uint8_t TEF6686::init() {
  uint8_t result;
  uint8_t counter = 0;
  uint8_t status;

  Tuner_I2C_Init();
  
  delay(5);
  while (true) {
    result = devTEF668x_APPL_Get_Operation_Status(&status);
    if (result == 1) {
      Tuner_Init();
      powerOff();
      return 1; //Ok
    }
    else if (++counter > 50) {
      return 2; //Doesn't exist
    }
    else {
      delay(5);
      return 0;  //Busy
    }
  } 
}

void TEF6686::powerOn() {
  devTEF668x_APPL_Set_OperationMode(0);
}

void TEF6686::powerOff() {
  devTEF668x_APPL_Set_OperationMode(1);
}

void TEF6686::setFrequency(uint16_t frequency) {
  Radio_SetFreq(Radio_PRESETMODE, Radio_GetCurrentBand(), frequency);
}

uint16_t TEF6686::getFrequency() {
  return Radio_GetCurrentFreq();
}

uint16_t TEF6686::getLevel() {
  return Radio_Get_Level(1);
}

uint16_t TEF6686::getBandwidth() {
  return Radio_Get_Bandwidth();
}

uint8_t TEF6686::getStereoStatus() {
  return Radio_CheckStereo();
}

void TEF6686::setVolume(uint16_t volume) {
  devTEF668x_Audio_Set_Volume(volume);
}

void TEF6686::setMute() {
  devTEF668x_Audio_Set_Mute(1);
}

void TEF6686::setUnMute() {
  devTEF668x_Audio_Set_Mute(0);
}

void TEF6686::clearRDS ()
{
  uint8_t i;
  for(i=0; i<9; i++) rdsProgramService[i] = 0;
  for(i=0; i<65; i++) rdsRadioText[i] = 0;
  for(i=0; i<9; i++) rdsProgramType[i] = 0;
  for(i=0; i<5; i++) rdsProgramId[i] = 0;
}

uint8_t TEF6686::readRDS() {
  char status;
  uint8_t rdsBHigh, rdsBLow, rdsCHigh, rdsCLow, rdsDHigh, isReady, rdsDLow;

  uint16_t rdsStat, rdsA, rdsB, rdsC, rdsD, rdsErr;
  uint16_t result = devTEF668x_Radio_Get_RDS_Data(1, &rdsStat, &rdsA, &rdsB, &rdsC, &rdsD, &rdsErr);

  if (!(result && (rdsB != 0x0) && ((rdsStat & 0x8000) != 0x0) && ((rdsErr & 0x0a00) == 0x0))) {
    return isReady; 
  }

  rdsBHigh = (uint8_t)(rdsB >> 8);
  rdsBLow = (uint8_t)rdsB;
  rdsCHigh = (uint8_t)(rdsC >> 8);
  rdsCLow = (uint8_t)rdsC;
  rdsDHigh = (uint8_t)(rdsD >> 8);
  rdsDLow = (uint8_t)rdsD;
 
  uint8_t programType = ((rdsBHigh & 3) << 3) | ((rdsBLow >> 5) & 7);
  strcpy_P(rdsProgramType, (char*)pgm_read_word(&(pty[programType])));

  uint8_t type = (rdsBHigh >> 4) & 15;
  uint8_t version = bitRead(rdsBHigh, 4);

  // PI
  char Hex[16]={ '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  rdsProgramId[0]=Hex[(rdsA & 0xF000U) >> 12];
  rdsProgramId[1]=Hex[(rdsA & 0x0F00U) >> 8];
  rdsProgramId[2]=Hex[(rdsA & 0x00F0U) >> 4];
  rdsProgramId[3]=Hex[(rdsA & 0x000FU)];
  rdsProgramId[4]='\0';
  
  // Groups 0A & 0B
  // Basic tuning and switching information only
  if (type == 0) {
    uint8_t address = rdsBLow & 3;
    // Groups 0A & 0B: to extract PS segment we need blocks 1 and 3
    if (address >= 0 && address <= 3) {
      if (rdsDHigh != '\0') {
        rdsProgramService[address * 2] = rdsDHigh;
      }  
      if (rdsDLow != '\0') {
        rdsProgramService[address * 2 + 1] = rdsDLow;
      }  
      isReady = (address == 3) ? 1 : 0;
    } 
    rdsFormatString(rdsProgramService, 8);
  }
  // Groups 2A & 2B
  // Radio Text
  else if (type == 2) {
    uint16_t addressRT = rdsBLow & 15;
    uint8_t ab = bitRead(rdsBLow, 4);
    uint8_t cr = 0;
    uint8_t len = 64;
    if (version == 0) {
      if (addressRT >= 0 && addressRT <= 15) {
        if (rdsCHigh != 0x0D) {
          rdsRadioText[addressRT*4] = rdsCHigh;
        }  
        else {
          len = addressRT * 4;
          cr = 1;
        }
        if (rdsCLow != 0x0D) {
          rdsRadioText[addressRT * 4 + 1] = rdsCLow;
        }  
        else {
          len = addressRT * 4 + 1;
          cr = 1;
        }
        if (rdsDHigh != 0x0D) {
          rdsRadioText[addressRT * 4 + 2] = rdsDHigh;
        }  
        else {
          len = addressRT * 4 + 2;
          cr = 1;
        }
        if (rdsDLow != 0x0D) {
          rdsRadioText[addressRT * 4 + 3] = rdsDLow;
        }
        else {
          len = addressRT * 4 + 3;
          cr = 1;
        }
      }
    }
    else {
      if (addressRT >= 0 && addressRT <= 7) {
        if (rdsDHigh != '\0') {
          rdsRadioText[addressRT * 2] = rdsDHigh;
        }  
        if (rdsDLow != '\0') {
          rdsRadioText[addressRT * 2 + 1] = rdsDLow;
        }
      }
    }
    if (cr) {
      for (uint8_t i = len; i < 64; i++) {
        rdsRadioText[i] = ' ';
      }
    }
    if (ab != rdsAb) {      
      for (uint8_t i = len; i < 64; i++) {
        rdsRadioText[i] = ' ';
      }
      rdsRadioText[64] = '\0';     
      isRdsNewRadioText = 1;
    }
    else {
      isRdsNewRadioText = 0;
    }
    rdsAb = ab;
    rdsFormatString(rdsRadioText, 64);
  }
  return isReady; 
}
 
void TEF6686::getRDS(RdsInfo* rdsInfo) {
  strcpy(rdsInfo->programType, rdsProgramType);
  strcpy(rdsInfo->programId, rdsProgramId);
  strcpy(rdsInfo->programService, rdsProgramService);
  strcpy(rdsInfo->radioText, rdsRadioText); 
}

void TEF6686::rdsFormatString(char* str, uint16_t length) {  
  for (uint16_t i = 0; i < length; i++) {    
    if ((str[i] != 0 && str[i] < 32) || str[i] > 126 ) {
      str[i] = ' ';  
    }
  }
}

uint16_t TEF6686::seek(uint8_t up, uint8_t step) {
  uint16_t mode = 20;
  uint16_t startFrequency = Radio_GetCurrentFreq();

  while (true) {
    switch(mode){
      case 20:
        Radio_ChangeFreqOneStep(up, step);
        Radio_SetFreq(Radio_SEARCHMODE, Radio_GetCurrentBand(), Radio_GetCurrentFreq());
      
        mode = 30;
        Radio_CheckStationInit();
        Radio_ClearCurrentStation();
        
        break;
      
      case 30:
        delay(20);
        Radio_CheckStation();
        if (Radio_CheckStationStatus() >= NO_STATION) {
          mode = 40;
        }   
        
        break;

      case 40:
        if (Radio_CheckStationStatus() == NO_STATION) {        
          mode = (startFrequency == Radio_GetCurrentFreq()) ? 50 : 20;
        }
        else if (Radio_CheckStationStatus() == PRESENT_STATION) {
          mode = 50;
        }
        
        break;
      
      case 50:
        Radio_SetFreq(Radio_PRESETMODE, Radio_GetCurrentBand(), Radio_GetCurrentFreq());
        return Radio_GetCurrentFreq();
    }
  }
  return 0;
}

uint16_t TEF6686::seekSync(uint8_t up, uint8_t step) {
  if (seekMode == 0) {	
	seekMode = 20;
	seekStartFrequency = Radio_GetCurrentFreq();
  }
  switch(seekMode) {
    case 20:
	  Radio_ChangeFreqOneStep(up, step);
      Radio_SetFreq(Radio_SEARCHMODE, Radio_GetCurrentBand(), Radio_GetCurrentFreq());
      
      seekMode = 30;
      Radio_CheckStationInit();
      Radio_ClearCurrentStation();
        
      return 0;
      
    case 30:
      delay(20);
      Radio_CheckStation();
      if (Radio_CheckStationStatus() >= NO_STATION) {
        seekMode = 40;
      }   
        
      return 0;

    case 40:
      if (Radio_CheckStationStatus() == NO_STATION) {        
        seekMode = (seekStartFrequency == Radio_GetCurrentFreq()) ? 50 : 20;
      }
      else if (Radio_CheckStationStatus() == PRESENT_STATION) {
        seekMode = 50;
      }
        
      return 0;
      
    case 50:
	  seekMode = 0;
      Radio_SetFreq(Radio_PRESETMODE, Radio_GetCurrentBand(), Radio_GetCurrentFreq());
      return 1;  
  }
  return 0;
}	

uint16_t TEF6686::tune(uint8_t up, uint8_t step) {
  Radio_ChangeFreqOneStep(up, step);

  Radio_SetFreq(Radio_PRESETMODE, Radio_GetCurrentBand(), Radio_GetCurrentFreq());
  Radio_ClearCurrentStation();
  return Radio_GetCurrentFreq();
}

void TEF6686::setBand (uint8_t Band) {
  Radio_SetBand(Band);
}

void TEF6686::setStereo(uint16_t mode)
{
  Radio_SetStereo(mode);
}

void TEF6686::setBW (uint16_t mode, uint16_t bandwidth)
{
  Radio_SetBW(mode, bandwidth);
}
