#include "mbed.h"
#include <string>
#include "HTTPServer.h"
#include "WebsocketHandlers.h"
#include "HTTPHandlers.h"
#include "globalVars.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

using namespace std;

const int OFF = 0;
const int ON  = 1;

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

    NetworkInterface *net;
    net = NetworkInterface::get_default_instance();
    net->set_blocking(true);
    if (!net) {
        printf("Error! No network inteface found.\n");
        return 0;
    }
    net->connect();

    HttpServer server(net, 3, 1);               // max 5 threads, 4 websockets

    server.addStandardHeader("Server", "JojoS_Mbed_Server");
    server.addStandardHeader("DNT", "1");

    server.setHTTPHandler("/", &request_handler);
    server.setHTTPHandler("/stats/", &request_handler_getStatus);
    
    server.setWSHandler("/ws/", WSHandler::createHandler);

    nsapi_error_t res = server.start(80);

    if (res == NSAPI_ERROR_OK) {
        SocketAddress socketAddress;
        net->get_ip_address(&socketAddress);
        printf("Server is listening at http://%s:80\n", socketAddress.get_ip_address());
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

