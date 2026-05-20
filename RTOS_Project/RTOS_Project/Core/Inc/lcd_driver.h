/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "main.h"
#include "spi.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>	
#include <stdint.h>
#include "stm32f4xx.h"
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 
  
/** @addtogroup LCD
  * @{
  */

/** @defgroup LCD_Exported_Types
  * @{
  */
   
/**
  * @}
  */ 

/** @defgroup LCD_Exported_Constants
  * @{
  */
      
/** 
  * @brief  LCD Size  
  */
#define LCD_WIDTH				240
#define LCD_HEIGHT			320

/** 
  * @brief  LCD ID  
  */  


/**
  * @}
  */
	
#define  MAX_BMP_FILES  25 
	
/** 
  * @brief  LCD ID  
  */  
#define LCD_CMD					0
#define LCD_DATA				1
/**
  * @}
  */

#define FONT_1206				12
#define FONT_1608				16
	
#define WHITE						0xFFFF
#define BLACK						0x0000	  
#define BLUE						0x001F  
#define BRED						0XF81F
#define GRED						0XFFE0
#define GBLUE						0X07FF
#define RED							0xF800
#define MAGENTA					0xF81F
#define GREEN						0x07E0
#define CYAN						0x7FFF
#define YELLOW					0xFFE0
#define BROWN						0XBC40 
#define BRRED						0XFC07 
#define GRAY						0X8430

 /* PB3( SCK )   PB4( TIRQ )  PB6 ( BKL )  PB7( LCD_CS )  */
 /* PB8( DC )    PB9( TCS )   PA6 ( SDO )  PA7( SDI )  	 */
 /* PB2( RST ) */
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))	
/**
  * @brief  LCD Control Lines management
  */
#define __LCD_CS_CLR()      HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_Pin, GPIO_PIN_RESET)
#define __LCD_CS_SET()      HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_Pin, GPIO_PIN_SET)
#define __LCD_DC_CLR()      HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_Pin, GPIO_PIN_RESET)
#define __LCD_DC_SET()      HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_Pin, GPIO_PIN_SET)
#define __LCD_BKL_SET()			HAL_GPIO_WritePin(LCD_BL_GPIO_PORT, LCD_BL_Pin, GPIO_PIN_SET)
#define __LCD_BKL_CLR()			HAL_GPIO_WritePin(LCD_BL_GPIO_PORT, LCD_BL_Pin, GPIO_PIN_RESET)
#define __LCD_RST_SET()			HAL_GPIO_WritePin(LCD_RST_GPIO_PORT, LCD_RST_Pin, GPIO_PIN_SET)
#define __LCD_RST_CLR()			HAL_GPIO_WritePin(LCD_RST_GPIO_PORT, LCD_RST_Pin, GPIO_PIN_RESET)
//#define __TP_CS_CLR()     HAL_GPIO_WritePin(TP_CS_GPIO_PORT,   TP_CS_Pin,   GPIO_PIN_RESET)
//#define __TP_CS_SET()    HAL_GPIO_WritePin(TP_CS_GPIO_PORT,   TP_CS_Pin,   GPIO_PIN_SET)
//#define __TP_IRQ_READ()   HAL_GPIO_ReadPin (TP_IRQ_GPIO_PORT,  TP_IRQ_Pin)
/**
  * @brief  LCD Control Interface pins (shield D10)
  *
  */
#define TP_CS_Pin 			GPIO_PIN_9
#define TP_CS_GPIO_Port 		GPIOA
#define TP_IRQ_Pin 			GPIO_PIN_4
#define TP_IRQ_GPIO_Port 		GPIOB

#define LCD_CS_Pin                                GPIO_PIN_7
#define LCD_CS_GPIO_PORT                           GPIOB
#define LCD_CS_GPIO_CLK_ENABLE()                   __HAL_RCC_GPIOB_CLK_ENABLE()
#define LCD_CS_GPIO_CLK_DISABLE()                  __HAL_RCC_GPIOB_CLK_DISABLE()
#define LCD_BL_Pin                                 GPIO_PIN_6
#define LCD_BL_GPIO_PORT                           GPIOB
#define LCD_RST_Pin                                GPIO_PIN_2
#define LCD_RST_GPIO_PORT                          GPIOB
/**
  * @brief  LCD Data/Command Interface pins
  */
#define LCD_DC_Pin                                 GPIO_PIN_8
#define LCD_DC_GPIO_PORT                           GPIOB
#define LCD_DC_GPIO_CLK_ENABLE()                   __HAL_RCC_GPIOB_CLK_ENABLE()
#define LCD_DC_GPIO_CLK_DISABLE()                  __HAL_RCC_GPIOB_CLK_DISABLE()
/*------------------------------------------------------------------------------------------------------*/

#define  ST7789V		0x52
#define  HX8347D		0x47


 ///touch

#define TP_CS_Pin          GPIO_PIN_9
#define TP_IRQ_Pin         GPIO_PIN_4

#define TP_CS_GPIO_Port         GPIOA
#define TP_IRQ_GPIO_Port        GPIOB

#define __TP_CS_SET()      HAL_GPIO_WritePin(TP_CS_GPIO_Port, TP_CS_Pin, GPIO_PIN_SET)
#define __TP_CS_CLR()      HAL_GPIO_WritePin(TP_CS_GPIO_Port, TP_CS_Pin, GPIO_PIN_RESET)

#define __TP_IRQ_READ()    HAL_GPIO_ReadPin(TP_IRQ_GPIO_Port, TP_IRQ_Pin)

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup ADAFRUIT_SPI_LCD_Exported_Functions
  * @{
  */ 
void lcd_init(void);
uint8_t lcd_read_id(void);
void lcd_write_byte(uint8_t chByte, uint8_t chCmd);
void lcd_set_cursor( uint16_t hwXpos, uint16_t hwYpos);
void lcd_write_ram_prepare(void);
void lcd_write_ram( uint16_t hwData);
void lcd_set_window( uint16_t hwXpos, uint16_t hwYpos, uint16_t hwWidth, uint16_t hwHeight);
void lcd_draw_point( uint16_t hwXpos, uint16_t hwYpos, uint16_t hwColor);
void lcd_clear_screen( uint16_t hwColor);
void lcd_display_char(	uint16_t hwXpos, //specify x position.
															uint16_t hwYpos, //specify y position.
															uint8_t chChr,   //a char is display.
															uint8_t chSize,  //specify the size of the char
															uint16_t hwColor); //specify the color of the char
void lcd_display_num(	uint16_t hwXpos,  //specify x position.
															uint16_t hwYpos, //specify y position.
															uint32_t chNum,  //a number is display.
															uint8_t chLen,   //length ot the number
															uint8_t chSize,  //specify the size of the number
															uint16_t hwColor); //specify the color of the number
void lcd_display_string( uint16_t hwXpos, //specify x position.
																uint16_t hwYpos,   //specify y position.
																const uint8_t *pchString,  //a pointer to string
																uint8_t chSize,    // the size of the string 
																uint16_t hwColor);  // specify the color of the string 
void lcd_draw_line( 	uint16_t hwXpos0, //specify x0 position.
														uint16_t hwYpos0, //specify y0 position.
														uint16_t hwXpos1, //specify x1 position.
														uint16_t hwYpos1, //specify y1 position.
														uint16_t hwColor); //specify the color of the line
void lcd_draw_circle( 	uint16_t hwXpos,  //specify x position.
															uint16_t hwYpos,  //specify y position.
															uint16_t hwRadius, //specify the radius of the circle.
															uint16_t hwColor);  //specify the color of the circle.
void lcd_fill_rect( uint16_t hwXpos,  //specify x position.
													 uint16_t hwYpos,  //specify y position.
													 uint16_t hwWidth, //specify the width of the rectangle.
													 uint16_t hwHeight, //specify the height of the rectangle.
													 uint16_t hwColor);  //specify the color of rectangle.
void lcd_draw_v_line( 	uint16_t hwXpos, //specify x position.
															uint16_t hwYpos, //specify y position. 
															uint16_t hwHeight, //specify the height of the vertical line.
															uint16_t hwColor);  //specify the color of the vertical line.
void lcd_draw_h_line(	uint16_t hwXpos, //specify x position.
															uint16_t hwYpos,  //specify y position. 
															uint16_t hwWidth, //specify the width of the horizonal line.
															uint16_t hwColor); //specify the color of the horizonal line.
void lcd_draw_rect( 	uint16_t hwXpos,  //specify x position.
														uint16_t hwYpos,  //specify y position.
														uint16_t hwWidth, //specify the width of the rectangle.
														uint16_t hwHeight, //specify the height of the rectangle.
														uint16_t hwColor);  //specify the color of rectangle.

void lcd_draw_triangle (uint16_t x0, uint16_t y0,
                        uint16_t x1, uint16_t y1,
                        uint16_t x2, int16_t  y2,
                        uint16_t hwColor);

void fillTriangle      (uint16_t x0, uint16_t y0,
                        uint16_t x1, uint16_t y1,
                        uint16_t x2, int16_t  y2,
                        uint16_t color);

void lcd_fill_triangle (uint16_t x0, uint16_t y0,
                        uint16_t x1, uint16_t y1,
                        uint16_t x2, int16_t  y2,
                        uint16_t color);


void setRotation( uint8_t rotation);
void lcd_write_word( uint16_t hwData);
void LCD_ShowModeStatus(uint8_t is_playing);
/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif /* __LCD_H */

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */
