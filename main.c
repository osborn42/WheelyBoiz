/* Basic ESPNOW Example - Master

   This is a very basic example of receiving data from the complementary Slave example.
   By default, it receives an unencrypted data packet containing a random value and the state of a button from the slave, via the broadcast mac address.

   To customise for your data:
    - Alter my_data_t in espnow_basic_config
    - Alter my_data_receive my_master.c
    - Alter my_data_populate my_slave.c

   For basic demonstration, do not edit this file
*/

#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"

#include "sdkconfig.h"

#include "math.h"

#include "espnow_basic_config.h"

#include "driver/ledc.h"



static const char *TAG = "Basic_Master";

void my_data_receive(const uint8_t *sender_mac_addr, const my_data_t *data);

static xQueueHandle s_recv_queue;

typedef struct {
    uint8_t sender_mac_addr[ESP_NOW_ETH_ALEN];
    my_data_t data;
} recv_packet_t;



static void queue_process_task(void *p)
{
	//"static" when applied to a variable declared inside a function, the value of that variable will be preserved between function calls.
    static recv_packet_t recv_packet;

    ESP_LOGI(TAG, "Listening");

    for(;;)
    {
    	//xQueueReceive:
    	//s_recv_queue = the handle to the queue from which the item is to be received
    	//&recv_packet = pointer to the buffer into which the received item will be copied.
    	//portMAX_DELAY = The maximum amount of time the task should block waiting for an item to receive should the queue be empty at the time of the call.
    	//Returns pdTRUE if an item was successfully received from the queue, otherwise pdFALSE

        if(xQueueReceive(s_recv_queue, &recv_packet, portMAX_DELAY) != pdTRUE)
        {
            continue;
        }

        // Refer to user function in my_master.c
        //This function only gets data if xQueueReceive got an item successfully
        my_data_receive(recv_packet.sender_mac_addr, &recv_packet.data);
    }
}


#define MY_ESPNOW_WIFI_MODE WIFI_MODE_STA
#define MY_ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
// #define MY_ESPNOW_WIFI_MODE WIFI_MODE_AP
// #define MY_ESPNOW_WIFI_IF   ESP_IF_WIFI_AP




//Callback function of receiving ESPNOW data
//mac_addr = peer MAC address
//data = received data
//len = length of received data
static void recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
	//"static" when applied to a variable declared inside a function, the value of that variable will be preserved between function calls.
    static recv_packet_t recv_packet;

    ESP_LOGI(TAG, "%d bytes incoming from " MACSTR, len, MAC2STR(mac_addr));
    
    if(len != sizeof(my_data_t))
    {
        ESP_LOGE(TAG, "Unexpected data length: %d != %u", len, sizeof(my_data_t));
        return;
    }

    //Copy the content of mac_addr into recv_packet.sender_mac_addr
    memcpy(&recv_packet.sender_mac_addr, mac_addr, sizeof(recv_packet.sender_mac_addr));

    //Copy the content of data into recv_packet.data
    memcpy(&recv_packet.data, data, len);



    //xQueueSend:Post an item on a queue. The item is queued by copy, not by reference.
    //s_recv_queue = The handle to the queue on which the item is to be posted
    //recv_packet = A pointer to the item that is to be placed on the queue.
    //0 = The maximum amount of time the task should block waiting for space to become available on the queue, should it already be full.
    //The call will return immediately if the queue is full and xTicksToWait is set to 0.
    //Returns:pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
    if (xQueueSend(s_recv_queue, &recv_packet, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Queue full, discarded");
        return;
    }
}



static void init_espnow_master(void)
{
    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    //Initialize NVS (Non-volatile storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    //Initialize WiFi
    ESP_ERROR_CHECK( esp_netif_init());
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(MY_ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start() );

#if MY_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(MY_ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif

    //Initialize ESPNOW
    ESP_ERROR_CHECK( esp_now_init() );

    //Register callback function of receiving ESPNOW data (recv_cb)
    //Register a callback function that is triggered upon receiving data.
    //When data is received via ESP-NOW, recv_cb function is called.
    ESP_ERROR_CHECK( esp_now_register_recv_cb(recv_cb) );

    //Set primary master key
    ESP_ERROR_CHECK( esp_now_set_pmk((const uint8_t *)MY_ESPNOW_PMK) );
}




static void initPWM(void) {

    //CAMERA

	ledc_timer_config_t camtimer = {
	    .freq_hz = 50,                          // PWM signal frequency
		.duty_resolution = LEDC_TIMER_13_BIT,  // resolution of PWM duty

		.speed_mode = LEDC_LOW_SPEED_MODE,     // LEDC speed speed_mode, high-speed mode or low-speed mode
		.timer_num = LEDC_TIMER_1,             // The timer source of channel (0 - 3)
		.clk_cfg = LEDC_AUTO_CLK,              // Configure LEDC source clock.
	};
	ledc_timer_config(&camtimer);

	ledc_channel_config_t camchannel = {
		.gpio_num = 18,
		.duty = 0,
		//range of duty setting is [0, (2**duty_resolution)]

		.speed_mode = LEDC_LOW_SPEED_MODE,
		.channel =  LEDC_CHANNEL_1,
		.timer_sel = LEDC_TIMER_1,
		.hpoint = 0,
	};
    ledc_channel_config(&camchannel);



	//MOTORS

	ledc_timer_config_t motortimer = {
	    .freq_hz = 1000,                          // PWM signal frequency
		.duty_resolution = LEDC_TIMER_13_BIT,  // resolution of PWM duty

		.speed_mode = LEDC_LOW_SPEED_MODE,     // LEDC speed speed_mode, high-speed mode or low-speed mode
		.timer_num = LEDC_TIMER_2,             // The timer source of channel (0 - 3)
		.clk_cfg = LEDC_AUTO_CLK,              // Configure LEDC source clock.
	};
	ledc_timer_config(&motortimer);


	ledc_channel_config_t motorchannelsteer= {
		.gpio_num = 16,
		.duty = 0,
		//range of duty setting is [0, (2**duty_resolution)]

		.speed_mode = LEDC_LOW_SPEED_MODE,
		.channel =  LEDC_CHANNEL_2,
		.timer_sel = LEDC_TIMER_2,
		.hpoint = 0,
	};
    ledc_channel_config(&motorchannelsteer);


	ledc_channel_config_t motorchanneldrive = {
		.gpio_num = 17,
		.duty = 0,
		//range of duty setting is [0, (2**duty_resolution)]

		.speed_mode = LEDC_LOW_SPEED_MODE,
		.channel =  LEDC_CHANNEL_3,
		.timer_sel = LEDC_TIMER_2,
		.hpoint = 0,
	};
    ledc_channel_config(&motorchanneldrive);

}


void app_main(void)
{
	//Queues are used to send and receive messages between tasks.
	//A message queue is a kind of FIFO (first-in first-out) buffer that holds fixed-size data items.
	//Tasks write data to the end of the buffer and read from the front end of the buffer.
    s_recv_queue = xQueueCreate(10, sizeof(recv_packet_t));
    assert(s_recv_queue);


    //queue_process_task is the function that implements the task
    //"recv_tasl" is the name for the task
    //Tasks are normally implemented as an infinite loop
    //If the task was created successfully then pdPASS is returned
    BaseType_t err = xTaskCreate(queue_process_task, "recv_task", 8192, NULL, 4, NULL);
    assert(err == pdPASS);
    
    init_espnow_master();

    initPWM();

}
