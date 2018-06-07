/* SPWFInterface Example
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef BG96_H
#define BG96_H

 
#include "ATParser.h"
#include "PursuitAlertRevCInterface.h"
#include "setupSWOForPrint.h" 		/******* Redirect print(); to SWO via debug printf viewer  *******/

//debug messages enable 
#define DBG_MSG				0

#define BG96_CONNECT_TIMEOUT    15000
#define BG96_SEND_TIMEOUT       500
#define BG96_RECV_TIMEOUT       500 //some commands like AT&F/W takes some time to get the result back!
#define BG96_MISC_TIMEOUT       1000
#define BG96_SOCKQ_TIMEOUT      8000

Timer main_timer;

/** BG96Interface class.
    This is an interface to a BG96 module.
 */
class BG96
{
public:
    
    BG96(PinName tx, PinName rx, PinName reset=NC, PinName wakeup=NC, bool debug=false);
    
    /**
    * Init the BG96
    *
    * @param mode mode in which to startup
    * @return true only if BG96 has started up correctly
    */
    bool startup(int mode);
 
    void waitBG96Ready(void);
    /**
    * Reset BG96
    *
    * @return true only if BG96 resets successfully
    */
    bool reset(void);
    
    bool hw_reset(void);
 
    /**
    * Enable/Disable DHCP
    *
    * @param mode mode of DHCP 2-softAP, 1-on, 0-off
    * @return true only if BG96 enables/disables DHCP successfully
    */
    bool dhcp(int mode);
 
    /**
    * Connect BG96 to AP
    *
    * @param ap the name of the AP
    * @param passPhrase the password of AP
    * @param securityMode the security mode of AP (WPA/WPA2, WEP, Open)
    * @return true only if BG96 is connected successfully
    */
    bool connect(const char *apn, const char *username, const char *password);
 
    /**
    * Disconnect BG96 from AP
    *
    * @return true only if BG96 is disconnected successfully
    */
    bool disconnect(void);
 
    /**
    * Get the IP address of BG96
    *
    * @return null-teriminated IP address or null if no IP address is assigned
    */
    const char *getIPAddress(void);
 
    /**
    * Get the MAC address of BG96
    *
    * @return null-terminated MAC address or null if no MAC address is assigned
    */
    const char *getMACAddress(void);
 
    /**
    * Check if BG96 is conenected
    *
    * @return true only if the chip has an IP address
    */
    bool isConnected(void);
 
    /**
    * Open a socketed connection
    *
    * @param type the type of socket to open "u" (UDP) or "t" (TCP)
    * @param id id to get the new socket number, valid 0-7
    * @param port port to open connection with
    * @param addr the IP address of the destination
    * @return true only if socket opened successfully
    */
    bool open(const char *type, int* id, const char* addr, int port);
 
    /**
    * Sends data to an open socket
    *
    * @param id id of socket to send to
    * @param data data to be sent
    * @param amount amount of data to be sent - max 1024
    * @return true only if data sent successfully
    */
    bool send(int id, const void *data, uint32_t amount);
 
    /**
    * Receives data from an open socket
    *
    * @param id id to receive from
    * @param data placeholder for returned information
    * @param amount number of bytes to be received
    * @return the number of bytes received
    */
    int32_t recv(int id, void *data, uint32_t amount);
 
    /**
    * Closes a socket
    *
    * @param id id of socket to close, valid only 0-4
    * @return true only if socket is closed successfully
    */
    bool close(int id);
 
    /**
    * Checks if data is available
    */
    bool readable();
 
    /**
    * Checks if data can be written
    */
    bool writeable();
 
private:
    BufferedSerial _serial;
    ATParser _parser;
    DigitalInOut _wakeup;
    DigitalInOut _reset; 
    char _ip_buffer[16];
    char _mac_buffer[18];
    bool dbg_on;
//    int _timeout; // FIXME LICIO we have "virtual" socket tmo, module socket tmo, 
// AT parser tmo, recv/send tmo, actually used the NetworksocketAPI socket tmo
    unsigned int _recv_timeout; // see SO_RCVTIMEO setsockopt
    unsigned int _send_timeout; // see SO_SNDTIMEO setsockopt   
    unsigned int socket_closed;
};
 
static void delay_ms(uint32_t del)
{
	do{
			wait_ms(1);
	}while (del--);
	
}

BG96::BG96(PinName CELL_TX, PinName CELL_RX, PinName cell_reset, PinName cell_wakeup, bool debug)
    : _serial(CELL_TX, CELL_RX, 1024), _parser(_serial),
      _reset(cell_reset, PIN_OUTPUT, PullNone, 1),
      _wakeup(cell_wakeup, PIN_OUTPUT, PullNone, 0),
      dbg_on(debug)
{
    _serial.baud(115200);  // LICIO  FIXME increase the speed
    _parser.debugOn(debug);
}

bool BG96::startup(int mode)
{
    //Timer timer_s;
		//uint32_t time;
    _parser.setTimeout(BG96_MISC_TIMEOUT);
    /*Test module before reset*/
    waitBG96Ready();
    
    wait(1.0);
    _parser.send("ATE0");
    while (1) {
        if (_parser.recv("OK")){
            print("ATE ok\r\n");
            break;
            }
    }	

    print("waiting for network...\r\n");
		//wait for 10 seconds to connect provider network
		//green led lamps 0,2 seconds
		uint8_t wloop = 0;
		while(1)
		{
			if (wloop >= (10/0.2))
				break;
			led0_n=0;
			wait_ms(100);
			led0_n=1;
			wait_ms(100);
			wloop++;
		}
	  
		led0_n=0;
		//only to see cops data ... no matching or check 
		_parser.send("AT+COPS?");
    while (1) {
        if (_parser.recv("OK"))
					{
            break;
          }
    }

	#if 0	
    _parser.send("AT+QIACT?");
    while (1) {
        if (_parser.recv("OK"))
            break;
    }
  #endif

    /*Reset parser buffer*/
    _parser.flush();
    return true;
}

bool BG96::hw_reset(void)
{
	  BG96_reset = 0;
	  BG96_PWRKEY = 0;
	  //VBAT_3V8_EN = 0;
		
		delay_ms(200);
	
	 print("HW reset BG96\r\n");

    BG96_reset = 1;

    //VBAT_3V8_EN = 1;

//    BG96_W_DISABLE = 0;

    BG96_PWRKEY = 1;
	
		delay_ms(300);

    BG96_reset = 0;

		delay_ms(5);

    return 1;
}

bool BG96::reset(void)
{
    print("\n\rInit BG96 ...\r\n");
    hw_reset();
    return true;
}

void BG96::waitBG96Ready(void)
{
    reset();
    print("Wait for ready...");

		//after HW reset,if no RDY from BG96 program stops 
		//green led lamps 0.5 seconds
    while(1) 
		{
        if (_parser.recv("RDY")) 
				{
						print("BG96 ready\r\n");
						led0_n = 0;
						return ;
        }
				if (led0_n == 0)
					led0_n = 1;
				else
					led0_n = 0;
    }

}

char pdp_string[100];

bool BG96::connect(const char *apn, const char *username, const char *password)
{
			int i = 0;
			char* search_pt;

	    Timer timer_s;
			uint32_t time;
	
			int pdp_retry = 0;
	
			memset(pdp_string, 0, sizeof(pdp_string));
			print("Checking APN ...\r\n");
			_parser.send("AT+QICSGP=1");
	    while(1){
						//read and store answer
						_parser.read(&pdp_string[i], 1);
						i++;
						//if OK rx, end string; if not, program stops
            search_pt = strstr(pdp_string, "OK\r\n");
						if (search_pt != 0)
						{
								break;
						}
						//TODO: add timeout if no aswer from module!!
    }
		
		//compare APN name, if match, no store is needed
		search_pt = strstr(pdp_string, apn);
		if (search_pt == 0)
		{
			//few delay to purge serial ...
			delay_ms(2);
			print("Storing APN %s ...\r\n", apn);
			//program APN and connection parameter only for PDP context 1, authentication NONE
			//TODO: add program for other context
			if (!(_parser.send("AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",0", &apn[0], &username[0], &password[0])
            && _parser.recv("OK"))) 
					{
						return false;
					}
		}
		print("End APN check\r\n\n");
		
		//activate PDP context 1 ...
		print("PDP activating ...\r\n");
		int a = 1;
		while(a==1)
		{
				_parser.send("AT+QIACT=1");
				timer_s.reset();
				timer_s.start();
				while (1)
				{
					if (_parser.recv("OK")){
						a=0;
						break;
					}
						
					time = timer_s.read_ms();
					uint32_t end_time = (BG96_MISC_TIMEOUT*(5+(pdp_retry*3)));
					if (time > end_time) 
					{
						pdp_retry++;
						if (pdp_retry > 3)
							{
								print("ERROR --->>> PDP not valid, program stoppped!!\r\n");
								print("***********************************************");
								while (1);
							}
						break;
					}
				}		
		}
		print("PDP started\r\n\n");
    return true;
}

bool BG96::disconnect(void)
{
    //TODO add needed further action after ACT close command ...
    if(!(_parser.send("AT+QIDEACT=%d\r", 1) && _parser.recv("OK"))) {
        //debug_if(dbg_on, "BG96> error while disconnet...\r\n");
        return false;
    }
    //reset();
    return true;
}

const char *BG96::getIPAddress(void)
{
		//TODO this function needs further debug ... now always returns IP 0.0.0.0
    char act_string[32];
    //IPV4 mode
    uint8_t n1, n2, n3, n4;

    if (!(_parser.send("AT+QIACT?")
						//NOTE the %s on parser.read function returns only the first character of string ... 
						//TODO verify parser.read  function
						//here used only to check if commands is well processed by BG96
            && _parser.recv("+QIACT: %s", &act_string[0])
            && _parser.recv("OK"))) {
        //debug_if(dbg_on, "BG96> getIPAddress error\r\n");
        return 0;
    }
    //TODO parse string to find IP ...
    //now this functions returns always IPV4 0.0.0.0
    n1 = n2 = n3 = n4 = 0;
    sprintf((char*)_ip_buffer,"%u.%u.%u.%u", 	n1, n2, n3, n4);

    return _ip_buffer;
}


#define IMEI_OFFS			2
#define	IMEI_LEN 			14+IMEI_OFFS

const char *BG96::getMACAddress(void)
{
    uint32_t n1, n2, n3, n4, n5, n6;
	
  #if ID_ON_FLASH
    n1=DEVICE_ID[0];		//0x00;
    n2=DEVICE_ID[1];		//0x0b;
    n3=DEVICE_ID[2];		//0x57;
    n4=DEVICE_ID[3];		//0x55;
    n5=DEVICE_ID[4];		//0xdb;
    n6=DEVICE_ID[5];		//0x45;
  #else
    char sn[32];
    memset (sn, 0, 32);
    if (!(_parser.send("AT+CGSN")
            && _parser.read(&sn[0], IMEI_LEN)
            && _parser.recv("OK"))) {
        print("BG96> IMEI error\r\n");
        return 0;
    }
    n1=0x00;
    n2=0x0b;
    n3=(uint8_t)(((sn[4+IMEI_OFFS]&0x0f)<<4 | (sn[5+IMEI_OFFS]&0x0f)) + ((sn[12+IMEI_OFFS]&0x0f)<<4));
    n4=(uint8_t)(((sn[6+IMEI_OFFS]&0x0f)<<4 | (sn[7+IMEI_OFFS]&0x0f)) + (uint8_t)((sn[0+IMEI_OFFS]&0x0f)<<4 | (sn[1+IMEI_OFFS]&0x0f)));
    n5=(uint8_t)(((sn[8+IMEI_OFFS]&0x0f)<<4 | (sn[9+IMEI_OFFS]&0x0f)) + ((sn[13+IMEI_OFFS]&0x0f)));
    n6=(uint8_t)(((sn[10+IMEI_OFFS]&0x0f)<<4 | (sn[11+IMEI_OFFS]&0x0f)) + (uint8_t)((sn[2+IMEI_OFFS]&0x0f)<<4 | (sn[3+IMEI_OFFS]&0x0f)));
  #endif
		sprintf((char*)_mac_buffer,"%02X%02X%02X%02X%02X%02X", n1, n2, n3, n4, n5, n6);
		print("DEVICE ID is %s\r\n",_mac_buffer);
    return _mac_buffer;
}

bool BG96::isConnected(void)
{
    return getIPAddress() != 0;
}
uint8_t sock_id = 0;
bool socket_opening = false;

bool BG96::open(const char *type, int* id, const char* addr, int port)
{
    Timer timer;
    timer.start();
    socket_closed = 0;
    uint32_t time;
    bool res;
    char send_type[16];
    const char *send_type_pt = send_type;
    int result;

		//check socket type ...
    switch(*type) {
        case 'u':
            send_type_pt = "UDP";
						//UDP on sock id = 1
            sock_id = 1;
            break;
        case 't':
            send_type_pt = "TCP";
						//TCP on sock id = 0
            sock_id = 0;
            break;
        default:
            send_type_pt = "TCP";
						sock_id = 0;
            break;
    };
    _parser.setTimeout(BG96_SOCKQ_TIMEOUT);

		//open socket for context 1
    while(1) {
			
        if((_parser.send("AT+QIOPEN=1,%d,\"%s\",\"%s\",%d,0,0\r", sock_id, send_type_pt, addr, port)) 
					&& (_parser.recv("OK")))

        {

            while(1) 
						{
                res = _parser.recv("+QIOPEN: %d,%d", id, &result);
                if (res == true) 
									{
                    print("Open socket type %s #%d result = %d\r\n", send_type_pt, *id, result);
                    if (result == 0)
										{	
												//wait for socket open result ... start RECV timeout timer
												socket_opening = true;
												main_timer.reset();
												main_timer.start();
											  return true;
										}
										else
										{
										int s_id = *id;
                    //if (result == 563) 
                    close(s_id);
                    return false;
										}
									}

                time = timer.read_ms();
                if (time > BG96_CONNECT_TIMEOUT)
									{
                    print("Timeout sock open!!\r\n");
                    return false;
									}

           }

        }
        return false;
    }

}
uint8_t err_counter = 0;

bool BG96::send(int id, const void *data, uint32_t amount)
{
    char _buf[32];

		char	result_string[100];
		char* search_pt;
    _parser.setTimeout(BG96_SEND_TIMEOUT);

		//pclog.printf("Sending %d len ...", amount);
	
		//check if previous sent is good, otherwise after 5 consecutive fails close socket and error!!
		if (err_counter > 5)
		{
				print("Closing socket #%d\r\n", id);
				close(id);
				socket_closed = 1;
				err_counter = 0;
				return false;
		}		
	
		#if DBG_MSG


		uint8_t* dt = (uint8_t*)data;
		//these tests and print is only for debug purposes
		if (amount == 2)
		{
			if ((dt[0] == 0xc0) && (dt[1] == 0x00))
				print("\r\n\nPING ");
		}
		
		_parser.send("AT+COPS?");
    while (1) 
		{
        if (_parser.recv("OK")){
            break;
            }
		}

		#endif

		_parser.send("AT+QISEND=%d,0", id);
		#if 0
				err_counter = 0;
				while(1)
				{
					if( _parser.recv("+QISEND: %s", &result_string[0])
            && _parser.recv("OK")) 
						{
							break;
						}	
				}
		#else
			
		int i = 0;
		memset(result_string, 0, sizeof(result_string));
		const char OK_str[] = {"OK\r\n"};
		const char OK0_str[] = {",0"};
		while(1){
						//read and store answer
						_parser.read(&result_string[i], 1);
						i++;
						//if OK rx, end string; if not, program stops
            search_pt = strstr(result_string, OK_str);
						if (search_pt != 0)
						{
								break;
						}
						//TODO: add timeout if no aswer from module!!
    }
		
		//if send fails, the string doesn't have ",0" sequence, then 
		search_pt = strstr(result_string, OK0_str);		
		if (search_pt == 0)
		{
				err_counter++;
		}
		else
		{
				err_counter = 0;
		}
		
		#endif

		
		//here send data to socket ...
    sprintf((char*)_buf,"AT+QISEND=%d,%d\r", id, amount);

    //May take more try if device is busy
    for (unsigned i = 0; i < 3; i++) {
        if (_parser.write((char*)_buf, strlen(_buf)) >=0) {
            if (_parser.recv("> ")) {
                if ((_parser.write((char*)data, (int)amount) >= 0)
                        && (_parser.recv("SEND OK"))) 
									{
										#if DBG_MSG
										//print for debug
										print("Sent!\r\n");
										#endif
                    return true;
									}
            }
        }
        //if send fails, wait before further attempt
        print("Send retry #%d\r\n", (i+1));
        wait_ms(2000);
    }
		
		//FAIL to send data to socket, after 3 retry!!
		#if DBG_MSG
		//these test and print only for debug
		_parser.send("AT+COPS?");
    while (1) {
        if (_parser.recv("OK")){
            break;
            }
    }
		_parser.send("AT+QISTATE?");
    while (1) {
        if (_parser.recv("OK")){
            break;
            }
    }
		#endif
		
	  print("Closing socket #%d\r\n", id);
		close(id);
		socket_closed = 1;
    return false;

}

uint8_t local_data_store[16];
uint8_t local_amount = 0;
uint8_t local_pt = 0;

int32_t BG96::recv(int id, void *data, uint32_t amount)
{
    uint32_t recv_amount=0;
		uint32_t now_time;
		#if DBG_MSG
		uint8_t* dt = (uint8_t*)data;
		#endif

		//received data pending??
    if (local_amount > 0) {
        memcpy(data, &local_data_store[local_pt], amount);
        local_amount = local_amount-amount;
        local_pt = local_pt+amount;
				//return pending data...
        return amount;
    }
		
    local_amount = 0;
    local_pt = 0;


    if (socket_closed) {
        socket_closed = 0;
        return -3;
    }

		//wait dato from open socket request ... check timeout
		if (socket_opening == true)
		{
			now_time  = main_timer.read_ms();
			if (now_time > BG96_CONNECT_TIMEOUT)
				{
					 socket_opening = false;
					 print("-->>> SOCK tout\r\n");
					 main_timer.stop();
					 return -3;
				}
		}
		
    /*
    int par_timeout = _parser.getTimeout();
    _parser.setTimeout(0);
    _parser.setTimeout(par_timeout);
    */
    switch (id) {
        case 0:
            if (!(_parser.recv("+QIURC: \"recv\",0")))
                return -1;
            break;
        case 1:
            if (!(_parser.recv("+QIURC: \"recv\",1")))
                return -1;
            break;
        default:
            return -1;
            //break;
    }

    if ((_parser.send("AT+QIRD=%d,1000\r", id))  //send a query (will be required for secure sockets)
            && (_parser.recv("+QIRD: %u", &recv_amount))) {

        _parser.read((char*)data, recv_amount);
        while( _parser.recv("OK"));
				
			  #if DBG_MSG
				if ((recv_amount == 2) && (dt[0] == 0xd0) && (dt[1] == 0x00))
					print("PING Recv!\r\n");
				#endif
							
				//if data amount received more than expected, store it for next call of recv function
        if(recv_amount > amount) {
            local_amount = recv_amount-amount;
            local_pt = local_pt+amount;
            memcpy(local_data_store, data, recv_amount);
            recv_amount = amount;
        }
				socket_opening = false;
        return recv_amount;

    }
    return -2;

}

bool BG96::close(int id)
{
    uint32_t recv_amount=0;
    void * data = NULL;

    _parser.setTimeout(BG96_SOCKQ_TIMEOUT*2);
    _parser.flush();
    /* socket flush */
    if(!(_parser.send("AT+QIRD=%d,1000\r", id) //send a query (will be required for secure sockets)
            && _parser.recv("+QIRD: %u", &recv_amount))
            //&& _parser.recv("OK"))
      ) {
        return -2;
    }

    if (recv_amount>0) {
        data = malloc (recv_amount+4);
//       printf ("--->>>Close flushing recv_amount: %d  \n\r",recv_amount);
        if(!((_parser.read((char*)data, recv_amount) >0)
                && _parser.recv("OK"))) {
            free (data);
            return -2;
        }
        free (data);
    } else {
        if (!(_parser.recv("OK")))
            return -2;
    }

    //May take a second try if device is busy or error is returned
    for (unsigned i = 0; i < 2; i++) {
        if ((_parser.send("AT+QICLOSE=%d", id))
                && _parser.recv("OK")) {
            socket_closed = 1;
            return true;
        } else {
            if(_parser.recv("ERROR")) {
                debug_if(dbg_on, "BG96> ERROR!!!!\r\n");
                return false;
            }
        }
        //TODO: Deal with "ERROR: Pending data" (Closing a socket with pending data)
    }
    return false;

}


bool BG96::readable()
{
    return _serial.readable();
}

bool BG96::writeable()
{
    return _serial.writeable();
}
#endif  //BG96_H

