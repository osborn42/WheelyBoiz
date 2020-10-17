
#include "esp_log.h"
#include "espnow_basic_config.h"
#include "driver/ledc.h"
#include "math.h"

static const char *TAG = "My_Master";

#define MOTOR_DUTY_RES 13
#define CAMERA_DUTY_RES 13

#define GPIO_DIR_FORWARD 19
#define GPIO_DIR_BACKWARD 21
#define GPIO_DIR_LEFT 22
#define GPIO_DIR_RIGHT 23

#define GPIO_LIGHTS 25
#define GPIO_HORN 26



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


    int cam_duty; //This is a number not a percentage
    int cam_duty_center = .075 * pow(2,CAMERA_DUTY_RES); //Center is 7.5%
    int cam_duty_range =  .025 * pow(2,CAMERA_DUTY_RES); //Range on either side of center
    int x_val;

    int SteerPWMduty; //This is a number not a percentage
    int DrivePWMduty; //This is a number not a percentage

    float drivepercent;
    float steerpercent;
    float campercent;


    //CAMERA

    if (x_value > 1900) {
    	x_val = x_value - 1900;
    	cam_duty = cam_duty_center + cam_duty_range * (x_val / 2195);
    	cam_duty = (int)cam_duty;

        campercent = cam_duty / pow(2,CAMERA_DUTY_RES) * 100;
    	printf("Camera duty cycle = %f\n", campercent);

    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, cam_duty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    }
    else if (x_value < 1700) {
    	cam_duty = cam_duty_center - cam_duty_range * (x_value / 1700);
    	cam_duty = (int)cam_duty;

    	campercent = cam_duty / pow(2,CAMERA_DUTY_RES) * 100;
    	printf("Camera duty cycle = %f\n", campercent);

    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, cam_duty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    }
    else {
    	cam_duty = cam_duty_center;

    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, cam_duty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    	campercent = cam_duty / pow(2,CAMERA_DUTY_RES) * 100;
    	printf("duty = %f\n", campercent);
    }


    //DRIVING

    if (y_value > 2000) {
    	gpio_set_level(GPIO_DIR_FORWARD, 1);
    	gpio_set_level(GPIO_DIR_BACKWARD, 0);

    	DrivePWMduty = pow(2,MOTOR_DUTY_RES) * (y_value-2000)/2095;
    	DrivePWMduty = (int)DrivePWMduty;

        drivepercent = DrivePWMduty / pow(2,MOTOR_DUTY_RES) * 100;
    	printf("Drive duty cycle = %f\n", drivepercent);
    	printf("Forward\n");

    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, DrivePWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
    }
    else if(y_value < 1800) {
    	gpio_set_level(GPIO_DIR_FORWARD, 0);
    	gpio_set_level(GPIO_DIR_BACKWARD, 1);

    	DrivePWMduty = pow(2,MOTOR_DUTY_RES) * (1800-y_value)/1800;
    	DrivePWMduty = (int)DrivePWMduty;

        drivepercent = DrivePWMduty / pow(2,MOTOR_DUTY_RES) * 100;
    	printf("Drive duty cycle = %f\n", drivepercent);
    	printf("Backward\n");


    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, DrivePWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
    }
    else {
    	gpio_set_level(GPIO_DIR_FORWARD, 0);
    	gpio_set_level(GPIO_DIR_BACKWARD, 0);

    	DrivePWMduty = 0;

        drivepercent = DrivePWMduty / pow(2,MOTOR_DUTY_RES) * 100;
    	printf("Drive duty cycle = %f\n", drivepercent);
    	printf("Stop\n");


    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, DrivePWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
    }


    //STEERING

    if (x_value > 1900){
    	gpio_set_level(GPIO_DIR_RIGHT, 1);
    	gpio_set_level(GPIO_DIR_LEFT, 0);

    	SteerPWMduty = pow(2,MOTOR_DUTY_RES) * (x_value-1900)/2195;
    	SteerPWMduty = (int)SteerPWMduty;

        steerpercent = SteerPWMduty / pow(2,MOTOR_DUTY_RES) * 100;
    	printf("Steer duty cycle = %f\n", steerpercent);
    	printf("Right\n");


    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, SteerPWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    }
    else if(x_value < 1700){
    	gpio_set_level(GPIO_DIR_RIGHT, 0);
    	gpio_set_level(GPIO_DIR_LEFT, 1);

    	SteerPWMduty = pow(2,MOTOR_DUTY_RES) * (1700-x_value)/1700;
    	SteerPWMduty = (int)SteerPWMduty;

        steerpercent = SteerPWMduty / pow(2,MOTOR_DUTY_RES) * 100;
    	printf("Steer duty cycle = %f\n", steerpercent);
    	printf("Left\n");


    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, SteerPWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    }
    else{
    	gpio_set_level(GPIO_DIR_RIGHT, 0);
    	gpio_set_level(GPIO_DIR_LEFT, 0);

    	SteerPWMduty = 0;

        steerpercent = SteerPWMduty / pow(2,MOTOR_DUTY_RES) * 100;
    	printf("Steer duty cycle = %f\n", steerpercent);
    	printf("Straight\n");


    	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, SteerPWMduty);
    	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    }


    //LIGHTS
    if (data->button_pushed == 1) {
    	gpio_set_level(GPIO_LIGHTS, 1);
    }
    else if (data->button_pushed == 0) {
        gpio_set_level(GPIO_LIGHTS, 0);
    }


    //HORN
    if (data->switch_flicked == 1) {
        	gpio_set_level(GPIO_HORN, 1);
        }
    else if (data->switch_flicked == 0) {
            gpio_set_level(GPIO_HORN, 0);
     }


}
