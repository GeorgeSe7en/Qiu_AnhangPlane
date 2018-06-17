/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include "GUI.h"
#include "Global.h"
#include "SignalPicture.h"
#include "record.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
static uint8_t keyState = 0;
static uint32_t shutdownTime = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART4_Init(void);
static void MX_SPI2_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM3_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void POWER_On(void);
void POWER_Off(void);
void DisplayInitialize(void);
void DisplayTable(void);
void set_UpdateFlag(void);
void DisplayUpdate(void);
void GPS_CoorRecord(void);
void GPS_CoorOutput(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void HAL_SYSTICK_Callback(void)
{
	static uint16_t msCnt = 0;

	if(++msCnt >= 1000)
	{
		msCnt = 0;
		time_increase();
	}
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  POWER_On();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CRC_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_UART4_Init();
  MX_SPI2_Init();
  MX_USB_PCD_Init();
  #if WATCHDOG_ENABLE
  MX_IWDG_Init();
  #endif
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  DisplayInitialize();
  GUI_setFonts(MS_Serif_12x12);
  //Flash_WRTest();
  GUI_ShowString("Init Flash...", 0, 16);
  FILE_Init();
  GUI_ShowString("Init Flash Success.", 0, 16 + 12);
  //uart_printf("Start Application!");
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  GUI_clearWindow(0, 16, 128, 48);
  DisplayTable();
  uart_rx_init();
  while (1)
  {
	uart_rx_processing();
	GPS_MessgeDecode();
	set_UpdateFlag();
	DisplayUpdate();
	GPS_CoorRecord();
	GPS_CoorOutput();
	#if WATCHDOG_ENABLE
	HAL_IWDG_Refresh(&hiwdg);
	#endif
	if(keyState == 1)
	{
		if(HAL_GetTick() - shutdownTime >= 3000)
		{
			POWER_Off();
		}
	}
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* CRC init function */
static void MX_CRC_Init(void)
{

  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* IWDG init function */
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* SPI2 init function */
static void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_IC_InitTypeDef sConfigIC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 32768;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* UART4 init function */
static void MX_UART4_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART3 init function */
static void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USB init function */
static void MX_USB_PCD_Init(void)
{

  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.ep0_mps = DEP0CTL_MPS_8;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB12 PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PB4 PB6 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */
static uint8_t gpsLevel = 1, stars = 0;
static uint32_t pwmCh1Counter = 0;
static uint32_t pwmCh2Counter = 0;
static uint32_t pwmCh3Counter = 0;
static uint32_t pwmCh4Counter = 0;
static uint32_t pwmCh5Counter = 0;
static uint32_t pwmCh6Counter = 0;

// update display
#define UPDATE_GPS		0x01
#define UPDATE_CHL		0x02
#define UPDATE_TIM		0x04
#define UPDATE_LOG		0x80
static uint8_t	updateFlag = UPDATE_GPS | UPDATE_CHL | UPDATE_TIM | UPDATE_LOG;

// channel
#define CHANNEL_ONE		0x01
#define CHANNEL_TWO		0x02
#define CHANNEL_THREE	0x04
#define CHANNEL_FOUR	0x08
#define CHANNEL_FIVE	0x10
#define CHANNEL_SIX		0x20

static uint8_t channelFlag = CHANNEL_ONE | CHANNEL_TWO | CHANNEL_THREE | CHANNEL_FOUR | CHANNEL_FIVE | CHANNEL_SIX;

// Display Date
static char displayDate[] = "2018-05-31";
static char displayTime[] = "18:30";
//Function
void Display_gpsLevel(uint8_t level)
{
	if(level < 1) level = 1;
	if(level > 5) level = 5;
	
	GUI_DrawBMP(gps_Signal[level], 118, 6, 10, 10);
	return;
}

void Display_stars(uint8_t stars)
{
	#define STARS_POSITION	106
	GUI_DrawBMP(led_Digital[stars / 10], STARS_POSITION, 0, 5, 6);
	GUI_DrawBMP(led_Digital[stars % 10], STARS_POSITION + 5, 0, 5, 6);
}

#define T_FONTS		Segoe_UI_Histore_12x12
#define N_FONTS		MS_Serif_12x12

// relation postion
#define T_Y_BASE_POS	18
#define N_Y_BASE_POS	18
#define X_SHEFT			16

#define Y_RELAT_POS		13
// update pwm channel counter
void DisplayPWMCounter(uint8_t ch, uint32_t times)
{
	char buffer[10];
	
	sprintf(buffer, "%d", times);
	switch(ch)
	{
		case 1:
			// show channel
			GUI_setFonts(T_FONTS);
			GUI_ShowString("T1:", 0, T_Y_BASE_POS);
			// show counter
			GUI_setFonts(N_FONTS);
			GUI_ShowString(buffer, X_SHEFT, N_Y_BASE_POS);
			
			GUI_DrawHLine(0, 29, 128);
			break;
		case 2:
			// show channel
			GUI_setFonts(T_FONTS);
			GUI_ShowString("T2:", 0, T_Y_BASE_POS + Y_RELAT_POS);
			// show counter
			GUI_setFonts(N_FONTS);
			GUI_ShowString(buffer, X_SHEFT, N_Y_BASE_POS + Y_RELAT_POS);
		
			GUI_DrawHLine(0, 42, 128);
			break;
		case 3:
			// show channel
			GUI_setFonts(T_FONTS);
			GUI_ShowString("T3:", 0, T_Y_BASE_POS + Y_RELAT_POS*2);
			// show counter
			GUI_setFonts(N_FONTS);
			GUI_ShowString(buffer, X_SHEFT, N_Y_BASE_POS + Y_RELAT_POS*2);
		
			GUI_DrawHLine(0, 55, 128);
			break;
		case 4:
			// show channel
			GUI_setFonts(T_FONTS);
			GUI_ShowString("T4:", 65, T_Y_BASE_POS);
			// show counter
			GUI_setFonts(N_FONTS);
			GUI_ShowString(buffer, 65+X_SHEFT, N_Y_BASE_POS);
		
			GUI_DrawHLine(0, 29, 128);
			break;
		case 5:
			// show channel
			GUI_setFonts(T_FONTS);
			GUI_ShowString("T5:", 65, T_Y_BASE_POS + Y_RELAT_POS);
			// show counter
			GUI_setFonts(N_FONTS);
			GUI_ShowString(buffer, 65+X_SHEFT, N_Y_BASE_POS + Y_RELAT_POS);
		
			GUI_DrawHLine(0, 42, 128);
			break;
		case 6:
			// show channel
			GUI_setFonts(T_FONTS);
			GUI_ShowString("T6:", 65, T_Y_BASE_POS + Y_RELAT_POS*2);
			// show counter
			GUI_setFonts(N_FONTS);
			GUI_ShowString(buffer, 65+X_SHEFT, N_Y_BASE_POS + Y_RELAT_POS*2);
		
			GUI_DrawHLine(0, 55, 128);
			break;
	}
	
	//table
	//GUI_DrawHLine(0, 16, 128);
	//GUI_DrawHLine(0, 29, 128);
	//GUI_DrawHLine(0, 42, 128);
	//GUI_DrawHLine(0, 55, 128);
	//GUI_DrawVLine(63, 16, 39);
	
	return;
}

void DisplayShowTime(void)
{
	GUI_setFonts(MS_Serif_12x12);
	GUI_ShowString(displayDate, 14, 55);
	
	GUI_setFonts(MS_Serif_12x12);
	GUI_ShowString(displayTime, 128-5*6-14, 55);
	
	GUI_DrawHLine(0, 55, 128);
}

void DisplayInitialize(void)
{
	GUI_Init();
  
	GUI_setFonts(SongFonts_16X16);
	GUI_ShowString("安航无人机", 12, 0);
  
	// GPS
	GUI_DrawBMP(gps_Signal[0], 106, 6, 10, 10);
}

void DisplayTable(void)
{
	GUI_DrawHLine(0, 16, 128);
	GUI_DrawHLine(0, 29, 128);
	GUI_DrawHLine(0, 42, 128);
	GUI_DrawHLine(0, 55, 128);
	GUI_DrawVLine(63, 16, 39);
}


void DisplayUpdate(void)
{
	if(updateFlag&UPDATE_GPS)
	{
		updateFlag &= ~UPDATE_GPS;
		//level
		if(stars<3) gpsLevel = 0;
		else if(stars > 8) gpsLevel = 5;
		else gpsLevel = stars - 3;
		
		Display_gpsLevel(gpsLevel);
		//stars
		Display_stars(stars);
	}
	if(updateFlag&UPDATE_CHL)
	{
		updateFlag &= ~UPDATE_CHL;
		if(channelFlag&CHANNEL_ONE)
		{
			channelFlag &= ~CHANNEL_ONE;
			DisplayPWMCounter(1, pwmCh1Counter);
		}
		if(channelFlag&CHANNEL_TWO)
		{
			channelFlag &= ~CHANNEL_TWO;
			DisplayPWMCounter(2, pwmCh2Counter);
		}
		if(channelFlag&CHANNEL_THREE)
		{
			channelFlag &= ~CHANNEL_THREE;
			DisplayPWMCounter(3, pwmCh3Counter);
		}
		if(channelFlag&CHANNEL_FOUR)
		{
			channelFlag &= ~CHANNEL_FOUR;
			DisplayPWMCounter(4, pwmCh4Counter);
		}
		if(channelFlag&CHANNEL_FIVE)
		{
			channelFlag &= ~CHANNEL_FIVE;
			DisplayPWMCounter(5, pwmCh5Counter);
		}
		if(channelFlag&CHANNEL_SIX)
		{
			channelFlag &= ~CHANNEL_SIX;
			DisplayPWMCounter(6, pwmCh6Counter);
		}
	}
	if(updateFlag&UPDATE_TIM)
	{
		updateFlag &= ~UPDATE_TIM;
		DisplayShowTime();
	}
}

void set_UpdateFlag(void)
{
	if(!get_timeDate(displayTime, displayDate))
	{
		updateFlag |= UPDATE_TIM;
	}
	
	if(get_gpsLevel(&stars))
	{
		updateFlag |= UPDATE_GPS;
	}
	
}

// soft delay ms
void soft_delay_ms(unsigned int x)
{
	unsigned int cnt;
	
	while(x--)
	{
		for(cnt = SystemCoreClock/1000 >> 2; cnt>0;cnt--)
		{
		}
	}
}
// exti callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
		case GPIO_PIN_2:		// Key
			soft_delay_ms(2);		// filter
			if(GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2))		// 释放按键
			{
				keyState = 0;
			}
			else		// 按下按键
			{
				keyState = 1;
				shutdownTime = HAL_GetTick();
			}
			break;
		case GPIO_PIN_4:		// PWM5
			soft_delay_ms(2);		// filter
			if(GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)) break;
			pwmCh5Counter++;
			channelFlag |= CHANNEL_FIVE;
			updateFlag |= UPDATE_CHL;
			break;
		case GPIO_PIN_6:		// PWM4
			soft_delay_ms(2);		// filter
			if(GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6)) break;
			pwmCh4Counter++;
			channelFlag |= CHANNEL_FOUR;
			updateFlag |= UPDATE_CHL;
			break;
		case GPIO_PIN_8:		// PWM6
			soft_delay_ms(2);		// filter
			if(GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8)) break;
			pwmCh6Counter++;
			channelFlag |= CHANNEL_SIX;
			updateFlag |= UPDATE_CHL;
			break;
		case GPIO_PIN_9:		// PWM3
			soft_delay_ms(2);		// filter
			if(GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9)) break;
			pwmCh3Counter++;
			channelFlag |= CHANNEL_THREE;
			updateFlag |= UPDATE_CHL;
			break;
	}
}

// timer caputer
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim3)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
		{
			soft_delay_ms(2);		// filter
			if(GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8)) return;
			// PWM1
			pwmCh1Counter++;
			channelFlag |= CHANNEL_ONE;
			updateFlag |= UPDATE_CHL;
		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			soft_delay_ms(2);		// filter
			if(GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6)) return;
			// PWM2
			pwmCh2Counter++;
			channelFlag |= CHANNEL_TWO;
			updateFlag |= UPDATE_CHL;
		}
	}
}

// record file pointer
static FILE_Message_t *pReadFile = NULL;
static FILE_Message_t *pWriteFile = NULL;
struct PwmCounter pwmRecord = {0, 0, 0, 0, 0, 0};
// gpsCoorFlag
void GPS_CoorRecord(void)
{
	static uint8_t record = 0;
	
	struct RecordTime time;
	char strbuf[42];
	int i;
	
	//start record
	if(record == 0 && (pwmCh1Counter + pwmCh2Counter + pwmCh3Counter + pwmCh4Counter + pwmCh5Counter + pwmCh6Counter) > 0)
	{
		if(0 == FILE_writeInit(&pWriteFile))
		{
			record = 1;
			get_mytime(&time);
			FILE_writeTime(pWriteFile, &time);

			// 
			if(pWriteFile->pwm.pwm1 != pwmCh1Counter) pWriteFile->pwm.pwm1 = pwmCh1Counter;
			if(pWriteFile->pwm.pwm2 != pwmCh2Counter) pWriteFile->pwm.pwm2 = pwmCh2Counter;
			if(pWriteFile->pwm.pwm4 != pwmCh4Counter) pWriteFile->pwm.pwm4 = pwmCh4Counter;
			if(pWriteFile->pwm.pwm5 != pwmCh5Counter) pWriteFile->pwm.pwm5 = pwmCh5Counter;
			if(pWriteFile->pwm.pwm6 != pwmCh6Counter) pWriteFile->pwm.pwm6 = pwmCh6Counter;
			if(pWriteFile->pwm.pwm3 < pwmCh3Counter)
			{
				get_LatLon(strbuf);
				i = strlen(strbuf);
				strbuf[i++] = 0x0D;
				strbuf[i++] = 0x0A;
				strbuf[i++] = '\0';
				FILE_writeGPSCache(pWriteFile, strbuf);
				pWriteFile->pwm.pwm3++;
			}
		}
	}
	else if(record)
	{
		if(pWriteFile->pwm.pwm1 != pwmCh1Counter) pWriteFile->pwm.pwm1 = pwmCh1Counter;
		if(pWriteFile->pwm.pwm2 != pwmCh2Counter) pWriteFile->pwm.pwm2 = pwmCh2Counter;
		if(pWriteFile->pwm.pwm4 != pwmCh4Counter) pWriteFile->pwm.pwm4 = pwmCh4Counter;
		if(pWriteFile->pwm.pwm5 != pwmCh5Counter) pWriteFile->pwm.pwm5 = pwmCh5Counter;
		if(pWriteFile->pwm.pwm6 != pwmCh6Counter) pWriteFile->pwm.pwm6 = pwmCh6Counter;
		if(pWriteFile->pwm.pwm3 < pwmCh3Counter)
		{
			get_LatLon(strbuf);
			i = strlen(strbuf);
			strbuf[i++] = 0x0D;
			strbuf[i++] = 0x0A;
			strbuf[i++] = '\0';
			if(FILE_writeGPSCache(pWriteFile, strbuf))
			{
				FILE_writeFlash(pWriteFile, 0);
				FILE_writeGPSCache(pWriteFile, strbuf);
			}
			pWriteFile->pwm.pwm3++;
		}
	}
}

// ouput the record
void GPS_CoorOutput(void)
{
	static struct RecordTime time;
	static uint8_t readFlag = 0;

	static Direct_t direct;
	static uint8_t status = 0;

	uint8_t flag;
	uint32_t address;

	if(status == 0)
	{
		flag = CMD_getFlag();
		if(flag & 0x01)
		{
			if(CMD_getDirect(&direct))
			{
				status = 1;
			}
		}
		if(flag & 0x80)
		{
			CMD_SendRXError();
			HAL_Delay(10);
		}
		
		if(flag)
		{
			CMD_clearFlag(0x81);
			CMD_receive();
		}
	}
	else if(status == 1)
	{
		switch(direct.cmd)
		{
			case 0x01:
			case 0x02:
			case 0x03:
			if(readFlag)
			{
				if(!uart_printStatus())
				{
					if(readFlag == 1)
					{
						if(FILE_readPage(pReadFile))
						{
							FILE_close(pReadFile);
							readFlag = 2;
						}
						else
						{
							//uart_print((char *)&pReadFile->dataBuffer[4]);
							if(pReadFile->dataLength > 78)
							{
								uart_send(&pReadFile->dataBuffer[78], pReadFile->dataLength - 78);
							}
						}
					}
					
					if(readFlag == 2)
					{
						if(FILE_findFront(&address))
						{
							uart_print("\r\nNo more record!");
							readFlag = 0;
							status = 0;
							return;
						}
						if(FILE_readInit(address, &pReadFile))
						{
							uart_print("\r\nRead record fail!");
							readFlag = 0;
							status = 0;
							return;
						}
						if(time.tag == 'L')
						{
							time.tag = pReadFile->time.tag;
							time.year = pReadFile->time.year;
							time.month = pReadFile->time.month;
							time.day = pReadFile->time.day;
						}
						else if(pReadFile->time.tag == 'L')
						{
							// nothing, No check
						}
						else if(time.year != pReadFile->time.year ||
								time.month != pReadFile->time.month ||
								time.day != pReadFile->time.day)
						{
							if(direct.cmd > 1)
							{
								direct.cmd--;
								time.year = pReadFile->time.year;
								time.month = pReadFile->time.month;
								time.day = pReadFile->time.day;
							}
							else
							{
								FILE_close(pReadFile);
								uart_print("\r\nNo more record!");
								readFlag = 0;
								status = 0;
								return;
							}
						}
						uart_print("\r\n\r\n---Record---\r\nTime ");
						uart_print((char *)&pReadFile->dataBuffer[4]);
						uart_print((char *)&pReadFile->dataBuffer[28]);
						if(pReadFile->dataLength > 102)
						{
	
							uart_send(&pReadFile->dataBuffer[102], pReadFile->dataLength - 102);
						}
						readFlag = 1;
					}
				}
			}
			else
			{
				if(FILE_findLast(&address))
				{
					uart_print("\r\nNo record!");
					status = 0;
					return;
				}
				if(FILE_readInit(address, &pReadFile))
				{
					uart_print("\r\nRead record fail!");
					status = 0;
					return;
				}
				uart_print("\r\n\r\n---Record---\r\nTime ");
				uart_print((char *)&pReadFile->dataBuffer[4]);
				uart_print((char *)&pReadFile->dataBuffer[28]);
				
				if(pReadFile->dataLength > 102)
				{
					uart_send(&pReadFile->dataBuffer[102], pReadFile->dataLength - 102);
				}
				time.tag = pReadFile->time.tag;
				time.year = pReadFile->time.year;
				time.month = pReadFile->time.month;
				time.day = pReadFile->time.day;

				readFlag = 1;
				return;
			}
			break;
			case 0x80:
				if(FILE_clearRecord())
				{
					uart_print("\r\nBusy, delete record fail!");
				}
				else
				{
					uart_print("\r\nDelete record success!");
				}
				status = 0;
			break;
			default:
				status = 0;
			break;
		}
	}
}

// power off
void POWER_OffCallback(void)
{
	// save the record file
	//uart_printf("\r\n Shutdown, save!");
	FILE_writeFlash(pWriteFile, 1);
	FILE_close(pWriteFile);
	//GUI_setFonts(SongFonts_16X16);
	//GUI_ShowString("POWER OFF", 24, 24);
	GUI_Off();
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
