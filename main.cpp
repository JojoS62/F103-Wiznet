#include "mbed.h"
#include "W5500Interface.h"
#include "HTTPServer.h"
#include "WebsocketHandlers.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#include <string>

using namespace std;

const int OFF = 0;
const int ON  = 1;

DigitalOut      led1(PC_13);

//W5500Interface   eth(&spi, PB_12, NC);       // MOSI, MISO, SCK, CS, RESET

// The MAC number must be unique within the connected network. Modify as appropriate.
uint8_t       MY_MAC[6] = { 0xaa, 0x01, 0x10, 0xaa, 0x10, 0x10 };

const char          MY_IP[] = "192.168.1.181";
const char          MY_NETMASK[] = "255.255.255.0";
const char          MY_GATEWAY[] = "192.168.1.1";
int                 MY_PORT = 80;


/**
 * @brief
 * @note
 * @param
 * @retval
 */
int main(void) {
//    int     ret = eth.init(MY_MAC, MY_IP, MY_NETMASK, MY_GATEWAY);
//    int     ret = eth.init(MY_IP, MY_NETMASK, MY_GATEWAY);
    W5500Interface   eth(PB_15, PB_14, PB_13, PB_12, NC);       // MOSI, MISO, SCK, CS, RESET
    
    int     ret = eth.init(MY_MAC);   // use DHCP

    if(!ret) {
        printf("Initialized, MY_MAC: %s\n", eth.get_mac_address());
    }
    else {
        printf("Error eth.init() - ret = %d\n", ret);
        return -1;
    }

    bool ret_connect = eth.connect();
    if (!ret_connect) {
        printf("Connected, MY_IP: %s, MY_NETMASK: %s, MY_GATEWAY: %s\n",
                eth.get_ip_address(),
                eth.get_netmask(),
                eth.get_gateway()
            );
    } else {
        printf("ERROR: connect %d", ret_connect);
    }

    HttpServer server(&eth, 2, 1);               // max 5 threads, 4 websockets
//    server.setHTTPHandler("/", &request_handler);
//    server.setHTTPHandler("/stats/", &request_handler_getStatus);
    
 //   server.setWSHandler("/ws/", WSHandler::createHandler);

    nsapi_error_t res = server.start(8080);

    if (res == NSAPI_ERROR_OK) {
        //SocketAddress socketAddress;
        //eth.get_ip_address(&socketAddress);
        printf("Server is listening at http://%s:8080\n", eth.get_ip_address());
    }
    else {
        printf("Server could not be started... %d\n", res);
    }

    while(1) {
        led1 = !led1;
        ThisThread::sleep_for(1000);
    }
}

