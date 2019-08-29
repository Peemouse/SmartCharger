/*Library for communicating with Ruideng DPS50xx power supply
Clement Le Priol
July 2019
*/

#include "dpsxxxx_comm.h"
#include "HW_limits.h"
#include <ModbusMaster.h>
#include <Arduino.h>

ModbusMaster node;

DPScomm::DPScomm(void){

}

void DPScomm::DPScommInit(){
	node.begin(1, Serial2); //Modbus node number is 1 by default on DPSxxxx
}

uint16_t DPScomm::checkDPSmodel() {
	uint16_t model = 0;

	uint8_t result = node.readHoldingRegisters(11, 1);//read registers from 0 to 10

	if (result == node.ku8MBSuccess){
		model = node.getResponseBuffer(0);
		return model;
	}
	else{
		return 0;
	}
}

bool DPScomm::readDPSOperatingValues() {

	uint8_t result = node.readHoldingRegisters(0, 10);//read registers from 0 to 10

	if (result == node.ku8MBSuccess){

		DPSvalues.setVoltage = float(node.getResponseBuffer(0)) / 100.0;
		DPSvalues.setAmp = float(node.getResponseBuffer(1)) / 100.0;
		DPSvalues.outVoltage = float(node.getResponseBuffer(2)) / 100.0;
		DPSvalues.outAmp = float(node.getResponseBuffer(3)) / 100.0;
		DPSvalues.power = float(node.getResponseBuffer(4)) / 100.0;
		DPSvalues.inputVoltage = float(node.getResponseBuffer(5)) / 100.0;
		DPSvalues.lock = (node.getResponseBuffer(6)==1) ? true : false;
		DPSvalues.protect = node.getResponseBuffer(7);
		DPSvalues.cccvState = (node.getResponseBuffer(8)==1) ? true : false;
		DPSvalues.onoff = (node.getResponseBuffer(9)==1) ? true : false;
		return true;
	}
	else{
		return false;
	}
}

bool DPScomm::readLCDBrightness(){

	uint8_t result = node.readHoldingRegisters(10, 1);//read registers from 0 to 10

	if (result == node.ku8MBSuccess){
		DPSvalues.brightness = node.getResponseBuffer(0);
		return true;
	}
	else{
		return false;
	}
}

bool DPScomm::setLCDBrightness(uint8_t brigthness){
	if (brigthness >5){
		node.writeSingleRegister(10, 5);
		return true;
	}
	else {
		node.writeSingleRegister(10, uint16_t(brigthness));
		return true;
	}
}


bool DPScomm::setOutVoltage(float voltage){
	if (voltage > MAX_OUTPUT_VOLTAGE){
		return false;
	}
	else if (DPSvalues.inputVoltage < voltage * MIN_OUT_IN_RATIO) {
		return false;
	}
	else {
		uint16_t Vsetpoint = uint16_t(voltage * 100.0);
		node.writeSingleRegister(0, Vsetpoint);
		return true;
	}
}

bool DPScomm::setOutCurrent(float current){
	if (current > MAX_OUTPUT_CURRENT){
		return false;
	}
	else {
		uint16_t Asetpoint = uint16_t(current * 100.0);
		node.writeSingleRegister(1, Asetpoint);
		return true;
	}
}

void DPScomm::lockDPS(){
	node.writeSingleRegister(6, 1);
}

void DPScomm::unlockDPS(){
	node.writeSingleRegister(6, 0);
}

void DPScomm::switchOutputON(){
	node.writeSingleRegister(9, 1);
}

void DPScomm::switchOutputOFF(){
	node.writeSingleRegister(9, 0);
}

float DPScomm::getMaxPower(){
	return MAX_OUTPUT_POWER;
}
