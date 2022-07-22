#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include "arm.h"
#include "iot_i2c.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"

/*舵机控制高电平标准时间（单位：us）*/
//水平（下方）舵机
#define DUTY_TIME_MIN_H 500//最右侧
#define DUTY_TIME_MAX_H 2100//最左侧
#define DUTY_TIME_CENTER_H 1300//中间
//垂直（上方）舵机
#define DUTY_TIME_MIN_V 1800//最前侧
#define DUTY_TIME_MAX_V 2500//最后侧
#define DUTY_TIME_CENTER_V 2150//中间

void InitArmController(ArmController *arm_contorller){
    //水平方向舵机
    IoTGpioInit(*(arm_contorller->h_servo));
    IoSetFunc(*(arm_contorller->h_servo), PIN_GPIO_MODE);
    IoTGpioSetDir(*(arm_contorller->h_servo), IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(*(arm_contorller->h_servo), 1);
    //垂直方向舵机
    IoTGpioInit(*(arm_contorller->v_servo));
    IoSetFunc(*(arm_contorller->v_servo), PIN_GPIO_MODE);
    IoTGpioSetDir(*(arm_contorller->v_servo), IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(*(arm_contorller->v_servo), 1);
}

void SetAngle(Servo *servo, unsigned int duty_time){
    //舵机标准：周期20ms，角度对应高电平时间范围[0.5ms,2.5ms]
    unsigned int period = 20000;
    //unsigned int duty_time = (int)((0.5 + (float)angle/180*2) * 1000);
    for (unsigned int i = 0; i < 8; i++){
        IoTGpioSetOutputVal(*servo, 1);
        hi_udelay(duty_time);
        IoTGpioSetOutputVal(*servo, 0);
        hi_udelay(period - duty_time);
    }
}

/*由转向系数转换为高电平时间*/
unsigned int FromTurn2DutyTime_H(float turn){
    float distance = DUTY_TIME_MAX_H - DUTY_TIME_CENTER_H;
    distance *= -turn;
    return DUTY_TIME_CENTER_H + (int)distance;
}
unsigned int FromTurn2DutyTime_V(float turn){
    float distance = DUTY_TIME_MAX_V - DUTY_TIME_CENTER_V;
    distance *= -turn;
    return DUTY_TIME_CENTER_V + (int)distance;
}

float AdjustTurn(float turn){
    if(turn > 1.0f){
        return 1.0f;
    }
    else if(turn < -1.0f){
        return -1.0f;
    }
    else{
        return turn;
    }
}

//判断是否改变过小
bool IsApproximatelyEqual(float new_val, float last_val){
    if (fabsf(new_val - last_val) < 0.01){
        return true;
    }
    else{
        return false;
    }
}


void Aim(ArmController *arm_controller, float h_turn, float v_turn){
    static float last_h_turn = 0.0f;
    static float last_v_turn = 0.0f;
    h_turn = AdjustTurn(h_turn);
    v_turn = AdjustTurn(v_turn);
    //若云台驱动值差别不过小，执行舵机驱动，否则不执行
    if(!IsApproximatelyEqual(h_turn, last_h_turn)){
        unsigned int h_duty_time = FromTurn2DutyTime_H(h_turn);
        SetAngle(arm_controller->h_servo, h_duty_time);
        last_h_turn = h_turn;
    }
    if(!IsApproximatelyEqual(v_turn, last_v_turn)){
        unsigned int v_duty_time = FromTurn2DutyTime_V(v_turn);
        SetAngle(arm_controller->v_servo, v_duty_time);
        last_v_turn = v_turn;
    }
}
