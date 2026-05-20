#ifndef FRAM_H
#define FRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "i2c.h"
#include "usart.h"

/* Adjust FRAM_SIZE_BYTES for your exact FM24CLxx density. */
#define FRAM_I2C_ADDR          0x50u
#define FRAM_I2C_ADDR_8BIT     (FRAM_I2C_ADDR << 1)
#define FRAM_SIZE_BYTES        (8u * 1024u)
#define FRAM_RECORD_SIZE       8u
#define FRAM_MAX_RECORDS       (FRAM_SIZE_BYTES / FRAM_RECORD_SIZE)

typedef struct {
    uint32_t timestamp_ms;
    int16_t  temp_x10;
    uint16_t reserved;
} FRAM_Record_t;

extern volatile FRAM_Record_t g_fram_read_record;

HAL_StatusTypeDef FRAM_Init(void);
HAL_StatusTypeDef FRAM_Write(uint16_t addr, const uint8_t *data, uint16_t len);
HAL_StatusTypeDef FRAM_Read(uint16_t addr, uint8_t *data, uint16_t len);
HAL_StatusTypeDef FRAM_WriteRecord(volatile uint16_t *index, uint32_t timestamp_ms, float temp_c);
HAL_StatusTypeDef FRAM_ReadRecord(uint16_t index, UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* FRAM_H */
