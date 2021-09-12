/*
 * Copyright (c) 2001 - 2005 Bernd Walter Computer Technology
 * http://www.bwct.de
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

// TODO: add interleaving support
// TODO: properly handle broadcast packets and don't parse results

#include <bwctmb/bwctmb.h>

#define READ_COILS                              0x01
#define READ_DISCRETE_INPUTS                    0x02
#define READ_HOLDING_REGISTERS                  0x03
#define READ_INPUT_REGISTERS                    0x04
#define WRITE_SINGLE_COIL                       0x05
#define WRITE_SINGLE_REGISTER                   0x06
#define READ_EXCEPTION_STATUS                   0x07
#define DIAGNOSTICS                             0x08
#define GET_COMM_EVENT_COUNTER                  0x0b
#define GET_COMM_EVENT_LOG                      0x0c
#define WRITE_MULTIPLE_COILS                    0x0f
#define WRITE_MULTIPLE_REGISTERS                0x10
#define REPORT_SLAVE_ID                         0x11
#define READ_FILE_RECORD                        0x14
#define WRITE_FILE_RECORD                       0x15
#define MASK_WRITE_REGISTER                     0x16
#define READ_WRITE_MULTIPLE_REGISTERS           0x17
#define READ_FIFO_QUEUE                         0x18
#define ENCAPSULATED_INTERFACE_TRANSPORT        0x2b
# define READ_DEVICE_IDENTIFICATION             0x0d
#define VENDOR_SET_ADDRESS                      0x41
#define VENDOR_READ_MAGCARD                     0x42
#define VENDOR_PAGEDATA                         0x43
#define VENDOR_WRITE_PAGE                       0x44
#define VENDOR_RESET                            0x45
#define VENDOR_BOOTLOADER                       0x49

void
Modbus::do_packet() {
	uint8_t header[6];
	uint8_t spacket[300];
	ssize_t res;
	int ntry;

	sequence++;

	ntry = 0;
	header[0] = sequence >> 8; // transaction identifier
	header[1] = sequence & 0xff;
	header[2] = 0x00; // protocol identifier
	header[3] = 0x00;
	header[4] = 0x00;
	header[5] = packetlen;
	if (!bus.isinit() || !bus->opened()) {
		reconnect();
		ntry = 1;
	}

	bcopy(header, &spacket[0], sizeof(header));
	bcopy(packet, &spacket[sizeof(header)], packetlen);

retry:
	try {
		res = bus->write(spacket, sizeof(header) + packetlen);
		if (res != (ssize_t)sizeof(header) + packetlen)
			goto failure;

		res = bus->read(header, sizeof(header));
		if (res != sizeof(header))
			goto failure;
		packetlen = header[5];
		if (packetlen == 0)
			throw ::Error(String("Invalid response"));
		res = bus->read(packet, packetlen);
		if (res != packetlen)
			goto failure;
		if (header[0] != (sequence >> 8) ||
		    header[1] != (sequence & 0xff)) {
			// we are off sequence and should reconnect
			goto failure;
		}
	} catch (...) {
		goto failure;
	}
	if (packet[1] & 0x80) {
		switch (packet[2]) {
		case 0x01:
			throw Error(String("modbus: illegal function"), packet[2]);
		case 0x02:
			throw Error(String("modbus: illegal data address"), packet[2]);
		case 0x03:
			throw Error(String("modbus: illegal data value"), packet[2]);
		case 0x04:
			throw Error(String("modbus: slave device failure"), packet[2]);
		case 0x05:
			throw Error(String("modbus: acknowledge"), packet[2]);
		case 0x06:
			throw Error(String("modbus: slave device busy"), packet[2]);
		case 0x08:
			throw Error(String("modbus: memory parity error"), packet[2]);
		case 0x0a:
			throw Error(String("modbus: gateway path unavailable"), packet[2]);
		case 0x0b:
			throw Error(String("modbus: gateway target device failed to respond"), packet[2]);
		default:
			throw Error(String("modbus: unknown error ") + packet[2], packet[2]);
		}
	}
	return;

failure:
	if (ntry < retries) {
		usleep(100);
		ntry++;
		reconnect();
		goto retry;
	}
	throw ::Error(String("TCP connection failure"));
}

void
Modbus::reconnect() {
	bus = new Network::Net();
	bus->connect_tcp(IP, Port);
	bus->settimeout(1000);
	bus->nodelay(1);
}

bool
Modbus::read_discrete_input(uint8_t address, uint16_t num) {
	bool ret;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = READ_DISCRETE_INPUTS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = 0x00;
	packet[5] = 0x01;
	packetlen = 6;
	do_packet();
	if (packetlen < 4) {
		throw ::Error(String("wrong response size"));
	}
	ret = packet[3];
	mutex.unlock();

	return ret;
}

SArray<bool>
Modbus::read_discrete_inputs(uint8_t address, uint16_t num, uint16_t count) {
	SArray<bool> ret;
	int i;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = READ_DISCRETE_INPUTS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = count >> 8;
	packet[5] = count & 0xff;
	packetlen = 6;
	do_packet();
	if (packetlen < (4 + (count - 1) / 8)) {
		throw ::Error(String("wrong response size"));
	}
	for (i = 0; i < count; i++) {
		if (packet[3 + i / 8] & (1 << (i & 0x07)))
			ret[i] = 1;
		else
			ret[i] = 0;
	}
	mutex.unlock();

	return ret;
}

bool
Modbus::read_coil(uint8_t address, uint16_t num) {
	bool ret;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = READ_COILS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = 0x00;
	packet[5] = 0x01;
	packetlen = 6;
	do_packet();
	if (packetlen < 4) {
		throw ::Error(String("wrong response size"));
	}
	ret = packet[3];
	mutex.unlock();

	return ret;
}

SArray<bool>
Modbus::read_coils(uint8_t address, uint16_t num, uint16_t count) {
	SArray<bool> ret;
	int i;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = READ_COILS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = count >> 8;
	packet[5] = count & 0xff;
	packetlen = 6;
	do_packet();
	if (packetlen < (4 + (count - 1) / 8)) {
		throw ::Error(String("wrong response size"));
	}
	for (i = 0; i < count; i++) {
		if (packet[3 + i / 8] & (1 << (i & 0x07)))
			ret[i] = 1;
		else
			ret[i] = 0;
	}
	mutex.unlock();

	return ret;
}

void
Modbus::write_coil(uint8_t address, uint16_t num, bool val) {
	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = WRITE_SINGLE_COIL;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = (val) ? 0xff : 0x00;
	packet[5] = 0x00;
	packetlen = 6;
	do_packet();
	if (packetlen < 6) {
		throw ::Error(String("wrong response size"));
	}
	mutex.unlock();
}

void
Modbus::write_coils(uint8_t address, uint16_t num, SArray<bool> val) {
	uint16_t i;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = WRITE_MULTIPLE_COILS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = (val.max + 1) >> 8;
	packet[5] = (val.max + 1) & 0xff;
	packet[6] = (val.max + 8) / 8;
	for (i = 0; i <= val.max; i++) {
		if ((i & 0x7) == 0)
			packet[7 + i / 8] = 0;
		if (val[i])
			packet[7 + i / 8] |= 1 << (i & 0x7);
	}
	packetlen = 7 + (val.max + 8) / 8;
	do_packet();
	if (packetlen < 6) {
		throw ::Error(String("wrong response size"));
	}
	mutex.unlock();
}

uint16_t
Modbus::read_input_register(uint8_t address, uint16_t num) {
	uint16_t ret;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = READ_INPUT_REGISTERS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = 0x00;
	packet[5] = 0x01;
	packetlen = 6;
	do_packet();
	if (packetlen < 5) {
		throw ::Error(String("wrong response size"));
	}
	ret = (uint16_t)packet[3] << 8 | packet[4];
	mutex.unlock();

	return ret;
}

SArray<uint16_t>
Modbus::read_input_registers(uint8_t address, uint16_t num, uint16_t count) {
	SArray<uint16_t> ret;
	int i;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = READ_INPUT_REGISTERS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = count >> 8;
	packet[5] = count & 0xff;
	packetlen = 6;
	do_packet();
	if (packetlen < (3 + count * 2)) {
		throw ::Error(String("wrong response size"));
	}
	for (i = 0; i < count; i++) {
		ret[i] = (uint16_t)packet[3 + 2 * i] << 8 | packet[4 + 2 * i];
	}
	mutex.unlock();

	return ret;
}

uint16_t
Modbus::read_holding_register(uint8_t address, uint16_t num) {
	uint16_t ret;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = READ_HOLDING_REGISTERS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = 0x00;
	packet[5] = 0x01;
	packetlen = 6;
	do_packet();
	if (packetlen < 5) {
		throw ::Error(String("wrong response size"));
	}
	ret = (uint16_t)packet[3] << 8 | packet[4];
	mutex.unlock();

	return ret;
}

SArray<uint16_t>
Modbus::read_holding_registers(uint8_t address, uint16_t num, uint16_t count) {
	SArray<uint16_t> ret;
	int i;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = READ_HOLDING_REGISTERS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = count >> 8;
	packet[5] = count & 0xff;
	packetlen = 6;
	do_packet();
	if (packetlen < (3 + count * 2)) {
		throw ::Error(String("wrong response size"));
	}
	for (i = 0; i < count; i++) {
		ret[i] = (uint16_t)packet[3 + 2 * i] << 8 | packet[4 + 2 * i];
	}
	mutex.unlock();

	return ret;
}

void
Modbus::write_register(uint8_t address, uint16_t num, uint16_t val) {
	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = WRITE_SINGLE_REGISTER;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = val >> 8;
	packet[5] = val & 0xff;
	packetlen = 6;
	do_packet();
	if (packetlen < 6) {
		throw ::Error(String("wrong response size"));
	}
	mutex.unlock();
}

void
Modbus::write_registers(uint8_t address, uint16_t num, SArray<uint16_t> val) {
	int i;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = WRITE_MULTIPLE_REGISTERS;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = (val.max + 1) >> 8;
	packet[5] = (val.max + 1) & 0xff;
	packet[6] = (val.max + 1) * 2;
	for (i = 0; i <= val.max; i++) {
		packet[7 + 2 * i] = val[i] >> 8;
		packet[8 + 2 * i] = val[i] & 0xff;
	}
	packetlen = 7 + 2 * (val.max + 1);
	do_packet();
	if (packetlen < 6) {
		throw ::Error(String("wrong response size"));
	}
	mutex.unlock();
}

SArray<uint16_t>
Modbus::read_write_registers(uint8_t address, uint16_t rnum, uint16_t count, uint16_t wnum, SArray<uint16_t> val) {
	SArray<uint16_t> ret;
	int i;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = READ_WRITE_MULTIPLE_REGISTERS;
	packet[2] = rnum >> 8;
	packet[3] = rnum & 0xff;
	packet[4] = count >> 8;
	packet[5] = count & 0xff;
	packet[6] = wnum >> 8;
	packet[7] = wnum & 0xff;
	packet[8] = val.max >> 8;
	packet[9] = val.max & 0xff;
	for (i = 0; i <= val.max; i++) {
		packet[10 + 2 * i] = val[i] >> 8;
		packet[11 + 2 * i] = val[i] & 0xff;
	}
	packetlen = 12 + 2 * val.max;
	do_packet();
	if (packetlen < (3 + count * 2)) {
		throw ::Error(String("wrong response size"));
	}
	for (i = 0; i < count; i++) {
		ret[i] = (uint16_t)packet[3 + 2 * i] << 8 | packet[4 + 2 * i];
	}
	mutex.unlock();

	return ret;
	// TODO: fall back to split read/write
}

String
Modbus::identification(uint8_t address, uint8_t num) {
	String ret;
	int i;
	char tmpstr[2];

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = ENCAPSULATED_INTERFACE_TRANSPORT;
	packet[2] = READ_DEVICE_IDENTIFICATION;
	packet[3] = 4;
	packet[4] = num;
	packetlen = 5;
	do_packet();
	tmpstr[1] = 0;
	for (i = 0; i < packet[9]; i++) {
		tmpstr[0] = packet[10 + i];
		ret += tmpstr;
	}
	mutex.unlock();

	return ret;
}

void
Modbus::mask_write_register(uint8_t address, uint16_t num, uint16_t andval, uint16_t orval) {
	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = MASK_WRITE_REGISTER;
	packet[2] = num >> 8;
	packet[3] = num & 0xff;
	packet[4] = andval >> 8;
	packet[5] = andval & 0xff;
	packet[6] = orval >> 8;
	packet[7] = orval & 0xff;
	packetlen = 8;
	do_packet();
	if (packetlen < 8) {
		throw ::Error(String("wrong response size"));
	}
	// TODO: fall back to read-modify-write
	mutex.unlock();
}

void
Modbus::bwct_set_address(uint8_t address, uint8_t naddress, String serial) {
	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = VENDOR_SET_ADDRESS;
	packet[2] = naddress;
	if (serial.length() < 10)
		throw ::Error(String("Serial has wrong length"));
	bcopy(serial.c_str(), &packet[3], 10);
	packetlen = 13;
	do_packet();
	mutex.unlock();
}

String
Modbus::bwct_read_magcard(uint8_t address) {
	String ret;
	int i;
	uint16_t numbits;

	Mutex::Guard mutex(mtx_bus);
	packet[0] = address;
	packet[1] = VENDOR_READ_MAGCARD;
	packetlen = 2;
	do_packet();
	numbits = packet[2] << 8 | packet[3];
	for (i = 0; i < numbits; i++) {
		if (packet[4 + (i / 8)] &
		    (0x80 >> (i & 7))) {
			ret += "0";
		} else {
			ret += "1";
		}
	}
	mutex.unlock();

	return ret;
}

void
Modbus::bwct_fw_update(uint8_t address, const String& fwpath) {
	File firmware;
	ssize_t pagesize;
	uint16_t fwaddress = 0;
	uint8_t pagedata[64];

	firmware.open(fwpath);
	int retries = 0;
	int retries_sum = 0;

	Mutex::Guard mutex(mtx_bus);
	try {
		packet[0] = address;
		packet[1] = VENDOR_BOOTLOADER;
		packetlen = 2;
		do_packet();
	} catch (std::exception& e) {
		fprintf(stderr, "Error: %s - ignoring\n", e.what());
	}

	while ((pagesize = firmware.read(pagedata, 64)) > 0) {
		bool written = 0;
		retries = 0;
		do {
			try {
				packet[0] = address;
				packet[1] = VENDOR_PAGEDATA;
				packet[2] = fwaddress >> 8;
				packet[3] = fwaddress & 0xff;
				packetlen = 4;
				bcopy(pagedata, &packet[4], pagesize);
				packetlen += 64;
				if (retries == 0) {
					printf("sending pagedata %i\n", fwaddress);
				} else {
					printf("sending pagedata %i retry %i\n", fwaddress, retries);
				}
				do_packet();

				packet[0] = address;
				packet[1] = VENDOR_WRITE_PAGE;
				packetlen = 2;
				printf("writing pagedata %i\n", fwaddress);
				do_packet();

				fwaddress += 64;
				written = true;
			} catch (...) {
				retries++;
				retries_sum++;
				if (retries > 4) {
					throw;
				}
			}
		} while (!written);
	};

	packet[0] = address;
	packet[1] = VENDOR_RESET;
	packetlen = 2;
	do_packet();

	printf("retries %i\n", retries_sum);
	mutex.unlock();
}

