#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "iot_gpio.h"
#include "iot_pwm.h"
#include "hi_pwm.h"

#include "iot_errno.h"
#include "hi_stdlib.h"

#include "uart.h"
#include "tools.h"
#include "wheel.h"
#include "arm.h"
#include "pid.h"

#define FRAME_HEIGHT 1080 
#define FRAME_WIDTH 1920  

/*全局变量*/
/*底盘*/
//左轮电机
PIN wheel_left_motor_pin1 = {10, HI_PWM_PORT_PWM1};
PIN wheel_left_motor_pin2 = {5, HI_PWM_PORT_PWM2};
Motor left_motor = {&wheel_left_motor_pin1, &wheel_left_motor_pin2};
//右轮电机
PIN wheel_right_motor_pin1 = {14, HI_PWM_PORT_PWM5};
PIN wheel_right_motor_pin2 = {13, HI_PWM_PORT_PWM4};
Motor right_motor = {&wheel_right_motor_pin1, &wheel_right_motor_pin2};
//底盘控制器
WheelController wheel_controller = {&left_motor, &right_motor};

/*云台*/
Servo arm_h_servo = {6};//水平转动（下方）舵机
Servo arm_v_servo = {7};//垂直转动（上方）舵机
//云台控制器
ArmController arm_controller = {&arm_h_servo, &arm_v_servo};

/*蓝牙遥控*/
PIN command_pin = {2, -1};

/*PID参数*/
#define DELAY_TIME 100//延迟时间，单位：ms
time_t last_time = 0;
time_t new_time = 0;
time_t d_time = 0;
PersonInfo new_info;//信息记录

//*检测框面积信息参数*//
#define TARGET_AREA (int)(0.3 * FRAME_HEIGHT * FRAME_WIDTH)//目标检测框面积大小
//面积控制PID控制器
PIDController wheel_advance_controller;
#define WHEEL_ADVANCE_KP (0.025f)
#define WHEEL_ADVANCE_KI (0.0f)
#define WHEEL_ADVANCE_KD (0.0f)

//*检测框中心点信息参数*//
#define TARGET_X 0
#define TARGET_Y 0
//中心点控制PID控制器
PIDController wheel_turn_controller;
#define WHEEL_TURN_KP (6.0f)
#define WHEEL_TURN_KI (0.0f)
#define WHEEL_TURN_KD (0.15f)

PIDController arm_h_turn_controller;
#define ARM_H_TURN_KP (3.0f)
#define ARM_H_TURN_KI (0.017f)
#define ARM_H_TURN_KD (0.68f)

PIDController arm_v_turn_controller;
#define ARM_V_TURN_KP (3.9f)
#define ARM_V_TURN_KI (0.0f)
#define ARM_V_TURN_KD (0.5f)

///*执行器参数*///
/*底盘控制参数，Move函数参数*/
float wheel_turn = 0.0f;
float wheel_advance = 0.0f;
/*云台控制参数，Aim函数参数*/
float arm_h_turn = 0.0f; 
float arm_v_turn = 0.0f;
float d_arm_h_turn = 0.0f;
float d_arm_v_turn = 0.0f;

/*串口通讯变量*/
int uart_flag = 0;
unsigned char* uart_data[20];

float test1;
float test2;

//循环流程主函数
static void DemoMain(){

    /*传感器初始化*/
    InitFrameSize(FRAME_HEIGHT, FRAME_WIDTH);//初始化画面总大小参数

    /*控制器初始化*/
    InitPIDController(&wheel_advance_controller, TARGET_AREA, WHEEL_ADVANCE_KP, WHEEL_ADVANCE_KI, WHEEL_ADVANCE_KD, DELAY_TIME);
    InitPIDController(&wheel_turn_controller, TARGET_X, WHEEL_TURN_KP, WHEEL_TURN_KI, WHEEL_TURN_KD, DELAY_TIME);
    InitPIDController(&arm_h_turn_controller, TARGET_X, ARM_H_TURN_KP, ARM_H_TURN_KI, ARM_H_TURN_KD, DELAY_TIME);
    InitPIDController(&arm_v_turn_controller, TARGET_Y, ARM_V_TURN_KP, ARM_V_TURN_KI, ARM_V_TURN_KD, DELAY_TIME);
    new_time = time(NULL);
    last_time = new_time;

    /*执行器初始化*/
    BTCommand command = COMMAND_STOP;//蓝牙控制命令
    InitPinInGPIO(&command_pin, IOT_GPIO_PULL_DOWN);//初始化蓝牙信号引脚，默认下拉
    InitWheelController_PWM(&wheel_controller);//初始化底盘执行控制器
    InitArmController(&arm_controller);//初始化云台执行控制器

    /*串口通讯初始化*/
    InitUart();//初始化串口配置
    UartTransmit();//新建线程以查询方式读取串口数据

    /*位置信息*/
    int top = -1; int left = -1; int bottom = -1; int right = -1;

    while (1){

        IoTGpioGetInputVal(command_pin.gpio_id, &command);
        if (uart_flag == 0) { continue; }//检查传感器数据
        /*更新计时器*/
        new_time = time(NULL);
        d_time = new_time - last_time;
        ExtractPositionInfos(uart_data, &top, &left, &bottom, &right);//更新坐标信息
        uart_flag = 0;
        //printf("data: top: %d, left: %d, bottom: %d, right: %d\n", top, left, bottom, right);
        GetPersonInfo(&top, &left, &bottom, &right, &new_info);

        //printf("_________________________\n");
        //printf("SENSOR INFO:\n");
        //printf("target area: %d, area: %d, d_area: %d\n", TARGET_AREA, new_info.area, TARGET_AREA - new_info.area);
        //printf("target x: %d, x: %d, d_x: %d\n", TARGET_X, new_info.central_point.x, TARGET_X - new_info.central_point.x);
        //printf("target y: %d, y: %d, d_y: %d\n", TARGET_Y, new_info.central_point.y, TARGET_Y - new_info.central_point.y);
        //printf("_________________________\n");

        /*若当前信号为停止跟踪，只有云台移动*/
        if (command == COMMAND_STOP){
            printf("mode1\n");
            //控制器重置
            ResetPIDController(&wheel_advance_controller); ResetPIDController(&wheel_turn_controller);
            Stop(&wheel_controller);

            //从PID控制器获取参数
            /*云台水平旋转参数获取*/
            d_arm_h_turn = -GetPIDResult(&arm_h_turn_controller, new_info.central_point.x, 0, d_time, false);
            arm_h_turn += d_arm_h_turn;
            //极限值限制
            arm_h_turn = arm_h_turn > 0.9f ? 0.9f : arm_h_turn;
            arm_h_turn = arm_h_turn < -0.9f ? -0.9f : arm_h_turn;
            /*云台垂直旋转参数获取*/
            d_arm_v_turn = -GetPIDResult(&arm_v_turn_controller, new_info.central_point.y - 40, (int)(0.02 * FRAME_HEIGHT), d_time, false);
            arm_v_turn += d_arm_v_turn;
            //极限值限制
            arm_v_turn = arm_v_turn > 0.4f ? 0.4f : arm_v_turn;
            arm_v_turn = arm_v_turn < -0.5f ? -0.5f : arm_v_turn;

            //printf("_________________________\n");
            //printf("CONTROL INFO:\n");
            //printf("arm_h_turn: %f\n", arm_h_turn);
            //printf("arm_h_turn change:%f\n", d_arm_h_turn);
            //printf("arm_v_turn: %f\n", arm_v_turn);
            //printf("arm_v_turn change:%f\n", d_arm_v_turn);
            //printf("_________________________\n");

            Aim(&arm_controller, arm_h_turn, arm_v_turn);

            hi_udelay(DELAY_TIME * 1000);
            last_time = new_time;
            continue;
        }
        
        else{
            /*若当前信号为开始跟踪，只有底盘移动*/
            printf("mode2\n");
            //控制器重置
            ResetPIDController(&arm_h_turn_controller); ResetPIDController(&arm_v_turn_controller);
            Aim(&arm_controller, 0.0f, -0.5f);

            //从PID控制器获取参数
            /*底盘前进参数获取*/
            wheel_advance = GetPIDResult(&wheel_advance_controller, new_info.area, (int)(0.25 * wheel_advance_controller.target), new_time - last_time, false);
            //极限值限制
            wheel_advance = wheel_advance > 1.0f ? 1.0f : wheel_advance;
            wheel_advance = wheel_advance < -1.0f ? -1.0f : wheel_advance;
            wheel_advance = fabsf(wheel_advance) < 0.2f ? 0.0f : wheel_advance;
            /*底盘旋转参数获取*/
            wheel_turn = -GetPIDResult(&wheel_turn_controller, new_info.central_point.x, (int)(0.09 * FRAME_WIDTH), d_time, true);
            //极限值限制
            wheel_turn = fabsf(wheel_turn) < 0.15f ? 0.0f : wheel_turn;

            Stop(&wheel_controller);
            Move(&wheel_controller, wheel_turn, wheel_advance);

            //printf("_________________________\n");
            //printf("CONTROL INFO:\n");
            //printf("wheel_advance: %f\n", wheel_advance);
            //printf("wheel_turn: %f\n", wheel_turn);
            //printf("test1:%f, test2:%f\n", test1, test2);
            //printf("_________________________\n");

            hi_udelay(DELAY_TIME * 1000);
            last_time = new_time;
            continue;
        }
    }
}

static void DemoEntry(void)
{
    osThreadAttr_t attr;

    IoTWatchDogDisable();
    
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 65536;
    attr.priority = osPriorityNormal;

    if (osThreadNew(DemoMain, NULL, &attr) == NULL) {
        printf("[OledDemo] Falied to create OledmentTask!\n");
    }

}
SYS_RUN(DemoEntry);