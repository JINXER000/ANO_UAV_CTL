#include "sbus.h"
#include "stdbool.h"
#include "time.h"
#define SBUS_MAX_CHANNEL 8
#define SBUS_FRAME_SIZE 25
#define SBUS_SYNCBYTE 0x0F

static bool sbusFrameDone = false;


// external vars (ugh)
extern int16_t failsafeCnt;
extern u16 Rc_Pwm_In[8];
static uint32_t sbusChannelData[SBUS_MAX_CHANNEL];


struct sbus_dat {
    unsigned int chan0 : 11;
    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;
    unsigned int chan9 : 11;
    unsigned int chan10 : 11;
    unsigned int chan11 : 11;
} __attribute__ ((__packed__));

typedef union {
    uint8_t in[SBUS_FRAME_SIZE];
    struct sbus_dat msg;
} sbus_msg;

static sbus_msg sbus;

void sbusDataReceive(uint16_t c)
{
    uint32_t sbusTime;
    static uint32_t sbusTimeLast;
    static uint8_t sbusFramePosition;

    sbusTime = GetSysTime_us();
    if ((sbusTime - sbusTimeLast) > 2500) // sbus2 fast timing
        sbusFramePosition = 0;
    sbusTimeLast = sbusTime;

    if (sbusFramePosition == 0 && c != SBUS_SYNCBYTE)
        return;

    sbusFrameDone = false; // lazy main loop didnt fetch the stuff
    if (sbusFramePosition != 0)
        sbus.in[sbusFramePosition - 1] = (uint8_t)c;

    if (sbusFramePosition == SBUS_FRAME_SIZE - 1) {
        sbusFrameDone = true;
        sbusFramePosition = 0;
    } else {
        sbusFramePosition++;
    }
}

bool sbusFrameComplete(void)
{
    if (!sbusFrameDone) {
        return false;
    }
    sbusFrameDone = false;
    if (((sbus.in[22] >> 3) & 0x0001)) {
        // internal failsafe enabled and rx failsafe flag set
        return false;
    }
    //failsafeCnt = 0; // clear FailSafe counter
    Rc_Pwm_In[0] = 0.625f *sbus.msg.chan0+880;
    Rc_Pwm_In[1] = 0.625f *sbus.msg.chan1+880;
    Rc_Pwm_In[2] = 0.625f *sbus.msg.chan2+880;
    Rc_Pwm_In[3] = 0.625f *sbus.msg.chan3+880;
    Rc_Pwm_In[4] = 0.625f *sbus.msg.chan4+880;
    Rc_Pwm_In[5] = 0.625f *sbus.msg.chan5+880;
    Rc_Pwm_In[6] = 0.625f *sbus.msg.chan6+880;
    Rc_Pwm_In[7] = 0.625f *sbus.msg.chan7+880;
		
		Feed_Rc_Dog(1);//RC
    // need more channels? No problem. Add them.
    return true;
}

static uint16_t sbusReadRawRC(uint8_t chan)
{
    // Linear fitting values read from OpenTX-ppmus and comparing with values received by X4R
    // http://www.wolframalpha.com/input/?i=linear+fit+%7B173%2C+988%7D%2C+%7B1812%2C+2012%7D%2C+%7B993%2C+1500%7D
    // No actual Futaba hardware to test with. Sorry.
    return (0.625f * sbusChannelData[0]) + 880;
}
