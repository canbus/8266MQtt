
# Simple MQTT Client Demo

This example implement:
- MQTT Publish the Message
- MQTT Subscribe the Topic
- MQTT works with SSL/TLS
- MQTT works with one-way anthentication
- MQTT works with two-way anthentication

# How to make the Demo work

### Step 1: Start the MQTT broker

you could choose [mosquitto](https://mosquitto.org) or [EMQTT](https://github.com/emqtt/emqttd) as your MQTT broker

### Step 2: Configurate your demo

All you need to configurate are in mqtt_config.h, please according to the comment to modify:
- CFG_HOLDER
- MQTT_HOST
- MQTT_PORT
- MQTT_CLIENT_ID
- MQTT_USER
- MQTT_PASS
- STA_SSID
- STA_PASS
- DEFAULT_SECURITY
- CA_CERT_FLASH_ADDRESS
- CLIENT_CERT_FLASH_ADDRESS

### Step 3(optional): Generate your certificate for SSL/TLS

if you configurate DEFAULT_SECURITY to ONE_WAY_ANTHENTICATION or TWO_WAY_ANTHENTICATION, please according to [tools/README.txt](../../tools/README.md) to generate your certificate

### Step 4: Build your demo

```
$./gen_misc.sh
```

compile options: 1 1 2 0 5, or others if you are familar to it.

### Step 5: Flash your demo

Any way is OK,such as:
```
$~/esp/esp-idf/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash 0x00000 ~/ESP8266_NONOS_SDK/bin/boot_v1.6.bin 0x1000 ../bin/upgrade/user1.2048.new.5.bin 0x1fe000 ~/ESP8266_NONOS_SDK/bin/blank.bin 0x1fc000 ~/ESP8266_NONOS_SDK/bin/esp_init_data_default.bin
```

### Step 6(optional): Flash your certificate

Any way is OK, but your flashing address is same as the Step 2,such as:

```
$~/esp/esp-idf/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash  0x77000 ~/ESP8266_NONOS_SDK/tools/bin/esp_ca_cert.bin
$~/esp/esp-idf/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash  0x78000 ~/MQTTESP8266/tools/bin/esp_cert_private_key.bin
```
### Run the demo and Result Shows

when the demo starts up:
- it would subscribe the topic `/mqtt/topic/0` , `/mqtt/topic/1` , `/mqtt/topic/2` 
- it would publish the topic `/mqtt/topic/0` , `/mqtt/topic/1` , `/mqtt/topic/2` 
- MQTT broker would receive subscribe and publish

### Troubleshooting

**why the demo connect the WiFi failed?**
- try to modify CFG_HOLDER

**why the handshake failed?**

- try to uncomment espconn_secure_set_size and modify memory allocate
- make sure your MQTT broker SSL/TLS certificate configurate valid 

**本地Mqtt测试**
WIFIConfig;ssid;12345678;endl;
MQTTHostConfig;192.168.43.195;1883;0;endl;  <--不加密
MQTTClientCfg;myclient_id:admin;public;endl;

publish;/mqtt/mytopic/0:hello;0;0;endl;
subscribe;/mqtt/mytopic/0;0;endl;

subscribe;/mqtt/mytopic/1;1;endl;
publish;/mqtt/mytopic/1;hello;1;0;endl;

**阿里物联网Mqtt接入**
WIFIConfig;ssid;12345678;endl;
MQTTHostConfig;a1enYbL9dlS.iot-as-mqtt.cn-shanghai.aliyuncs.com;1883;0;endl;
MQTTClientCfg;001|securemode=3,signmethod=hmacsha1|;Port1&a1enYbL9dlS;F9A79B92DE6519131B92E2FCA758BB7569E724;endl;

subscribe;/a1enYbL9dlS/Port1/user/get;endl;
publish;/sys/a1enYbL9dlS/Port1/thing/event/property/post;{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"Status\":1,\"Data\":\"Hello\",\"RemainParkSpace\":12,\"V\":230},\"version\":\"1.0\"};endl;
