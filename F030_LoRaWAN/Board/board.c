/*______
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
(C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "stm32f0xx_hal.h"
#include "board.h"



/*!
* Unique Devices IDs register set ( STM32F0xxx )
*/
#define         ID1                                 ( 0x1FFFF7AC )
#define         ID2                                 ( 0x1FFFF7B0 )
#define         ID3                                 ( 0x1FFFF7B4 )

Gpio_t ENrf;
Gpio_t ENSensor;

//Gpio_t rftrx1;
//Gpio_t rftrx2;

Gpio_t UsbDetect;
//IWDG_HandleTypeDef hiwdg;
//extern ADXL357_t ADXL357;

/*
* MCU objects
*/
Uart_t Uart1;
#if defined( USE_USB_CDC )
Uart_t UartUsb;
#endif

/*!
* Initializes the unused GPIO to a know status
*/


/*!
* System Clock Configuration
*/
static void SystemClockConfig( void );
static void BoardUnusedIoInit( void );
/*!
* Used to measure and calibrate the system wake-up time from STOP mode
*/
static void CalibrateSystemWakeupTime( void );

/*!
* System Clock Re-Configuration when waking up from STOP mode
*/
static void SystemClockReConfig( void );

/*!
* Timer used at first boot to calibrate the SystemWakeupTime
*/
static TimerEvent_t CalibrateSystemWakeupTimeTimer;
static TimerEvent_t TEST5S;
static TimerEvent_t TEST10S;
static TimerEvent_t TEST15S;
/*!
* Flag to indicate if the MCU is Initialized
*/
static bool McuInitialized = false;

/*!
* Flag to indicate if the SystemWakeupTime is Calibrated
*/
static bool SystemWakeupTimeCalibrated = false;

/*!
* Callback indicating the end of the system wake-up time calibration
*/
static void OnCalibrateSystemWakeupTimeTimerEvent( void )
{
	//DebugPrintf("SystemWakeupTimeCalibrated = true;\r\n");
  SystemWakeupTimeCalibrated = true;
}
static void TEST5( void )
{
	DebugPrintf("\r\n\r\n  5S \r\n\r\n");
  //SystemWakeupTimeCalibrated = true;
}
static void TEST10( void )
{
	DebugPrintf("\r\n\r\n  10S \r\n\r\n");
  //SystemWakeupTimeCalibrated = true;
}
static void TEST15( void )
{
	DebugPrintf("\r\n\r\n  15S \r\n\r\n");
  //SystemWakeupTimeCalibrated = true;
}
/*!
* Nested interrupt counter.
*
* \remark Interrupt should only be fully disabled once the value is 0
*/
static uint8_t IrqNestLevel = 0;

void BoardDisableIrq( void )
{
  __disable_irq( );
  IrqNestLevel++;
}

void BoardEnableIrq( void )
{
  IrqNestLevel--;
  if( IrqNestLevel == 0 )
  {
    __enable_irq( );
  }
}

void BoardInitPeriph( void )
{
//    GpioInit( &ENrf, EN_RF, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );	
//	  GpioWrite(&ENrf, 0 );// Turn ON RF(SX1278) power	
//		HAL_Delay(800);
//	  GpioInit( &ENSensor, EN_SENSOR, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );	
//	  GpioWrite(&ENSensor, 0 );// Turn ON sensor power
//    HAL_Delay(200);

	  //UartPutBuffer(&Uart1, (uint8_t *)"SPI2 Init done\r\n",strlen("SPI2 Init done\r\n")); 
}

#define FIFO_TX_SIZE      512
uint8_t UARTTxBuffer[FIFO_TX_SIZE];

#define FIFO_RX_SIZE      512
uint8_t UARTRxBuffer[FIFO_RX_SIZE];

void loraMcuIrqNotify( UartNotifyId_t id )
{
  uint8_t data;
  
  if( id == UART_NOTIFY_RX )
  {
    if( UartGetChar( &Uart1, &data ) == 0 )
    {
      //to do ...
    }
  }
}
//static void MX_IWDG_Init(void)
//{

//  hiwdg.Instance = IWDG;
//  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
//  hiwdg.Init.Reload = 0XFFF;
//  HAL_IWDG_Init(&hiwdg);
//}

void BoardInitMcu( void )
{
  if( McuInitialized == false )
  {
#if defined( USE_BOOTLOADER )
    // Set the Vector Table base location at 0x3000
    SCB->VTOR = FLASH_BASE | 0x3000;
#endif
		//SystemCoreClockUpdate();
    HAL_Init( );
    
    SystemClockConfig( );
    //MX_IWDG_Init();
#if defined( USE_USB_CDC )
    UartInit( &UartUsb, UART_USB_CDC, NC, NC );
    UartConfig( &UartUsb, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    
    DelayMs( 1000 ); // 1000 ms for Usb initialization
#endif
         
    RtcInit( );
    
    BoardUnusedIoInit( );
//#warning "Commented for test!"
  }
  else
  {
    SystemClockReConfig( );
//#warning "Commented for test!"
  } 
  
#if defined( USE_DEBUGGER )  
    //uart init
    FifoInit( &Uart1.FifoRx, UARTRxBuffer, FIFO_RX_SIZE );     
    FifoInit( &Uart1.FifoTx, UARTTxBuffer, FIFO_TX_SIZE );
    Uart1.IrqNotify = loraMcuIrqNotify;    
    
    UartInit( &Uart1, UART_1, UART_TX, UART_RX );
    UartConfig( &Uart1, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    //DebugPrintf("Heltec lora node demo\r\n");
    //UartPutBuffer(&Uart1,(uint8_t *)"\r\nHeltec lora node demo\r\n",strlen("\r\nHeltec lora node demo\r\n"));    
#endif
//		GpioInit( &ENrf, EN_RF, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );	
//		GpioWrite(&ENrf, 0 );
		SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
		SX1276IoInit( );
  if( McuInitialized == false )
  {
		//DebugPrintf("Frist Open\r\n");
    McuInitialized = true;
    if( GetBoardPowerSource( ) == BATTERY_POWER )
    {
      CalibrateSystemWakeupTime( );
    }

  }
}

void BoardDeInitMcu( void )
{
  Gpio_t ioPin;
  
  SpiDeInit( &SX1276.Spi );
  SX1276IoDeInit( );
  
  GpioInit( &ioPin, OSC_HSE_IN, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
  GpioInit( &ioPin, OSC_HSE_OUT, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
  
//  GpioInit( &ioPin, OSC_LSE_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, OSC_LSE_OUT, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint32_t BoardGetRandomSeed( void )
{
  return ( ( *( uint32_t* )ID1 ) ^ ( *( uint32_t* )ID2 ) ^ ( *( uint32_t* )ID3 ) );
}

void BoardGetUniqueId( uint8_t *id )
{
  id[7] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
  id[6] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
  id[5] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
  id[4] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
  id[3] = ( ( *( uint32_t* )ID2 ) ) >> 24;
  id[2] = ( ( *( uint32_t* )ID2 ) ) >> 16;
  id[1] = ( ( *( uint32_t* )ID2 ) ) >> 8;
  id[0] = ( ( *( uint32_t* )ID2 ) );
}

/*!
* Factory power supply
*/
#define FACTORY_POWER_SUPPLY                        3300 // mV

/*!
* VREF calibration value
*/
#define VREFINT_CAL                                 ( *( uint16_t* )0x1FF80078 )

/*!
* ADC maximum value
*/
#define ADC_MAX_VALUE                               4095

/*!
* Battery thresholds
*/
#define BATTERY_MAX_LEVEL                           4150 // mV
#define BATTERY_MIN_LEVEL                           3200 // mV
#define BATTERY_SHUTDOWN_LEVEL                      3100 // mV

static uint16_t BatteryVoltage = BATTERY_MAX_LEVEL;

uint16_t BoardBatteryMeasureVolage( void )
{
  //    uint16_t vdd = 0;
  //    uint16_t vref = VREFINT_CAL;
  //    uint16_t vdiv = 0;
  uint16_t batteryVoltage = 0;
  
  //    vdiv = AdcReadChannel( &Adc, BAT_LEVEL_CHANNEL );
  //    //vref = AdcReadChannel( &Adc, ADC_CHANNEL_VREFINT );
  //
  //    vdd = ( float )FACTORY_POWER_SUPPLY * ( float )VREFINT_CAL / ( float )vref;
  //    batteryVoltage = vdd * ( ( float )vdiv / ( float )ADC_MAX_VALUE );
  //
  //    //                                vDiv
  //    // Divider bridge  VBAT <-> 470k -<--|-->- 470k <-> GND => vBat = 2 * vDiv
  //    batteryVoltage = 2 * batteryVoltage;
  return batteryVoltage;
}

uint32_t BoardGetBatteryVoltage( void )
{
  return BatteryVoltage;
}

uint8_t BoardGetBatteryLevel( void )
{
  uint8_t batteryLevel = 0;
  
  BatteryVoltage = BoardBatteryMeasureVolage( ); //现在是0
  
  if( GetBoardPowerSource( ) == USB_POWER )
  {
    batteryLevel = 0;
  }
  else
  {
    if( BatteryVoltage >= BATTERY_MAX_LEVEL )
    {
      batteryLevel = 254;
    }
    else if( ( BatteryVoltage > BATTERY_MIN_LEVEL ) && ( BatteryVoltage < BATTERY_MAX_LEVEL ) )
    {
      batteryLevel = ( ( 253 * ( BatteryVoltage - BATTERY_MIN_LEVEL ) ) / ( BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL ) ) + 1;
    }
    else if( ( BatteryVoltage > BATTERY_SHUTDOWN_LEVEL ) && ( BatteryVoltage <= BATTERY_MIN_LEVEL ) )
    {
      batteryLevel = 1;
    }
    else //if( BatteryVoltage <= BATTERY_SHUTDOWN_LEVEL )
    {
      batteryLevel = 255;
      //GpioInit( &DcDcEnable, DC_DC_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
      //GpioInit( &BoardPowerDown, BOARD_POWER_DOWN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    }
  }
	DebugPrintf("batteryLevel is %d\r\n",batteryLevel);
  return batteryLevel;
}

static void BoardUnusedIoInit( void )
{
  Gpio_t ioPin;
  //为了满足低功耗需求，没有使用的GPIO需要这样处理：
  //如果你要使用某些引脚，需要把这个地方相应的引脚注释掉
//  GpioInit( &ioPin, UNUSEDPINPA0, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPA1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );           
//  GpioInit( &ioPin, UNUSEDPINPA2, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPA3, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPA4, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPA5, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPA6, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPA7, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 ); 
//	
//  GpioInit( &ioPin, UNUSEDPINPA8, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPA9, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPA10, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
  GpioInit( &ioPin, UNUSEDPINPA11, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
  GpioInit( &ioPin, UNUSEDPINPA12, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPA13, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );           
//  GpioInit( &ioPin, UNUSEDPINPA14, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 ); 
//	
//  GpioInit( &ioPin, UNUSEDPINPA15, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  
//  GpioInit( &ioPin, UNUSEDPINPB0, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPB1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );           
//  GpioInit( &ioPin, UNUSEDPINPB2, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );      
//  GpioInit( &ioPin, UNUSEDPINPB3, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//	
//  GpioInit( &ioPin, UNUSEDPINPB4, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPB5, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPB6, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//	
//  GpioInit( &ioPin, UNUSEDPINPB7, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );           
//  GpioInit( &ioPin, UNUSEDPINPB8, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPB9, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//	
//  GpioInit( &ioPin, UNUSEDPINPB10, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPB11, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//	
//  GpioInit( &ioPin, UNUSEDPINPB12, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPB13, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );           
//  GpioInit( &ioPin, UNUSEDPINPB14, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//  GpioInit( &ioPin, UNUSEDPINPB15, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );    
//  
//	//abut
//  GpioInit( &ioPin, UNUSEDPINPC13, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
  
#if defined( USE_DEBUGGER )
  HAL_DBGMCU_EnableDBGStopMode( );
  //HAL_DBGMCU_EnableDBGSleepMode( );
  HAL_DBGMCU_EnableDBGStandbyMode( );

#else
  //HAL_DBGMCU_DisableDBGSleepMode( );
  //HAL_DBGMCU_DisableDBGStopMode( );
  //HAL_DBGMCU_DisableDBGStandbyMode( );
  
  //慎用！！低功耗下关闭SWD将导致无法用STLINK仿真烧录程序！！！
  //    GpioInit( &ioPin, SWDIO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
  //    GpioInit( &ioPin, SWCLK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

void SleepIOConfig(void)
{
	//SpiDeInit(&ADXL357.Spi);
	//GpioInit( &ADXL357.Spi.Nss, SPI2_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
}

void WakeupIOConfig(void)
{
	//SpiInit(&ADXL357.Spi,SPI2_MOSI, SPI2_MISO, SPI2_SCLK, NC);
//	DelayMs(100);
	//ADXL357Init();
}

void SystemClockConfig( void )
{

  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	
	__HAL_RCC_PWR_CLK_ENABLE( ); //要访问 RTC 和 RTC 备份区域就必须先使能电源时钟
  HAL_PWR_EnableBkUpAccess(); //关闭写保护，允许访问备份区域
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
															|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	//RCC ->APB2ENR |=0x01<<12;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
  
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
	
	HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq( ) / 1000 );

	HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

	// SysTick_IRQn interrupt configuration
	HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
	HAL_NVIC_EnableIRQ( SysTick_IRQn );
}
		

void CalibrateSystemWakeupTime( void )
{
  if( SystemWakeupTimeCalibrated == false )
  {
    TimerInit( &CalibrateSystemWakeupTimeTimer, OnCalibrateSystemWakeupTimeTimerEvent );
    TimerSetValue( &CalibrateSystemWakeupTimeTimer, 1000 );
    TimerStart( &CalibrateSystemWakeupTimeTimer );
//		DelayMs(2000);
//		TimerInit( &TEST5S, TEST5 );
//		TimerSetValue( &TEST5S, 5000 );
//		TimerStart( &TEST5S );
//		
//		TimerInit( &TEST10S, TEST10 );
//		TimerSetValue( &TEST10S, 10000 );
//		TimerStart( &TEST10S );
//		
//		TimerInit( &TEST15S, TEST15 );
//		TimerSetValue( &TEST15S, 15000 );
//		TimerStart( &TEST15S );		
		
		//DebugPrintf("Alarm start\r\n");
    while( SystemWakeupTimeCalibrated == false )
    {
			//DebugPrintf("SystemWakeupTimeCalibrated = %d\r\n",SystemWakeupTimeCalibrated);
			DelayMs(10);
      //TimerLowPowerHandler( );
    }
  }
}

void SystemClockReConfig( void )
{
  __HAL_RCC_PWR_CLK_ENABLE( );
  //__HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );
  
  /* Enable HSI */
  __HAL_RCC_HSI_ENABLE();
  __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(RCC_HSICALIBRATION_DEFAULT);   
    
  /* Wait till HSI is ready */
  while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSIRDY ) == RESET )
  {
  }
   
  /* Enable PLL */
  __HAL_RCC_PLL_ENABLE( );
  
  /* Wait till PLL is ready */
  while( __HAL_RCC_GET_FLAG( RCC_FLAG_PLLRDY ) == RESET )
  {
  }
  
  /* Select PLL as system clock source */
  __HAL_RCC_SYSCLK_CONFIG ( RCC_SYSCLKSOURCE_PLLCLK );
  
  /* Wait till PLL is used as system clock source */
  while( __HAL_RCC_GET_SYSCLK_SOURCE( ) != RCC_SYSCLKSOURCE_STATUS_PLLCLK )
  {
  }
}

void SysTick_Handler( void )
{
  HAL_IncTick( );
  HAL_SYSTICK_IRQHandler( );
}

uint8_t GetBoardPowerSource( void )
{
#if defined( USE_USB_CDC )
  if( GpioRead( &UsbDetect ) == 1 )
  {
    return BATTERY_POWER;
  }
  else
  {
    return USB_POWER;
  }
#else
  return BATTERY_POWER;
#endif
}

uint32_t HexToString(/*IN*/  const char    * pHex,  
                     /*IN*/  uint32_t           hexLen,  
                     /*OUT*/ char          * pByteString)  
{  
  unsigned long i;  
  
  if (pHex==NULL)  
    return 1;  
  
  if(hexLen <= 0)  
    return 2;  
  
  for(i=0;i<hexLen;i++)  
  {  
    if(((pHex[i]&0xf0)>>4)>=0 && ((pHex[i]&0xf0)>>4)<=9)  
      pByteString[2*i]=((pHex[i]&0xf0)>>4)+0x30;  
    else if(((pHex[i]&0xf0)>>4)>=10 && ((pHex[i]&0xf0)>>4)<=16)  
      pByteString[2*i]=((pHex[i]&0xf0)>>4)+0x37;   //  小写：0x37 改为 0x57   
    
    if((pHex[i]&0x0f)>=0 && (pHex[i]&0x0f)<=9)  
      pByteString[2*i+1]=(pHex[i]&0x0f)+0x30;  
    else if((pHex[i]&0x0f)>=10 && (pHex[i]&0x0f)<=16)  
      pByteString[2*i+1]=(pHex[i]&0x0f)+0x37;      //  小写：0x37 改为 0x57   
  }  
  return 0;  
} 



#ifdef USE_FULL_ASSERT
/*
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*/
void assert_failed( uint8_t* file, uint32_t line )
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while( 1 )
  {
  }
}
#endif
