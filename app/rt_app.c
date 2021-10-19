#if defined(ZEPHYR_OS)

#elif defined(STM32L475_NO_OS)
#include <stdbool.h>
#include "stm32l4xx.h"
#include "led.h"
#include "delay.h"
#include "hotpatch/include/utils.h"
#include "hotpatch/include/profiling.h"


/*
Use PWM to control LED and Servo Motor.
*/

int16_t ts[1000];
int32_t ts_start;

static bool pwm_init = false;

TIM_HandleTypeDef TIM2_Handler;         //定时器2PWM句柄
TIM_OC_InitTypeDef TIM2_CHnHandler;	    //定时器2句柄

/**
 * @brief	TIM2 PWM输出初始化函数
 *
 * @param   arr		自动重装值
 * @param   psc		时钟预分频数
 *
 * @return  void
 */
void PWM_Init(int arr, int psc)
{
    TIM2_Handler.Instance = TIM2;          				//定时器2
    TIM2_Handler.Init.Prescaler = psc;     				//定时器分频
    TIM2_Handler.Init.CounterMode = TIM_COUNTERMODE_UP; //向上计数模式
    TIM2_Handler.Init.Period = arr;        				//自动重装载值
    TIM2_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&TIM2_Handler);       				//初始化PWM

    TIM2_CHnHandler.OCMode = TIM_OCMODE_PWM1; 			//模式选择PWM1
    TIM2_CHnHandler.Pulse = arr / 2;       				//设置比较值,此值用来确定占空比，默认比较值为自动重装载值的一半,即占空比为50%
    TIM2_CHnHandler.OCPolarity = TIM_OCPOLARITY_HIGH; 	//输出比较极性为低

    HAL_TIM_PWM_ConfigChannel(&TIM2_Handler, &TIM2_CHnHandler, TIM_CHANNEL_1); //配置TIM2通道1
    HAL_TIM_PWM_Start(&TIM2_Handler, TIM_CHANNEL_1); 	//开启PWM通道1


    HAL_TIM_PWM_ConfigChannel(&TIM2_Handler, &TIM2_CHnHandler, TIM_CHANNEL_2); //配置TIM2通道2
    HAL_TIM_PWM_Start(&TIM2_Handler, TIM_CHANNEL_2); 	//开启PWM通道2
}

/**
 * @brief	定时器底层驱动，时钟使能，引脚配置，此函数会被HAL_TIM_PWM_Init()调用
 *
 * @param   htim		定时器句柄
 *
 * @return  void
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_TIM2_CLK_ENABLE();				//使能定时器2
    __HAL_RCC_GPIOA_CLK_ENABLE();				//开启GPIOA时钟

    GPIO_Initure.Pin = GPIO_PIN_0 | GPIO_PIN_1; //PA0.1
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;  		//复用推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;        	//上拉
    GPIO_Initure.Speed = GPIO_SPEED_LOW;   	
    GPIO_Initure.Alternate = GPIO_AF1_TIM2;		//PA1复用为TIM2_CH1.CH2
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
}


/**
 * @brief	设置TIM2通道1的占空比
 *
 * @param   compare		比较值
 *
 * @return  void
 */
void TIM_SetTIM2Compare1(int compare)
{
    TIM2->CCR1 = compare;
}

/**
 * @brief	设置TIM2通道2的占空比
 *
 * @param   compare		比较值
 *
 * @return  void
 */
void TIM_SetTIM2Compare2(int compare)
{
    TIM2->CCR2 = compare;
}



int init_rt_task_shrump() {};

int change_direction(int pwmval) {
	if(pwmval >= 1000) 	return 0;
	if(pwmval <= 500)
	{
		return 1;
	}
	return 0;
}

void inline dir_forward(int speed) {
	LED_B(1);
	LED_G(0);
}

void inline dir_backward(int speed) {
	LED_B(0);
	LED_G(1);
}

//char 

int init_rt_task_servo_motor() {
	profile_add_event("start");
	profile_start(EV0);
	DEBUG_LOG("init_rt_task_servo_motor\n");
	profile_end(EV0);
	profile_dump(EV0);
	return 0;
	DEBUG_LOG("init_rt_task_servo_motor \n");
	if (!pwm_init) {
		PWM_Init(1000 - 1, 80 - 1);	//TIM2时钟频率 80M/80=1M   计数频率1M/1000=1KHZ     默认占空比为50%
		pwm_init = true;
	}
	
	int time = 0;
	int speed = 1;				//速度控制		0:加速		1：减速
	int dir = 1;					//正反转标志	0:电机正转	1：电机反转
	int pwmval = 500;			//默认值
	
	ts_start = get_start_tick();
	
	while(1)
	{
		speed = change_direction(pwmval);
		if(speed)	pwmval += 5;
		else		pwmval -= 5;
		
		if (speed == 1) {
			dir = dir ^ 0x01;	//速度变为最小时改变电机方向
		}
		
		/*
		if(speed)	pwmval += 5;
		else		pwmval -= 5;
		if(pwmval >= 1000) 	speed = 0;
		if(pwmval <= 500)
		{
			speed = 1;
			dir = dir ^ 0x01;	//速度变为最小时改变电机方向
		}
		*/

		if(dir)
		{
			dir_forward(pwmval);
			TIM_SetTIM2Compare1(pwmval);
			TIM_SetTIM2Compare2(0);
		}
		else
		{
			dir_backward(pwmval);
			TIM_SetTIM2Compare1(0);
			TIM_SetTIM2Compare2(pwmval);
		}

		time++;
		if(time%20 == 0 )
			LED_R_TogglePin;

		delay_ms(15);
	}
}

/*定义结构体和公用体*/

typedef struct {
	float setpoint;       //设定值

	float proportiongain;     //比例系数

	float integralgain;      //积分系数

	float derivativegain;    //微分系数

	float lasterror;     //前一拍偏差

	float result; //输出值

	float integral;//积分值

} PID;

void PIDRegulation(PID *vPID, float processValue)
{

	float thisError;

	thisError=vPID->setpoint-processValue;

	vPID->integral+=thisError;

	vPID->result=vPID->proportiongain*thisError+vPID->integralgain*vPID->integral+vPID->derivativegain*(thisError-vPID->lasterror);

	vPID->lasterror=thisError;
}

// PID 算法
int init_rt_task_pid() {

}

#endif