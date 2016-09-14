/*
 * SerialComm.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: teqbox
 */

#include "SerialComm.h"


void SerialComm::writeData(const char* data , int length){
	serialPort.write_some(boost::asio::buffer(data, length));
}

SerialComm::~SerialComm() {
	serialPort.close();
}

