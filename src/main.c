
#include "stm32wlxx_hal_subghz.h"

#include "stm32wlxx_ll_rcc.h"
#include "stm32wlxx_ll_pwr.h"
#include "stm32wlxx_ll_gpio.h"
#include "stm32wlxx_ll_utils.h"

#include "stm32wlxx_ll_bus.h"


/* SMPS clock detection defines */
#define SUBGHZ_SMPSC0R           0x0916 /* SMPS control 0 register address */
#define SUBGHZ_SMPSC0R_CLKDE     0x40   /* SMPS control 0 register, bit SMPS clock detection enable */


#define SMPS_CTRL0_REG_ADDR         0x0916
#define SUBGHZ_PCR_ADDR             0x091A
#define SUBGHZ_REGDRVCR_ADDR        0x091F
#define SUBGHZ_SMPSC2R_ADDR         0x0923

/* Private variables ---------------------------------------------------------*/
SUBGHZ_HandleTypeDef hsubghz;

/* USER CODE BEGIN PV */
// __IO uint8_t ubUserButtonEvent = 0;   /* Flag of user button set on interrupt handler */
uint32_t smps_requested_mode;         /* SMPS requested mode (value "PWR_SMPS_STEP_DOWN" for SMPS is in mode step-down (switching enable), value "PWR_SMPS_STEP_DOWN" SMPS is in mode bypass (switching disable) */
uint32_t smps_effective_mode;         /* SMPS requested mode (value "PWR_SMPS_STEP_DOWN" for SMPS is in mode step-down (switching enable), value "PWR_SMPS_STEP_DOWN" SMPS is in mode bypass (switching disable) */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SUBGHZ_Init(void);
void SMPSClockDetectionEnable(void);

int main(void)
{
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SUBGHZ_Init();
  
  /* SMPS clock detection enable */
  SMPSClockDetectionEnable();
  
  /* Set SMPS operating mode */
  LL_PWR_SMPS_SetMode(LL_PWR_SMPS_STEP_DOWN);
  smps_requested_mode = LL_PWR_SMPS_STEP_DOWN;

  /* Infinite loop */
  while (1)
  {

    /* Toggle LED */
    // BSP_LED_Toggle(LED2);
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
    
    /* Get SMPS effective operating mode */
    smps_effective_mode = LL_PWR_SMPS_GetEffectiveMode();
    if(smps_effective_mode == LL_PWR_SMPS_STEP_DOWN)
    {
      /* SMPS effective operating mode: step-down */
      /* Toggle LED quickly */
      LL_mDelay(50);
    }
    else
    {
      /* SMPS effective operating mode: bypass */
      /* Toggle LED slowly */
      LL_mDelay(500);
    }
    
    /* Toggle SMPS between step-down and bypass mode at each press on User push-button (B1) */
    // if (ubUserButtonEvent == 1)
    // {
    //   ubUserButtonEvent = 0;
      
    //   if (smps_requested_mode == PWR_SMPS_BYPASS)
    //   {
    //     HAL_PWREx_SMPS_SetMode(PWR_SMPS_STEP_DOWN);
    //     smps_requested_mode = PWR_SMPS_STEP_DOWN;
    //   }
    //   else
    //   {
    //     HAL_PWREx_SMPS_SetMode(PWR_SMPS_BYPASS);
    //     smps_requested_mode = PWR_SMPS_BYPASS;
    //   }
    // }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_RCC_DeInit();

  // update the global variable SystemCoreClock to reflect the new core clock
  SystemCoreClockUpdate();

  LL_RCC_ClocksTypeDef clk_struct;

  LL_RCC_GetSystemClocksFreq(&clk_struct);
  LL_Init1msTick(clk_struct.HCLK1_Frequency);
}

/**
  * @brief SUBGHZ Initialization Function
  * @param None
  * @retval None
  */
static void MX_SUBGHZ_Init(void)
{
  LL_APB3_GRP1_EnableClock(LL_APB3_GRP1_PERIPH_SUBGHZSPI);
  hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_8;
  if (HAL_SUBGHZ_Init(&hsubghz) != HAL_OK)
  {
    // Error_Handler();
  }

  uint8_t regVal = 0;

  HAL_SUBGHZ_ReadRegister(&hsubghz, SUBGHZ_PCR_ADDR, &regVal);
  HAL_SUBGHZ_ReadRegister(&hsubghz, SUBGHZ_REGDRVCR_ADDR, &regVal);
  HAL_SUBGHZ_ReadRegister(&hsubghz, SUBGHZ_SMPSC2R_ADDR, &regVal);

  HAL_SUBGHZ_WriteRegister(&hsubghz, SUBGHZ_REGDRVCR_ADDR, 0x09);
  HAL_SUBGHZ_ReadRegister(&hsubghz, SUBGHZ_REGDRVCR_ADDR, &regVal);

  HAL_SUBGHZ_WriteRegister(&hsubghz, SUBGHZ_PCR_ADDR, 0x49);
  HAL_SUBGHZ_ReadRegister(&hsubghz, SUBGHZ_PCR_ADDR, &regVal);

  HAL_SUBGHZ_WriteRegister(&hsubghz, SUBGHZ_SMPSC2R_ADDR, 0x6);
  HAL_SUBGHZ_ReadRegister(&hsubghz, SUBGHZ_SMPSC2R_ADDR, &regVal);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  // /*Configure GPIO pin : B1_Pin */
  // GPIO_InitStruct.Pin = B1_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  // GPIO_InitStruct.Pull = GPIO_PULLUP;
  // HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  // /* EXTI interrupt init*/
  // HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);

  /* Configure GPIO pin for RF Switch */
  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_9);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/**
  * @brief SMPS clock detection enable.
  * @note  SMPS clock detection is recommended to be enabled before enable SMPS
  *        in switching mode. 
  *        In case of clock failure, it will automaticcaly switch off SMPS
  *        (refer to reference manual).
  * @note  SMPS clock detection is controlled through HAL SUBGHZ peripheral,
  *        due to SMPS and radio related to the same HSE clock.
  * @param None
  * @retval None
  */
void SMPSClockDetectionEnable(void)
{
  uint8_t radio_register_data;
  uint8_t radio_register_data_readback;
  uint8_t radio_command;
  
  /* Enable SMPS clock detection through HAL SUBGHZ peripheral */
  HAL_SUBGHZ_ReadRegister(&hsubghz, SUBGHZ_SMPSC0R, &radio_register_data);
  radio_register_data |= SUBGHZ_SMPSC0R_CLKDE;
  HAL_SUBGHZ_WriteRegister(&hsubghz, SUBGHZ_SMPSC0R, radio_register_data);
  
  /* Check back data written */
  HAL_SUBGHZ_ReadRegister(&hsubghz, SUBGHZ_SMPSC0R, &radio_register_data_readback);
  if(radio_register_data != radio_register_data_readback)
  {
    // Error_Handler();
  }
  
  /* Set back radio in Sleep mode (optional) */
  /* Note: Once clock detection is enabled the radio can be put back in Sleep mode,
           clock detection remains active.
           SMPS can still be controlled through PWR registers */
  /* Note: Using radio middleware, this action can be done more easily 
           with function "RadioSleep()" */
  radio_command = 4; /* Command "Set_Sleep": warm startup and RTC wakeup disabled is value 4 */
  HAL_SUBGHZ_ExecSetCmd( &hsubghz, RADIO_SET_SLEEP, ( uint8_t* )&radio_command, 1 );
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//  if (GPIO_Pin == BUTTON_SW1_PIN)
//  {
//    /* Set variable to report push button event to main program */
//    ubUserButtonEvent = 1;
//  }
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
  while(1) 
  {
    // HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
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
  /* Infinite loop */
  while (1)
  {
  }

  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
