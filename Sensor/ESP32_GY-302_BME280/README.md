# ESP32_GY-302_BME280

该部分代码主要用于阳台上的光照强度传感器，使用 MQTT 协议进行数据的上传与配置的下发

### 功能
 - 订阅 `sensor/{sensorId}/config` 主题配置下发
   ```json
   {
       "pub_interval": 10,
       "light_enable": true
   }
   ```
   
   `pub_interval`: 数据上传间隔，单位: 秒  
   `light_enable`: 是否启用光照传感器

 - 根据配置的频率间隔向 `home/balcony` 主题发送温度(℃)，湿度(%)，气压(hPa)
   ```json
   {
       "sensorId": "1c87c3182ec8",
       "type": "thp",
       "timestamp": 1720708139,
       "data": {
           "temperature": 29.05999947,
           "pressure": 993.9020386,
           "humidity": 35.59570313
        }
   }
   ```
   
 - 根据配置的频率间隔向 `home/balcony/light` 主题发送光照强度(lux)
   ```json
   {
       "sensorId": "1c87c3182ec8",
       "type": "light",
       "timestamp": 1720708239,
       "data": {
           "light": 36
       }
   }
   ```

### TODO
 - [ ] 支持更多配置
 - [ ] 实现 OTA 更新功能，以便在需要时能远程更新设备的固件。
 - [ ] 实现通过移动设备蓝牙更新配置文件
