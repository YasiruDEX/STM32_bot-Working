/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
ADC_ChannelConfTypeDef sConfigPrivate = {0};

volatile uint32_t millis_counter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void motor(int motor,int direction);
void speed(int Left, int Right);
void updateIR();
void calibrate();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define ENR TIM_CHANNEL_1
#define ENL TIM_CHANNEL_2

#define Motor_RF_pin Motor_2_Pin
#define Motor_RB_pin Motor_1_Pin

#define Motor_RF_gpio Motor_2_GPIO_Port
#define Motor_RB_gpio Motor_1_GPIO_Port

#define Motor_LF_pin Motor_4_Pin
#define Motor_LB_pin Motor_3_Pin

#define Motor_LF_gpio Motor_4_GPIO_Port
#define Motor_LB_gpio Motor_3_GPIO_Port

#define left 0
#define right 1

#define Forward 1
#define Backward -1

#define IR_ARRAY_LENGTH 8

int IR_array[] = {0,0,0,0,0,0,0,0};

int digital_IR[] = {0,0,0,0,0,0,0,0};

int Ir_thresholds[] = {2000,2000,2000,2000,2000,2000,2000,2000};

GPIO_TypeDef* sensor_ports[] = {
  Sensor_1_GPIO_Port,
  Sensor_2_GPIO_Port,
  Sensor_3_GPIO_Port,
  Sensor_4_GPIO_Port,
  Sensor_5_GPIO_Port,
  Sensor_6_GPIO_Port,
  Sensor_7_GPIO_Port,
  Sensor_8_GPIO_Port
};

uint16_t sensor_pins[] = {
  Sensor_1_Pin,
  Sensor_2_Pin,
  Sensor_3_Pin,
  Sensor_4_Pin,
  Sensor_5_Pin,
  Sensor_6_Pin,
  Sensor_7_Pin,
  Sensor_8_Pin
};

//uint16_t ADC_channel[] = {
//	ADC_CHANNEL_0,
//	ADC_CHANNEL_1,
//	ADC_CHANNEL_2,
//	ADC_CHANNEL_3,
//	ADC_CHANNEL_4,
//	ADC_CHANNEL_6,
//	ADC_CHANNEL_7,
//	ADC_CHANNEL_8
//};



// PID parameters for Line Following
double Kp = 16;
double Ki = 0;
double Kd = 7;


int Drive_constant = 200;

// Initialize PID variables
double prevError = 0;
double integral = 0;
double derivative = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, ENR);
  HAL_TIM_PWM_Start(&htim1, ENL);

  HAL_TIM_Base_Start_IT(&htim3);

  HAL_ADC_Start(&hadc1);

  sConfigPrivate.Rank = ADC_REGULAR_RANK_1;
  sConfigPrivate.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

  calibrate();
  HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  updateIR();


//	  if(HAL_GPIO_ReadPin(Caliberation_button_GPIO_Port, Caliberation_button_Pin) == RESET){
//		  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, RESET);
//	  } else{
//		  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, SET);
//	  }


	  motor(left, Forward);
	  motor(right, Forward);

	  int array_lit_amount = 0;
	  for (int i = 0; i < IR_ARRAY_LENGTH; i++)
	  {
		  array_lit_amount += digital_IR[i];
	  }

	  float left_sum = - 4 * digital_IR[0] - 3 * digital_IR[1] - 2 * digital_IR[2] - 1 * digital_IR[3];
	  float right_sum = 1 * digital_IR[4] + 2 * digital_IR[5] + 3 * digital_IR[6] + 4 * digital_IR[7];

	  int position = left_sum + right_sum;

	  int derivative = position - prevError;

	  int PID_constant = Kp * position + Kd * derivative;

	  prevError = position;

	  Drive_constant = 200;

	  int offset = 30; // For correcting motor speeds
	  int Left_drive = Drive_constant + offset - PID_constant;
	  int Right_drive = Drive_constant - offset + PID_constant;

	  const int MIN_VALUE = 0;
	  const int MAX_VALUE = 625;

	  if (Left_drive < MIN_VALUE) {
	      Left_drive = MIN_VALUE;
	  } else if (Left_drive > MAX_VALUE) {
	      Left_drive = MAX_VALUE;
	  }

	  if (Right_drive < MIN_VALUE) {
	      Right_drive = MIN_VALUE;
	  } else if (Right_drive > MAX_VALUE) {
	      Right_drive = MAX_VALUE;
	  }

	  speed(Left_drive, Right_drive);




    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 127;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 625;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 6400-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_GREEN_Pin|Motor_3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Motor_2_Pin|Motor_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Motor_4_GPIO_Port, Motor_4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Caliberation_button_Pin */
  GPIO_InitStruct.Pin = Caliberation_button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Caliberation_button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_GREEN_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Motor_2_Pin Motor_1_Pin */
  GPIO_InitStruct.Pin = Motor_2_Pin|Motor_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : Motor_3_Pin */
  GPIO_InitStruct.Pin = Motor_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Motor_3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Motor_4_Pin */
  GPIO_InitStruct.Pin = Motor_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Motor_4_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM3) {
    millis_counter++;
  }
}

uint32_t millis() {
  return millis_counter;
}

void speed(int Left, int Right){
	  __HAL_TIM_SET_COMPARE(&htim1,ENR,Right);
	  __HAL_TIM_SET_COMPARE(&htim1,ENL,Left);
}

void motor(int motor,int direction){
	if (motor == 0){
		if (direction == 1){
			HAL_GPIO_WritePin(Motor_LB_gpio, Motor_LB_pin, 0);
			HAL_GPIO_WritePin(Motor_LF_gpio, Motor_LF_pin, 1);
		}else{
			HAL_GPIO_WritePin(Motor_LB_gpio, Motor_LB_pin, 1);
			HAL_GPIO_WritePin(Motor_LF_gpio, Motor_LF_pin, 0);
		}
	}else{
		if (direction == 1){
			HAL_GPIO_WritePin(Motor_RB_gpio, Motor_RB_pin, 0);
			HAL_GPIO_WritePin(Motor_RF_gpio, Motor_RF_pin, 1);
		}else{
			HAL_GPIO_WritePin(Motor_RB_gpio, Motor_RB_pin, 1);
			HAL_GPIO_WritePin(Motor_RF_gpio, Motor_RF_pin, 0);
		}
	}
}

void updateIR(){
	  sConfigPrivate.Channel = ADC_CHANNEL_0;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);

	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1,1000);
	  IR_array[0] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  sConfigPrivate.Channel = ADC_CHANNEL_1;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);

	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1,1000);
	  IR_array[1] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  sConfigPrivate.Channel = ADC_CHANNEL_2;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);

	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1,1000);
	  IR_array[2] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  sConfigPrivate.Channel = ADC_CHANNEL_9;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);

	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1,1000);
	  IR_array[3] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  sConfigPrivate.Channel = ADC_CHANNEL_4;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);

	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1,1000);
	  IR_array[4] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  sConfigPrivate.Channel = ADC_CHANNEL_6;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);

	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1,1000);
	  IR_array[5] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  sConfigPrivate.Channel = ADC_CHANNEL_7;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);

	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1,1000);
	  IR_array[6] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  sConfigPrivate.Channel = ADC_CHANNEL_8;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);

	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1,1000);
	  IR_array[7] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  	  for (int i = 0; i < 8; ++i) {
	  	      digital_IR[i] = IR_array[i] > Ir_thresholds[i];
	  	  }

	  //	  for (int i = 0; i < 8; ++i) {
	  ////	      IR_array[i] = HAL_GPIO_ReadPin(sensor_ports[i], sensor_pins[i]);
	  //
	  //		  sConfigPrivate.Channel = ADC_channel[i];
	  //		  HAL_ADC_ConfigChannel(&hadc1, &sConfigPrivate);
	  //
	  //		  HAL_ADC_Start(&hadc1);
	  //		  HAL_ADC_PollForConversion(&hadc1,1000);
	  //		  IR_array[i] = HAL_ADC_GetValue(&hadc1);
	  //		  HAL_ADC_Stop(&hadc1);
	  //	  }
}

void calibrate()
{

  motor(right, Backward);
  motor(left, Forward);

  // Stop motors when calibrating
  speed(255,255);

  // make sensor_max_values array of length 8 equal to sensor calibration array
  int sensor_max_values[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  // sensor min values array
  int sensor_min_values[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  int now_time_for_calibration = millis();

  HAL_GPIO_WritePin(GPIOA, LED_GREEN_Pin, GPIO_PIN_SET);

  while (millis() - now_time_for_calibration < 5)
  {

	updateIR();
    uint8_t allZeros = 1;
    for (int i = 0; i < 8; i++)
    {
      if (sensor_max_values[i] != 0)
      {
        allZeros = 0;
        break; // No need to continue checking if we find a non-zero value
      }
    }

    if (allZeros)
    {
      for (int i = 0; i < 8; i++)
      {
        sensor_max_values[i] = IR_array[i];
        sensor_min_values[i] = IR_array[i];
      }
    }

    for (int i = 0; i < 8; i++)
    {
      if (IR_array[i] > sensor_max_values[i])
      {
        sensor_max_values[i] = IR_array[i];
      }
      if (IR_array[i] < sensor_min_values[i])
      {
        sensor_min_values[i] = IR_array[i];
      }
    }
  }

  HAL_GPIO_WritePin(GPIOA, LED_GREEN_Pin, GPIO_PIN_RESET);

  speed(0,0);

  for (int i = 0; i < 8; i++)
  {
    Ir_thresholds[i] = (sensor_max_values[i] + sensor_min_values[i]) / 2;
  }

}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
