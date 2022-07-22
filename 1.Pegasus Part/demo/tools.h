#ifndef TOOLS_H
#define TOOLS_H

#include "iot_gpio_ex.h"
#include "iot_gpio.h"
#include "hi_pwm.h"
#include "iot_pwm.h"
#include "hi_time.h"

#define PIN_GPIO_MODE 0
#define PIN_PWM_MODE 5


/*GPIO信号上拉、下拉信息*/
typedef enum {
    /** No pull */
    IOT_GPIO_PULL_NONE,
    /** Pull-up */
    IOT_GPIO_PULL_UP,
    /** Pull-down */
    IOT_GPIO_PULL_DOWN,
    /** Maximum value */
    IOT_GPIO_PULL_MAX,
} IotGpioPull;


/*引脚信息*/
typedef struct {
    unsigned int gpio_id;
    hi_pwm_port pwm_id;
} PIN;

/*电机*/
typedef struct {
    PIN *pin1;
    PIN *pin2;
} Motor;

/*蓝牙遥控命令*/
typedef enum{
    COMMAND_STOP,
    COMMAND_START
}BTCommand;

/*初始化引脚，PWM模式*/
void InitPinPWM(PIN *pin);
/*初始化引脚，GPIO输出模式*/
void InitPinOutGPIO(PIN *pin);
/*初始化引脚，GPIO输入模式*/
void InitPinInGPIO(PIN *pin, IotGpioPull pull);


#endif //TOOLS_H