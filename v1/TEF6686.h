#ifndef TEF6686_h
#define TEF6686_h

#include <Wire.h>

#include "Arduino.h"
#include "Tuner_Api.h"
#include "Tuner_Drv_Lithio.h"
#include "Tuner_Interface.h"

#define I2C_PORT 2
#define I2C_ADDR 0x64

struct RdsInfo {
  char programType[9]; 
  char programService[9];
  char programId[5];
  char radioText[65];
  bool newRadioText;
};

class TEF6686 {
  public:
    TEF6686();
    uint8_t init();
    void powerOn();					// call in setup
    void powerOff();
    void setStereo(uint16_t mode);
    void setBand (uint8_t Band);			
    void setFrequency(uint16_t frequency);    // frequency as int, i.e. 100.00 as 10000
    void setBW (uint16_t mode, uint16_t bandwidth);
    uint16_t getFrequency(); // returns the current frequency
    uint16_t getLevel();
    uint16_t getBandwidth();
    uint8_t getStereoStatus();
    uint16_t seek(uint8_t up, uint8_t step);
	  uint16_t seekSync(uint8_t up, uint8_t step); // returns 1 if seek was completed and 0 if seek in progress
    uint16_t tune(uint8_t up, uint8_t step);
    void setVolume(uint16_t volume); 	//-60 -- +24 dB volume
    void setMute();
    void setUnMute();
    void clearRDS();
    uint8_t readRDS();
    void getRDS(RdsInfo* rdsInfo);
     
  private:
    void rdsFormatString(char* str, uint16_t length);
    char rdsRadioText[65];
    char rdsProgramService[9];
    uint8_t rdsAb;
    char rdsProgramId[5];
    char rdsProgramType[9];
    uint8_t isRdsNewRadioText;
	
	uint16_t seekMode;
	uint16_t seekStartFrequency;
};

#endif
