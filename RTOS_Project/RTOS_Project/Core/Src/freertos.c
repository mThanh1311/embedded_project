/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "lcd_driver.h"
#include "spi.h"
#include "gpio.h"
#include "XPT2046.h"
#include "fram.h"
#include "string.h"
#include <math.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* ================================================================
   CẤU TRÚC DỮ LIỆU NHIỆT ĐỘ — gửi qua tempQueue
   ================================================================ */
typedef struct {
    float    celsius;        /* Nhiệt độ °C                       */
    uint32_t timestamp_ms;   /* HAL_GetTick() tại thời điểm đọc   */
    uint8_t  overheat;       /* 1 = vượt ngưỡng 40°C              */
    uint8_t  pad[3];         /* Padding cho căn chỉnh 4 byte      */
} TempMsg_t;
/* ================================================================
   HẰNG SỐ CẤU HÌNH
   ================================================================ */
#define TEMP_THRESHOLD      40.0f   /* Ngưỡng quá nhiệt (°C)        */
#define TEMP_DEMO_OFFSET    7.7f    /* phục vụ Demo 				*/
#define TEMP_READ_MS        500     /* Chu kỳ đọc nhiệt độ (ms)     */
#define OVERHEAT_TICKS      10      /* 10 lần × 500ms = 5 giây     	*/
#define Z_THRESHOLD         60      /* Ngưỡng lực chạm XPT2046      */

#define TEMP_TEXT_X         115
#define TEMP_TEXT_Y         57
/* ================================================================
   ĐỊA CHỈ CALIBRATION ADC — System Flash STM32F405
   ================================================================ */
#define TS_CAL1  (*((volatile uint16_t *)0x1FFF7A2CU))  /* Raw tại 30°C  */
#define TS_CAL2  (*((volatile uint16_t *)0x1FFF7A2EU))  /* Raw tại 110°C */


/* Biến trạng thái toàn cục (volatile — dùng chung giữa tasks) */
/* Vùng hiển thị nhiệt độ (bên trong khung y=50) */

static volatile uint8_t    	s_overheat_count = 0;    /* Bộ đếm 5 giây     */
static volatile uint16_t 	s_pause_overheat_count = 0; /* Bộ đếm 5 giây cho PAUSE*/
static volatile uint16_t   	s_total_overheat = 0;    /* Tổng số lần quá nhiệt */
static volatile uint16_t   	s_fram_index     = 0;    /* Con trỏ ghi FRAM   */
static volatile float      	s_last_temp      = 0.0f; /* Nhiệt độ lần đọc cuối */
static volatile uint8_t    	s_monitoring_enabled = 0;/* 1 = Play, 0 = Pause */
static volatile uint8_t    	s_fram_ready     = 0;    /* 1 = FRAM OK        */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern void tp_adjust(void);
extern ADC_HandleTypeDef hadc1;

extern SPI_HandleTypeDef hspi1;
/* USER CODE BEGIN PV */
extern UART_HandleTypeDef huart2;

/* USER CODE END PV */
// --- HÀM THAY ĐỔI TỐC ĐỘ SPI ---
// XPT2046 cần tốc độ chậm (VD: Prescaler 128)
void SPI_SetSpeed_Slow(void) {
    __HAL_SPI_DISABLE(&hspi1);
    hspi1.Instance->CR1 = (hspi1.Instance->CR1 & ~SPI_CR1_BR) | SPI_BAUDRATEPRESCALER_64;
    __HAL_SPI_ENABLE(&hspi1);
}

// LCD ST7789/HX8347 cần tốc độ nhanh nhất (VD: Prescaler 2 hoặc 4)
void SPI_SetSpeed_Fast(void) {
    __HAL_SPI_DISABLE(&hspi1);
    hspi1.Instance->CR1 = (hspi1.Instance->CR1 & ~SPI_CR1_BR) | SPI_BAUDRATEPRESCALER_4;
    __HAL_SPI_ENABLE(&hspi1);
}
/* USER CODE END Variables */

static void uart_print(const char *s)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)s, strlen(s), 100);
}

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for touchTask */
osThreadId_t touchTaskHandle;
const osThreadAttr_t touchTask_attributes = {
  .name = "touchTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for TempTask */
osThreadId_t TempTaskHandle;
const osThreadAttr_t TempTask_attributes = {
  .name = "TempTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for tempQueue */
osMessageQueueId_t tempQueueHandle;
const osMessageQueueAttr_t tempQueue_attributes = {
  .name = "tempQueue"
};
/* Definitions for spiMutex */
osMutexId_t spiMutexHandle;
const osMutexAttr_t spiMutex_attributes = {
  .name = "spiMutex"
};
/* Definitions for touchSem */
osSemaphoreId_t touchSemHandle;
const osSemaphoreAttr_t touchSem_attributes = {
  .name = "touchSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void LCD_Displaytest(void){
	lcd_clear_screen(BLUE);
	//khung ten nhom
	lcd_fill_rect(7, 7, 98, 30, WHITE); // Vẽ khung
	lcd_display_string(14,14,(const uint8_t *)"NHOM 11",FONT_1608,BLACK);

	//khung Nhiet do:
	lcd_fill_rect(7, 50, 226, 30, WHITE);
	lcd_display_string(14, 57, (const uint8_t *)"Nhiet do chip: ",FONT_1608,BLACK);

	//khung buttons
	lcd_fill_rect(7, 93, 226, 80, WHITE);
	//playbutton();
	lcd_draw_rect(20,95,68,70,BLUE);
	lcd_fill_triangle(40,110,40,152, 70,132, BLUE);

	//pausebutton();
	lcd_draw_rect(132, 95, 72, 70, BLUE);
	lcd_fill_rect(152, 110, 15, 42, BLUE);
	lcd_fill_rect(175, 110, 15, 42, BLUE);

	//khung data
	lcd_fill_rect(7,186, 226, 320-7-186, WHITE); //tô màu trong khung
	lcd_display_string(14, 193, (const uint8_t *)"Data: PAUSED!     ",FONT_1608,BLUE);
}

/* ================================================================
   LCD — Màn hình cảnh báo SHUTDOWN
   ================================================================ */
static void LCD_DrawShutdown(void)
{
    lcd_clear_screen(BLACK);

    /* Tam giác cảnh báo */
    lcd_fill_triangle(120, 40, 40, 180, 200, 180, RED);
    lcd_fill_triangle(120, 55, 52, 168, 188, 168, 0xFC00);

    /* Dấu chấm than */
    lcd_fill_rect(114, 80,  12, 50, RED);
    lcd_fill_rect(114, 142, 12, 12, RED);

    lcd_display_string(30, 200,
        (const uint8_t *)"SYSTEM SHUTDOWN", FONT_1608, RED);
    lcd_display_string(10, 220,
        (const uint8_t *)"Chip > 40C trong 5s", FONT_1608, 0xFC00);
    lcd_display_string(5, 245,
        (const uint8_t *)"Nhan WAKEUP (RESET) de bat", FONT_1608, WHITE);
    lcd_display_string(20, 265,
        (const uint8_t *)"FRAM luu: ", FONT_1608, WHITE);

    char buf[16];
    sprintf(buf, "%d ban ghi", s_total_overheat);
    lcd_display_string(110, 265, (const uint8_t *)buf, FONT_1608, 0xFFE0);
}

static void EnterStopMode(void)
{
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
//	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

	HAL_SuspendTick();
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	HAL_ResumeTick();

	NVIC_SystemReset();
}

static void EnterStandbyMode(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

    /* Clear wakeup flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

    /* Enable PA0 WKUP pin */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

    HAL_SuspendTick();

    /* Enter Standby mode. Wakeup by PA0 will reset/boot MCU */
    HAL_PWR_EnterSTANDBYMode();
}

/* ----------------------------------------------------------------
   Kiểm tra lực chạm hợp lệ
   ---------------------------------------------------------------- */
uint8_t is_touch_pressed(void) {
    uint16_t z1 = xpt2046_read_average(0xB0);
    return (z1 > Z_THRESHOLD) ? 1 : 0;
}


static float Read_ChipTemp(void)
{
    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel      = ADC_CHANNEL_TEMPSENSOR;
    cfg.Rank         = 1;
    cfg.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &cfg);

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        HAL_ADC_Stop(&hadc1);
        return -999.0f;
    }
    uint16_t raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    /* Hiệu chuẩn và tính nhiệt độ theo CAL1 (30°C) và CAL2 (110°C) */
    float temp = ((110.0f - 30.0f) / (float)((int32_t)TS_CAL2 - TS_CAL1))
                 * (float)((int32_t)raw - TS_CAL1)
                 + 30.0f;

    /* Debug UART */
//    char dbg[80];
//    snprintf(dbg, sizeof(dbg),
//             "[ADC] raw=%u temp=%.1f C CAL1=%lu CAL2=%lu\r\n",
//             raw,
//             temp,
//             (uint32_t)TS_CAL1,
//             (uint32_t)TS_CAL2);
//
//    HAL_UART_Transmit(&huart2, (uint8_t*)dbg, strlen(dbg), 100);

    return temp;
}


/* ================================================================
   HÀM LÀM NÓNG CHIP — Kết hợp CPU + FPU + Flash access
   Gọi mỗi chu kỳ 500ms khi STATE_MONITOR
   ================================================================ */
//static void CPU_HeatUp(void)
//{
//    const uint32_t target_ms = 900U;
//    uint32_t start = HAL_GetTick();
//
//    volatile float a = 1.23456789f;
//    volatile float b = 9.87654321f;
//
//    static volatile uint32_t mem_buf[512];
//
//    while ((HAL_GetTick() - start) < target_ms)
//    {
//        for (uint32_t k = 0; k < 1200; k++)
//        {
//            a = a * b + 0.000001f;
//            b = b / (a + 1e-6f) + 0.000001f;
//            a = sqrtf(a * a + b * b);
//        }
//
//        for (uint32_t i = 0; i < 8192; i++)
//        {
//            uint32_t idx = i & 0x1FFu;
//            mem_buf[idx] =
//                (mem_buf[idx] << 3) ^
//                (mem_buf[(idx + 37) & 0x1FFu] + 0x9E3779B1u);
//        }
//    }
//
//    (void)a;
//    (void)b;
//}

/* USER CODE BEGIN FunctionPrototypes */

/* ================================================================
   CÁC PHƯƠNG PHÁP LÀM NÓNG — Gọi tuần tự trong TempTask
   ================================================================ */

/* ---- Phương pháp 1: FPU tính toán nặng ---- */
static void Heat_FPU(void)
{
    volatile float a = 1.23456789f;
    volatile float b = 9.87654321f;
    volatile float c = 0.0f;

    for (uint32_t i = 0; i < 300000; i++) {
        a = sqrtf(a * a + b * b);   /* sqrt — nặng nhất cho FPU */
        b = sinf(a) + cosf(b);      /* sin/cos — rất tốn điện   */
        c = a * b + sqrtf(c + 1.0f);
        a = c / (b + 0.001f);
    }
    (void)a; (void)b; (void)c;
}

/* ---- Phương pháp 2: ALU + Memory thrashing ---- */
static void Heat_ALU_MEM(void)
{
    static volatile uint32_t buf[64];  /* RAM access liên tục */
    volatile uint32_t x = 0xDEADBEEF;

    for (uint32_t i = 0; i < 200000; i++) {
        /* Phép tính ALU + ghi đọc RAM */
        x  = x * 1664525UL + 1013904223UL;
        x ^= (x >> 13);
        x ^= (x << 17);
        x ^= (x >>  5);
        buf[i % 64] = x;        /* Cache thrashing */
        x += buf[(i+1) % 64];
    }
    (void)x;
}

/* ---- Phương pháp 3: Bật nhiều peripheral cùng lúc ---- */
static void Heat_Peripheral(void)
{
    /* Bật clock cho nhiều peripheral → tiêu thụ điện tăng */
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_CRC_CLK_ENABLE();

    /* CRC tính liên tục trên block data */
    CRC->CR = CRC_CR_RESET;
    for (uint32_t i = 0; i < 50000; i++) {
        CRC->DR = i * 0x1234ABCDUL;
    }
    volatile uint32_t crc_val = CRC->DR;
    (void)crc_val;
}

/* ---- Phương pháp 4: Flash read liên tục ---- */
static void Heat_FlashRead(void)
{
    /* Đọc liên tục từ Flash → tốn điện trên Flash controller */
    extern uint32_t _stext;   /* Địa chỉ đầu Flash */
    volatile uint32_t *ptr = (volatile uint32_t *)0x08000000;
    volatile uint32_t  sum = 0;

    for (uint32_t i = 0; i < 100000; i++) {
        sum += ptr[i % 4096];  /* Đọc 16KB Flash liên tục */
    }
    (void)sum;
}

/* ================================================================
   CPU_HeatUp — Tổng hợp tất cả phương pháp
   Gọi mỗi chu kỳ TEMP_READ_MS (500ms) trong TempTask
   ================================================================ */
static void CPU_HeatUp(void)
{
    Heat_FPU();         /* ~150ms — FPU nặng nhất   */
    Heat_ALU_MEM();     /* ~100ms — ALU + RAM        */
    Heat_Peripheral();  /* ~50ms  — CRC peripheral   */
    /* Heat_FlashRead() — tuỳ chọn thêm nếu cần     */
}

/* USER CODE END FunctionPrototypes */


static void LCD_UpdateTemp(float celsius, uint8_t overheat)
{
    char buf[24];
    int  t_int  = (int)celsius;
    int  t_frac = (int)((celsius - (float)t_int) * 10.0f);
    if (t_frac < 0) t_frac = -t_frac;

    sprintf(buf, "%d.%01d C  ", t_int, t_frac);

    /* Xóa vùng số cũ */
    lcd_fill_rect(TEMP_TEXT_X + 10, TEMP_TEXT_Y, 100, 14, WHITE);

    uint16_t col = overheat ? RED : BLACK;
    lcd_display_string(TEMP_TEXT_X + 15, TEMP_TEXT_Y,
        (const uint8_t *)buf, FONT_1608, col);
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */
	/* Create the mutex(es) */
	/* creation of spiMutex */
	spiMutexHandle = osMutexNew(&spiMutex_attributes);

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* Create the semaphores(s) */
	/* creation of touchSem */
	// ban đầu chưa có touch interrupt thì TouchTask không nên tự chạy
	touchSemHandle = osSemaphoreNew(1, 0, &touchSem_attributes);

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* creation of tempQueue */
	tempQueueHandle = osMessageQueueNew (4, sizeof(TempMsg_t), &tempQueue_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

	/* creation of touchTask */
	touchTaskHandle = osThreadNew(StartTask02, NULL, &touchTask_attributes);

	/* creation of TempTask */
	TempTaskHandle = osThreadNew(StartTask03, NULL, &TempTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
/* USER CODE BEGIN StartDefaultTask */

	// Vẽ layout giao diện tĩnh ban đầu. Xin Mutex trước khi vẽ
	if(osMutexAcquire(spiMutexHandle, 100) == osOK) {
		LCD_Displaytest();
		osMutexRelease(spiMutexHandle);
	}
	/* Infinite loop */
	TempMsg_t  msg;
	//char       line1[24], line2[24], line3[24];

	for(;;)
	{
		if (osMessageQueueGet(tempQueueHandle, &msg,  NULL, 100) == osOK) {
			/* Cập nhật ô nhiệt độ */
			if (osMutexAcquire(spiMutexHandle, 50) == osOK) {
				SPI_SetSpeed_Fast();
				LCD_UpdateTemp(msg.celsius, msg.overheat);
				osMutexRelease(spiMutexHandle);
			}
		}

		// Ngủ 50ms (Được khoảng 50 khung hình/giây)
		osDelay(20);
	}
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the touchTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */

	uint16_t raw_x = 0;
	uint16_t raw_y = 0;
	uint16_t pixel_x = 0;
	uint16_t pixel_y = 0;

	xpt2046_init();  // Khởi tạo chip cảm ứng


	//***********************************************
	for(;;)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(TP_IRQ_Pin); // Xóa cờ ngắt
		// Chờ ngắt EXTI báo có người chạm
		if(osSemaphoreAcquire(touchSemHandle, osWaitForever) == osOK)
		{
			osDelay(20); // Debounce: Chờ 20ms để tay chạm ổn định
			if(osMutexAcquire(spiMutexHandle, 100) == osOK)
			{
				SPI_SetSpeed_Slow();
				// Đọc thẳng giá trị trung bình 1 lần duy nhất
				raw_x = xpt2046_filter_read(0xD0); // Trục X
				raw_y = xpt2046_filter_read(0x90); // Trục Y

				SPI_SetSpeed_Fast();
				osMutexRelease(spiMutexHandle);

				if(raw_x > 100 && raw_x < 3900 && raw_y > 100 && raw_y < 3900)
				{
					// Chuyển đổi tọa độ Raw sang Pixel
					// 1. Ép kiểu và tính toán tọa độ X (Từ 1370 về 168 thành 0 đến 240)
					// Công thức: pixel = (raw - max) * width / (min - max)
					// 2. Ép kiểu và tính toán tọa độ Y (Từ 1957 về 197 thành 0 đến 320)
					// Công thức: pixel = (raw - max) * height / (min - max)
					pixel_x = (raw_x - 1370) * 240 / (168 - 1370) + 50;
					pixel_y = (raw_y - 1957) * 320 / (197 - 1957);
					if(pixel_x > 240) pixel_x = 240;
					if(pixel_y > 320) pixel_y = 320;
				}
				// ==========================================
				// XỬ LÝ TÁC VỤ DỰA TRÊN VÙNG CHẠM
				// ==========================================
				// Kiểm tra nếu chạm vào vùng nút PAUSE
				if (pixel_x >= 132 && pixel_x <= 204 && pixel_y >= 95 && pixel_y <= 165) //132 95 72 70
				{
				  if (s_monitoring_enabled != 0)
					{
					    s_monitoring_enabled = 0;
					    s_overheat_count = 0;
					    s_pause_overheat_count = 0;

						if (osMutexAcquire(spiMutexHandle, 50) == osOK) {
							SPI_SetSpeed_Fast();

							lcd_fill_triangle(40,110,40,152, 70,132, BLUE);
							lcd_fill_rect(152, 110, 15, 42, GREEN);
							lcd_fill_rect(175, 110, 15, 42, GREEN);

							LCD_ShowModeStatus(0);

							osMutexRelease(spiMutexHandle);
						}
					}
				}
				// Kiểm tra nếu chạm vào vùng nút PLAY
				else if (pixel_x >= 20 && pixel_x <= 88 && pixel_y >= 95 && pixel_y <= 165)
				{
					if (s_monitoring_enabled != 1)
					{
						s_monitoring_enabled = 1;
						s_overheat_count = 0;

						if (osMutexAcquire(spiMutexHandle, 50) == osOK) {
							SPI_SetSpeed_Fast();
							lcd_fill_rect(152, 110, 15, 42, BLUE);
							lcd_fill_rect(175, 110, 15, 42, BLUE);
							lcd_fill_triangle(40,110,40,152, 70,132, GREEN);
							LCD_ShowModeStatus(1);
							osMutexRelease(spiMutexHandle);
						}
					}
				}
				// Nếu chạm vào chỗ khác
				else {
//					if (is_touch_pressed() == 1){
//						if (osMutexAcquire(spiMutexHandle, 50) == osOK) {
//							  SPI_SetSpeed_Fast();
//							  lcd_draw_circle(pixel_x, pixel_y, 3, RED);
//							  osMutexRelease(spiMutexHandle);
//						}
//					}
					// Nothing...

				}
				// CHỜ CHO ĐẾN KHI NGƯỜI DÙNG NHẤC TAY LÊN MỚI ĐI TIẾP
				uint32_t wait_start = HAL_GetTick();
				while(HAL_GPIO_ReadPin(TP_IRQ_GPIO_Port, TP_IRQ_Pin) == GPIO_PIN_RESET)
				{
					osDelay(30);
				    if (HAL_GetTick() - wait_start > 1000U)
				    {
				        break;
				    }
				}
			}
		}
	}
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the TempTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
	TempMsg_t msg;
	uint32_t last_fram_read_ms = 0;
	static uint32_t last_uart_ms = 0;
	static uint32_t overheat_start_ms = 0;

	s_fram_ready = (FRAM_Init() == HAL_OK) ? 1 : 0;
	if (s_fram_ready) {
		FRAM_ReadRecord(0, &huart2);
	}
	//char      uart_buf[64];
	/* Infinite loop */
	for(;;)
	{
		/*  Đọc nhiệt độ ADC --- */
		/* ADC độc lập với SPI, không cần spiMutex */
//		float temp = Read_ChipTemp();
//		s_last_temp = temp;
//
//		// Tạo msg nhiệt độ
//		msg.celsius      = temp;
//		msg.timestamp_ms = HAL_GetTick();
//		msg.overheat     = (temp > TEMP_THRESHOLD) ? 1 : 0;

		// Phục vụ Demo
		float temp_real = Read_ChipTemp();
		float temp = temp_real + TEMP_DEMO_OFFSET;

		s_last_temp = temp;

		msg.celsius      = temp;
		msg.timestamp_ms = HAL_GetTick();
		msg.overheat     = (temp > TEMP_THRESHOLD) ? 1 : 0;

		// Debug ADC thay vì để ở Read_ChipTemp thì đặt ở đây.
		if (HAL_GetTick() - last_uart_ms >= 500) {
		    int32_t temp_x10 = (int32_t)(temp * 10.0f);

		    char buf[100];
		    snprintf(buf, sizeof(buf),
		             "[LIVE] temp=%ld.%ld oC | mode=%s | overheat=%d | play_count=%u | pause_count=%u\r\n",
		             temp_x10 / 10,
		             temp_x10 % 10,
		             s_monitoring_enabled ? "PLAY" : "PAUSE",
		             msg.overheat,
		             s_overheat_count,
		             s_pause_overheat_count);

		    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);

		    last_uart_ms = HAL_GetTick();
		}

		/*  Gửi vào queue cho DefaultTask hiển thị --- */
		// Tức là, gửi dữ liệu sang LCD task
		osMessageQueuePut(tempQueueHandle, &msg, 0, 0);
		CPU_HeatUp();
		// Tức là, nếu đang PAUSE thì bỏ qua toàn bộ phần ghi FRAM/Shutdown
		if (s_monitoring_enabled) {
			if (msg.overheat) {
//				s_overheat_count++;
		        if (overheat_start_ms == 0) {
		            overheat_start_ms = HAL_GetTick();   // bắt đầu tính 5 giây
		        }

		        s_overheat_count = (HAL_GetTick() - overheat_start_ms) / 500;

				// Nếu FRAM sẵn sàng, == 1
				if (s_fram_ready) {
					// nếu nhiệt độ > ngưỡng liên tục 5 giây, nó có thể ghi khoảng 10 bản ghi
					// Lưu ý: đây là thời gian tính từ lúc boot, chưa phải realtime ngày/giờ thật
					if (FRAM_WriteRecord(&s_fram_index, msg.timestamp_ms, temp) == HAL_OK) {
						uint16_t latest_index = (s_fram_index == 0) ? (FRAM_MAX_RECORDS - 1) : (s_fram_index - 1);
						s_total_overheat++;
						FRAM_ReadRecord(latest_index, &huart2);

						// In UART2
					    char buf[160];
					    snprintf(buf, sizeof(buf),
					             "[COMPARE] write_temp=%.1f | read_temp=%.1f oC | write_ts=%lu | read_ts=%lu | index=%u\r\n",
					             temp,
					             g_fram_read_record.temp_x10 / 10.0f,
					             msg.timestamp_ms,
					             g_fram_read_record.timestamp_ms,
					             latest_index);

					    HAL_UART_Transmit(&huart2, (uint8_t *)buf, strlen(buf), 100);

						last_fram_read_ms = HAL_GetTick();
					} else {
						s_fram_ready = 0;
					}
				}

				/* ---- Kiểm tra 5 giây liên tục ---- */
				// 1. Reset s_overheat_count
				// 2. Vẽ màn hình shutdown
				// 3. Delay 800ms
				// 4. Stop ADC
				// 5. Gọi EnterStopMode()
		        if ((HAL_GetTick() - overheat_start_ms) >= 5000U) {
		            s_overheat_count = 0;
		            overheat_start_ms = 0;

		            if (osMutexAcquire(spiMutexHandle, 200) == osOK) {
		                SPI_SetSpeed_Fast();
		                LCD_DrawShutdown();
		                osMutexRelease(spiMutexHandle);
		            }

		            uart_print("[SYSTEM] Overheat 5s. Enter Standby mode. Press WAKEUP to reboot....\r\n");

		            osDelay(2000);
		            HAL_ADC_Stop(&hadc1);
		            EnterStopMode();
//		            EnterStandbyMode();
//		            uart_print("[SYSTEM] Would enter Standby here\r\n");
//		            while (1) {
//		                GPIO_PinState wk = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
//
//		                char buf[64];
//		                snprintf(buf, sizeof(buf), "[WAKEUP TEST] PA0=%d\r\n", wk);
//		                HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);
//
//		                osDelay(500);
//		            }
//		            uint8_t wk = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
//
//		            char buf[64];
//		            snprintf(buf, sizeof(buf), "[WAKEUP TEST] PA0=%d\r\n", wk);
//		            HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);
		        }
			} else {
		        s_overheat_count = 0;
		        overheat_start_ms = 0;
			}
		} else {
	        s_overheat_count = 0;
	        overheat_start_ms = 0;
	        // PAUSE mode: không ghi FRAM, không shutdown
	        s_overheat_count = 0;

	        if (msg.overheat) {
	            s_pause_overheat_count++;
	        } else {
	            s_pause_overheat_count = 0;
	        }
		}

//		if (s_fram_ready && s_total_overheat > 0) {
//			uint32_t now = HAL_GetTick();
//			if ((now - last_fram_read_ms) >= 1000U) {
//				uint16_t latest_index = (s_fram_index == 0) ? (FRAM_MAX_RECORDS - 1) : (s_fram_index - 1);
//				FRAM_ReadRecord(latest_index, &huart2);
//				last_fram_read_ms = now;
//			}
//		}

		/* Đọc mỗi 500ms */
		 osDelay(TEMP_READ_MS);
	}
/* USER CODE END StartTask03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* ================================================================
   EXTI CALLBACK — Ngắt từ TP_IRQ (chân cảm ứng)
   Gọi osSemaphoreRelease để đánh thức TouchTask
   ================================================================ */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == TP_IRQ_Pin) {
        osSemaphoreRelease(touchSemHandle);
    }
    if (GPIO_Pin == WAKEUP_Pin) {
//        uart_print("[BUTTON] WAKEUP pressed\r\n");
    }
}
/* USER CODE END Application */

