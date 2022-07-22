#ifndef WHEEL_H
#define WHEEL_H

#include "tools.h"

/*底盘控制器*/
typedef struct {
    Motor *left_motor;
    Motor *right_motor;
} WheelController;

/*PWM方式初始化底盘控制器*/
void InitWheelController_PWM(WheelController *wheel_controller);
/*底盘行进控制*/
void Move(WheelController *wheel_controller, float turn, float advance);
/*停止*/
void Stop(WheelController *wheel_controller);

////*测试接口*////
/*GPIO方式初始化底盘控制器（测试用）*/
void InitWheelController_GPIO(WheelController *wheel_controller);
/*全速前进（测试用）*/
void SetFullSpeed(WheelController *wheel_controller);
/*设置单电机油门（测试用）*/
void SetThrottle(Motor *motor, float val);

#endif //WHEEL_H