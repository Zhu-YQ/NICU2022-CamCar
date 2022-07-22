#ifndef PID_PID_H
#define PID_PID_H
#include <stdbool.h>

typedef struct {
    int x;
    int y;
}CentralPoint;

typedef struct {
    CentralPoint central_point;
    int height;
    int width;
    int area;
}PersonInfo;

typedef struct {
    int target;
    float kp, ki, kd;
    int delay_time;
    int integral;
    int last_error;
}PIDController;

/*初始化PID控制器*/
void InitPIDController(PIDController* controller, int target, float kp, float ki, float kd, int delay_time);
/*重置PID控制器*/
void ResetPIDController(PIDController* controller);
/*获取PID控制器输出值*/
float GetPIDResult(PIDController* controller, int new_val, int error_tolerance, int unit_time_counter, bool show_info);
/*从数据缓冲区提取总位置信息*/
void ExtractData(const unsigned char* read_buffer, unsigned char* data);
/*从串口数据字符串提取位置信息*/
void ExtractPositionInfos(const unsigned char *data, int *top, int *left, int *bottom, int *right);
/*初始化设定捕获图像总大小*/
void InitFrameSize(int height, int width);
/*获取人体预测框信息*/
void GetPersonInfo(const int *top, const int *left, const int *bottom, const int *right, PersonInfo* person_info);
/*打印人体预测框信息*/
void PrintPersonInfo(PersonInfo* person_info);
/*判断机械臂水平方向旋转是否已达到极限*/
bool ArmHTurnReachesLimit(float new_arm_h_turn, float last_arm_h_turn);

#endif //PID_PID_H
