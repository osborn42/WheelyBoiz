#include "esp_system.h"
#include "esp_log.h"
// This example needs rtc driver to read button state
#include "driver/rtc_io.h"
#include <driver/adc.h>
#include "espnow_basic_config.h"

#define GPIO_INPUT_0	14   //horn
#define GPIO_INPUT_1	12   //lights

static const char *TAG = "My_Slave";
int switchFlickedState = 0;
int history = 0;

// Your function to populate a my_data_t to send
void my_data_populate(my_data_t *data)
{

/*  If need to configure the adc channels here is some code
    adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_11db);  //xCar
	adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db);  //yCar
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_11db);  //xCam
    
*/
    ESP_LOGI(TAG, "Populating my_data_t");

    data->button_pushed = gpio_get_level(GPIO_INPUT_0);
    printf("test: %d \n", gpio_get_level(GPIO_INPUT_0));

    int yvalue = adc1_get_raw(ADC1_CHANNEL_7);  //xCar
    int xvalue = adc1_get_raw(ADC1_CHANNEL_6);  //yCar
    int xvalue2 = adc1_get_raw(ADC1_CHANNEL_4); //xCam

    printf("xvalue: %d\n", xvalue);
    printf("yvalue: %d\n", yvalue);
    printf("xvalue2: %d\n", xvalue2);

    
    if(gpio_get_level(GPIO_INPUT_1) == 1){
    	if(history == 1){
    		history = 0;
    	}
    	if(history == 0){
    		history = 1;
    	}
        switchFlickedState = history;
    }
    
    data->switch_flicked = switchFlickedState;

    data->xcar = xvalue;
    data->ycar = yvalue;
    data->xcam = xvalue2;

}
