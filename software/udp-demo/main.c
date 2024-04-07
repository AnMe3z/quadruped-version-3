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

#define DEVICE_ID 0 // or 1

int led;

int commands[18];

struct instruction {
        int ff;
        int fk;
        int bf;
        int bk;
};
struct instruction inst;

void process_data(){
        if (DEVICE_ID == 0) {
                inst.ff = 10*commands[1] + commands[2];
                inst.fk = 10*commands[3] + commands[4];
                inst.bf = 10*commands[5] + commands[6];
                inst.bk = 10*commands[7] + commands[8];
                printf("inst.ff %d ", inst.ff);
                printf("inst.fk %d ", inst.fk);
                printf("inst.bf %d ", inst.bf);
                printf("inst.bk %d ", inst.bk);
        }
        else {
                inst.ff = 10*commands[10] + commands[11];
                inst.fk = 10*commands[12] + commands[12];
                inst.bf = 10*commands[13] + commands[13];
                inst.bk = 10*commands[14] + commands[15];
                printf("inst.ff %d ", inst.ff);
                printf("inst.fk %d ", inst.fk);
                printf("inst.bf %d ", inst.bf);
                printf("inst.bk %d ", inst.bk);
        }
}

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
        if (p != NULL) {
                // Received packet buffer 'p' is not NULL
                // You can process the packet here
                char *packet_data = (char *)p->payload;
                printf("Received UDP message: %s\n", packet_data);
                
                if (strlen(packet_data) == 36) {
                        int i;
                        
                        while (packet_data[i] != '\0' && packet_data[i+1] != '\0') {
                                commands[i/2] = ((packet_data[i] - '0') * 10 + (packet_data[i+1] - '0')) - 30;
                                printf("%d\n", commands[i/2]);
                                i += 2;
                        }
                        
                        process_data();
                }
                else {
                        printf("ERROR: Message is not 36 characters long\n");
                }

                //return 0;
                
                led = led ^ 1;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);

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
        led = led ^ 1;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);
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

