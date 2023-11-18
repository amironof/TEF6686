// Код, касающийся отправки
// serial-порт к которому подключён Wi-Fi модуль
#define WIFI_SERIAL Serial3

//*-- Параметры полученные при регистрации на http://open-monitoring.online
String IP = "open-monitoring.online";
String ID = "3023"; // ID                                             (!)
String KEY = "0Cnh00"; // Код доступа                                (!)
long interval = 120000; // Периодичность отправки пакетов на сервер (120 секунд)

//*-- GET Information
String GET = "GET /get?cid=" + ID + "&key=" + KEY; // GET request
String HOST = "Host: " + IP + "\r\n\r\n";

boolean isSended = false;
int test_val = 48;
// Конец кода, касаеющегося отправки

#include "TEF6686.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <Wire.h>
#include <EEPROM.h> 
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

hd44780_I2Cexp lcd;  // set the LCD address to 0x27 for a 16 chars and 2 line display

int frequency = 8750;
int pFrequency;
int volume = 40;
int step = 1;
int bwfm = 16;
int bwam = 0;
int nivel = 0;
int levelToSend = 0;
int getbw = 0;
int banda = 0;
int menub = 0;
int enRds = 0;
int enRT = 0;
int enBl = 1;
int enBw = 1;
int enNivel = 1;
int rtLen = 64;
int rtEmpty = 0;
int monster = 0;
byte stereo = 0;

byte empty[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte sm[] = {
  B00000,
  B00001,
  B00001,
  B00101,
  B00101,
  B10101,
  B10101,
  B00000
};

byte vol[] = {
  B00000,
  B11111,
  B00000,
  B01110,
  B00000,
  B00100,
  B00000,
  B00000
};

byte str[] = {
  B01110,
  B00000,
  B00100,
  B00000,
  B00100,
  B00000,
  B01110,
  B00000
};

byte bw[] = {
  B00000,
  B00100,
  B10101,
  B10101,
  B00000,
  B10001,
  B11111,
  B00000
};

byte stp[] = {
  B00001,
  B00000,
  B00001,
  B00000,
  B00001,
  B00000,
  B00001,
  B00000
};

byte j = -1;

uint8_t isRDSReady;  

char programTypePrevious[8];
char programServicePrevious[8];
char programIdPrevious[4];
char radioTextPrevious[64];

long elapsednivel = millis();
long timer_scroll = millis();
long storeTime = millis();
long last_time;
const int eeprom_address = 0;

const uint16_t FMFilterMap[] PROGMEM = {560, 640, 720, 840, 970, 1140, 1330, 1510, 1680, 1840, 2000, 2170, 2360, 2540, 2870, 3110};
const uint16_t AMFilterMap[] PROGMEM = {30, 40, 60, 80};

TEF6686 radio;
RdsInfo rdsInfo;

#define volumeplus 10
#define volumeminus 9
#define left 7
#define right 6
#define menuplus 5
#define menuminus 8
#define sleep 4
#define wake 3

void setup() {
  pinMode(volumeplus, INPUT_PULLUP);
  pinMode(volumeminus, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(menuplus, INPUT_PULLUP);
  pinMode(menuminus, INPUT_PULLUP);
  pinMode(sleep, INPUT_PULLUP);
  pinMode(wake, INPUT_PULLUP);
  
  lcd.begin(16, 2); // initialize the LCD
  lcd.createChar(0, empty);
  lcd.createChar(1, sm);
  lcd.createChar(2, vol);
  lcd.createChar(3, str);
  lcd.createChar(4, bw);
  lcd.createChar(5, stp);
  lcd.backlight();
  lcd.clear(); 
  // Print a logo message to the LCD.
  lcd.setCursor(0,0);
  lcd.print(" TEF6686 radio  ");
  lcd.setCursor(0,1);
  lcd.print("   V102 p2.09   ");
  delay (3000);
  lcd.clear();

  if (digitalRead(sleep) == LOW)
  {
    EEPROM.write(eeprom_address, 0);
    lcd.setCursor(0, 1);
    lcd.print(" EEPROM RESETED ");
    delay(2000);
    lcd.clear();
  }

  radio.init();
  radio.powerOn();
  if (EEPROM.read(eeprom_address) == 1) readAllReceiverInformation();
  radio.setBand(banda);
  banda = Radio_GetCurrentBand();
  radio.setFrequency(pFrequency);
  frequency = radio.getFrequency();
  if (banda < 3) {
    if (bwfm == 16) radio.setBW (1, 311);
    else radio.setBW (0, pgm_read_word(&FMFilterMap[bwfm]));
  }
  else radio.setBW (0, pgm_read_byte(&AMFilterMap[bwam]));
  radio.setVolume(volume-60);
  if (monster == 1) radio.setStereo(0);
  if (enBl == 0) lcd.noBacklight();
  displayInfo();

// Код, касающийся отправки
  // открываем последовательный порт для мониторинга действий в программе
  // и передаём скорость 9600 бод
  Serial.begin(9600);
  while (!Serial) {
  // ждём, пока не откроется монитор последовательного порта
  // для того, чтобы отследить все события в программе
  }
  Serial.print("Serial init OK\r\n");
  // открываем Serial-соединение с Wi-Fi модулем на скорости 115200 бод
  WIFI_SERIAL.begin(9600);
  Serial.print("Setup completed!\r\n");

  last_time = millis();

// Конец кода, касающегося отправки

}

void saveAllReceiverInformation() {
  EEPROM.update(eeprom_address, 1); //
  EEPROM.update(eeprom_address + 1, volume); // stores the current Volume
  EEPROM.update(eeprom_address + 2, banda); // Stores the current band
  EEPROM.update(eeprom_address + 3, step); // Stores the current Mode (FM / AM / SSB)
  EEPROM.update(eeprom_address + 4, bwfm);
  EEPROM.update(eeprom_address + 5, bwam);
  EEPROM.update(eeprom_address + 6, enRds);
  EEPROM.update(eeprom_address + 7, enRT);
  EEPROM.update(eeprom_address + 8, enBw);
  EEPROM.update(eeprom_address + 9, enNivel);
  EEPROM.update(eeprom_address + 10, monster);
  EEPROM.update(eeprom_address + 11, enBl);
  EEPROM.update(eeprom_address + 12, (frequency >> 8) );
  EEPROM.update(eeprom_address + 13, (frequency & 0xFF)); 
}

void readAllReceiverInformation() {
  volume = EEPROM.read(eeprom_address + 1); // Gets the stored volume;
  banda = EEPROM.read(eeprom_address + 2);
  step = EEPROM.read(eeprom_address + 3);
  bwfm = EEPROM.read(eeprom_address + 4);
  bwam = EEPROM.read(eeprom_address + 5);
  enRds = EEPROM.read(eeprom_address + 6);
  enRT = EEPROM.read(eeprom_address + 7);
  enBw = EEPROM.read(eeprom_address + 8);
  enNivel = EEPROM.read(eeprom_address + 9);
  monster = EEPROM.read(eeprom_address + 10);
  enBl = EEPROM.read(eeprom_address + 11);
  frequency = EEPROM.read(eeprom_address + 12) << 8;
  frequency |= EEPROM.read(eeprom_address + 13);
  pFrequency = frequency;
}

void resetEepromDelay() {
    storeTime = millis();
    pFrequency = 0;
}

void delRds() {
  radio.clearRDS();
  strcpy(programIdPrevious, "    ");
  strcpy(programTypePrevious, "        ");
  strcpy(programServicePrevious, "        ");
  strcpy(radioTextPrevious, "                                                                ");
  radioTextPrevious[0] = '\0';
  rtLen = 64;
  rtEmpty = 0;
}

void reDisplay() {
  delRds();
  displayInfo();
}

void loadRds() {
  if (enRds == 1) {
    isRDSReady = radio.readRDS();
    if (isRDSReady == 1) {
      radio.getRDS(&rdsInfo);
      showPI();
      showPS();
      if (enRT == 0) showPTY();
      if (enRT == 1) showRadioText();
    }
  }
}

void showPI() {
  if (!strcmp(rdsInfo.programId, programIdPrevious, 4)) {
    strcpy(programIdPrevious, rdsInfo.programId);
    lcd.setCursor(10,0);
    lcd.print("Pi0000");
    lcd.setCursor(12,0);
    lcd.print(programIdPrevious);
  }  
}

void showPTY() {
  if (!strcmp(rdsInfo.programType, programTypePrevious, 8)) { 
    strcpy(programTypePrevious, rdsInfo.programType);
    lcd.setCursor(4,1);
    lcd.print(programTypePrevious); 
  }
}

void showPS() {
  if ((strlen(rdsInfo.programService) == 8) && !strcmp(rdsInfo.programService, programServicePrevious, 8)) {
    strcpy(programServicePrevious, rdsInfo.programService);
    lcd.setCursor(0,0);
    lcd.print(programServicePrevious);
    lcd.print("  ");
  }
}

void showRadioText() {
  if (rdsInfo.radioText[0] != '\0') {
    if (rdsInfo.radioText[1] != radioTextPrevious[1] ||
        rdsInfo.radioText[4] != radioTextPrevious[4] ||
        rdsInfo.radioText[7] != radioTextPrevious[7])
    {
      j = -1;
      rtLen = 64;
    }
    if (!strcmp(rdsInfo.radioText, radioTextPrevious, 64)) strcpy(radioTextPrevious, rdsInfo.radioText);
    if ((millis() - timer_scroll) > 500) {
      j = j+1;
      timer_scroll = millis();
    }
    if (j > 63) j = 0;
    if (radioTextPrevious[j-8] == ' ' && radioTextPrevious[j-7] == ' ' && radioTextPrevious[j-6] == ' ') {
      rtLen = j;
      if ( j == 8) rtEmpty = 1; 
      j = 0;
    }
    if (rtLen < 9 && rtEmpty == 1) {
      lcd.setCursor(0,1);
      lcd.print(" None RadioText ");
    }
    else if (rtLen > 8 && rtLen < 25) {
      radioTextPrevious[16] = '\0';
      lcd.setCursor(0,1);
      lcd.print(radioTextPrevious);
    }
    else {
      for (int i = 0; i < 16; i++) 
      {
        lcd.setCursor(i,1);
        lcd.print(radioTextPrevious[(i+j)%rtLen]);   
      }
    }
  }
  if (radioTextPrevious[0] == '\0') {
    lcd.setCursor(0,1);
    lcd.print(" None RadioText ");
  }
}

bool strcmp(char* str1, char* str2, int length) {
  for (int i = 0; i < length; i++) {
    if (str1[i] != str2[i]) {
      return false;
    }    
  }  
  return true;
}

void displayInfo() {
    frequency = radio.getFrequency();
    banda = Radio_GetCurrentBand();
    lcd.clear();
    
    // frequency
    lcd.setCursor(0, 0);
    if (banda < 3)
    {
      if (frequency < 10000) lcd.print(" ");
      lcd.print(frequency/100); 
      lcd.print(".");
      lcd.print(frequency%100/10); // thanks to carkiller08
      lcd.print(frequency%10);
      lcd.print("MHz");
    }
    else
    {
      if (frequency < 1000) lcd.print("  ");
      if (frequency > 999 && frequency < 10000) lcd.print(" ");
      lcd.print(frequency); 
      lcd.print("kHz");
    }
  
    // band indicator
    lcd.setCursor(10,0);
    if (banda == 0) lcd.print("   FM1");
    else if (banda == 1) lcd.print("   FM2");
    else if (banda == 2) lcd.print("   FM3");
    else if (banda == 3) lcd.print("    LW");
    else if (banda == 4) lcd.print("    MW");
    else if (banda == 5) lcd.print("   SW1");
    else if (banda == 6) lcd.print("   SW2");
    else if (banda == 7) lcd.print("   SW3");
    else if (banda == 8) lcd.print("   SW4");
    else if (banda == 9) lcd.print("   SW5");
    else if (banda == 10) lcd.print("   SW6");
    
    if (banda < 3) {
      if ((enNivel == 1 && enRT == 0) || (enNivel == 1 && enRT == 1 && enRds == 0)) showNivel();
      if (enRds == 0) {
        if (enBw == 1) showBandwidth();
        else {
          lcd.setCursor(4,1);
          lcd.print("    ");
        }
        showStep();
      }
      showStereo();
    }
    else {
      lcd.setCursor(0,1);
      lcd.print("    ");
      if (enBw == 1) showBandwidth();
      else {
          lcd.setCursor(4,1);
          lcd.print("    ");
        }
      showStep();
    }
    
    // volume
    showVolume();
}    

void showVolume() {
  lcd.setCursor(13,1);
  lcd.write(2);
  lcd.print("  ");
  lcd.setCursor(14,1);
  lcd.print(volume/2);
}

void showNivel() {
  nivel = radio.getLevel();
  if (nivel > 99) nivel = 99;
  lcd.setCursor(2,1);
  lcd.print(" ");
  lcd.setCursor(0,1);
  lcd.write(1);  // antenna sign     
  lcd.print(nivel+7);
}

void showBandwidth() {
  getbw = radio.getBandwidth();
  lcd.setCursor(4,1);
  lcd.write(4);
  lcd.setCursor(6,1);
  lcd.print("  ");
  lcd.setCursor(5,1);
  lcd.print(getbw);
}

void showStep() {
  lcd.setCursor(8,1);
  lcd.write(5);
  lcd.setCursor(10,1);
  lcd.print("  ");
  lcd.setCursor(9,1);
  if (banda < 3) lcd.print(step * 10);
  else lcd.print(step);
}

void showStereo() {
  if (monster == 1) {
    stereo = radio.getStereoStatus();
    lcd.setCursor(13,1);
    if (stereo == 1) lcd.write(3);  // speakers sign
  }
}

void menu() {
  if (menub == 0) {
    lcd.setCursor(0,1);
    lcd.print("  Manual Tune   ");
    delay(1000);
    reDisplay();
  }
  else if (menub == 1) {
    lcd.setCursor(0,1);
    lcd.print("Select Tune Step");
    delay(1000);
    reDisplay();
  }
  else if (menub == 2) {
    lcd.setCursor(0,1);
    lcd.print("  Auto Search   ");
    delay(1000);
    reDisplay();
  }
  else if (menub == 3) {
    lcd.setCursor(0,1);
    lcd.print("  Select Band   ");
    delay(1000);
    reDisplay();
  }
  else if (menub == 4) {
    lcd.setCursor(0,1);
    lcd.print("Select BandWidth");
    delay(1000);
    reDisplay();
  }
  else if (menub == 5) {
    lcd.setCursor(0,1);
    lcd.print(" FM Mono/Stereo ");
    delay(1000);
    reDisplay();
  }
  else if (menub == 6) {
    lcd.setCursor(0,1);
    lcd.print("   RDS On/Off   ");
    delay(1000);
    reDisplay();
  }
  else if (menub == 7) {
    lcd.setCursor(0,1);
    lcd.print("RadioText On/Off");
    delay(1000);
    reDisplay();
  }
  else if (menub == 8) {
    lcd.setCursor(0,1);
    lcd.print(" S-Meter On/Off ");
    delay(1000);
    reDisplay();
  }
  else if (menub == 9) {
    lcd.setCursor(0,1);
    lcd.print("BandWidth On/Off");
    delay(1000);
    reDisplay();
  }
  else if (menub == 10) {
    lcd.setCursor(0,1);
    lcd.print("Backlight On/Off");
    delay(1000);
    reDisplay();
  }
}

void sleepNow()
{
  lcd.noBacklight();
  lcd.noDisplay();
  attachInterrupt(digitalPinToInterrupt(wake), empt, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  sleep_mode();
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(wake));
  lcd.display();
  reDisplay();
  if (enBl == 1) lcd.backlight(); 
}

void empt() {}

void loop() {
  
    if (digitalRead(volumeplus) == LOW)
    {
      volume += 2;
      if (volume >= 84) volume = 84;
      radio.setVolume(volume-60);
      delay(300);
      resetEepromDelay();
      showVolume();
    }
    
    if (digitalRead(volumeminus) == LOW)
    {
      volume -= 2;
      if (volume < 0) volume = 0;
      radio.setVolume(volume-60);
      delay(300);
      resetEepromDelay();
      showVolume();
    }

    if (digitalRead(left) == LOW)
    {
      if (menub == 0) {
        frequency = radio.tune(0, step);
        delay(300);
        j = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 1) {
        step = step - 2;
        if (step == 8) step = 9;
        if (banda < 3 && step == 9 || step == 7) step = 5;
        if (step < 1) step = 1;
        lcd.setCursor(14,1);
        lcd.print("  ");
        lcd.setCursor(0,1);
        lcd.print("Tune Step ");
        if (banda < 3) lcd.print(step * 10);
        else lcd.print(step);
        lcd.print("kHz");
        delay(1000);
        j = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 2) {
        lcd.setCursor(0,1);
        lcd.print(" Searching <<<< ");
        frequency = radio.seek(0, step);
        delay(300);
        j = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 3) {
        Radio_PreviousBand();
        delay(300);
        resetEepromDelay();
        reDisplay();
        if (banda < 3 && step == 9)
        {
          step = 10;
          resetEepromDelay();
          reDisplay();
        }
      }
      else if (menub == 4) {
        if (banda < 3) {
          bwfm = bwfm - 1;
          if (bwfm < 0) bwfm = 0;
          lcd.setCursor(14,1);
          lcd.print("  ");
          lcd.setCursor(0,1);
          lcd.print("FM BWidth ");
          lcd.print(pgm_read_word(&FMFilterMap[bwfm])/10);
          lcd.print("kHz");
          radio.setBW (0, pgm_read_word(&FMFilterMap[bwfm]));
        }
        else {
          bwam = bwam - 1;
          if (bwam < 0) bwam = 0;
          lcd.setCursor(0,1);
          lcd.print(" AM BWidth ");
          lcd.print(pgm_read_byte(&AMFilterMap[bwam])/10);
          lcd.print("kHz ");
          radio.setBW (0, pgm_read_byte(&AMFilterMap[bwam]));
        }
        delay(1000);
        j = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 5) {
        lcd.setCursor(0,1);
        lcd.print("    FM Mono     ");
        if (banda < 3) radio.setStereo(2);
        delay(1000);
        monster = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 6) {
        enRds = 0;
        lcd.setCursor(0,1);
        lcd.print("     RDS Off    ");
        delay(1000);
        j = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 7) {
        enRT = 0;
        lcd.setCursor(0,1);
        lcd.print("  RadioText Off ");
        delay(1000);
        resetEepromDelay();      
        reDisplay();
      }
      else if (menub == 8) {
        enNivel = 0;
        lcd.setCursor(0,1);
        lcd.write(1);
        lcd.print("Off            ");
        delay(1000);
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 9) {
        enBw = 0;
        lcd.setCursor(0,1);
        lcd.print("    ");
        lcd.write(4);
        lcd.print("Off        ");
        delay(1000);
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 10) {
        enBl = 0;
        lcd.noBacklight();
        resetEepromDelay();
      }
    }
    
    if (digitalRead(right) == LOW)
    {
      if (menub == 0) {
        frequency = radio.tune(1, step);
        delay(300);
        j = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 1) {
        step = step + 2;
        if (step == 7) step = 9;
        if (banda < 3 && step == 9) step = 10;
        if (step > 10) step = 10;
        lcd.setCursor(14,1);
        lcd.print("  ");
        lcd.setCursor(0,1);
        lcd.print("Tune Step ");
        if (banda < 3) lcd.print(step * 10);
        else lcd.print(step);
        lcd.print("kHz");
        delay(1000);
        j = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 2) {
        lcd.setCursor(0,1);
        lcd.print(" Searching >>>> ");
        frequency = radio.seek(1, step);
        delay(300);
        j = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 3) {
        Radio_NextBand();
        delay(300);
        resetEepromDelay();
        reDisplay();
        if (banda < 3 && step == 9)
        {
          step = 10;
          resetEepromDelay();
          reDisplay();
        }
      }
      else if (menub == 4) {
        if (banda < 3) {
          bwfm = bwfm + 1;
          if (bwfm > 16) bwfm = 16;
          lcd.setCursor(14,1);
          lcd.print("  ");
          lcd.setCursor(0,1);
          lcd.print("FM BWidth ");
          if (bwfm == 16) {
            lcd.print("Auto  ");
            radio.setBW (1, 311);
          }
          else {
            lcd.print(pgm_read_word(&FMFilterMap[bwfm])/10);
            lcd.print("kHz");
            radio.setBW (0, pgm_read_word(&FMFilterMap[bwfm]));
          }
        }
        else {
          bwam = bwam + 1;
          if (bwam > 3) bwam = 3;
          lcd.setCursor(0,1);
          lcd.print(" AM BWidth ");
          lcd.print(pgm_read_byte(&AMFilterMap[bwam])/10);
          lcd.print("kHz ");
          radio.setBW (0, pgm_read_byte(&AMFilterMap[bwam]));
        }
        delay(1000);
        j = 0;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 5) {
        lcd.setCursor(0,1);
        lcd.print("   FM Stereo    ");
        if (banda < 3) radio.setStereo(0);
        delay(1000);
        monster = 1;
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 6) {
        enRds = 1;
        lcd.setCursor(0,1);
        lcd.print("     RDS On     ");
        delay(1000);
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 7) {
        enRT = 1;
        lcd.setCursor(0,1);
        lcd.print("  RadioText On  ");
        delay(1000);
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 8) {
        enNivel = 1;
        lcd.setCursor(0,1);
        lcd.write(1);
        lcd.print("On             ");
        delay(1000);
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 9) {
        enBw = 1;
        lcd.setCursor(0,1);
        lcd.print("    ");
        lcd.write(4);
        lcd.print("On         ");
        delay(1000);
        resetEepromDelay();
        reDisplay();
      }
      else if (menub == 10) {
        enBl = 1;
        lcd.backlight();
        resetEepromDelay();
      }
    }

    if (digitalRead(menuplus) == LOW)
    {
      menub = menub + 1;
      if (menub > 10) menub = 0;
      menu();
    }

    if (digitalRead(menuminus) == LOW)
    {
      menub = menub - 1;
      if (menub < 0) menub = 10;
      menu();
    }
    
    if (digitalRead(sleep) == LOW) sleepNow();
    
    if (banda < 3) { // for FM
      if ((millis() - elapsednivel) > 1350)
      {
        if (enBw == 1 && enRds == 0) showBandwidth();
        if ((enNivel == 1 && enRT == 0) || (enNivel == 1 && enRT == 1 && enRds == 0)) {
          showNivel();
          showStereo();
        }
        elapsednivel = millis();
      }
      loadRds();
    }
    if (frequency != pFrequency)
    {
    if ( (millis() - storeTime ) > 100) {
      saveAllReceiverInformation();
      storeTime = millis();
      pFrequency = frequency;
    }

    }

    if ((millis() - last_time) > 60000) {
      levelToSend = radio.getLevel() + 7;
      send_data((String)levelToSend);
      last_time = millis();

      Serial.print("levelToSend: ");
      Serial.print(levelToSend);
      Serial.print("\r\n");

    }

  delay(3);
}

void send_data(String SignalLevel) {

  String cmd = "AT+CIPSTART=\"TCP\",\"";// Setup TCP connection
  cmd += IP;
  cmd += "\",80";
  WIFI_SERIAL.println(cmd);
  delay(1000);

  if ( WIFI_SERIAL.find( "Error" ) )
  {
    Serial.print( "RECEIVED: Error\nExit1" );
    return;
  }

  // Отправка строки на сервер
  cmd = GET + "&p1=" + SignalLevel ; //
  // cmd += "&p3=" + T3 ;                 // Раскомментировать для 3-го датчика 
  cmd += " HTTP/1.1\r\n"  + HOST;
  WIFI_SERIAL.print( "AT+CIPSEND=" );
  WIFI_SERIAL.println( cmd.length() );
  if (WIFI_SERIAL.find( ">" ) )
  {
    Serial.print(">");
    Serial.print(cmd);
    WIFI_SERIAL.print(cmd);
    delay(30);
  }
  else
  {
    WIFI_SERIAL.println( "AT+CIPCLOSE" );//close TCP connection
  }
  if ( WIFI_SERIAL.find("OK") )
  {
    Serial.println( "RECEIVED: OK" );
  }
  else
  {
    Serial.println( "RECEIVED: Error\nExit2" );
  }

}
