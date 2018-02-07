#include "stdint.h"
#include "stdbool.h"
void sbusDataReceive(uint16_t c);
uint16_t sbusReadRawRC(uint8_t chan);
bool sbusFrameComplete(void);