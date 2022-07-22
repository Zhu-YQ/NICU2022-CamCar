#ifndef ARM_H
#define ARM_H

#include <stdbool.h>
#include "tools.h"

typedef unsigned int Servo;
typedef struct{
    Servo *h_servo;//水平方向，底座舵机
    Servo *v_servo;//垂直方向，竖直舵机
} ArmController;

/*初始化云台控制器*/
void InitArmController(ArmController *arm_contorller);
/*调整云台姿态*/
void Aim(ArmController *arm_controller, float h_turn, float v_turn);
//判断是否改变过小
bool IsApproximatelyEqual(float new_val, float last_val);

////*测试接口*////
/*设置舵机角度*/
void SetAngle(Servo *servo, unsigned int angle);

#endif //ARM_H