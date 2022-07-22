#include "wheel.h"
#include "hi_pwm.h"
#include "iot_pwm.h"
#include "iot_gpio.h"
#include "math.h"

/*电机转速矫正参数*/
float wheel_motor_forward_PWM_bias[2] = {0, -0.06f};
float wheel_motor_backward_PWM_bias[2] = {0.06f, 0};

void InitWheelController_PWM(WheelController *wheel_controller){
    InitPinPWM(wheel_controller->left_motor->pin1);
    InitPinPWM(wheel_controller->left_motor->pin2);
    InitPinPWM(wheel_controller->right_motor->pin1);
    InitPinPWM(wheel_controller->right_motor->pin2);
}

void InitWheelController_GPIO(WheelController *wheel_controller){
    InitPinOutGPIO(wheel_controller->left_motor->pin1);
    InitPinOutGPIO(wheel_controller->left_motor->pin2);
    InitPinOutGPIO(wheel_controller->right_motor->pin1);
    InitPinOutGPIO(wheel_controller->right_motor->pin2);
}

void SetFullSpeed(WheelController *wheel_controller){
    //左正转
    IoTGpioSetOutputVal(wheel_controller->left_motor->pin1->gpio_id, 0);
    IoTGpioSetOutputVal(wheel_controller->left_motor->pin2->gpio_id, 1);
    //右反转
    IoTGpioSetOutputVal(wheel_controller->right_motor->pin1->gpio_id, 1);
    IoTGpioSetOutputVal(wheel_controller->right_motor->pin2->gpio_id, 0);
}

void SetThrottle(Motor *motor, float val) {
    //hi_pwm_stop(motor->pin1->pwm_id);
    //hi_pwm_stop(motor->pin2->pwm_id);
    unsigned int freq = 1500;
    if(val == 0){
        return;
    }
    else if(val > 0){
        //正转
        hi_pwm_start(motor->pin2->pwm_id, (int)(val * freq), freq);
    }
    else{
        //反转
        val = -val;
        hi_pwm_start(motor->pin1->pwm_id, (int)(val * freq), freq);
    }

}

/*调整油门值以使电机正常运转*/
float AdjustThrottle(float throttle_val) {
    // if (throttle_val > 1) {
    //     return 1;
    // } else if (throttle_val < -1) {
    //     return -1;
    // } else {
    //     //防止油门过小
    //     float throttle_thresh = 0.25f;
    //     if (fabsf(throttle_val) >= throttle_thresh) {
    //         return throttle_val;
    //     }
    //     //若油门过小
    //     else {
    //         if (throttle_val < 0) {
    //             return -throttle_thresh;
    //         } else {
    //             return throttle_thresh;
    //         }
    //     }
    // }
    if(throttle_val > 1.0f){
        return 1.0f;
    }
    if(throttle_val < -1.0f){
        return -1.0f;
    }
    if(fabsf(throttle_val) < 0.25f){
        return 0.0f;
    }
    return throttle_val;
}

/*由旋转量与前进量计算油门值*/
float GetThrottle(float turn, float advance) {
    //printf("turn:%f, advance:%f\n", turn, advance);
    float throttle_val = turn + advance;
    throttle_val = AdjustThrottle(throttle_val);
    return throttle_val;
}

extern float test1;
extern float test2;

void Move(WheelController *wheel_controller, float turn, float advance) {
    //左电机驱动
    float left_throttle_val = GetThrottle(turn, advance);
    test1 = left_throttle_val;
    //printf("left throttle: %f\n", left_throttle_val);
    SetThrottle(wheel_controller->left_motor, left_throttle_val);
    //右电机驱动
    float right_throttle_val = GetThrottle(-turn, advance);
    test2 = right_throttle_val;
    //printf("right throttle: %f\n", right_throttle_val);
    SetThrottle(wheel_controller->right_motor, right_throttle_val);
}

void Stop(WheelController *wheel_controller){
    hi_pwm_stop(wheel_controller->left_motor->pin1->pwm_id);
    hi_pwm_stop(wheel_controller->left_motor->pin2->pwm_id);
    hi_pwm_stop(wheel_controller->right_motor->pin1->pwm_id);
    hi_pwm_stop(wheel_controller->right_motor->pin2->pwm_id);
}