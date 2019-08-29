/*
SmartCharger Project
Universal CCCV charger
Clément Le Priol
July 2019
v0.1
*/

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
//#include <task.h>

#include "dpsxxxx_comm.h"
#include "chargerControl.h"
#include "parameters.h"
#include "webServerRes.h"

#define INCLUDE_uxTaskGetStackHighWaterMark 1

#define DEBUG //Output some infos on the serial for debugging purpose
//#define IGNORE_FAULTS //USE FOR DEBUGGING ONLY !!! DANGEROUS !

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//Hardware config
#define BUTTON_UP 			12
#define BUTTON_DOWN 		13
#define BUTTON_ENTER 		14

#define OLED_RESET     	27 // Reset pin # (or -1 if sharing Arduino reset pin)

//Working variables
float mahCharged;
float endVoltage;
float setAmp = 5.0;
bool lastStateEnterBtn = false;
bool sleepMode = false;
bool longPressEnterBtnMemo;
uint16_t commLostCounter = 0;

bool btnUp;
bool btnDown;
bool btnEnter;
bool enterBtnLongPress = false;
bool enterBtnShortPress = false;

bool suspendDPSValuesRetrieving = false;
bool DPSValuesRetrievingSuspended = false;
bool flagSetOutVoltage = false;
bool flagSetBrightness = false;
bool flagChangeOutputState = false;

bool rampingInProgress;
float currentNow;
uint8_t blinkMode;
uint8_t cellsNum = 6;
bool blink, hasScreenBlinked;
uint16_t chargeTimeMin, chargeTimeSec;
float stepPerSecond;

//timers
unsigned long mahCountingTimer;
unsigned long chargeTimeMemo;
unsigned long chargeStartTime;
unsigned long rampingTimer;
unsigned long memoBlink;
unsigned long confirmDelay;
unsigned long btnEnterTimer;
unsigned long sleepTimer;
unsigned long timeoutDPSvaluesSuspended;
unsigned long DPSvaluesRefreshTimer;

// Initiate DPScomm class
DPScomm DPScomm;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//WebServer server(80);

void TaskDPScomm( void *pvParameters );
void TaskChargerControl( void *pvParameters );
void TaskBtn( void *pvParameters );

void setup() {

  pinMode(BUTTON_DOWN, INPUT_PULLUP);
	pinMode(BUTTON_ENTER, INPUT_PULLUP);
	pinMode(BUTTON_UP, INPUT_PULLUP);

	Serial.begin(115200); //For debugging
	Serial2.begin(19200);	// need to be changed on the DPS (default is 9600). While DPS is off, hold V button (top button) and switch power on. Use the encoder knob to set the baudrate at 19200).

	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

	display.clearDisplay(); //clear the buffer
	display.display();
	display.setRotation(1); //Use the screen in portrait mode

	if (!digitalRead(BUTTON_UP) && !digitalRead(BUTTON_DOWN)){ //Wifi credentials reset procedure by holding buttons UP and DOWN simultaneously while starting
		display.clearDisplay();
		display.setCursor(0,20);
		display.setTextColor(WHITE);
		display.println("Wifi");
		display.println("reset OK");
		display.display();
		Serial.println("Wifi reset OK");
		delay(2000);
	}

	display.clearDisplay();
	Serial.println("SmartCharger initialization");

	DPScomm.DPScommInit();

  activeChargerState = STANDBY;

	display.drawBitmap(0, 0, initScreen, 64, 128, WHITE); //Draw the init logo
	//TODO animation
	display.setCursor(0,0);
	display.setTextColor(WHITE);
	display.print("v");
	display.print(FIRMWARE_MAJ);
	display.print(".");
	display.print(FIRMWARE_MIN);
	display.print(".");
	display.print(FIRMWARE_PATCH);
	display.display();

	Serial.print("FW ");
	Serial.print(FIRMWARE_MAJ);
	Serial.print(".");
	Serial.print(FIRMWARE_MIN);
	Serial.print(".");
	Serial.println(FIRMWARE_PATCH);

	WiFi.begin(ssid, password);

	DPScomm.lockDPS();
  DPScomm.setLCDBrightness(5);
	display.dim(false);
	delay(1000);

	//Retrieving last used charging values
	typeOfChargeSelected = static_cast<typeOfCharge>(EEPROM.read(EEPROM_ADDR_CHARGETYPE));
	cellsNum = EEPROM.read(EEPROM_ADDR_SNUM);
	EEPROM.get(EEPROM_ADDR_CHARGECURRENT,setAmp);

	//Timers init
	sleepTimer = millis();
	btnEnterTimer = millis();

	display.clearDisplay();

  // Now set up three tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskDPScomm
    ,  "TaskDPScomm"   // A name just for humans
    ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskChargerControl
    ,  "TaskChargerControl"
    ,  4096  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskBtn
    ,  "TaskBtn"
    ,  2048  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

}

void loop (){} //Nothing to write here, everything is managed in tasks

void TaskDPScomm(void *pvParameters) {// Take care of DPS comm as it's slow (but with higher priority)

  (void) pvParameters;
  UBaseType_t uxHighWaterMark;

  while(1){

    if (!suspendDPSValuesRetrieving){
      DPSValuesRetrievingSuspended = false;

      if (millis() - DPSvaluesRefreshTimer > DPS_VALUES_REFRESH_INTERVAL){
        if (!DPScomm.readDPSOperatingValues()){ //No comm with DPS
      		Serial.println("No comm with DPS");
      		commLostCounter++;

  #ifndef IGNORE_FAULTS
    		if (commLostCounter > 3){ //sometimes the comm is lost for example when un/locking manually the DPS UI. This counter helps not to trigger COMM fault in that case.
    			activeFault = NO_COMM;
    		}
  #endif

      	}
      	else{ // comm with DPS is alive
          DPSvaluesRefreshTimer = millis();
          commLostCounter = 0;
      		if(activeFault == NO_COMM){ //Automatically resets the NO_COMM fault if comm is back
      			activeFault = NONE;
      		}

  #ifdef DEBUG
      		debugPrintDPSvalues();
  #endif
      	}
      }

    }
    else{
      DPSValuesRetrievingSuspended = true;
    }

    vTaskDelay(50);

    //uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    // Serial.print("TaskDPScomm ");
    // Serial.println(uxHighWaterMark);
  }

}

void TaskChargerControl (void *pvParameters) {// Take care of DPS comm as it's slow (but with higher priority)

  (void) pvParameters;
  UBaseType_t uxHighWaterMark;

  while(1){

#ifndef IGNORE_FAULTS
		if(DPScomm.DPSvalues.protect>0){
			activeFault = DPS_PROTECT;
		}
#endif

  	//Manual mode force
  	if(activeChargerState != MANUAL_OPERATION && btnDown && btnUp){
  		DPScomm.unlockDPS();
  		switchOutput(false);
  	}

  	if (activeFault != NONE) {
  		switchOutput(false);
  		activeChargerState = FAULT;
  		serialPrintState();
  	}
  	else if (!DPScomm.DPSvalues.lock){
  		activeChargerState = MANUAL_OPERATION;
  		serialPrintState();
  	}
  	else if (activeChargerState == MANUAL_OPERATION){ //User manually locks the DPS : return into Standby mode
  		activeChargerState = STANDBY;
  		serialPrintState();
  	}

  	display.clearDisplay();
  	drawNonMoving();

  	if (blinkMode>0 && (millis() - memoBlink) > 300){
  		blink = !blink;
  		memoBlink = millis();
  	}

  //State management
  	switch (activeChargerState){
  		case STANDBY : {

  			if(DPScomm.DPSvalues.onoff){ //if the charger has been used in manual mode and is still ON, shut off the output
  				switchOutput(false);
  			}

  			//Wakeup the charger if in sleep Mode
  			if( btnUp || btnDown || btnEnter){
  				sleepTimer = millis();
  			}

  			if (typeOfChargeSelected == STORAGE){
  				endVoltage = cellsNum * STORAGE_VOLTAGE;
  			}
  			else if (typeOfChargeSelected == FULL) {
  				endVoltage = cellsNum * END_OF_CHARGE_VOLTAGE;
  			}

  			//UI
  			if (enterBtnShortPress){
  				blinkMode++;
  				if (blinkMode>3){
  					blinkMode = 0;
  				}
          enterBtnShortPress = false;
  			}
#ifdef DEBUG
  			Serial.print("blinkMode ");
  			Serial.println(blinkMode);
#endif

  			switch (blinkMode){
  				case 1: {
  					if (btnUp && cellsNum < 12) {
  						cellsNum++;
  					}
  					else if (btnDown && cellsNum > 1){
  						cellsNum--;
  					}

  					break;
  				}
  				case 2: {
  					if (btnUp && (setAmp * endVoltage)< DPScomm.getMaxPower()) {
  						setAmp+=0.1;
  					}
  					else if (btnDown && setAmp>0.5){
  						setAmp-=0.1;
  					}

  					break;
  				}
  				case 3: {
  					if (btnUp) {
  						typeOfChargeSelected = FULL;
  					}
  					else if (btnDown) {
  						typeOfChargeSelected = STORAGE;
  					}

  					break;
  				}
  				default:
  					break;
  			}

  			//Display
  			display.setCursor(0,0);
  			display.setTextSize(1);
  			display.print("STANDBY");
  			display.setTextSize(0);
  			display.setFont(&FreeSans9pt7b);
  			if(blinkMode!=1 || !blink){
  				display.setCursor(10,25);
  				display.print(cellsNum);
  			}
  			display.setCursor(52,25);
  			display.print("S");
  			if(blinkMode!=2 || !blink){
  				display.setCursor(0,41);
  				display.print(setAmp,1);
  			}
  			display.setCursor(52,41);
  			display.print("A");
  			if(blinkMode!=3 || !blink){
  				display.setCursor(10,64);
  				if (typeOfChargeSelected == STORAGE){
  					display.print("STOR");
  				}
  				else {
  					display.print("FULL");
  				}
  			}

  			if (enterBtnLongPress){ //Switch to confirmation menu
  				activeChargerState = CONFIRMATION;
  				blinkMode = 0;
  				serialPrintState();
  				confirmDelay = millis();
          enterBtnLongPress = false;
  			}

  			break;
  		}
  		case CONFIRMATION : {

  			sleepTimer = millis();

#ifndef IGNORE_FAULTS
  			if (DPScomm.DPSvalues.outVoltage < 2.0){ //No voltage detected means that no battery is connected to the output
  				activeFault = NO_PACK_DETECTED;
  				break;
  			}
#endif

  			display.setCursor(0,0);
  			display.setTextSize(1);
  			display.print("CONFIRM ?");
  			display.setTextSize(0);
  			display.setCursor(0,40);
  			display.setFont(&FreeSans9pt7b);
  			display.print("SURE ?");
  			display.setCursor(0,74);
  			display.setFont();
  			display.setTextSize(1);
  			display.println("Press");
  			display.println("ENTER");
  			display.print("to confirm");

  			if (btnUp || btnDown || (millis() - confirmDelay) > 5000) {
  				activeChargerState = STANDBY;
  				break;
  			}
  			else if (enterBtnShortPress){ //Lauch the charge

  				if (activeFault == NONE){
  					activeChargerState = CHARGING;
  					serialPrintState();
  					if (!chargeVoltageApply(cellsNum,typeOfChargeSelected)){
              activeChargerState = STANDBY;
              break;
            };
  					EEPROM.write(EEPROM_ADDR_CHARGETYPE, typeOfChargeSelected);
  					EEPROM.write(EEPROM_ADDR_SNUM, cellsNum);
  					EEPROM.put(EEPROM_ADDR_CHARGECURRENT, setAmp);
  					rampingInProgress = true;
  					currentNow = STARTING_CURRENT;
  					if (!setDPSOutCurrent(currentNow)){
              activeChargerState = STANDBY;
              break;
            }
  					stepPerSecond = setAmp * RAMP_SCALE;
  					switchOutput(true);
  					rampingTimer = millis();
  					chargeStartTime = millis();
  				}
  				else {
  					activeChargerState = FAULT;
  					serialPrintState();
  				}
          enterBtnShortPress = false;
  			}

  			break;
  		}
  		case CHARGING : {
  			sleepTimer = millis(); //disable sleep mode in this state

#ifndef IGNORE_FAULTS
  			if (DPScomm.DPSvalues.outVoltage / cellsNum < CELL_UNDERVOLTAGE){
  				activeFault = PACK_VOLTAGE_MISMATCH;
  				break;
  			}
#endif
  			//current ramping
  			if (rampingInProgress && (millis()-rampingTimer) > 500){
  				currentNow += (float(millis()-rampingTimer))/1000.0*stepPerSecond;
  				if(currentNow >= setAmp){
  					currentNow = setAmp;
  					rampingInProgress = false;
  				}
  				setDPSOutCurrent(currentNow);
  				rampingTimer = millis();
  			}

  			//mAh counting
  			if (millis() - mahCountingTimer > 1000){
  				mahCharged += DPScomm.DPSvalues.outAmp * (millis()-mahCountingTimer)/3600;
  				mahCountingTimer = millis();
  			}
  			//Time counting
  			unsigned long chargeTime = (millis() - chargeStartTime) / 1000; //time in seconds
  			chargeTimeMin = uint16_t(chargeTime / 60);
  			chargeTimeSec = uint8_t(chargeTime % 60);

  			//Display
  			display.setTextSize(1);
  			display.setCursor(0,0);
  			display.print("CHARGING");
  			display.setCursor(45,90);
  			display.print("mAh");
  			display.setCursor(45,120);
  			display.print("min");

  			display.setTextSize(0);
  			display.setCursor(10,20);
  			display.setFont(&FreeSans9pt7b);
  			display.print(cellsNum);

  			display.setCursor(52,25);
  			display.print("S");

  			display.setCursor(0,41);
  			display.print(setAmp,1);

  			display.setCursor(52,41);
  			display.print("A");

  			display.setCursor(5,58);
  			if (typeOfChargeSelected == FULL){
  				display.print("FULL");
  			}
  			else {
  				display.print("STOR");
  			}

  			display.drawLine(14, 65, 54, 65, WHITE);
  			display.setCursor(0,88);
  			display.print(mahCharged,0);
  			display.setCursor(0,114);
  			displayTimeWithLeadingZero(chargeTimeMin);
  			display.print(":");
  			displayTimeWithLeadingZero(chargeTimeSec);

  			//Detecting end of charge conditions
  			if (!rampingInProgress && DPScomm.DPSvalues.outAmp < max((DPScomm.DPSvalues.setAmp * END_OF_CHARGE_CURRENT_RATIO), END_OF_CHARGE_MIN_CURRENT) || btnEnter){
  				switchOutput(false);
  				if (btnEnter){
  					activeChargerState = STANDBY;
  				}
  				else {
  					activeChargerState = CHARGE_COMPLETE;
  				}
  				serialPrintState();
  				//TODO buzzer alert
  			}

  			break;
  		}
  		case CHARGE_COMPLETE : {
  			sleepTimer = millis(); //disable sleep mode in this state

  			blinkEntireScreen();

  			//Display
  			display.setCursor(0,0);
  			display.setTextSize(1);
  			display.print("CHARGE END");
  			display.setCursor(45,35);
  			display.print("mAh");
  			display.setCursor(0,50);
  			display.print("CHARGED IN");
  			display.setCursor(45,90);
  			display.print("min");

  			display.setTextSize(0);
  			display.setFont(&FreeSans9pt7b);
  			display.setCursor(0,30);
  			display.print(mahCharged,0);
  			display.setCursor(0,80);
  			displayTimeWithLeadingZero(chargeTimeMin);
  			display.print(":");
  			displayTimeWithLeadingZero(chargeTimeSec);

  			if (enterBtnShortPress) {//acknowledgment of charge complete
  				activeChargerState = STANDBY;
  				serialPrintState();
  				mahCharged = 0.0;
  				hasScreenBlinked = false;
          enterBtnShortPress = false;
  			}

  			break;
  		}

  		case MANUAL_OPERATION : {
  			sleepTimer = millis(); //disable sleep mode in this state

  			if (DPScomm.DPSvalues.lock == true){
  				activeChargerState = STANDBY;
  				serialPrintState();
  				hasScreenBlinked = false;
  			}

  			//Display

  			display.setCursor(0,0);
  			display.setTextSize(1);
  			display.print("MANUAL");

  			display.setCursor(0,70);
  			display.println("Lock the");
  			display.println("DPS for");
  			display.println("using it");
  			display.println("as battery");
  			display.print("charger.");

  			blinkEntireScreen();

        //TODO buzzer
  			break;
  		}

  		case FAULT : {
  			sleepTimer = millis(); //disable sleep mode in this state

  			//Display
  			display.setCursor(0,0);
  			display.setTextSize(1);
  			display.print("FAULT");
  			display.setCursor(0,18);
  			display.print(faultToString(activeFault));

  			display.setCursor(0,50);

  			switch(activeFault){
  				case (OUTPUT_VOLTAGE_TOO_LOW):
  					display.print("The output");
  					display.println("voltage is");
  					display.println("too low.");
  					display.println("Check");
  					display.println("connection");
  				break;
  				case (NO_PACK_DETECTED):
  					display.println("Check");
  					display.println("battery");
  					display.println("connection");
  				break;
  				case (PACK_ALREADY_CHARGED):
  					display.println("The battery");
  					display.println("pack");
  					display.println("voltage");
  					display.println("is too high");
  				break;
  				case (PACK_VOLTAGE_MISMATCH):
  					display.println("The average");
  					display.println("cell");
  					display.println("voltage");
  					display.println("is too low");
  				break;
  				case (DPS_PROTECT):
  					display.println("The DPS");
  					display.println("trigerred");
  					display.println("protection");
  					display.println("state.");
  				break;
  				case (NO_COMM):
  					display.println("Internal");
  					display.println("comm. with");
  					display.println("DPS is not");
  					display.print("working.");
  				default:
  				break;
  			}

  			blinkEntireScreen();

  			if (enterBtnShortPress){
  				activeFault = NONE;
  				activeChargerState = STANDBY;
  				serialPrintState();
  				hasScreenBlinked = false;
          enterBtnShortPress = false;
  			}
  			break;
  		}

  		default:
  			break;
  	}

  	display.display();

  	//Sleep mode management
  	if(!sleepMode && (millis() - sleepTimer) > SLEEP_TIMER){
  		sleepMode = true;
  		setDPSLCDBrightness(0);
  		display.dim(true);
  	}
  	else if(sleepMode && (millis() - sleepTimer) < SLEEP_TIMER){
  		sleepMode = false;
  		setDPSLCDBrightness(5);
  		display.dim(false);
  	}

  	vTaskDelay(80);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    // Serial.print("TaskChargerControl ");
    // Serial.println(uxHighWaterMark);
  }
}

void TaskBtn(void *pvParameters) {  // Task for the rest

  UBaseType_t uxHighWaterMark;

  while(1) {
  	//Buttons management
  	btnUp = !digitalRead(BUTTON_UP);
  	btnDown = !digitalRead(BUTTON_DOWN);
  	btnEnter = !digitalRead(BUTTON_ENTER);

#ifdef DEBUG
  	// Serial.print("Btn ");
  	// Serial.print(btnDown);
  	// Serial.print(btnEnter);
  	// Serial.println(btnUp);
#endif

    //Enter button management
    if (lastStateEnterBtn != btnEnter){ //Button state change
      if (btnEnter) { //Rising Edge
        btnEnterTimer = millis();
      }
      else { //Falling edge
        if (!longPressEnterBtnMemo && (millis() - btnEnterTimer) < BTN_ENTER_LONGPUSH_TIMER){
          enterBtnShortPress = true;
        }
        longPressEnterBtnMemo = false;
      }
    }

    if (btnEnter && (millis() - btnEnterTimer) >= BTN_ENTER_LONGPUSH_TIMER){
      enterBtnLongPress = true;
      longPressEnterBtnMemo = true;
      btnEnterTimer = millis();
    }

    lastStateEnterBtn = btnEnter; //Save the state for next cycle

#ifdef DEBUG
    // Serial.print("Entr btn ");
    // Serial.print(enterBtnShortPress);
    // Serial.println(enterBtnLongPress);
#endif

    vTaskDelay(10);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    // Serial.print("TaskBtn ");
    // Serial.println(uxHighWaterMark);

  }
}

bool chargeVoltageApply(uint8_t Snum, typeOfCharge storage){

#ifndef IGNORE_FAULTS
	if (DPScomm.DPSvalues.outVoltage >= endVoltage){ // in case of the battery voltage is greater than the output voltage setting
		activeFault = PACK_ALREADY_CHARGED;
		return false;
	}
	else if (DPScomm.DPSvalues.outVoltage < 2.0){ //No voltage detected means that no battery is connected to the output
		activeFault = NO_PACK_DETECTED;
		return false;
	}
#endif

  suspendDPSValuesRetrieving = true;
  timeoutDPSvaluesSuspended = millis();
#ifdef DEBUG
  Serial.println("Suspend retr. DPS values");
#endif
  while(!DPSValuesRetrievingSuspended){
    if (millis() - timeoutDPSvaluesSuspended > TIMEOUT_DPS_VALUES_SUSPENDED){
      suspendDPSValuesRetrieving = false;
      Serial.println("Susp. timeout");
      return false;
      break;
    }
  }

  DPScomm.setOutVoltage(endVoltage);
  suspendDPSValuesRetrieving = false;
  return true;
}

bool setDPSOutCurrent(float current){

  suspendDPSValuesRetrieving = true;
  timeoutDPSvaluesSuspended = millis();
#ifdef DEBUG
  Serial.println("Suspend retr. DPS values");
#endif
  while(!DPSValuesRetrievingSuspended){
    if (millis() - timeoutDPSvaluesSuspended > TIMEOUT_DPS_VALUES_SUSPENDED){
      suspendDPSValuesRetrieving = false;
      Serial.print("Susp. timeout");
      return false;
    }
  }
  DPScomm.setOutCurrent(current);
  suspendDPSValuesRetrieving = false;
  return true;
}

void drawNonMoving(){
  display.setFont();
  display.drawLine(0, 9, 68, 9, WHITE);
}

char* faultToString(faults Fault) {
	switch (activeFault) {
		case NONE :
			return "NONE";
			break;

		case OUTPUT_VOLTAGE_TOO_LOW :
			return "OUTPUT_VOLTAGE_TOO_LOW";
			break;

		case NO_PACK_DETECTED:
			return "NO_PACK_DETECTED";
			break;

		case NO_COMM:
			return "NO_COMM";
			break;

		default :
			return "UNKNOWN_FAULT";
			break;
	}
}

bool wifiClientCheck(){
	/*
	if (WiFi.status() == WL_CONNECTED) {
		display.drawBitmap(0, 0, wifiLogo, 8, 8, WHITE); //Draw the Wifi logo in top right corner
		Serial.println("");
		Serial.print("Connected to ");
		Serial.println(ssid);
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());


		if (!MDNS.begin(host)) { //http://esp32.local
			Serial.println("Error setting up MDNS responder!");
			while (1) {
			  vTaskDelay(1000);
			}
		}
		Serial.println("mDNS responder started");
		//return index page
		server.on("/", HTTP_GET, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/html", webMainPage);
		});
		server.on("/updatePage", HTTP_GET, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/html", webUpdatePage);
		});
		//handling uploading firmware file
		server.on("/update", HTTP_POST, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
			ESP.restart();
		}, []() {
			HTTPUpload& upload = server.upload();
			if (upload.status == UPLOAD_FILE_START) {
			  Serial.printf("Update: %s\n", upload.filename.c_str());
			  if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
				Update.printError(Serial);
			  }
			} else if (upload.status == UPLOAD_FILE_WRITE) {
			  // flashing firmware to ESP
			  if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
				Update.printError(Serial);
			  }
			} else if (upload.status == UPLOAD_FILE_END) {
			  if (Update.end(true)) { //true to set the size to the current progress
				Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
			  } else {
				Update.printError(Serial);
			  }
			}
		});
		server.begin();
		server.handleClient();
		return true;
	}
	else{
		server.handleClient();
		return false;
	}
	*/
}

void serialPrintState(){
	Serial.print("->");
	switch(activeChargerState){
		case(STANDBY):
			Serial.println("STANDBY");
		break;
		case(CONFIRMATION):
			Serial.println("CONFIRMATION");
		break;
		case(CHARGING):
			Serial.println("CHARGING");
		break;
		case(CHARGE_COMPLETE):
			Serial.println("CHARGE_COMPLETE");
		break;
		case(MANUAL_OPERATION):
			Serial.println("MANUAL_OPERATION");
		break;
		case(FAULT):
			Serial.print("FAULT ");
			Serial.println(faultToString(activeFault));
		break;
		default:
			Serial.println("UNKNOWN");
		break;
	}
}

void blinkEntireScreen(){
	static uint8_t blinkCnt;

	if (!hasScreenBlinked){
		display.invertDisplay(blinkCnt & 1);
		blinkCnt++;
		if (blinkCnt>6){
			hasScreenBlinked = true;
			blinkCnt = 0;
		}
	}

}

void debugPrintDPSvalues(){
	String spacer = " ";
	Serial.print("DPSvalues");
	Serial.print(spacer);
	Serial.print(DPScomm.DPSvalues.setVoltage);
	Serial.print(spacer);
	Serial.print(DPScomm.DPSvalues.setAmp);
	Serial.print(spacer);
	Serial.print(DPScomm.DPSvalues.outVoltage);
	Serial.print(spacer);
	Serial.print(DPScomm.DPSvalues.outAmp);
	Serial.print(spacer);
	Serial.print(DPScomm.DPSvalues.power);
	Serial.print(spacer);
	Serial.print(DPScomm.DPSvalues.inputVoltage);
	Serial.print(spacer);
	Serial.print(DPScomm.DPSvalues.lock);
	Serial.print(spacer);
	Serial.print(DPScomm.DPSvalues.protect);
	Serial.print(spacer);
	Serial.print(DPScomm.DPSvalues.cccvState);
	Serial.print(spacer);
	Serial.println(DPScomm.DPSvalues.onoff);
}

void displayTimeWithLeadingZero(uint16_t time){
	if (time < 10){
		display.print(0);
	}
	display.print(time);
}

void switchOutput(bool state) {

   suspendDPSValuesRetrieving = true;
   timeoutDPSvaluesSuspended = millis();
#ifdef DEBUG
  Serial.println("Suspend retr. DPS values");
#endif
   while(!DPSValuesRetrievingSuspended){
     if (millis() - timeoutDPSvaluesSuspended > TIMEOUT_DPS_VALUES_SUSPENDED){
       suspendDPSValuesRetrieving = false;
       Serial.print("Susp. timeout");
       break;
     }
   }
   if (state){
     DPScomm.switchOutputON();
   }
   else {
     DPScomm.switchOutputOFF();
   }
   suspendDPSValuesRetrieving = false;
 }

void setDPSLCDBrightness(uint8_t value) {
   suspendDPSValuesRetrieving = true;
   timeoutDPSvaluesSuspended = millis();
#ifdef DEBUG
 Serial.println("Suspend retr. DPS values");
#endif
   while(!DPSValuesRetrievingSuspended){
     if (millis() - timeoutDPSvaluesSuspended > TIMEOUT_DPS_VALUES_SUSPENDED){
       suspendDPSValuesRetrieving = false;
       Serial.print("Susp. timeout");
       break;
     }
   }
   DPScomm.setLCDBrightness(value);
   suspendDPSValuesRetrieving = false;

}
