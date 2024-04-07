#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/udp.h"
#include "lwip/ip_addr.h"

#define UDP_SERVER_PORT 12345

#define UDP_PORT 4444
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "255.255.255.255"
#define BEACON_INTERVAL_MS 1000

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (p != NULL) {
        // Received packet buffer 'p' is not NULL
        // You can process the packet here
        char *packet_data = (char *)p->payload;
        printf("Received UDP message: %s\n", packet_data);

        // Don't forget to free the packet buffer
        pbuf_free(p);
    }
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms("andrey_shefa", "andreyshefa1", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    }

    struct udp_pcb *udp_server_pcb = udp_new();
    if (udp_server_pcb == NULL) {
        printf("Failed to create new UDP PCB.\n");
        return -1;
    }

    ip_addr_t ipaddr;
    IP4_ADDR(&ipaddr, 255,255,255,255); // Broadcast address

    err_t err = udp_bind(udp_server_pcb, &ipaddr, UDP_SERVER_PORT);
    if (err != ERR_OK) {
        printf("Failed to bind UDP server PCB.\n");
        return -1;
    }

    udp_recv(udp_server_pcb, udp_receive_callback, NULL);

    while (1) {
        // Your main program loop
    }

    cyw43_arch_deinit();
    return 0;
}

