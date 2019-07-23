
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
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
#include "main.h"
#include "stm32f4xx_hal.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "NRF24L01P_H.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

volatile StateMachineTypeDef			SM 	= SM_RECEIVING;
volatile OtherOneStateTypeDef			OOS	= OOS_RX;


NRF24L01P_ExTypeDef 				nrf;
NRF24L01P_MED_InitTypeDef 	nrf_init;
NRF24L01P_HandlerTypeDef 		hnrf;
uint8_t											nrf_stat_reg;

					uint32_t						tmp_ndtr						=	0;
					uint8_t							req2t_rcvd_flg			=	0;
volatile	uint32_t						sm_transmit_timeout	=	0;
					uint8_t							tmp_msg_1[32] 			= "";
					uint8_t							uart_rx_fifo[96] 		= "";
					uint8_t							uart_rx_fifo_one[1] = "";
					uint8_t							tmp_nrf_pckt[4] 		= "";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void NRF_Tx_DS_Callback(void);
void NRF_Rx_DR_Callback(void);
void NRF_MAX_RT_Callback(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
	nrf.hspi 									= &hspi2;
	nrf.spi_cs_port 					= NRF_CS_GPIO_Port;
	nrf.spi_cs_pin  					= NRF_CS_Pin;
						
	nrf_init.P_Mode						= NRF_PRIMARY_MODE_RX;
	
	hnrf.instance 						= &nrf;
	hnrf.init									=	&nrf_init;
	hnrf.STAT_Reg							=	&nrf_stat_reg;
	hnrf.CE_Port							= NRF_CE_GPIO_Port;
	hnrf.CE_Pin								= NRF_CE_Pin;
	hnrf.IRQ_Port							= NRF_IRQ_GPIO_Port;
	hnrf.IRQ_Pin							=	NRF_IRQ_Pin;
	hnrf.Tx_DS_IRQ_Callback		=	&NRF_Tx_DS_Callback;
	hnrf.Rx_DR_IRQ_Callback 	= &NRF_Rx_DR_Callback;
	hnrf.Max_RT_IRQ_Callback	= &NRF_MAX_RT_Callback;
	
	NRF_H_Init( &hnrf );
	NRF_MED_Set_PowerUp( hnrf.instance, NRF_POWER_UP, NULL);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_Delay(2000);
	HAL_UART_Transmit(&huart2, (uint8_t*)"Hello!\n", 7, HAL_MAX_DELAY);
	
	HAL_UART_Receive_IT( &huart2, uart_rx_fifo_one, 1);
	/*
	// test for TX
	NRF_H_ResetChipEn( &hnrf );
	NRF_MED_Set_PrimaryMode( hnrf.instance, NRF_PRIMARY_MODE_TX, NULL);
	
	for( uint8_t i=0; i < 20; i++)
	{
		NRF_EX_Write_Tx_PL( hnrf.instance, 1, (uint8_t*)0xF0U, NULL);
		NRF_H_SetChipEn( &hnrf );
		HAL_Delay(2);
		NRF_H_ResetChipEn( &hnrf );
		HAL_Delay(700);
	}
	*/
  while (1)
  {	
		switch (SM)
    {
    	case SM_RECEIVING:
			{				
				//HAL_UART_Transmit(&huart2, (uint8_t*)"SM_RECEIVING!\n", 14, HAL_MAX_DELAY);
				NRF_H_SetChipEn( &hnrf );
    		break;
			}
    	case SM_RECEIVING_WAIT2T:
			{
				HAL_UART_Transmit(&huart2, (uint8_t*)"SM_RECEIVING_WAIT2T!\n", 21, HAL_MAX_DELAY);
				HAL_Delay(rand() % 20);
				if( req2t_rcvd_flg == 0 )
				{
					SM = SM_REQ2T;
					HAL_UART_Transmit(&huart2, (uint8_t*)"==> SM_REQ2T!\n", 14, HAL_MAX_DELAY);				
					
					tmp_nrf_pckt[0] = 0;
					tmp_nrf_pckt[1] = 0;
					tmp_nrf_pckt[2] = 1;
					NRF_H_ResetChipEn( &hnrf );
					NRF_MED_Set_PrimaryMode( hnrf.instance, NRF_PRIMARY_MODE_TX, NULL);		
					NRF_EX_Write_Tx_PL( hnrf.instance, 3, tmp_nrf_pckt, NULL);
					NRF_H_SetChipEn( &hnrf );
					HAL_Delay(1);
					NRF_H_ResetChipEn( &hnrf );
				}
				else if( req2t_rcvd_flg == 1 )
				{
					SM 	= SM_RECEIVING;
					OOS	=	OOS_TX;
					
					tmp_nrf_pckt[0] = 0;
					tmp_nrf_pckt[1] = 0;
					tmp_nrf_pckt[2] = 0;
					tmp_nrf_pckt[3] = uart_rx_fifo_one[0];
					
					NRF_EX_W_ACK_PAYLOAD( hnrf.instance, 4, tmp_nrf_pckt, 1, NULL);
					HAL_UART_Receive_IT( &huart2, uart_rx_fifo_one, 1);
					
					req2t_rcvd_flg = 0;
				}
    		break;
			}
			case SM_REQ2T:
			{
				//HAL_UART_Transmit(&huart2, (uint8_t*)"SM_REQ2T!    \n", 14, HAL_MAX_DELAY);				
				break;
			}
			case SM_TRANSMITTING:
			{
				HAL_UART_Transmit(&huart2, (uint8_t*)"SM_TRANSMITTING!\n", 17, HAL_MAX_DELAY);
				if( sm_transmit_timeout > 2000 )
				{
					HAL_UART_Transmit(&huart2, (uint8_t*)"TX_TIMEOUT!  \n", 14, HAL_MAX_DELAY);
					SM = SM_REQ2R;
				}
    		break;
			}
			case SM_REQ2R:
			{
				HAL_UART_Transmit(&huart2, (uint8_t*)"SM_REQ2R!    \n", 14, HAL_MAX_DELAY);
				tmp_nrf_pckt[0] = 0;
				tmp_nrf_pckt[1] = 1;
				tmp_nrf_pckt[2] = 0;
				NRF_EX_Write_Tx_PL( hnrf.instance, 3, tmp_nrf_pckt, NULL);
				NRF_H_SetChipEn( &hnrf );
				HAL_Delay(1);
				NRF_H_ResetChipEn( &hnrf );
				
    		break;
			}
    	default:
    		break;
    }	
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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
void NRF_Tx_DS_Callback(void)
{
	if( SM == SM_REQ2T )
	{
		SM = SM_TRANSMITTING;
		OOS = OOS_RX;		
		HAL_UART_Receive_IT( &huart2, uart_rx_fifo_one, 1);
	}
	
}
void NRF_Rx_DR_Callback(void)
{
	uint8_t		tmp_reg;
	uint8_t		tmp_pld[32];
	NRF_EX_R_RX_PL_WID( hnrf.instance, &tmp_reg, NULL);
	NRF_EX_Read_Rx_PL( hnrf.instance, tmp_reg, tmp_pld, NULL);
	
	
	if( (tmp_pld[0] == 0) && (tmp_pld[1] == 0) && (tmp_pld[2] == 0) )
	{
		if( tmp_reg >= 4 )
		{
			HAL_UART_Transmit_IT(&huart2, tmp_pld+3, tmp_reg-3);
		}
	}
	else if( (tmp_pld[0] == 0) && (tmp_pld[1] == 0) && (tmp_pld[2] == 1) )
	{
		req2t_rcvd_flg = 1;
	}
	else if( (tmp_pld[0] == 0) && (tmp_pld[1] == 1) && (tmp_pld[2] == 0) )
	{
		OOS = OOS_RX;
	}
}
void NRF_MAX_RT_Callback(void)
{
	if( SM == SM_REQ2T )
	{
		SM = SM_RECEIVING_WAIT2T;
		
		NRF_MED_Set_PrimaryMode( hnrf.instance, NRF_PRIMARY_MODE_RX, NULL);
		NRF_H_SetChipEn( &hnrf );
	}
	else if( SM == SM_TRANSMITTING )
	{
		// Packet Trasmition Failed!
		UNUSED(0);
	}
	
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
