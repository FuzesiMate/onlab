/*
 * SerialComm.h
 *
 *  Created on: Sep 12, 2016
 *      Author: teqbox
 */

#ifndef SERIALCOMM_H_
#define SERIALCOMM_H_

#include <boost/asio/io_service.hpp>
#include <boost/asio/serial_port.hpp>

class SerialComm {
private:
	boost::asio::io_service ioService;
	boost::asio::serial_port serialPort;

public:
	SerialComm():serialPort(ioService , "/dev/ttyUSB0"){
		serialPort.set_option(boost::asio::serial_port::baud_rate(115200));
	};

	bool IsConnected(){
		return serialPort.is_open();
	}

	void writeData(const char* data , int length);

	virtual ~SerialComm();
};

#endif /* SERIALCOMM_H_ */
