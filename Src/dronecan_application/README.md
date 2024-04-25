The node has the following registers:

| №  | Register name           | Description |
| -- | ----------------------- | ----------- |
|  1 | uavcan.node.id          | Defines a node-ID. Allowed values [0,127]. |
|  2 | system.name             | Defines custom node name. If empty, the node will use the default name. |
|  3 | force.scaler_1          | Scaler for the first force sensor value in pct [%]. |
|  4 | force.scaler_2          | Scaler for the second force sensor value in pct [%]. |
|  5 | force.offset_1          | Offset for the second force sensor value. |
|  6 | force.offset_2          | Offset for the second force sensor value. |
|  7 | force.send_raw          | When set to 1 sends raw sensor data in info |
|  8 | force.mes_indx          | When set to 0, the ADC IN4 sensor is used as measurement, if 1 - right. |
|  9 | force.coefficient       | Coefficient k, used to calculate force F=k*x in pct [%], so value 10 means k=0.1. |

> This docs was automatically generated. Do not edit it manually.

