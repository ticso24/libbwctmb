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

#ifndef _BWCTMB
#define _BWCTMB

#include <bwct/bwct.h>

class Modbus : public Base {
private:
	uint8_t packet[256];
	uint8_t packetlen;
	uint8_t retries;
	uint16_t sequence;
	String IP;
	String Port;
	a_ptr<Network::Net> bus;
	Mutex mtx_bus;
	bool ignore_sequence;

	void do_packet();
	void reconnect();

public:
	class Error : public ::Error {
	public:
		uint8_t mb_error;
		Error(const String& msg, uint8_t nmb_error)
		    : ::Error(msg.c_str()) {
			mb_error = nmb_error;
		}
		Error(const char* msg, uint8_t nmb_error)
		    : ::Error(msg) {
			mb_error = nmb_error;
		}
		uint8_t get_errno() {
			return mb_error;
		}
	};

	Modbus(const String& nIP, const String& nPort) {
		IP = nIP;
		Port = nPort;
		retries = 4;
		sequence = getrandom();
		ignore_sequence = false;
	}

	void set_retries(int new_retries) {
		retries = new_retries;
	}
	void set_ignore_sequence(bool new_ignore_sequence) {
		ignore_sequence = new_ignore_sequence;
	}
	bool read_discrete_input(uint8_t address, uint16_t num);
	SArray<bool> read_discrete_inputs(uint8_t address, uint16_t num, uint16_t count);
	bool read_coil(uint8_t address, uint16_t num);
	SArray<bool> read_coils(uint8_t address, uint16_t num, uint16_t count);
	void write_coil(uint8_t address, uint16_t num, bool val);
	void write_coils(uint8_t address, uint16_t num, SArray<bool> val);
	uint16_t read_input_register(uint8_t address, uint16_t num);
	SArray<uint16_t> read_input_registers(uint8_t address, uint16_t num, uint16_t count);
	uint16_t read_holding_register(uint8_t address, uint16_t num);
	SArray<uint16_t> read_holding_registers(uint8_t address, uint16_t num, uint16_t count);
	void write_register(uint8_t address, uint16_t num, uint16_t val);
	void write_registers(uint8_t address, uint16_t num, SArray<uint16_t> val);
	String identification(uint8_t address, uint8_t num);
	void mask_write_register(uint8_t address, uint16_t num, uint16_t andval, uint16_t orval);
	SArray<uint16_t> read_write_registers(uint8_t address, uint16_t rnum, uint16_t count, uint16_t wnum, SArray<uint16_t> val);
	void bwct_set_address(uint8_t address, uint8_t naddress, String serial);
	String bwct_read_magcard(uint8_t address);
	void bwct_fw_update(uint8_t address, const String& fwpath);
};

#endif /* _BWCTMB */
