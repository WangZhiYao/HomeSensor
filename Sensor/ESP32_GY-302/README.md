# ESP32_GY-302

该部分代码主要用于阳台上的光照强度传感器，使用 MQTT 协议进行数据的上传与配置的下发

### 功能
 - 订阅 "sensor/{mac}/config" 主题配置下发
   ```json
   {
       "sleep": 10
       "publish_interval": 10000
   }
   ```
   `sleep`: 睡眠时间，收到该字段后立即进入深度睡眠，单位: 秒  
   `publish_interval`: 数据上传间隔，单位: 秒，该配置使用 nvs 持久化

 - 根据配置的频率间隔向 "home/balcony/light" 主题发送光照强度数据
   ```json
   {
       "mac": "C8:2E:18:C3:87:1C",
       "lux": 303,
       "timestamp": 1719224200
   }
   ```

### TODO
 - [ ] 支持更多配置
 - [ ] 实现 OTA 更新功能，以便在需要时能远程更新设备的固件。
 - [ ] 实现通过移动设备蓝牙更新配置文件
