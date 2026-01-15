#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#define JEDEC_ID        0xef4017

#define CMD_JEDEC_ID        0x9f
#define CMD_READ_DATA       0x03
#define CMD_FAST_READ       0x0b

/*
 * Read JEDEC ID
 * ID returned is 24 bits long
 */
void readChipID(rt_spim_t* spim, uint8_t* rx_buffer)
{
    uint8_t tx_buffer = CMD_JEDEC_ID;
    
    rt_spim_send(spim, &tx_buffer, 1 * 8, RT_SPIM_CS_KEEP, NULL);
    rt_spim_receive(spim, rx_buffer, 3 * 8, RT_SPIM_CS_AUTO, NULL);
}

/*
 * addr     : address in the flash to read
 * rx_buf   : buffer to return data
 * buf_size : size of the buffer in bytes
 */
void readData(rt_spim_t* spim, uint32_t addr, uint8_t* rx_buffer, int buf_size)
{
    uint8_t cmdaddr[4];
    
    cmdaddr[0] = CMD_READ_DATA;
    cmdaddr[1] = addr >> 16;
    cmdaddr[2] = addr >> 8;
    cmdaddr[3] = addr;
    
    rt_spim_send(spim, cmdaddr, 4 * 8, RT_SPIM_CS_KEEP, NULL);
    rt_spim_receive(spim, rx_buffer, buf_size * 8, RT_SPIM_CS_AUTO, NULL);
}

#endif // __SPI_FLASH_H__
