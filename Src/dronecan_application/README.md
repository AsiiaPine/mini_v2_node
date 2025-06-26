The node has the following registers:

| №  | Register name           | Description |
| -- | ----------------------- | ----------- |
|  1 | uavcan.node.id          | Defines a node-ID. Allowed values [0,127]. |
|  2 | system.name             | Defines custom node name. If empty, the node will use the default name. |
|  3 | led_pull_down.ttl_cmd   | Defines the time to live for the command in milliseconds. |
|  4 | led_pull_down.pwm_cmd_type | Defines the type of the command. |
|  5 | led_pull_down.channel   | Index of setpoint channel. [-1; 255]. -1 means disabled, |

> This docs was automatically generated. Do not edit it manually.

