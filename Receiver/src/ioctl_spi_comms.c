/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <string.h>
#include <stdio.h>
#include <fcntl.h> 
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
//#include <pthread.h>
//#include "pltf_spi.h"
//#include "st_errno.h"
#include "../inc/ioctl_spi_comms.h"


//#define ARRAY_SIZE(a)		(sizeof(a) / sizeof(a[0]))

/*
 ******************************************************************************
 * STATIC VARIABLES
******************************************************************************
*/
/* ST25R3911XX is connected with Linux host's SPI port /dev/spidev0.0 */
static const char *device = "/dev/spidev0.0";
static int fd = 0;
static int isSPIInit = 0;
/* Lock to serialize SPI communication */
//static pthread_mutex_t lockCom;

/*
 ******************************************************************************
 * GLOBAL AND HELPER FUNCTIONS
 ******************************************************************************
 */
ReturnCode spi_init(void)
{
	int ret = 0;
	uint32_t mode = SPI_MODE_CONFIG;
	uint8_t bitsperword = SPI_BITS_PER_WORD; 
	uint32_t speed = SPI_MAX_FREQ; 

	fd = open(device, O_RDWR);
	if (fd < 0) {
		printf("Error: spi device open = %d\n", fd);
		ret = fd;
		goto error;
	}
	
	/* set spi mode */
	ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
	if (ret < 0) {
		printf("Error: SPI mode setting\n");
		goto error;
	}

	/* set spi bits per word */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bitsperword);
 	if (ret < 0) {
		printf("Error: setting spi bitsperword\n");
		goto error;
	}

	/* set spi frequency */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret < 0) {
		printf("Error: setting SPI frequency\n");
		goto error;
 	}
	
	isSPIInit = 1;
	return ERR_NONE;

error:
	return ERR_IO;
}


HAL_statusTypeDef spiTxRx(const uint8_t *txData, uint8_t *rxData, uint8_t length)
{  
	
	int ret = 0;

	/* check if SPI init is done */
	if (!isSPIInit) {
		printf(" error: spi is used for communication before its initialization\n");
		return	HAL_ERROR;
	}

	struct spi_ioc_transfer transfer;
	// ensure the structure is empty to start with
	memset(&transfer, 0, sizeof(struct spi_ioc_transfer));

	if (txData)
		transfer.tx_buf = (unsigned long) txData;
        //printf("Sending:%x\n", txData);
	if (rxData)
		transfer.rx_buf = (unsigned long) rxData;
        //printf("Receive:%x\n", rxData);
	transfer.len 		= (unsigned int) length;
	transfer.speed_hz 	= SPI_MAX_FREQ;
	transfer.bits_per_word 	= SPI_BITS_PER_WORD;
	transfer.delay_usecs 	= 0;
	transfer.cs_change	= 0;
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
	if (ret < 0) {
		printf("Error: SPI error in data transfer=%d\n",ret);
		return HAL_ERROR;
	}

	return HAL_OK;
}

