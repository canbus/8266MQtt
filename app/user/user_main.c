//#include "ets_sys.h"
//#include "driver/uart.h"
//#include "osapi.h"
//
//#include "user_interface.h"

#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"



#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0xfd000

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
    { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
    { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
}

typedef void(*UART_RX_Callback)(uint8* pData_buf, uint16 data_len);
extern void UART_RX_CallBack_Register(UART_RX_Callback callBack);

LOCAL void uart0_rx(uint8 *rxBuf,uint16 data_len) {

	os_printf("rx:%d:%s\n",data_len,rxBuf);
}

LOCAL void ICACHE_FLASH_ATTR timer1_cb(void *arg) {
    static bool ledStatus = false;
	os_printf("timer1_cb()\n");
    if(ledStatus == false){
    	ledStatus = true;
    	GPIO_OUTPUT_SET(GPIO_ID_PIN(0), 0); //D3
    }else{
    	ledStatus = false;
    	GPIO_OUTPUT_SET(GPIO_ID_PIN(0), 1); //D3
    }
}

LOCAL os_timer_t timer1;

void user_init(void) {

	uart_init(115200, 115200); //设置串口0和串口1的波特率
	UART_RX_CallBack_Register(uart0_rx);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO0); //选择GPIO0(D3)
	GPIO_OUTPUT_SET(GPIO_ID_PIN(0), 1); //D3 灭

    os_timer_disarm(&timer1);
    os_timer_setfn(&timer1, (os_timer_func_t *)timer1_cb, (void *)0);
    os_timer_arm(&timer1, 1000, 1);

}
