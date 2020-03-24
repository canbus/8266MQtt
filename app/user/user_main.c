/* main.c -- MQTT client example
 *
 * Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of Redis nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"

#include "sntp.h"

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0xfd000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#else
#error "The flash map is not supported"
#endif

MQTT_Client mqttClient;
typedef unsigned long u32_t;
static ETSTimer sntp_timer;

void sntpfn() {
	u32_t ts = 0;
	ts = sntp_get_current_timestamp();
	os_printf("current time : %s\n", sntp_get_real_time(ts));
	if (ts == 0) {
		//os_printf("did not get a valid time from sntp server\n");
	} else {
		os_timer_disarm(&sntp_timer);
		MQTT_Connect(&mqttClient);
	}
}

void wifiConnectCb(uint8_t status) {
	if (status == STATION_GOT_IP) {
		sntp_setservername(0, "pool.ntp.org"); // set sntp server after got ip address
		sntp_init();
		os_timer_disarm(&sntp_timer);
		os_timer_setfn(&sntp_timer, (os_timer_func_t *) sntpfn, NULL);
		os_timer_arm(&sntp_timer, 1000, 1);        //1s
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}
MQTT_Client *MyMqttClient = NULL;
void mqttConnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	MyMqttClient = client;
	INFO("MQTT: Connected\r\n");
//	MQTT_Subscribe(client, "/mqtt/topic/0", 0);
//	MQTT_Subscribe(client, "/mqtt/topic/1", 1);
//	MQTT_Subscribe(client, "/mqtt/topic/2", 2);

//	MQTT_Publish(client, "/mqtt/topic/0", "hello0", 6, 0, 0);
//	MQTT_Publish(client, "/mqtt/topic/1", "hello1", 6, 1, 0);
//	MQTT_Publish(client, "/mqtt/topic/2", "hello2", 6, 2, 0);

}

void mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len,
		const char *data, uint32_t data_len) {
	char *topicBuf = (char*) os_zalloc(topic_len+1), *dataBuf =
			(char*) os_zalloc(data_len+1);

	MQTT_Client* client = (MQTT_Client*) args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	INFO("MQTT Receive topic:%s,data:%s\r\n", topicBuf, dataBuf);
	os_free(topicBuf);
	os_free(dataBuf);
}

#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM                SYSTEM_PARTITION_CUSTOMER_BEGIN
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
    { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
    { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
    { SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM,             SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR,          0x1000},
};

//static const partition_item_t at_partition_table[] = { {
//		SYSTEM_PARTITION_BOOTLOADER, 0x0, 0x1000 },
//		{ SYSTEM_PARTITION_OTA_1, 0x1000, SYSTEM_PARTITION_OTA_SIZE },
//		{ SYSTEM_PARTITION_OTA_2, SYSTEM_PARTITION_OTA_2_ADDR, SYSTEM_PARTITION_OTA_SIZE },
//		{SYSTEM_PARTITION_RF_CAL, SYSTEM_PARTITION_RF_CAL_ADDR, 0x1000 },
//		{SYSTEM_PARTITION_PHY_DATA, SYSTEM_PARTITION_PHY_DATA_ADDR, 0x1000 },
//		{SYSTEM_PARTITION_SYSTEM_PARAMETER,
//		SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000 }, };

void ICACHE_FLASH_ATTR user_pre_init(void) {
	if (!system_partition_table_regist(at_partition_table,
			sizeof(at_partition_table) / sizeof(at_partition_table[0]),
			SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while (1)
			;
	}
}
/*
分割成功,返回个数,没找到,返回-1
*/
int splitStr(char *ptr,char *cmd,char *str1,char *str2,char *str3,char *str4)
{
	if(strstr(ptr,cmd))
		{
			ptr += strlen(cmd)+1;

			if(str1 != NULL){
				char *p = strstr(ptr,";");
				if(p != NULL){
					strncpy(str1, ptr, p - ptr);
					ptr += strlen(str1)+1;
				}else{
					strncpy(str1, ptr, strlen(ptr));
					return 1;
				}
			}
			if(str2 != NULL){
				char *p = strstr(ptr,";");
				if(p != NULL){
					strncpy(str2, ptr, p - ptr);
					ptr += strlen(str2)+1;
				}else{
					strncpy(str2, ptr, strlen(ptr));
					return 2;
				}
			}
			if(str3 != NULL){
				char *p = strstr(ptr,";");
				if(p != NULL){
					strncpy(str3, ptr, p - ptr);
					ptr += strlen(str3)+1;
				}else{
					strncpy(str3, ptr, strlen(ptr));
					return 3;
				}
			}
			if(str4 != NULL){
				char *p = strstr(ptr,";");
				if(p != NULL){
					strncpy(str4, ptr, p - ptr);
					ptr += strlen(str4)+1;
				}else{
					strncpy(str4, ptr, strlen(ptr));
					return 4;
				}
			}
			return 0;
		}
		return -1;
}

void doUartCmd(uint8* pData_buf, uint16 data_len) {
	char *ptr = pData_buf;
	if (os_strstr(ptr, "publish"))     //publish:topic:msg:qos:retain
	{                            		//publish:/mqtt/topic/0:hello:0:0
		char topic[512];				//MQTT_Publish(MyMqttClient, "/mqtt/topic/0", "hello0", 6, 0, 0);
		char msg[512];
		char qosStr[5];
		char retainStr[5];
		u8 qos;
		u8 retain;
		os_memset(topic,0,sizeof(topic));
		os_memset(msg,0,sizeof(msg));

		if(splitStr(ptr,"publish",topic,msg,qosStr,retainStr) >=0 )
		{
			qos = qosStr[0] - '0';
			if (qos > 2 || qos < 0)
				qos = 0;
			retain = retainStr[0] - '0';
			if (retain != 1)
				retain = 0;

			MQTT_Publish(MyMqttClient, topic, msg, strlen(msg), qos, retain);
			os_printf("publish topic:%s,%s\n", topic, msg);
		}
	}
	else if (os_strstr(ptr, "subscribe"))//subscribe:topic:qos
	{								 //subscribe:/mqtt/topic/0:0 --> MQTT_Subscribe(MyMqttClient, "/mqtt/topic/0", 0);
		char topic[512];
		char qosStr[5];
		u8 qos;
		os_memset(topic, 0, sizeof(topic));
		if(splitStr(ptr,"subscribe",topic,qosStr ,NULL ,NULL) >= 0)
		{
			qos = qosStr[0] - '0';
			if (qos > 2 || qos < 0)
				qos = 0;
			MQTT_Subscribe(MyMqttClient, topic, qos);
			os_printf("subscribe topic:%s:%d\n", topic, qos);
		}
	}else if (os_strstr(ptr, "WIFIConfig"))//WIFIConfig:qwer:12345678
	{
		char ssid[20];
		char pwd[20];
		os_memset(ssid, 0, sizeof(ssid));
		os_memset(pwd, 0, sizeof(pwd));
		if (splitStr(ptr, "WIFIConfig", ssid, pwd, NULL ,NULL) >= 0 ) {
			os_strncpy(sysCfg.sta_ssid, ssid, sizeof(sysCfg.sta_ssid) - 1);
			os_strncpy(sysCfg.sta_pwd, pwd, sizeof(sysCfg.sta_pwd) - 1);
			CFG_Save();
			WIFI_Connect(ssid, pwd, wifiConnectCb);
			os_printf("WIFIConfig :%s:%s\n", ssid, pwd);
		}
	}else if (os_strstr(ptr, "MQTTHostConfig"))//MQTTHostConfig:192.168.43.195:1883:0
	{
		char host[64*2];
		char port[20];
		char security[20];
		os_memset(host, 0, sizeof(host));
		os_memset(port, 0, sizeof(port));
		if (splitStr(ptr, "MQTTHostConfig", host, port, security ,NULL) >=0) {
			os_strncpy(sysCfg.mqtt_host, host, sizeof(sysCfg.mqtt_host) - 1);
			sysCfg.mqtt_port = atoi(port);
			sysCfg.security = security[0]=='0'?0:1;
			CFG_Save();
			MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port,sysCfg.security );
			os_printf("MQTTHostConfig:%s:%s,%d\n", host, port,sysCfg.security);
		}
	}else if (os_strstr(ptr, "MQTTClientCfg"))//MQTTClientCfg:client_id:username:password
	{
		char client_id[128];
		char username[64];
		char password[128];
		os_memset(client_id, 0, sizeof(client_id));
		os_memset(username, 0, sizeof(username));
		os_memset(password, 0, sizeof(password));
		if (splitStr(ptr, "MQTTClientCfg", client_id, username, password ,NULL) >=0) {
			os_strncpy(sysCfg.device_id, client_id, sizeof(sysCfg.device_id) - 1);
			os_strncpy(sysCfg.mqtt_user, username, sizeof(sysCfg.mqtt_user) - 1);
			os_strncpy(sysCfg.mqtt_pass, password, sizeof(sysCfg.mqtt_pass) - 1);
			CFG_Save();
			MQTT_InitClient(&mqttClient, client_id, username, password, 120, 1);
			os_printf("MQTTClientCfg:%s:%s:%s\n", client_id, username, password);
		}
	}
	else {
		os_printf("invalid cmd:%d,%s\n", data_len, pData_buf);
	}
}

UART_RX_Callback uart0CB(uint8* pData_buf, uint16 data_len) {
	static uint8 buf[1024];
	static uint32 len = 0;
	os_strncpy(&buf[len],pData_buf,data_len);
	len += data_len;
	if(strstr(pData_buf,";endl;") == NULL){
		os_printf("wait ;endl;\n");
	}else{
		doUartCmd(buf,len);
		len = 0;
	}
}
void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200, uart0CB);
	os_delay_us(60000);

	CFG_Load();//CFG_Save();

	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port,
			sysCfg.security);
	//MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user,
			sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	//MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);

	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);
	INFO("%s,%s\n",sysCfg.sta_ssid,sysCfg.sta_pwd);
	WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);

	INFO("\r\nSystem started ...\r\n");
}
