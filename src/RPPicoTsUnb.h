/* -----------------------------------------------------------------------------

Software License for the Fraunhofer TS-UNB-Lib

(c) Copyright  2019 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. All rights reserved.


1. INTRODUCTION

The Fraunhofer Telegram Splitting - Ultra Narrowband Library ("TS-UNB-Lib") is software
that implements only the uplink of the ETSI TS 103 357 TS-UNB standard ("MIOTY") for wireless 
data transmission in the field of IoT. Patent licenses for any patent claim regarding the 
ETSI TS 103 357 TS-UNB standard implementation (including those of Fraunhofer) may be 
obtained through Sisvel International S.A. 
(https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms)
or through the respective patent owners individually. The purpose of this TS-UNB-Lib is 
academic and non-commercial use. Therefore, Fraunhofer does not offer any support for the 
TS-UNB-Lib. Furthermore, the TS-UNB-Lib is NOT identical and on the same quality level as 
the commercially-licensed MIOTY software also available from Fraunhofer. Users are encouraged
to check the Fraunhofer website for additional applications information and documentation.


2. COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification, are 
permitted without payment of copyright license fees provided that you satisfy the following 
conditions: You must retain the complete text of this software license in redistributions
of the TS-UNB-Lib software or your modifications thereto in source code form. You must retain 
the complete text of this software license in the documentation and/or other materials provided
with redistributions of the TS-UNB-Lib software or your modifications thereto in binary form.
You must make available free of charge copies of the complete source code of the TS-UNB-Lib 
software and your modifications thereto to recipients of copies in binary form. The name of 
Fraunhofer may not be used to endorse or promote products derived from this software without
prior written permission. You may not charge copyright license fees for anyone to use, copy or
distribute the TS-UNB-Lib software or your modifications thereto. Your modified versions of the
TS-UNB-Lib software must carry prominent notices stating that you changed the software and the
date of any change. For modified versions of the TS-UNB-Lib software, the term 
"Fraunhofer TS-UNB-Lib" must be replaced by the term
"Third-Party Modified Version of the Fraunhofer TS-UNB-Lib."


3. NO PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without limitation the patents 
of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE. Fraunhofer provides no warranty of patent 
non-infringement with respect to this software. You may use this TS-UNB-Lib software or modifications
thereto only for purposes that are authorized by appropriate patent licenses.


4. DISCLAIMER

This TS-UNB-Lib software is provided by Fraunhofer on behalf of the copyright holders and contributors
"AS IS" and WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, including but not limited to the implied warranties
of merchantability and fitness for a particular purpose. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE for any direct, indirect, incidental, special, exemplary, or consequential damages,
including but not limited to procurement of substitute goods or services; loss of use, data, or profits,
or business interruption, however caused and on any theory of liability, whether in contract, strict
liability, or tort (including negligence), arising in any way out of the use of this software, even if
advised of the possibility of such damage.


5. CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Division Communication Systems
Am Wolfsmantel 33
91058 Erlangen, Germany
ks-contracts@iis.fraunhofer.de

----------------------------------------------------------------------------- */

/**
 * @brief	TS-UNB abstractions for Raspberry Pi Pico
 *
 * @authors	Joerg Robert, Augusto Kloth
 * @file	RPPicoTsUnb.h
 *
 */



#ifndef RPPICO_TSUNB_H_
#define RPPICO_TSUNB_H_

#include <inttypes.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <pico/binary_info.h>
#include <hardware/spi.h>
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>

#include "../TsUnb/RadioBurst.h"
#include "../TsUnb/FixedMac.h"
#include "../TsUnb/Phy.h"
#include "../TsUnb/SimpleNode.h"



/**
 * @brief Port number where the transceivers slave select pin is connected to
 */
#define SPI_INTERFACE	spi0

/**
 * @brief Default GPIOs pins for SPI0
 */
#define SPI0_RX 	 16
#define SPI0_CSn 	 17
#define SPI0_SCK 	 18
#define SPI0_TX 	 19

/**
 * @brief Baud rate for SPI communication
 */
#define SPI_BAUDRATE	4000000

namespace TsUnbLib {
namespace RPPico {


//! Flag to indicate end of timer
volatile absolute_time_t TimeAddedDelay;
volatile bool TsUnbTimerFlag;
volatile bool ExtraDelaySet;
volatile float preciseTsUnbTimer_us;
volatile float TsUnbBitDuration_us;
volatile int64_t TsUnbTimeNextCycle_us;


/**
 * @brief Interrupt function for compare match of timer to set TimerFlag
 */
int64_t timer_callback(alarm_id_t id, void *user_data) {

	if(!ExtraDelaySet) { //fire and then reload timer for next cycle
		TsUnbTimerFlag = true;
		preciseTsUnbTimer_us += TsUnbBitDuration_us;
		TsUnbTimeNextCycle_us = (int64_t)(preciseTsUnbTimer_us + 0.5f);
		preciseTsUnbTimer_us -= (float)TsUnbTimeNextCycle_us;		
	}
	else { //AddTimerDelay has already calculated, just adjusting the time
		TsUnbTimeNextCycle_us = (int64_t)(preciseTsUnbTimer_us + 0.5f);
		preciseTsUnbTimer_us -= (float)TsUnbTimeNextCycle_us;	
		ExtraDelaySet = false;
	}

	return -TsUnbTimeNextCycle_us; 	// Negative menas that delay between calls will be kept regardless of callback time 
}

/**
 * @brief Platform dependent TS-UNB implementation for ATMega328p based Arduino systems.
 *
 * This class implents all plaform dependent methods for TS-UNB.
 * It mainly offer SPI communication and a timer to generate the TS-UNB symbol clock.
 *
 * @param CS_PIN              SPI chip select pin (default is 17)
 * @param SYMBOL_RATE_MULT    TS-UNB, symbol rate in multiples of 49.591064453125, set for 48 for 2380.371sym/s and 8 for 396.729sym/s. For higher rates the clock divider of the timer may have to be adjusted.
 * @param SPI_INIT            Init SPI before use and de-init after use (default is true).
 */
template <uint16_t SYMBOL_RATE_MULT = 48>
class RPPicoTsUnb {
public:
	RPPicoTsUnb() {
	}

	~RPPicoTsUnb() {
	}

	/**
	 * @brief Bit duration in microseconds
	 *
	 * This value is the duration of a single TS-UNB symbol, e.g. 1/2380.372s for the normal mode.
	 *
	 */
	static constexpr float TS_UNB_BIT_DURATION_US = (double) 1000000 / (49.591064453125 * (double)SYMBOL_RATE_MULT);


	/**
	 * @brief Init the timer
	 */
	void initTimer() {
		preciseTsUnbTimer_us = 0;
		TsUnbBitDuration_us = TS_UNB_BIT_DURATION_US;
	}

	/**
	 * @brief Start the timer
	 */
	void startTimer() {	
		if(preciseTsUnbTimer_us == 0){
			preciseTsUnbTimer_us += TS_UNB_BIT_DURATION_US;
		}

		ExtraDelaySet = false;
		TsUnbTimerFlag = false;	

		TsUnbTimeNextCycle_us = (int64_t)(preciseTsUnbTimer_us + 0.5f);
		preciseTsUnbTimer_us -= (float)TsUnbTimeNextCycle_us;	

		alarm_id = add_alarm_in_us((uint64_t)TsUnbTimeNextCycle_us, timer_callback, NULL, true);
	}



	/**
	 * @brief Stop the timer
	 */
	void stopTimer() {
		cancel_alarm(alarm_id);
	}

	/**
	 * @brief Add the counter compare value for the next interrupt
	 *
	 * @param count Delay in TX symbols
	 */
	void addTimerDelay(const int32_t count) {
		TimeAddedDelay = get_absolute_time();
		preciseTsUnbTimer_us += TS_UNB_BIT_DURATION_US * (count-1);
		ExtraDelaySet = true;		
	}

	/**
	 * @brief Wait until the timer values expires
	 */
	void waitTimer() const {
		//TODO check if timer is really running
		sleep_us(TsUnbTimeNextCycle_us-10);			
		while (true){
			if (TsUnbTimerFlag){
				break;
			}
		}
		TsUnbTimerFlag = false;
	}

	/**
	 * @brief Initialization of the SPI interface
	 */
	void spiInit(void) {
		gpio_init(SPI0_CSn);
		gpio_set_dir(SPI0_CSn, GPIO_OUT);
		gpio_put(SPI0_CSn, 1);

		gpio_set_function(SPI0_RX, GPIO_FUNC_SPI);
		gpio_set_function(SPI0_SCK, GPIO_FUNC_SPI);
		gpio_set_function(SPI0_TX, GPIO_FUNC_SPI);

		spi_init(SPI_INTERFACE, SPI_BAUDRATE);
	}
	
	/**
	 * @brief Deinitialization of the SPI interface
	 */
	void spiDeinit(void) {
		spi_deinit(SPI_INTERFACE);
		gpio_init(SPI0_RX);
		gpio_init(SPI0_SCK);
		gpio_init(SPI0_TX);
		gpio_init(SPI0_CSn);
	}


	/**
	 * @brief Sends multiple bytes using SPI and sets the slave select pin accordingly
	 * 
	 * @param dataOut Bytes to be transmitted
	 * @param numBytes Number of bytes to be transmitted
	 */
	void spiSend(const uint8_t* const dataOut, const uint8_t numBytes) {
		gpio_put(SPI0_CSn, 0);
		spi_write_blocking(SPI_INTERFACE, dataOut, numBytes);
		gpio_put(SPI0_CSn, 1);
	}

	/**
	 * @brief Sends multiple and receives bytes using SPI and sets the slave select pin accordingly
	 *
	 * This method write and reads the SPI data. Please not that the read data has a delay of one byte.
	 * Hence, the first returned byte normally has no meaning.
	 * 
	 * @param dataInOut Bytes to be transmitted and buffer containing the read data
	 * @param numBytes  Number of bytes to be transmitted
	 */
	void spiSendReceive(uint8_t* const dataInOut, const uint8_t numBytes) {
		// Allocate memory as we cannot read write on the same memory
		uint8_t readData[numBytes];

		gpio_put(SPI0_CSn, 0);
		spi_write_read_blocking(SPI_INTERFACE, dataInOut, readData, numBytes);
		gpio_put(SPI0_CSn, 1);

		for(int i=0;i<numBytes;i++) {
			dataInOut[i] = readData[i];
		}

	}

	alarm_id_t alarm_id;	


	/**
	 * @brief Reset watchdog (just stub, not implemented)
	 * 
	 */
	void resetWatchdog() {};
};


};	// namespace RPPico
};	// namespace TsUnbLib

#include "RPPicoTsUnbTemplates.h"

#endif 	// RPPICO_TSUNB_H_


