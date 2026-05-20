#include "fram.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c2;

volatile FRAM_Record_t g_fram_read_record;

HAL_StatusTypeDef FRAM_Init(void)
{
    return HAL_I2C_IsDeviceReady(&hi2c2, FRAM_I2C_ADDR_8BIT, 2, 10);
}

HAL_StatusTypeDef FRAM_Write(uint16_t addr, const uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Write(&hi2c2, FRAM_I2C_ADDR_8BIT, addr,
                             I2C_MEMADD_SIZE_16BIT, (uint8_t *)data, len, 20);
}

HAL_StatusTypeDef FRAM_Read(uint16_t addr, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(&hi2c2, FRAM_I2C_ADDR_8BIT, addr,
                            I2C_MEMADD_SIZE_16BIT, data, len, 20);
}

HAL_StatusTypeDef FRAM_WriteRecord(volatile uint16_t *index, uint32_t timestamp_ms, float temp_c)
{
    FRAM_Record_t record;
    int32_t temp_x10;
    uint16_t addr;
    uint16_t idx;

    if (index == NULL) {
        return HAL_ERROR;
    }

    temp_x10 = (int32_t)(temp_c * 10.0f);
    if (temp_x10 > 32767) {
        temp_x10 = 32767;
    } else if (temp_x10 < -32768) {
        temp_x10 = -32768;
    }

    record.timestamp_ms = timestamp_ms;
    record.temp_x10 = (int16_t)temp_x10;
    record.reserved = 0;

    idx = *index;
    if (idx >= FRAM_MAX_RECORDS) {
        idx = 0;
    }

    addr = (uint16_t)(idx * sizeof(FRAM_Record_t));
    if (FRAM_Write(addr, (const uint8_t *)&record, sizeof(record)) != HAL_OK) {
        return HAL_ERROR;
    }

    idx++;
    if (idx >= FRAM_MAX_RECORDS) {
        idx = 0;
    }
    *index = idx;

    return HAL_OK;
}

HAL_StatusTypeDef FRAM_ReadRecord(uint16_t index, UART_HandleTypeDef *huart)
{
    uint16_t addr;
    int16_t t_int;
    int16_t t_frac;
    int len;
    char buf[80];

    if (huart == NULL) {
        return HAL_ERROR;
    }

    if (index >= FRAM_MAX_RECORDS) {
        return HAL_ERROR;
    }

    addr = (uint16_t)(index * sizeof(FRAM_Record_t));
    if (FRAM_Read(addr, (uint8_t *)&g_fram_read_record, sizeof(g_fram_read_record)) != HAL_OK) {
        return HAL_ERROR;
    }

    t_int = (int16_t)(g_fram_read_record.temp_x10 / 10);
    t_frac = (int16_t)(g_fram_read_record.temp_x10 % 10);
    if (t_frac < 0) {
        t_frac = (int16_t)(-t_frac);
    }

    len = snprintf(buf, sizeof(buf),
                   "[FRAM READ] t=%lu ms | temp=%d.%01d oC\r\n",
                   (unsigned long)g_fram_read_record.timestamp_ms,
                   (int)t_int,
                   (int)t_frac);
    if (len <= 0) {
        return HAL_ERROR;
    }

     return HAL_UART_Transmit(huart, (uint8_t *)buf, (uint16_t)len, 50);
}
