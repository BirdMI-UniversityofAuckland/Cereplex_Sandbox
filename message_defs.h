#include "cbhwlib.h"


#define MID_PRODUCER 10
#define MID_CONSUMER 11

#define MT_REQUEST_TEST_DATA 101
#define MT_TEST_DATA 102

#define MT_SPIKE 150
#define MT_SPIKE_WAVE_FORM 160

typedef struct 
{ 
    int a; 
    int b; 

} MDF_TEST_DATA;

typedef struct 
{
	UINT32 time;
	UINT16 chid;	
}MDF_SPIKE;

typedef struct
{
	UINT32 time;
	UINT16 chid;
	INT16  wave[cbMAX_PNTS];
}MDF_SPIKE_WAVE_FORM;