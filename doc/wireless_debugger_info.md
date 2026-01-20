# Wireless Debugging

### Overview

This wireless debugging system provides real-time monitoring and status tracking for the spa control system.



### Currently Supported Monitoring

| Feature                   | Description                            |
| ------------------------- | -------------------------------------- |
| Device Status             | GET command succeeded                  |
| Firmware Version          | GET command succeeded                  |
| Setup Information         | GET command succeeded                  |
| Spa Configuration Status  | Whether the spa is properly configured |
| I2C Communication \*\*    | Read/Write operation status            |
| I2C Role \*\*             | Master/Slave mode detection            |
| I2C Failed Counter \*\*   | Tracks I2C communication failures      |
| WiFi Reconnection Counter | Tracks WiFi reconnections              |
| MQTT Reconnection Counter | Tracks MQTT broker reconnections       |
| Pump Operations           | SET command succeeded                  |
| Lighting                  | SET command succeeded                  |
| Blower                    | SET command succeeded                  |
| Waterfall \*\*            | SET command succeeded                  |
| Current Temperature       | GET command succeeded                  |
| Set Temperature           | SET command succeeded                  |
|                           | GET command succeeded                  |
| Temperature Scale         | SET command succeeded                  |
|                           | GET command succeeded                  |
| Temperature Range \*      | SET command succeeded                  |
|                           | GET command succeeded                  |
| Time Settings             | SET command succeeded                  |
|                           | GET command succeeded                  |
| Filter Cycle              | SET command succeeded                  |
|                           | GET command succeeded                  |
| Eco Cycle \*\*            | SET command succeeded                  |
|                           | GET command succeeded                  |
| Heat Mode \*              | SET command succeeded                  |
|                           | GET command succeeded                  |
| WiFi Station Reset        | SET command succeeded                  |
| Clock Mode \*             | SET command succeeded                  |
| OTA (Over-The-Air)        | SET command succeeded                  |

#### Notes

- Features marked with \* are available only for **Balboa Spa Controller**.
- Features marked with \*\* are available only for **Gecko Spa Controller**.

