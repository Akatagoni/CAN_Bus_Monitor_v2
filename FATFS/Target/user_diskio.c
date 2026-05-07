#include <string.h>
#include <stdio.h>
#include "ff_gen_drv.h"
#include "main.h"


extern SPI_HandleTypeDef hspi1;

static volatile DSTATUS Stat = STA_NOINIT;

#define CS_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define CS_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

static uint8_t SPI_SendByte(uint8_t byte)
{
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi1, &byte, &rx, 1, 100);
    return rx;
}

static uint8_t SD_WaitReady(void)
{
    uint8_t res;
    uint32_t timeout = HAL_GetTick();
    do { res = SPI_SendByte(0xFF); }
    while(res != 0xFF && (HAL_GetTick() - timeout) < 500);
    return res;
}

static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg)
{
    uint8_t crc = 0xFF;
    if(cmd == 0) crc = 0x95;
    if(cmd == 8) crc = 0x87;

    SD_WaitReady();
    SPI_SendByte(0x40 | cmd);
    SPI_SendByte((arg >> 24) & 0xFF);
    SPI_SendByte((arg >> 16) & 0xFF);
    SPI_SendByte((arg >> 8)  & 0xFF);
    SPI_SendByte((arg)       & 0xFF);
    SPI_SendByte(crc);

    uint8_t res;
    uint8_t n = 10;
    do { res = SPI_SendByte(0xFF); } while((res & 0x80) && --n);
    return res;
}


DSTATUS USER_initialize(BYTE pdrv)
{
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    HAL_SPI_Init(&hspi1);

    CS_HIGH();
    for(int i = 0; i < 10; i++) SPI_SendByte(0xFF);

    /* CMD0 */
    uint8_t res = 0xFF;
    for(int retry = 0; retry < 20; retry++)
    {
        CS_LOW();
        SPI_SendByte(0x40);
        SPI_SendByte(0x00);
        SPI_SendByte(0x00);
        SPI_SendByte(0x00);
        SPI_SendByte(0x00);
        SPI_SendByte(0x95);
        for(int i = 0; i < 10; i++)
        {
            res = SPI_SendByte(0xFF);
            if(res == 0x01) break;
        }
        CS_HIGH();
        SPI_SendByte(0xFF);
        if(res == 0x01) break;
        osDelay(10);
    }

    if(res != 0x01) return STA_NOINIT;

    /* CMD8 - check voltage range (SDHC) */
    CS_LOW();
    SPI_SendByte(0x48);
    SPI_SendByte(0x00);
    SPI_SendByte(0x00);
    SPI_SendByte(0x01);
    SPI_SendByte(0xAA);
    SPI_SendByte(0x87);
    res = 0xFF;
    for(int i = 0; i < 10; i++)
    {
        res = SPI_SendByte(0xFF);
        if(!(res & 0x80)) break;
    }
    /* Read 4 more bytes of CMD8 response */
    SPI_SendByte(0xFF);
    SPI_SendByte(0xFF);
    SPI_SendByte(0xFF);
    SPI_SendByte(0xFF);
    CS_HIGH();
    SPI_SendByte(0xFF);

    /* ACMD41 - initialize SDHC */
    uint32_t timeout = HAL_GetTick();
    while(1)
    {
        /* CMD55 */
        CS_LOW();
        SPI_SendByte(0x77);
        SPI_SendByte(0x00);
        SPI_SendByte(0x00);
        SPI_SendByte(0x00);
        SPI_SendByte(0x00);
        SPI_SendByte(0xFF);
        for(int i = 0; i < 10; i++)
        {
            res = SPI_SendByte(0xFF);
            if(!(res & 0x80)) break;
        }
        CS_HIGH();
        SPI_SendByte(0xFF);

        /* ACMD41 */
        CS_LOW();
        SPI_SendByte(0x69);
        SPI_SendByte(0x40);
        SPI_SendByte(0x00);
        SPI_SendByte(0x00);
        SPI_SendByte(0x00);
        SPI_SendByte(0xFF);
        res = 0xFF;
        for(int i = 0; i < 10; i++)
        {
            res = SPI_SendByte(0xFF);
            if(!(res & 0x80)) break;
        }
        CS_HIGH();
        SPI_SendByte(0xFF);

        if(res == 0x00) break;
        if(HAL_GetTick() - timeout > 3000) return STA_NOINIT;
        osDelay(10);
    }

    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    HAL_SPI_Init(&hspi1);

    Stat &= ~STA_NOINIT;
    return Stat;
}



DSTATUS USER_status(BYTE pdrv)
{
    return Stat;
}

DRESULT USER_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    if(Stat & STA_NOINIT) return RES_NOTRDY;

    while(count--)
    {
        CS_LOW();
        SD_SendCmd(17, sector);
        uint32_t timeout = HAL_GetTick();
        while(SPI_SendByte(0xFF) != 0xFE)
        {
            if(HAL_GetTick() - timeout > 500) { CS_HIGH(); return RES_ERROR; }
        }
        for(int i = 0; i < 512; i++) *buff++ = SPI_SendByte(0xFF);
        SPI_SendByte(0xFF);
        SPI_SendByte(0xFF);
        CS_HIGH();
        SPI_SendByte(0xFF);
        sector += 512;
    }
    return RES_OK;
}

#if _USE_WRITE == 1
DRESULT USER_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
	if(Stat & STA_NOINIT) return RES_NOTRDY;

    while(count--)
    {
        CS_LOW();
        SD_SendCmd(24, sector);
        SPI_SendByte(0xFF);
        SPI_SendByte(0xFE);
        for(int i = 0; i < 512; i++) SPI_SendByte(*buff++);
        SPI_SendByte(0xFF);
        SPI_SendByte(0xFF);
        uint8_t res = SPI_SendByte(0xFF);
        if((res & 0x1F) != 0x05) { CS_HIGH(); return RES_ERROR; }
        uint32_t timeout = HAL_GetTick();
        while(SPI_SendByte(0xFF) == 0)
        {
            if(HAL_GetTick() - timeout > 500) { CS_HIGH(); return RES_ERROR; }
        }
        CS_HIGH();
        SPI_SendByte(0xFF);
        sector += 512;
    }
    return RES_OK;
}
#endif

#if _USE_IOCTL == 1
DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    if(Stat & STA_NOINIT) return RES_NOTRDY;
    DRESULT res = RES_ERROR;
    CS_LOW();
    switch(cmd)
    {
        case CTRL_SYNC:
            if(SD_WaitReady() == 0xFF) res = RES_OK;
            break;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = 2048000;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 512;
            res = RES_OK;
            break;
    }
    CS_HIGH();
    SPI_SendByte(0xFF);
    return res;
}
#endif

Diskio_drvTypeDef USER_Driver = {
    USER_initialize,
    USER_status,
    USER_read,
#if _USE_WRITE
    USER_write,
#endif
#if _USE_IOCTL == 1
    USER_ioctl,
#endif
};
