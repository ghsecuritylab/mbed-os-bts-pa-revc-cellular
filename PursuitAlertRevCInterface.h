// ********************** Mbed I/O Notes ********************** //
//
// By default mbed DigitalOut is set to use push pull mode
//			DigitalOut modes = None, only push pull
//			DigitalOut (PinName pin, int value);
//
// By default DigitalIn is set to use pull down mode
// 			DigitalIn modes pull = PullUp, PullDown, PullNone, OpenDrain
//			DigitalIn (PinName pin, PinMode mode);
// 
// By default DigitalInOut is set to use 
// 			DigitalInOut modes pull = PullUp, PullDown, PullNone, OpenDrain
//			DigitalInOut (PinName pin, PinDirection direction, PinMode mode, int value);
//
// ************************************************************* //

#ifndef PURSUIT_ALERT_REVC_INTERFACE_
#define PURSUIT_ALERT_REVC_INTERFACE_

#include "PinNames.h"
#include "mbed.h"
#include "pinmap.h"         						// pinmap needed for hardware flow control
#include "em_system.h"
#include "PinDetect.h"

// LEDs
DigitalOut led0(LED0);  								// by default DigtalOut is set to use push pull 
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut purstled(PURST_LED);
DigitalOut emergled(EMERG_LED);
DigitalOut spareled(SPARE_LED);
DigitalOut dropled(DROP_LED);
DigitalOut poweronled(POWERON_LED);
DigitalOut redled(RED_LED);
DigitalOut greenled(GREEN_LED);
DigitalOut blueled(BLUE_LED);

// Push Buttons 
DigitalIn	purstpb(PURST_PB);						// by default DigtalIn is set to use pull down 
DigitalIn	emergpb(EMERG_PB);
DigitalIn	sparepb(SPARE_PB);
DigitalIn	droppb(DROP_PB);

PinDetect pushb1(PURST_PB);
PinDetect	pushb2(EMERG_PB);
PinDetect	pushb3(SPARE_PB);
PinDetect	pushb4(DROP_PB);

// Enables for LED output driver
DigitalOut GPA_EN_n(PB12);     					// GPA_EN_n (active low)
DigitalOut GPB_EN_n(PC6);      					// GPA_EN_n (active low)

// Serial I/O
Serial debug_pc(LEUART_TX, LEUART_RX); 	// Serial connection to PC
Serial cell(CELL_TX, CELL_RX);        	// Serial comms to Cellular Modem
Serial skygps(SKYGPS_TX, SKYGPS_RX);  	// Seperate Serial comms to Skywire GPS
Serial wifi(WIFI_TX, WIFI_RX); 					// Serial coms to Ethernet/Wi-Fi WF121

// Cellular modem control
//DigitalOut skywire_power(PA4); 				// ?
DigitalOut cell_reset(PF5, OpenDrain);  // When High Cell is Reset
DigitalOut cell_rts(PE13);							// CTS and RTS are connected
DigitalOut cell_on(PF4);    						// Turn on  modem
DigitalIn cell_cts(PE12);								// CTS and RTS are connected
DigitalIn cell_ring(PF4, PullUp);				// 
DigitalOut cell_wake(PE8); 							// 

// Wi-Fi control signals
DigitalOut wifi_cts(PC4); 
DigitalIn wifi_rts(PC5); 
DigitalOut wifi_wake_int(PA8); 

// EEPROM Control
DigitalOut ee_wc_n(EE_WC_L);
DigitalOut ee_e2(EE_E2);

// System Control
DigitalOut power_on_ctrl(PD8); 					// Power On Control 
DigitalIn pwr_5v_good(PC7, PullUp);			// +5.0v Vehicle Supply 
DigitalIn pwr_comp(PC11, PullUp);				// Vehicle Supply 
DigitalIn ign_comp(PC12, PullUp);  			// Vehicle Ignition Power

#endif