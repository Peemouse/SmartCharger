#ifndef CHARGERCONTROL_h
#define CHARGERCONTROL_h

#define FIRMWARE_MAJ					0
#define FIRMWARE_MIN					1
#define FIRMWARE_PATCH					0

#define EEPROM_ADDR_CHARGETYPE 			0x00
#define EEPROM_ADDR_SNUM 				     0x01
#define EEPROM_ADDR_CHARGECURRENT		0x02

#define TIMEOUT_DPS_VALUES_SUSPENDED    5000
#define DPS_VALUES_REFRESH_INTERVAL     500

// Init Screen width: 64 height: 128
static const unsigned char PROGMEM initScreen[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x01, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x01, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x01, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0xff, 0xff, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0xff, 0xff, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x1f, 0xff, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0x81, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xc7, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xff, 0x8f, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xfe, 0x03, 0xc0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x10, 0x02,
  0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x10, 0x02,
  0x40, 0x20, 0x0b, 0x9c, 0x1c, 0x14, 0x3c, 0x02,
  0x40, 0x10, 0x0c, 0x62, 0x22, 0x18, 0x10, 0x02,
  0x40, 0x0c, 0x08, 0x42, 0x02, 0x10, 0x10, 0x02,
  0x40, 0x02, 0x08, 0x42, 0x0e, 0x10, 0x10, 0x02,
  0x40, 0x01, 0x08, 0x42, 0x32, 0x10, 0x10, 0x02,
  0x40, 0x01, 0x08, 0x42, 0x22, 0x10, 0x10, 0x02,
  0x40, 0x22, 0x08, 0x42, 0x26, 0x10, 0x10, 0x02,
  0x40, 0x1c, 0x08, 0x42, 0x1a, 0x10, 0x0c, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x43, 0xc5, 0xe0, 0xe0, 0xa0, 0xe8, 0x18, 0x2a,
  0x44, 0x06, 0x11, 0x10, 0xc1, 0x18, 0x24, 0x32,
  0x48, 0x04, 0x10, 0x10, 0x82, 0x08, 0x42, 0x22,
  0x48, 0x04, 0x10, 0x70, 0x82, 0x08, 0x7e, 0x22,
  0x48, 0x04, 0x11, 0x90, 0x82, 0x08, 0x40, 0x22,
  0x48, 0x04, 0x11, 0x10, 0x82, 0x08, 0x40, 0x22,
  0x44, 0x04, 0x11, 0x30, 0x81, 0x18, 0x20, 0x22,
  0x43, 0xc4, 0x10, 0xd0, 0x80, 0xe8, 0x1e, 0x22,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00,
  0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
  0x00, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x07, 0x80,
  0x00, 0x3e, 0x00, 0x00, 0x7f, 0xff, 0xe3, 0xc0,
  0x00, 0x3c, 0x00, 0x00, 0xff, 0xff, 0xf3, 0xc0,
  0x00, 0x3c, 0x00, 0x00, 0xff, 0xff, 0xf9, 0xc0,
  0x00, 0xf8, 0x00, 0x01, 0xff, 0xff, 0xfd, 0xc0,
  0x01, 0xf8, 0x00, 0x01, 0xff, 0xff, 0xfd, 0xc0,
  0x01, 0xf0, 0x00, 0x03, 0xff, 0xff, 0xfd, 0xc0,
  0x01, 0xf0, 0x00, 0x07, 0xff, 0xff, 0xfd, 0xc0,
  0x01, 0xf0, 0x00, 0x07, 0xff, 0xff, 0xfd, 0xc0,
  0x01, 0xf0, 0x00, 0x0f, 0xff, 0xff, 0xfd, 0xc0,
  0x01, 0xf0, 0x00, 0x0f, 0xff, 0xff, 0xfd, 0xc0,
  0x01, 0xf0, 0x00, 0x1f, 0xff, 0xff, 0xfd, 0xc0,
  0x00, 0xf8, 0x00, 0x1f, 0xff, 0xff, 0xf9, 0xc0,
  0x00, 0x38, 0x00, 0x3f, 0xff, 0xff, 0xfb, 0xc0,
  0x00, 0x38, 0x00, 0x3f, 0xff, 0xff, 0xf3, 0xc0,
  0x00, 0x3c, 0x00, 0x7f, 0xff, 0xff, 0xc7, 0xc0,
  0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80,
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
  0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
  0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};



//Wifi logo 8x8
static const unsigned char PROGMEM wifiLogo[] =
{	B00111100,
  B01000010,
  B10011001,
  B00100100,
  B01000010,
  B00011000,
  B00011000,
  B00000000 };

//Wifi credentials
const char* host = "smartcharger";
const char* ssid = "Prrrtttt";
const char* password = "bakalegum";

typedef enum{
	NONE,
	OUTPUT_VOLTAGE_TOO_LOW,
	NO_PACK_DETECTED,
  PACK_ALREADY_CHARGED,
  PACK_VOLTAGE_MISMATCH,
  DPS_PROTECT,
  NO_COMM
}faults;

typedef enum {
	STANDBY,
	CONFIRMATION,
	CHARGING,
	CHARGE_COMPLETE,
	MANUAL_OPERATION,
	FAULT
}chargerState;

typedef enum {
	STORAGE,
	FULL
} typeOfCharge;

faults activeFault;
chargerState activeChargerState;
typeOfCharge typeOfChargeSelected;

//Prototypes

bool chargeVoltageApply(uint8_t Snum, typeOfCharge storage);
bool setDPSOutCurrent(float current);
void drawNonMoving(void);
char* faultToString(faults Fault);
bool wifiClientCheck(void);
void serialPrintState(void);
void switchOff(void);
void debugPrintDPSvalues(void);
void interruptBtnEnterChange(void);
void interruptBtnDown(void);
void interruptBtnUp(void);
void blinkEntireScreen(void);
void displayTimeWithLeadingZero(uint16_t time);
void switchOutput(bool state);
void setDPSLCDBrightness(uint8_t value);

#endif
