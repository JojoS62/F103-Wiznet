#include "mbed.h"
#include <string>
#include "easy-connect.h"
#include "HTTPServer.h"
#include "WebsocketHandlers.h"
#include "HTTPHandlers.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

using namespace std;

const int OFF = 0;
const int ON  = 1;

DigitalOut      led1(LED1);
DigitalOut      testPin(PB_8);

void print_mem() 
{
    mbed_stats_heap_t heap_info;
    mbed_stats_heap_get( &heap_info );

    printf("mem used: %8ld reserved: %8ld total: %8ld\n", heap_info.current_size, heap_info.reserved_size, heap_info.total_size); 
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
int main(void) {
    testPin = 0;
 
    printf("Hello from "  TOSTRING(TARGET_NAME) "\n");
    printf("Mbed OS version: %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    printf("SystemCoreClock %ld MHz\n\n", SystemCoreClock/1000000);

    NetworkInterface *eth;
    eth = easy_connect(true);
    if(eth == nullptr) {
        printf("ERROR: easy_connect failed\n");
    }

    HttpServer server(eth, 3, 1);               // max 5 threads, 4 websockets

    server.setHTTPHandler("/", &request_handler);
    server.setHTTPHandler("/stats/", &request_handler_getStatus);
    
    server.setWSHandler("/ws/", WSHandler::createHandler);

    nsapi_error_t res = server.start(8080);

    if (res == NSAPI_ERROR_OK) {
        //SocketAddress socketAddress;
        //eth->get_ip_address(&socketAddress);
        printf("Server is listening at http://%s:8080\n", eth->get_ip_address());
    }
    else {
        printf("Server could not be started... %d\n", res);
    }

    while(1) {
        char ch = getc(stdin);

        switch (ch) {
            case 'm': 
                print_mem();
                break;
        }
    }
}

