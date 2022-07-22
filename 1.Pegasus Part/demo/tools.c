#include "tools.h"


void InitPinPWM(PIN *pin){
    IoTGpioInit(pin->gpio_id);
    IoSetFunc(pin->gpio_id, PIN_PWM_MODE);
    IoTGpioSetDir(pin->gpio_id, IOT_GPIO_DIR_OUT);
    IoTPwmInit(pin->pwm_id);
}

void InitPinOutGPIO(PIN *pin){
    IoTGpioInit(pin->gpio_id);
    IoSetFunc(pin->gpio_id, PIN_GPIO_MODE);
    IoTGpioSetDir(pin->gpio_id, IOT_GPIO_DIR_OUT);
}

void InitPinInGPIO(PIN *pin, IotGpioPull pull){
    IoTGpioInit(pin->gpio_id);
    IoSetFunc(pin->gpio_id, PIN_GPIO_MODE);
    IoTGpioSetDir(pin->gpio_id, IOT_GPIO_DIR_IN);
    IoSetPull(pin->gpio_id, pull);
}
