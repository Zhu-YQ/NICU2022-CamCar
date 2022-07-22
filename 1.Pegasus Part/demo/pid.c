#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "pid.h"

int frame_height, frame_width;

/*初始化PID控制器*/
void InitPIDController(PIDController* controller, int target, float kp, float ki, float kd, int delay_time){
    controller->target = target;
    controller->kp = kp;
    controller->ki = ki;
    controller->kd = kd;
    controller->delay_time = delay_time;
    controller->integral = 0;
    controller->last_error = 0;
}

void ResetPIDController(PIDController* controller){
    controller->integral = 0;
    controller->last_error = 0;
}

int GetApproximatelyError(int error_tolerance, int target, int new_val){
    int error = target - new_val;
    if(abs(error) < error_tolerance){
        return 0;
    }
    else{
        return error;
    }
}

/*获取PID控制器输出值*/
float GetPIDResult(PIDController* controller, int new_val, int error_tolerance, int d_time, bool show_info){
    int error = GetApproximatelyError(error_tolerance, controller->target, new_val) / 10;
    if(controller->integral + error > 666666666){
        controller->integral = 666666666;
    }
    else if(controller->integral + error < -666666666){
        controller->integral = -666666666;
    }
    else{
        controller->integral += error;
    }
    
    int derivative = (error - controller->last_error) / d_time;

    controller->last_error = error;

    float p_result = controller->kp * (float)error;
    float i_result = controller->ki * (float)controller->integral;
    float d_result = controller->kd * (float)derivative;

    if(show_info){
        printf("error: %d ", error);
        printf("p_result: %f\n", p_result);

        printf("integral: %d ", controller->integral);
        printf("i_result: %f\n", i_result);

        printf("derivative:%d ", derivative);
        printf("d_result: %f\n", d_result);
    }
    
    return (p_result + i_result + d_result) / 1000;
}

/*初始化设定捕获图像总大小*/
void InitFrameSize(int height, int width){
    frame_height = height;
    frame_width = width;
}

/*计算中心点位置*/
void GetCentralPoint(const int *top, const int *left, const int *bottom, const int *right, CentralPoint* point){
    point->x = (int)(((*left) + (*right)) / 2);
    point->y = (int)(((*top) + (*bottom)) / 2);
}

/*打印中心点信息*/
void PrintPoint(CentralPoint* point){
    printf("central point: (%d,%d) ", point->x, point->y);
}

/*以画面中心为原点进行换算*/
void ChangeOrigin(CentralPoint* point){
    /*画面左至右X增大，上之下Y增大
     *舵机左转为负参数，上仰是负参数
    */
    point->x -= frame_width / 2;
    point->y -= frame_height / 2;
}

/*计算预测框大小*/
int CalculateArea(const int *top, const int *left, const int *bottom, const int *right){
    //printf("%d, %d, %d, %d\n", *top, *left, *bottom, *right);
    int dx = *right - *left;
    int dy = *bottom - *top;
    return dx * dy;
}

/*获取人体信息*/
void GetPersonInfo(const int *top, const int *left, const int *bottom, const int *right, PersonInfo* person_info){
    //中心点信息
    CentralPoint point_before_change;//坐标系变换前的中心点
    GetCentralPoint(top, left, bottom, right, &point_before_change);
    ChangeOrigin(&point_before_change);
    person_info->central_point = point_before_change;//更新中心点信息
    //预测框信息
    person_info->height = *bottom - *top;
    person_info->width = *right - *left;
    int area = CalculateArea(top, left, bottom, right);
    person_info->area = area;
}

/*打印人体预测框信息*/
void PrintPersonInfo(PersonInfo* person_info){
    PrintPoint(&(person_info->central_point));
    printf("  area: %d\n", person_info->area);
}

/*从数据缓冲区提取总位置信息*/
void ExtractData(const unsigned char* read_buffer, unsigned char* data){
    for(int i=0;i<22;i++){
        unsigned char read_buffer_char = read_buffer[i];
        data[i] = read_buffer_char;
        if(read_buffer_char == '\0'){
            return;
        }
    }
}

/*分割字符串，提取位置信息数值（与PC端file.c中实现区分）*/
int PIDCutString(const unsigned char* src, size_t start, size_t end){
    unsigned char dest[6];
    for (size_t i = start; i < end; i++) {
        dest[i-start] = src[i];
    }
    dest[end-start] = '\0';
    return atoi((char*)dest);
}

/*均值滤波*/
int GetMeanFilterResult(int new_value, int index){
    static int last_values[] = {-1, -1, -1, -1};//数据记录
    if(last_values[index] == -1){
        return new_value;
    }
    int result = (last_values[index] + new_value) / 2;
    last_values[index] = new_value;
    return result;
}

/*从字符串位置结果信息拆分整形位置信息*/
void ExtractPositionInfos(const unsigned char *data, int *top, int *left, int *bottom, int *right) {
    //data格式：使用空格分割数字，使用\0作为结束符
    size_t n_index_list[4];
    size_t n_index_list_index = 0;
    size_t n_index = 0;
    //获取换行符位置
    while (1) {
        unsigned char current_char = data[n_index];
        if (current_char == '\0') {
            n_index_list[n_index_list_index] = n_index;
            break;
        } else if (current_char == ' ') {
            n_index_list[n_index_list_index] = n_index;
            n_index_list_index++;
        }
        n_index++;
    }
    unsigned int new_top = PIDCutString(data, 0, n_index_list[0]);
    unsigned int new_left = PIDCutString(data, n_index_list[0] + 1, n_index_list[1]);
    unsigned int new_bottom = PIDCutString(data, n_index_list[1] + 1, n_index_list[2]);
    unsigned int new_right = PIDCutString(data, n_index_list[2] + 1, n_index_list[3]);
    //异常信息矫正
    //图像大小：1920X1080
    new_right = frame_width>=new_right?new_right:frame_width;
    new_bottom = frame_height>=new_bottom?new_bottom:frame_height;
    
    //若信息错误
    if((new_top == new_bottom) || (new_left == new_right) ){
        return;
    }
    else{
        *top = new_top;
        *left = new_left ;
        *bottom = new_bottom;
        *right = new_right;
    }
}

/*判断云台水平方向旋转是否已达到极限*/
#define ARM_H_TURN_LIMIT 0.8f
bool ArmHTurnReachesLimit(float new_arm_h_turn, float last_arm_h_turn){
    if(fabsf(last_arm_h_turn) >= ARM_H_TURN_LIMIT){
        if(new_arm_h_turn > 0.0f && last_arm_h_turn > 0.0f && fabsf(new_arm_h_turn - last_arm_h_turn) < 0.2f){
            return true;
        }
        else if(new_arm_h_turn < 0.0f && last_arm_h_turn < 0.0f && fabsf(new_arm_h_turn - last_arm_h_turn) < 0.2f){
            return true;
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
}