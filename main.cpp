#include <unistd.h>
#include "server/webserver.h"

int main() {
    WebServer server(1316, 3, 60000, false, 4);            
    server.Start();
} 