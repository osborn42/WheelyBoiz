#include "esp_log.h"

#include "espnow_basic_config.h"

#include "driver/ledc.h"

#include "math.h"

static const char *TAG = "My_Master";



// Your task to handle received my_data_t
void my_data_receive(const uint8_t *sender_mac_addr, const my_data_t *data)
{
    ESP_LOGI(TAG, "Data from "MACSTR": Random Value - %u, Button - %s, Switch - %s, xcar - %u, ycar - %u",
                MAC2STR(sender_mac_addr), 
                data->random_value, 
                data->button_pushed ? "Pushed" : "Released",
                data->switch_flicked ? "Pushed" : "Released",
                data->xcar, data->ycar);


    uint32_t x_value = data->xcar;
    uint32_t y_value = data->ycar;

    int camera_duty_resolution = 13;  //If this is changed, change value in initPWM()
    int cam_duty; //This is a number not a percentage
    int cam_duty_center;  //This is a number not a percentage
    int cam_duty_range;  //This is a number not a percentage



    int motor_duty_resolution = 13;  //If this is changed, change value in initPWM()
    bool dir_forward;
    bool dir_backward;
    bool dir_left;
    bool dir_right;
    int SteerPWMduty; //This is a number not a percentage
    int DrivePWMduty; //This is a number not a percentage



	//CAMERA

    cam_duty_center = .075 * pow(2,camera_duty_resolution); //Center is 7.5%
    cam_duty_range =  .025 * pow(2,camera_duty_resolution); //Range on either side of center


    if (x_value > 1900){
    	cam_duty = cam_duty_center + cam_duty_range * (x_value / 2195);
    	cam_duty = (int)cam_duty;

    	//printf("duty = %d\n", cam_duty);

    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, cam_duty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    }
    else if(x_value < 1700){
    	cam_duty = cam_duty_center - cam_duty_range * (x_value / 1700);
    	cam_duty = (int)cam_duty;

    	//printf("duty = %d\n", cam_duty);

    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, cam_duty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    }
    else {
    	cam_duty = cam_duty_center;
    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, cam_duty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    }



    //MOTOR - STEERING

    //deadband code for x position
    if (x_value > 1900){
    	dir_right = 1;
    	dir_left = 0;
    	SteerPWMduty = pow(2,motor_duty_resolution) * (x_value-1900)/2195; // calculate % duty cycle based on change in x pos
    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, SteerPWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    }
    else if(x_value < 1700){
    	dir_left = 1;
    	dir_right = 0;
    	SteerPWMduty = pow(2,motor_duty_resolution) * (1700-x_value)/1700; // calculate % duty cycle based on change in x pos
    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, SteerPWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    }
    else{
    	dir_right = 0;
    	dir_left = 0;
    	SteerPWMduty = 0;
    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, SteerPWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    }



    //MOTOR - DRIVING

    if (y_value > 2000){ // change deadband
    	dir_forward = 1;
    	dir_backward = 0;
    	DrivePWMduty = pow(2,motor_duty_resolution) * (y_value-2000)/2095; // calculate % duty cycle based on change in y pos
    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, DrivePWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
    }
    else if(y_value < 1800){
    	dir_backward = 1;
    	dir_forward = 0;
    	DrivePWMduty = pow(2,motor_duty_resolution) * (1800-y_value)/1800; // calculate % duty cycle based on change in y pos
    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, DrivePWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
    }
    else{
    	dir_forward = 0;
    	dir_backward = 0;
    	DrivePWMduty = 0;
    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, DrivePWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
    }


}
