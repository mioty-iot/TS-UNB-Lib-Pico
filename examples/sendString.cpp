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

#include "../src/RPPicoTsUnb.h"

// This is the node specific configuration
#define MAC_NETWORK_KEY 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
#define MAC_EUI64       0x70, 0xB3, 0xD5, 0x67, 0x70, 0xFF, 0x00, 0x00
#define MAC_SHORT_ADDR  0x70, 0xFF


#define TRANSMIT_PWR  	14      // Transmit power in dBm
#define LED_PIN			25	

using namespace TsUnbLib::RPPico;

// Select preset depending on TX chip
//TsUnb_EU1_Rfm69w_t TsUnb_Node;
TsUnb_EU1_Rfm69hw_t TsUnb_Node;

//TsUnb_US0_Rfm69w_t TsUnb_Node;
//TsUnb_US0_Rfm69hw_t TsUnb_Node;


//The setup function is called once at startup of the sketch
void setup() {
	sleep_ms(100);
  
	stdio_init_all();
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	
	gpio_put(LED_PIN, 1);
	sleep_ms(100);
	gpio_put(LED_PIN, 0);

	
	TsUnb_Node.init();
	TsUnb_Node.Tx.setTxPower(TRANSMIT_PWR);
	TsUnb_Node.Mac.setNetworkKey(MAC_NETWORK_KEY);
	TsUnb_Node.Mac.setEui64(MAC_EUI64);
	TsUnb_Node.Mac.setShortAddress(MAC_SHORT_ADDR);
	TsUnb_Node.Mac.extPkgCnt = 0x01;


	// Blink LED
	gpio_put(LED_PIN, 1);
	sleep_ms(1000);
	gpio_put(LED_PIN, 0);

}


int main() {
	//  wdt_enable();	// Enable 8s supervision TODO:watchdog not implemented
	setup();
	
	while(1){
		// Send the text "Hello"
		char str[] = "Hello";
		TsUnb_Node.send((uint8_t *)str, sizeof(str) / sizeof(str[0]) - 1);

		// Blink LED when TX is done
		gpio_put(LED_PIN, 1);
		sleep_ms(100);
		gpio_put(LED_PIN, 0);
		sleep_ms(100);
		gpio_put(LED_PIN, 1);
		sleep_ms(100);
		gpio_put(LED_PIN, 0);

	  	// Sleep some time
		sleep_ms(4000);
	}
}

