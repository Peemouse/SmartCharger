//HW Limits

#ifndef HW_LIMITS_h
#define HW_LIMITS_h

//Model of DPS (uncomment the one used)

//#define DPS5005
#define DPS5015

//DPS5005

#ifdef DPS5005

#define MAX_OUTPUT_VOLTAGE 50.0
#define MAX_OUTPUT_CURRENT 5.0
#define MAX_OUTPUT_POWER MAX_OUTPUT_VOLTAGE * MAX_OUTPUT_CURRENT
#define MIN_OUT_IN_RATIO 1.1 //Input voltage must x.x time greater than the output voltage

#endif

//DPS5015

#ifdef DPS5015

#define MAX_OUTPUT_VOLTAGE 50.0
#define MAX_OUTPUT_CURRENT 15.0
#define MAX_OUTPUT_POWER MAX_OUTPUT_VOLTAGE * MAX_OUTPUT_CURRENT
#define MIN_OUT_IN_RATIO 1.1 //Input voltage must x.x time greater than the output voltage

#endif

#endif