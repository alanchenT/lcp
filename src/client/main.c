#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client/app.h"
#include "client/client.h"
#include "client/gui.h"
#include "client/net_callbacks.h"
#include "debug.h"
#include "packet_queue.h"
#include "packets.h"

int main() {
    setenv("GSK_RENDERER", "cairo", 1);

    App* app = alloc_app();
    if (app == NULL) {
        fprintf(stderr, "client: failed to alloc app\n");
        exit(1);
    }

    app_start(app);
    app_cleanup(app);

    return 0;
}