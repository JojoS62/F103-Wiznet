#include "mbed.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#include "W5500Interface/EthernetInterface.h"
#include <string>

using namespace     std;

const int OFF = 0;
const int ON  = 1;

    /**
    * Constructor
    *
    * \param mosi mbed pin to use for SPI
    * \param miso mbed pin to use for SPI
    * \param sclk mbed pin to use for SPI
    * \param cs chip select of the WIZnet_Chip
    * \param reset reset pin of the WIZnet_Chip
    */
//    EthernetInterface(PinName mosi, PinName miso, PinName sclk, PinName cs, PinName reset);

#if defined(TARGET_LPC1768)
EthernetInterface   eth(p11, p12, p13, p8, p14);        // MOSI, MISO, SCK, CS, RESET
#elif defined(TARGET_LPC11U24)
EthernetInterface   eth(P16, P15, P13, P17, P18);       // MOSI, MISO, SCK, CS, RESET
#elif defined(TARGET_BLUEPILL_F103RB)
EthernetInterface   eth(PB_15, PB_14, PB_13, PB_12, NC);       // MOSI, MISO, SCK, CS, RESET
#elif defined(TARGET_NUCLEO_F103RB) || defined(TARGET_NUCLEO_L152RE) || defined(TARGET_NUCLEO_F030R8)  \
   || defined(TARGET_NUCLEO_F401RE) || defined(TARGET_NUCLEO_F302R8) || defined(TARGET_NUCLEO_L053R8)  \
   || defined(TARGET_NUCLEO_F411RE) || defined(TARGET_NUCLEO_F334R8) || defined(TARGET_NUCLEO_F072RB)  \
   || defined(TARGET_NUCLEO_F091RC) || defined(TARGET_NUCLEO_F303RE) || defined(TARGET_NUCLEO_F070RB)  \
   || defined(TARGET_KL25Z ) || defined(TARGET_KL46Z) || defined(TARGET_K64F) || defined(TARGET_KL05Z) \
   || defined(TARGET_K20D50M) || defined(TARGET_K22F) \
   || defined(TARGET_NRF51822) \
   || defined(TARGET_RZ_A1H)
EthernetInterface   eth(D4, D5, D3, D6, D7); // MOSI, MISO, SCK, CS, RESET

// If your board/plaform is not present yet then uncomment
// the following two lines and replace modify as appropriate.

//#elif defined(TARGET_YOUR_BOARD)
//EthernetInterface   eth(SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS, RESET); // MOSI, MISO, SCK, CS, RESET
#endif

// Note:
// If it happends that any of the SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS or RESET pins collide with LED1 pin
// then either use different SPI port (if available on the board) and change the pin names
// in the constructor spi(...) accordingly or instead of using LED1 pin, select
// a free pin (not used by SPI port) and connect to it an external LED which is connected
// to a 220 Ohm resitor that is connected to the groud.
// In the second case remember to replace LED1 in sw(LED1) constructor (see below).
// IP address must be also unique and compatible with your network. Change as appropriate.

// If instead of WIZ550io you'd like to use a W5500 module without a built-in MAC please uncommend the following line
//#define W5500   1

// The MAC number must be unique within the connected network. Modify as appropriate.
uint8_t       MY_MAC[6] = { 0xaa, 0x01, 0x10, 0xaa, 0x10, 0x10 };

const char          MY_IP[] = "192.168.1.181";
const char          MY_NETMASK[] = "255.255.255.0";
const char          MY_GATEWAY[] = "192.168.1.1";
int                 MY_PORT = 80;

TCPSocketServer     server;
TCPSocketConnection client;
bool                serverIsListening = false;

DigitalOut          sw(LED1);   // Change LED1 to a pin of your choice.
                                // However, make sure that it does not collide with any of the SPI pins
float               roomTemp = 21.8;    // A temperature sensor output

const string        PASSWORD = "secret";    // change as you like
const string        HTTP_OK = "HTTP/1.0 200 OK";
const string        MOVED_PERM = "HTTP/1.0 301 Moved Permanently\r\nLocation: ";
const string        UNAUTHORIZED = "HTTP/1.0 401 Unauthorized";

string              httpHeader;     // HTTP header
string              httpContent;    // HTTP content

/**
 * @brief   Analyses the received URL
 * @note    The string passed to this function will look like this:
 *          GET /password HTTP/1.....
 *          GET /password/ HTTP/1.....
 *          GET /password/?sw=1 HTTP/1.....
 *          GET /password/?sw=0 HTTP/1.....
 * @param   url URL string
 * @retval -1 invalid password
 *         -2 no command given but password valid
 *         -3 just refresh page
 *          0 switch off
 *          1 switch on
 */
int8_t analyseURL(string& url) {
    if(url.substr(5, PASSWORD.size()) != PASSWORD)
        return(-1);

    uint8_t pos = 5 + PASSWORD.size();

    if(url.substr(pos, 1) == " ")
        return(-2);

    if(url.substr(pos++, 1) != "/")
        return(-1);

    string  cmd(url.substr(pos, 5));

    if(cmd == "?sw=0")
        return(OFF);

    if(cmd == "?sw=1")
        return(ON);

    return(-3);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
string& movedPermanently(uint8_t flag) {
    if(flag == 1)
        httpContent = "/" + PASSWORD + "/";
    else
        httpContent = "";

    httpContent += "<h1>301 Moved Permanently</h1>\r\n";

    return(httpContent);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
string& showWebPage(uint8_t status) {
    char roomTempStr[5];

    //roomTemp = ds1820.read();
    sprintf(roomTempStr, "%3.1f", roomTemp);

    httpContent = "<h2><a href=\".\" title=\"Click to refresh the page\">Smart Home</a></h2>"; 
    httpContent += "<pre>Temperature:\t" + string(roomTempStr) + "&deg;C\r\n</pre>";

    if(status == ON) {
        httpContent += "<pre>\r\nHeating:\t<font color=#FF0000>On </font>";
        httpContent += " <a href=\"./?sw=0\"><button>Turn off</button></a>\r\n";
    }
    else {
        httpContent += "<pre>\r\nHeating:\t<font color=#999999>Off</font>";
        httpContent += " <a href=\"./?sw=1\"><button>Turn on</button></a>\r\n";
    }

    httpContent += "</pre>\r\n";
    httpContent += "<hr>\r\n";
    httpContent += "<pre>2017 ARMmbed</pre>";
    return httpContent;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void sendHTTP(TCPSocketConnection& client, string& header, string& content) {
    char    contentLeght[5] = {};

    header += "\r\nContent-Type: text/html\r\n";
    header += "Content-Length: ";
    sprintf(contentLeght, "%d", content.length());
    header += string(contentLeght) + "\r\n";
    header += "Pragma: no-cache\r\n";
    header += "Connection: close\r\n";
    header += "\r\n";

    string  webpage = header + content;
    client.send(const_cast<char*>(webpage.c_str()), webpage.length());
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void closeClient(void) {
    client.close();
    printf("Connection closed.\n\rTCP server is listening...\n\r");
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
int main(void) {
//    int     ret = eth.init(MY_MAC, MY_IP, MY_NETMASK, MY_GATEWAY);
//    int     ret = eth.init(MY_IP, MY_NETMASK, MY_GATEWAY);
    int     ret = eth.init(MY_MAC);   // use DHCP

    if(!ret) {
        printf("Initialized, MY_MAC: %s\n", eth.getMACAddress());
    }
    else {
        printf("Error eth.init() - ret = %d\n", ret);
        return -1;
    }

    ret = eth.connect();
    if (ret < 0) {
        printf("Connected, MY_IP: %s, MY_NETMASK: %s, MY_GATEWAY: %s\n",
                eth.getIPAddress(),
                eth.getNetworkMask(),
                eth.getGateway()
            );
    }

    //setup tcp socket
    if(server.bind(MY_PORT) < 0) {
        printf("TCP server bind failed.\n\r");
        return -1;
    }
    else {
        printf("TCP server bind succeeded.\n\r");
        serverIsListening = true;
    }

    if(server.listen(1) < 0) {
        printf("TCP server listen failed.\n\r");
        return -1;
    }
    else {
        printf("TCP server is listening...\n\r");
    }

    while(serverIsListening) {
        if(server.accept(client) >= 0) {
            char    buf[1024] = { };
            
            printf("Client connected!\n\rIP: %s\n\r", client.get_address());
            
            switch(client.receive(buf, 1023)) {
            case 0:
                printf("Recieved buffer is empty.\n\r");
                break;

            case -1:
                printf("Failed to read data from client.\n\r");
                break;

            default:
                string  received((char*)buf);

                if(received.substr(0, 3) != "GET") {
                    httpHeader = HTTP_OK;
                    httpContent = "<h1>200 OK</h1>";
                    sendHTTP(client, httpHeader, httpContent);
                    closeClient();
                    continue;
                }

                if(received.substr(0, 6) == "GET / ") {
                    httpHeader = HTTP_OK;
                    httpContent = "<p>Usage: http://host_or_ip/password</p>\r\n";
                    sendHTTP(client, httpHeader, httpContent);
                    closeClient();
                    continue;
                }

                int cmd = analyseURL(received);

                if(cmd == -2) {

                    // redirect to the right base url
                    httpHeader = MOVED_PERM;
                    sendHTTP(client, httpHeader, movedPermanently(1));
                    closeClient();
                    continue;
                }

                if(cmd == -1) {
                    httpHeader = UNAUTHORIZED;
                    httpContent = "<h1>401 Unauthorized</h1>";
                    sendHTTP(client, httpHeader, httpContent);
                    closeClient();
                    continue;
                }

                if(cmd == ON) {
                    sw = ON;    // turn the switch on
                }

                if(cmd == OFF) {
                    sw = OFF;   // turn the switch off
                }

                httpHeader = HTTP_OK;
                sendHTTP(client, httpHeader, showWebPage(sw));
            }
            closeClient();
        }
    }
}

