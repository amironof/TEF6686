#include "TEF6686.h"
#include "Tuner_Patch_Lithio_V102_p209.h"

//TwoWire Wire2 (I2C_PORT, I2C_FAST_MODE);
TwoWire Wire2;

#define INIT_FLAG_TIMER    0xff
#define INIT_FLAG_PATCH1  0xfe
#define INIT_FLAG_PATCH2  0xfd
#define TEF665X_SPLIT_SIZE 24

static const char tuner_init_tab[] PROGMEM =
{
3,  0x1c,0x00,0x00,//Clear Required Initialization Control
3,  0x1C,0x00,0x74,//Set Required Initialization Control(1)
//Load Required Initialization(s)....
1,  INIT_FLAG_PATCH1,

3,  0x1c,0x00,0x00,//Clear Required Initialization Control
3,  0x1C,0x00,0x75,//Set Required Initialization Control(2)
//Load Required Initialization(s)....
1,  INIT_FLAG_PATCH2,

3,  0x1c,0x00,0x00,//Clear Required Initialization Control
3,  0x14,0x00,0x01,//Start Firmware....

//wait 50ms
2,  INIT_FLAG_TIMER,50, //(Boot state to Idle State)

9,  0x40,0x04,0x01,0x00,0x8C,0xA0,0x00,0x00,0x00,// 9216 MHz crystal reference
//9, 0x40,0x04,0x01,0x00,0x3D,0x09,0x00,0x00,0x00,// 4000 MHz crystal reference

//Activate Device...(Idle state to Active state)
5,  0x40,0x05,0x01,0x00,0x01,// APPL_Activate
//wait 100ms
2,  INIT_FLAG_TIMER,100, //(Idle state to Active state)

11, 0x20,0x0A,0x01,0x00,0x01,0x0C,0x26,0x05,0xDC,0x05,0xDC, // FM_Set_Bandwidth (1, 1, 3110, 1000, 1000)
7,  0x21,0x0A,0x01,0x00,0x00,0x00,0x1E,   // AM_Set_Bandwidth (1, 0, 30)
5,  0x20,0x16,0x01,0x00,0x01,             // FM_Set_ChannelEqualizer (1, 1)
5,  0x20,0x1F,0x01,0x01,0xF4,             // FM_Set_Deemphasis (1, 500)
5,  0x20,0x1E,0x01,0x00,0x00,             // FM_Set_DigitalRadio (1, 0)
11, 0x20,0x53,0x01,0x00,0x00,0x00,0x32,0x00,0x32,0x00,0x00,       // FM_Set_DR_Blend (1, 0, 50, 50, 0)
9,  0x20,0x54,0x01,0x00,0x00,0x22,0x04,0x10,0x10,             // FM_Set_DR_Options (1, 0, 8708, 4112)
5,  0x20,0x14,0x01,0x00,0x01,             // FM_Set_MphSuppression (1, 1)
7,  0x20,0x17,0x01,0x00,0x01,0x03,0xE8,   // FM_Set_NoiseBlanker (1, 1, 1000)
7,  0x20,0x52,0x01,0x00,0x00,0x00,0x00,   // FM_Set_QualityStatus (1, 0, 0)
7,  0x20,0x52,0x01,0x00,0xC8,0x00,0x00,   // FM_Set_QualityStatus (1, 200, 0)
7,  0x20,0x0B,0x01,0x03,0x98,0x00,0x00,   // FM_Set_RFAGC (1, 920, 0)
5,  0x20,0x50,0x01,0x00,0x00,             // FM_Set_Scaler (1, 0)
5,  0x20,0x55,0x01,0x00,0x00,             // FM_Set_Specials (1, 0)
11, 0x20,0x02,0x01,0x00,0x00,0x09,0x38,0x03,0xE8,0x07,0xD0,      // FM_Set_Tune_Options (1, 0, 2360, 1000, 2000)
9,  0x20,0x51,0x01,0x00,0x01,0x00,0x02,0x00,0x00,             // FM_Set_RDS (1, 1, 2, 0)
17, 0x20,0x26,0x01,0xFF,0xEC,0xFF,0xE2,0xFF,0xD8,0xFF,0xCE,0xFF,0xC4,0xFF,0xC4,0xFF,0xC4, // FM_Set_LevelStep (1, -20, -30, -40, -50, -60, -60, -60)
17, 0x20,0x26,0x01,0xFF,0xEC,0xFF,0xE2,0xFF,0xD8,0xFF,0xCE,0xFF,0xC4,0xFF,0xC4,0xFF,0xC4, // FM_Set_LevelStep (1, -20, -30, -40, -50, -60, -60, -60)
17, 0x20,0x26,0x01,0xFF,0xEC,0xFF,0xE2,0xFF,0xD8,0xFF,0xCE,0xFF,0xC4,0xFF,0xC4,0xFF,0xC4, // FM_Set_LevelStep (1, -20, -30, -40, -50, -60, -60, -60)
17, 0x20,0x26,0x01,0xFF,0xEC,0xFF,0xE2,0xFF,0xD8,0xFF,0xCE,0xFF,0xC4,0xFF,0xC4,0xFF,0xC4, // FM_Set_LevelStep (1, -20, -30, -40, -50, -60, -60, -60)
17, 0x20,0x26,0x01,0xFF,0xEC,0xFF,0xE2,0xFF,0xD8,0xFF,0xCE,0xFF,0xC4,0xFF,0xC4,0xFF,0xC4, // FM_Set_LevelStep (1, -20, -30, -40, -50, -60, -60, -60)
17, 0x20,0x26,0x01,0xFF,0xEC,0xFF,0xE2,0xFF,0xD8,0xFF,0xCE,0xFF,0xC4,0xFF,0xC4,0xFF,0xC4, // FM_Set_LevelStep (1, -20, -30, -40, -50, -60, -60, -60)
17, 0x20,0x26,0x01,0xFF,0xEC,0xFF,0xE2,0xFF,0xD8,0xFF,0xCE,0xFF,0xC4,0xFF,0xC4,0xFF,0xC4, // FM_Set_LevelStep (1, -20, -30, -40, -50, -60, -60, -60)
5,  0x20,0x27,0x01,0x00,0x00,                 // FM_Set_LevelOffset (1, 0)
9,  0x20,0x2A,0x01,0x00,0x02,0x00,0x96,0x00,0xDC,             // FM_Set_SoftMute_Level (1, 2, 150, 220)
11, 0x20,0x28,0x01,0x00,0x78,0x01,0xF4,0x00,0x0A,0x00,0x14,   // FM_Set_SoftMute_Time (1, 120, 500, 10, 20)
9,  0x20,0x2C,0x01,0x00,0x00,0x00,0xC8,0x03,0xE8,             // FM_Set_SoftMute_Mph (1, 0, 200, 1000)
9,  0x20,0x2B,0x01,0x00,0x00,0x00,0xC8,0x03,0xE8,             // FM_Set_SoftMute_Noise (1, 0, 200, 1000)
7,  0x20,0x2D,0x01,0x00,0x01,0x00,0x00,                 // FM_Set_SoftMute_Max (1, 1, 0)
7,  0x21,0x2D,0x01,0x00,0x01,0x00,0x00,                 // AM_Set_SoftMute_Max (1, 1, 0)
9,  0x20,0x34,0x01,0x00,0x03,0x01,0x68,0x01,0x2C,             // FM_Set_HighCut_Level (1, 3, 360, 300)
11, 0x20,0x32,0x01,0x00,0xC8,0x07,0xD0,0x00,0x0A,0x00,0x50,       // FM_Set_HighCut_Time (1, 200, 2000, 10, 80)
11, 0x20,0x33,0x01,0x00,0x00,0x00,0xFA,0x00,0x82,0x01,0xF4,       // FM_Set_HighCut_Mod (1, 0, 250, 130, 500)
9,  0x20,0x36,0x01,0x00,0x03,0x00,0x78,0x00,0xA0,         // FM_Set_HighCut_Mph (1, 3, 120, 160)
9,  0x20,0x35,0x01,0x00,0x03,0x00,0x96,0x00,0xC8,            // FM_Set_HighCut_Noise (1, 3, 150, 200)
7,  0x20,0x3A,0x01,0x00,0x00,0x00,0x14,                  // FM_Set_LowCut_Min (1, 0, 20)
7,  0x20,0x37,0x01,0x00,0x01,0x09,0x60,                  // FM_Set_HighCut_Max (1, 1, 2400)
7,  0x20,0x39,0x01,0x00,0x01,0x00,0x64,                  // FM_Set_LowCut_Max (1, 1, 100)
5,  0x20,0x3B,0x01,0x00,0x01,                  // FM_Set_HighCut_Options (1, 1)
9,  0x20,0x3E,0x01,0x00,0x03,0x01,0xCC,0x00,0xF0,             // FM_Set_Stereo_Level (1, 3, 460, 240)
11, 0x20,0x3C,0x01,0x00,0xC8,0x0F,0xA0,0x00,0x14,0x00,0x50,      // FM_Set_Stereo_Time (1, 200, 4000, 20, 80)
11, 0x20,0x3D,0x01,0x00,0x00,0x00,0xD2,0x00,0x5A,0x01,0xF4,      // FM_Set_Stereo_Mod (1, 0, 210, 90, 500)
9,  0x20,0x40,0x01,0x00,0x03,0x00,0x64,0x00,0x96,             // FM_Set_Stereo_Mph (1, 3, 100, 150)
9,  0x20,0x3F,0x01,0x00,0x03,0x00,0x78,0x00,0xA0,             // FM_Set_Stereo_Noise (1, 3, 120, 160)
7,  0x20,0x42,0x01,0x00,0x02,0x01,0x90,               // FM_Set_Stereo_Min (1, 2, 400)
5,  0x20,0x41,0x01,0x00,0x00,                   // FM_Set_Stereo_Max (1, 0)
9,  0x20,0x48,0x01,0x00,0x03,0x02,0x58,0x00,0xF0,             // FM_Set_StHiBlend_Level (1, 3, 600, 240)
11, 0x20,0x46,0x01,0x01,0xF4,0x07,0xD0,0x00,0x14,0x00,0x14,       // FM_Set_StHiBlend_Time (1, 500, 2000, 20, 20)
11, 0x20,0x47,0x01,0x00,0x00,0x00,0xF0,0x00,0x78,0x02,0x9E,       // FM_Set_StHiBlend_Mod (1, 0, 240, 120, 670)
9,  0x20,0x4A,0x01,0x00,0x03,0x00,0x50,0x00,0x8C,        // FM_Set_StHiBlend_Mph (1, 3, 80, 140)
9,  0x20,0x49,0x01,0x00,0x03,0x00,0x50,0x00,0x8C,             // FM_Set_StHiBlend_Noise (1, 3, 80, 140)
7,  0x20,0x4C,0x01,0x00,0x00,0x1B,0x58,               // FM_Set_StHiBlend_Min (1, 0, 7000)
7,  0x20,0x4B,0x01,0x00,0x01,0x0F,0xA0,                   // FM_Set_StHiBlend_Max (1, 1, 4000)
5,  0x20,0x50,0x01,0x00,0x3C,                   // FM_Set_Scaler (1, 60)
5,  0x21,0x50,0x01,0x00,0x3C,                   // AM_Set_Scaler (1, 60)
//Set the Audio and Application related API settings...
9,  0x40,0x03,0x01,0x00,0x00,0x00,0x21,0x00,0x03,             // APPL_Set_GPIO (1, 0, 33, 3)
9,  0x40,0x03,0x01,0x00,0x01,0x00,0x21,0x00,0x00,            // APPL_Set_GPIO (1, 1, 33, 0)
9,  0x40,0x03,0x01,0x00,0x02,0x00,0x21,0x00,0x00,             // APPL_Set_GPIO (1, 2, 33, 0)
9,  0x40,0x03,0x01,0x00,0x00,0x00,0x20,0x00,0x03,             // APPL_Set_GPIO (1, 0, 32, 3)
9,  0x40,0x03,0x01,0x00,0x01,0x00,0x20,0x00,0x00,             // APPL_Set_GPIO (1, 1, 32, 0)
9,  0x40,0x03,0x01,0x00,0x02,0x00,0x20,0x00,0x00,             // APPL_Set_GPIO (1, 2, 32, 0)
7,  0x30,0x15,0x01,0x00,0x80,0x00,0x01,               // AUDIO_Set_Ana_Out (1, 128, 1)
13, 0x30,0x16,0x01,0x00,0x20,0x00,0x00,0x00,0x20,0x00,0x00,0x11,0x3A, // AUDIO_Set_Dig_IO (1, 32, 0, 32, 0, 4410)
13, 0x30,0x16,0x01,0x00,0x21,0x00,0x00,0x00,0x20,0x00,0x00,0x11,0x3A, // AUDIO_Set_Dig_IO (1, 33, 0, 32, 0, 4410)
5,  0x30,0x0C,0x01,0x00,0x00,                   // AUDIO_Set_Input (1, 0)
7,  0x30,0x17,0x01,0x00,0x20,0x00,0x00,         // AUDIO_Set_Input_Scaler (1, 32, 0)
5,  0x30,0x0B,0x01,0x00,0x00,                   // AUDIO_Set_Mute (1, 0)
5,  0x30,0x0A,0x01,0x00,0x00,                   // AUDIO_Set_Volume (1, 0)
15, 0x30,0x18,0x01,0x00,0x00,0x00,0x00,0xFF,0x38,0x01,0x90,0xFF,0x38,0x03,0xE8, // AUDIO_Set_WaveGen (1, 0, 0, -200, 400, -200, 1000)
7,  0x30,0x0D,0x01,0x00,0x21,0x00,0xE0,                  // AUDIO_Set_Output_Source (1, 33, 224)
7,  0x30,0x0D,0x01,0x00,0x80,0x00,0xE0,                 // AUDIO_Set_Output_Source (1, 128, 224)
};

//return 1 --> IIC sucess
unsigned char Tuner_WriteBuffer(unsigned char *buf, uint16_t len)
{
  uint16_t i;
  uint8_t r;
  Wire2.beginTransmission(I2C_ADDR);
  for (i = 0; i < len; i++) 
  {
    Wire2.write(buf[i]);
  }
  r = Wire2.endTransmission();
  delay(1);
  return (r == 0) ? 1 : 0;
}

unsigned char Tuner_ReadBuffer(unsigned char *buf, uint16_t len)
{
  uint16_t i;
  Wire2.requestFrom(I2C_ADDR, len);
  if (Wire2.available() == len) 
  {
    for (i = 0; i < len; i++) 
    {
      buf[i] = Wire2.read();
    }
    return 1;
  }
  return 0;
}

void Tuner_WaitMs (uint16_t ms)
{
  delay(ms);
}

static uint16_t Tuner_Patch_Load(const unsigned char * pLutBytes, uint16_t size)
{
  unsigned char buf[TEF665X_SPLIT_SIZE+1];
  uint16_t i,len;
  uint16_t r;

  buf[0] = 0x1b;

  while(size)
  {
    len = (size>TEF665X_SPLIT_SIZE)? TEF665X_SPLIT_SIZE : size;
    size -= len;

    for(i=0;i<len;i++)
      buf[1+i] = pLutBytes[i];

    pLutBytes+=len;
    
    if(1 != (r = Tuner_WriteBuffer(buf, len+1)))
    {
      break;
    }
  }

  return r;
}
static uint16_t Tuner_Table_Write(const unsigned char * tab)
{
//use a timer delay in init table
  if(tab[1] == INIT_FLAG_TIMER)
  {
    Tuner_WaitMs(tab[2]);
    return 1;
  }
//load patch table1 
  else if(tab[1] == INIT_FLAG_PATCH1)
  {
    return Tuner_Patch_Load(pPatchBytes, PatchSize);
  }
//load patch table2 
  else if(tab[1] == INIT_FLAG_PATCH2)
  {
    return Tuner_Patch_Load(pLutBytes, LutSize);
  }
  else
    return Tuner_WriteBuffer((unsigned char *)&tab[1], tab[0]);
}

void Tuner_I2C_Init() {
  Wire2.begin();
}

uint16_t Tuner_Init(void) //return 1 = sucsess
{
  uint16_t i;
  uint16_t r;
  uint16_t j;
  uint16_t limit;
  const unsigned char *p = &tuner_init_tab[0];
  unsigned char t[20];
  const size_t s = sizeof(tuner_init_tab);
  for(i = 0; i < s; i += pgm_read_byte(p + i) + 1)
  {
    limit = pgm_read_byte(p + i);
    for(j = 0; j <= limit; j++)
      t[j] = pgm_read_byte(p + i + j);
    if( (r=Tuner_Table_Write(t)) != 1)
      break;
  }
  return r;
} 
