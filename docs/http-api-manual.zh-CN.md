# Dobot V4 HTTP/JSON 接口调用手册

本文档是 `dobot_v4_bringup` HTTP 网关的统一调用约定。HTTP 服务默认监听 `0.0.0.0:8080`，可通过 ROS 私有参数 `~http_port` 修改。

## 1. 通用约定

请求地址：

```text
http://<机器人主机>:8080/api/v1
```

信息接口使用 `GET`，控制接口使用 `POST`，POST 请求必须使用：

```http
Content-Type: application/json
```

成功响应示例：

```json
{"success":true,"code":0,"message":"ok","data":{"res":0}}
```

失败响应示例：

```json
{"success":false,"code":400,"message":"missing field: index","data":null}
```

字段含义：

| 字段 | 类型 | 含义 |
|---|---|---|
| `success` | boolean | HTTP/业务请求是否成功 |
| `code` | integer | `0` 表示成功；非零时为机器人 `res` 或 HTTP 错误码 |
| `message` | string | 可读的结果说明 |
| `data` | object/null | 返回数据；没有返回字段时通常为 `{}` |
| `res` | integer | Dobot 原 ROS service 返回码，原样保留 |

通用错误：`400` 参数错误，`404` 路由不存在，`503` ROS service 不可用，`500` 网关内部错误。

## 2. 状态和信息接口

### 2.1 网关状态

```bash
curl http://127.0.0.1:8080/api/v1/health
```

返回：

```json
{"success":true,"code":0,"message":"ok","data":{"ros_ok":true,"robot_connected":true,"robot_enabled":true}}
```

| 字段 | 含义 |
|---|---|
| `ros_ok` | ROS 节点是否仍在运行 |
| `robot_connected` | 是否已连接机器人 |
| `robot_enabled` | 机器人是否已使能 |

### 2.2 当前状态

```bash
curl http://127.0.0.1:8080/api/v1/info/joint_state
curl http://127.0.0.1:8080/api/v1/info/tool_pose
```

返回示例：

```json
{"success":true,"code":0,"message":"ok","data":{"joints":[0,0,0,0,0,0]}}
```

```json
{"success":true,"code":0,"message":"ok","data":{"pose":[300,0,200,180,0,90]}}
```

`joints` 依次为 J1~J6；`pose` 依次为 X、Y、Z、Rx、Ry、Rz。

### 2.3 ROS 信息接口映射

以下接口统一使用：

```text
GET /api/v1/info/<接口名>
```

请求示例：

```bash
curl http://127.0.0.1:8080/api/v1/info/robot_mode
```

响应字段为对应 ROS service 的 response 字段。例如 `RobotMode`：

```json
{"success":true,"code":0,"message":"ok","data":{"mode":5,"res":0}}
```

| 接口 | 请求 JSON | 返回字段及含义 |
|---|---|---|
| `robot_mode` | `{}` | `mode`: 机器人模式；`res`: 结果码 |
| `get_error_id` | `{}` | `error_id`: 错误 ID 数组；`res`: 结果码 |
| `get_angle` | `{}` | `res`: 结果码 |
| `get_current_command_id` | `{}` | `res`: 当前指令 ID/结果码，具体含义以控制器版本为准 |
| `get_pose` | `{"user":"0","tool":"0"}` | `pose`: 位姿字符串；`res`: 结果码 |
| `get_start_pose` | `{"traceName":"demo"}` | `res`: 结果码 |
| `get_input_bool` | `{"address":1}` | `res`: 输入值/结果码 |
| `get_input_int` | `{"address":1}` | `res`: 输入值/结果码 |
| `get_input_float` | `{"address":1}` | `res`: 输入值/结果码 |
| `get_output_bool` | `{"address":1}` | `res`: 输出值/结果码 |
| `get_output_int` | `{"address":1}` | `res`: 输出值/结果码 |
| `get_output_float` | `{"address":1}` | `res`: 输出值/结果码 |
| `di` | `{"index":1}` | `res`: DI 值/结果码 |
| `tool_di` | `{"index":1}` | `res`: 工具 DI 值/结果码 |
| `ai` | `{"index":1}` | `res`: AI 值/结果码 |
| `tool_ai` | `{"index":1}` | `res`: 工具 AI 值/结果码 |
| `get_do` | `{"index":1}` | `res`: DO 值/结果码 |
| `get_ao` | `{"index":1}` | `res`: AO 值/结果码 |
| `get_do_group` | `{"index_group":[1,2]}` | `res`: DO 组值/结果码 |
| `get_in_bits` | `{"index":0,"addr":0,"count":1}` | `res`: 输入位结果 |
| `get_in_regs` | `{"index":0,"addr":0,"count":1,"valType":"U16"}` | `res`: 输入寄存器结果 |
| `get_coils` | `{"index":0,"addr":0,"count":1}` | `res`: 线圈结果 |
| `get_hold_regs` | `{"index":0,"addr":0,"count":1,"valType":"U16"}` | `res`: 保持寄存器结果 |
| `calc_tool` | `{"index":0,"matrix":0,"offset":"0,0,0,0,0,0"}` | `res`: 计算结果 |
| `calc_user` | `{"index":0,"matrix":0,"offset":"0,0,0,0,0,0"}` | `res`: 计算结果 |

说明：当前仓库的原 `.srv` 文件把部分读取值编码在 `res` 字段中，因此网关不会擅自改名；实际数值含义以控制器 SDK 返回定义为准。

## 3. 控制接口

以下接口统一使用：

```text
POST /api/v1/control/<接口名>
```

### 3.1 无参数控制

```bash
curl -X POST http://127.0.0.1:8080/api/v1/control/enable
```

```json
{"success":true,"code":0,"message":"EnableRobot completed","data":{}}
```

| 接口 | 作用 |
|---|---|
| `enable` | 机器人使能 |
| `disable` | 机器人失能 |
| `power_on` | 上电 |
| `clear_error` | 清除错误 |
| `stop` | 停止运动 |
| `pause` | 暂停 |
| `continue` | 继续 |
| `start_drag` | 开始拖拽 |
| `stop_drag` | 停止拖拽 |
| `stop_move_jog` | 停止 Jog |

### 3.2 安全控制

```bash
curl -X POST -H 'Content-Type: application/json' \
  -d '{"value":1}' \
  http://127.0.0.1:8080/api/v1/control/emergency_stop
```

| 接口 | 请求字段 | 字段含义 |
|---|---|---|
| `emergency_stop` | `value` | 急停控制值 |
| `enable_safe_skin` | `status` | 安全皮肤开关 |
| `set_safe_skin` | `part`, `status` | 安全皮肤部件和状态 |
| `set_safe_wall_enable` | `index`, `value` | 安全墙编号和开关 |
| `set_collision_level` | `level` | 碰撞等级 |
| `set_post_collision_mode` | `mode` | 碰撞后处理模式 |
| `brake_control` | `axisID`, `value` | 轴编号和刹车值 |

### 3.3 速度、加速度和负载

| 接口 | 请求示例 | 字段含义 |
|---|---|---|
| `speed_factor` | `{"ratio":50}` | 速度比例，百分比 |
| `acc_j` | `{"r":50}` | 关节加速度比例 |
| `acc_l` | `{"r":50}` | 直线加速度比例 |
| `vel_j` | `{"r":50}` | 关节速度比例 |
| `vel_l` | `{"r":50}` | 直线速度比例 |
| `cp` | `{"r":50}` | 连续路径比例 |
| `set_payload` | `{"load":1.2,"x":0,"y":0,"z":50}` | 负载和质心坐标 |
| `set_back_distance` | `{"distance":10}` | 回退距离 |

### 3.4 运动控制

通用运动请求示例：

```json
{"mode":false,"a":300,"b":0,"c":200,"d":180,"e":0,"f":90,"paramValue":[]}
```

其中 `a~f` 对应位置/姿态参数，`mode` 为运动模式，`paramValue` 为控制器扩展参数。

| 接口 | 额外字段 | 说明 |
|---|---|---|
| `mov_j` | `mode,a,b,c,d,e,f,paramValue` | 关节/点到点运动 |
| `mov_l` | `mode,a,b,c,d,e,f,paramValue` | 直线运动 |
| `rel_joint_mov_j` | `a,b,c,d,e,f,paramValue` | 相对关节运动 |
| `rel_mov_j_tool` | `a,b,c,d,e,f,paramValue` | 工具坐标系相对运动 |
| `rel_mov_l_tool` | `a,b,c,d,e,f,paramValue` | 工具坐标系相对直线运动 |
| `rel_mov_j_user` | `a,b,c,d,e,f,paramValue` | 用户坐标系相对运动 |
| `rel_mov_l_user` | `a,b,c,d,e,f,paramValue` | 用户坐标系相对直线运动 |
| `mov_j_io` | `mode,a,b,c,d,e,f,MDIS,paramValue` | 点到点运动并控制 IO |
| `mov_l_io` | `mode,a,b,c,d,e,f,MDIS,paramValue` | 直线运动并控制 IO |
| `arc` | `mode,a~f,a2~f2,paramValue` | 圆弧运动，两组位姿 |
| `circle` | `mode,a~f,a2~f2,count,paramValue` | 圆周运动，`count` 为圈数 |
| `move_jog` | `axisID,paramValue` | Jog 轴和参数 |
| `servo_j` | `a~f,paramValue` | 关节伺服 |
| `servo_p` | `a~f,paramValue` | 位姿伺服 |

运动成功返回示例：

```json
{"success":true,"code":0,"message":"MovJ completed","data":{"res":0}}
```

### 3.5 IO 控制

| 接口 | 请求示例 |
|---|---|
| `do` | `{"index":1,"status":1,"time":0}` |
| `do_instant` | `{"index":1,"status":1}` |
| `tool_do` | `{"index":1,"status":1}` |
| `tool_do_instant` | `{"index":1,"status":1}` |
| `ao` | `{"index":1,"value":5}` |
| `ao_instant` | `{"index":1,"value":5}` |
| `do_group` | `{"args":[1,1,0,1]}` |
| `set_output_bool` | `{"address":1,"value":1}` |
| `set_output_int` | `{"address":1,"value":100}` |
| `set_output_float` | `{"address":1,"value":1.5}` |
| `set_coils` | `{"index":0,"addr":0,"count":1,"valTab":"1"}` |
| `set_hold_regs` | `{"index":0,"addr":0,"count":1,"valTab":"100","valType":"U16"}` |

`index` 为 IO 编号，`status/value` 为输出值，`time` 为持续时间；具体有效范围由控制器型号决定。

### 3.6 用户坐标、工具和脚本

| 接口 | 请求示例 | 字段含义 |
|---|---|---|
| `user` | `{"index":0}` | 选择用户坐标 |
| `tool` | `{"index":0}` | 选择工具坐标 |
| `set_user` | `{"index":0,"value":"..."}` | 设置用户坐标 |
| `set_tool` | `{"index":0,"value":"..."}` | 设置工具坐标 |
| `set_tool_mode` | `{"mode":0,"type":0}` | 工具模式和类型 |
| `set_tool_power` | `{"status":1}` | 工具电源 |
| `set_tool_485` | `{"baudrate":115200,"parity":"N","stop":1,"identify":0}` | 工具 RS485 参数 |
| `run_script` | `{"projectName":"demo.lua"}` | 执行项目脚本 |
| `start_path` | `{"traceName":"demo","paramValue":[]}` | 执行轨迹 |
| `tcp_dashboard` | `{"command":"RobotMode()"}` | 发送 Dashboard 命令 |

`tcp_dashboard` 返回示例：

```json
{"success":true,"code":0,"message":"ok","data":{"result":"0,RobotMode():5"}}
```

### 3.7 Modbus

| 接口 | 请求示例 |
|---|---|
| `modbus_create` | `{"ip":"192.168.1.10","port":502,"slave_id":1,"isRTU":0}` |
| `modbus_rtu_create` | `{"slave_id":1,"baud":115200,"parity":"N","data_bit":8,"stop_bit":1}` |
| `modbus_close` | `{"index":0}` |

## 4. 与 ROS service 的对应关系

HTTP 接口名采用 ROS `.srv` 文件名的 lower_snake_case 形式，例如：

```text
MovJ.srv       -> POST /api/v1/control/mov_j
GetPose.srv    -> GET  /api/v1/info/get_pose
EnableRobot.srv-> POST /api/v1/control/enable
```

请求字段名称与 `.srv` request 字段保持一致，响应字段名称与 `.srv` response 字段保持一致。

## 5. 当前实现状态

当前代码已实际接入：

```text
/health
/info/joint_state
/info/tool_pose
/control/enable
/control/disable
/control/clear_error
/control/stop
/control/pause
/control/continue
/control/power_on
/control/emergency_stop
```

手册中的其他接口已经完成字段和调用协议定义，接入代码时必须按照本文档保持 URL、字段和返回格式不变。未接入的路由不应伪装成成功，应返回 `404` 或 `501`。

## 6. 全部接口 curl 调用示例

下面每条命令都可以直接复制。将 `127.0.0.1:8080` 替换为实际机器人主机和端口即可。

### 6.1 信息接口

```bash
curl http://127.0.0.1:8080/api/v1/health
curl http://127.0.0.1:8080/api/v1/info/joint_state
curl http://127.0.0.1:8080/api/v1/info/tool_pose
curl http://127.0.0.1:8080/api/v1/info/robot_mode
curl http://127.0.0.1:8080/api/v1/info/get_error_id
curl http://127.0.0.1:8080/api/v1/info/get_angle
curl http://127.0.0.1:8080/api/v1/info/get_current_command_id
curl -G --data-urlencode 'user=0' --data-urlencode 'tool=0' http://127.0.0.1:8080/api/v1/info/get_pose
curl -G --data-urlencode 'traceName=demo' http://127.0.0.1:8080/api/v1/info/get_start_pose
curl -G --data-urlencode 'address=1' http://127.0.0.1:8080/api/v1/info/get_input_bool
curl -G --data-urlencode 'address=1' http://127.0.0.1:8080/api/v1/info/get_input_int
curl -G --data-urlencode 'address=1' http://127.0.0.1:8080/api/v1/info/get_input_float
curl -G --data-urlencode 'address=1' http://127.0.0.1:8080/api/v1/info/get_output_bool
curl -G --data-urlencode 'address=1' http://127.0.0.1:8080/api/v1/info/get_output_int
curl -G --data-urlencode 'address=1' http://127.0.0.1:8080/api/v1/info/get_output_float
curl -G --data-urlencode 'index=1' http://127.0.0.1:8080/api/v1/info/di
curl -G --data-urlencode 'index=1' http://127.0.0.1:8080/api/v1/info/tool_di
curl -G --data-urlencode 'index=1' http://127.0.0.1:8080/api/v1/info/ai
curl -G --data-urlencode 'index=1' http://127.0.0.1:8080/api/v1/info/tool_ai
curl -G --data-urlencode 'index=1' http://127.0.0.1:8080/api/v1/info/get_do
curl -G --data-urlencode 'index=1' http://127.0.0.1:8080/api/v1/info/get_ao
curl -G --data-urlencode 'index_group=1,2' http://127.0.0.1:8080/api/v1/info/get_do_group
curl -G --data-urlencode 'index=0' --data-urlencode 'addr=0' --data-urlencode 'count=1' http://127.0.0.1:8080/api/v1/info/get_in_bits
curl -G --data-urlencode 'index=0' --data-urlencode 'addr=0' --data-urlencode 'count=1' --data-urlencode 'valType=U16' http://127.0.0.1:8080/api/v1/info/get_in_regs
curl -G --data-urlencode 'index=0' --data-urlencode 'addr=0' --data-urlencode 'count=1' http://127.0.0.1:8080/api/v1/info/get_coils
curl -G --data-urlencode 'index=0' --data-urlencode 'addr=0' --data-urlencode 'count=1' --data-urlencode 'valType=U16' http://127.0.0.1:8080/api/v1/info/get_hold_regs
curl -G --data-urlencode 'index=0' --data-urlencode 'matrix=0' --data-urlencode 'offset=0,0,0,0,0,0' http://127.0.0.1:8080/api/v1/info/calc_tool
curl -G --data-urlencode 'index=0' --data-urlencode 'matrix=0' --data-urlencode 'offset=0,0,0,0,0,0' http://127.0.0.1:8080/api/v1/info/calc_user
```

### 6.2 基础和安全控制接口

```bash
curl -X POST http://127.0.0.1:8080/api/v1/control/enable
curl -X POST http://127.0.0.1:8080/api/v1/control/disable
curl -X POST http://127.0.0.1:8080/api/v1/control/power_on
curl -X POST http://127.0.0.1:8080/api/v1/control/clear_error
curl -X POST http://127.0.0.1:8080/api/v1/control/stop
curl -X POST http://127.0.0.1:8080/api/v1/control/pause
curl -X POST http://127.0.0.1:8080/api/v1/control/continue
curl -X POST -H 'Content-Type: application/json' -d '{"value":1}' http://127.0.0.1:8080/api/v1/control/emergency_stop
curl -X POST -H 'Content-Type: application/json' -d '{"status":1}' http://127.0.0.1:8080/api/v1/control/enable_safe_skin
curl -X POST -H 'Content-Type: application/json' -d '{"part":1,"status":1}' http://127.0.0.1:8080/api/v1/control/set_safe_skin
curl -X POST -H 'Content-Type: application/json' -d '{"index":1,"value":1}' http://127.0.0.1:8080/api/v1/control/set_safe_wall_enable
curl -X POST -H 'Content-Type: application/json' -d '{"level":3}' http://127.0.0.1:8080/api/v1/control/set_collision_level
curl -X POST -H 'Content-Type: application/json' -d '{"mode":0}' http://127.0.0.1:8080/api/v1/control/set_post_collision_mode
curl -X POST -H 'Content-Type: application/json' -d '{"axisID":1,"value":1}' http://127.0.0.1:8080/api/v1/control/brake_control
curl -X POST http://127.0.0.1:8080/api/v1/control/start_drag
curl -X POST http://127.0.0.1:8080/api/v1/control/stop_drag
curl -X POST http://127.0.0.1:8080/api/v1/control/stop_move_jog
```

### 6.3 参数和负载控制接口

```bash
curl -X POST -H 'Content-Type: application/json' -d '{"ratio":50}' http://127.0.0.1:8080/api/v1/control/speed_factor
curl -X POST -H 'Content-Type: application/json' -d '{"r":50}' http://127.0.0.1:8080/api/v1/control/acc_j
curl -X POST -H 'Content-Type: application/json' -d '{"r":50}' http://127.0.0.1:8080/api/v1/control/acc_l
curl -X POST -H 'Content-Type: application/json' -d '{"r":50}' http://127.0.0.1:8080/api/v1/control/vel_j
curl -X POST -H 'Content-Type: application/json' -d '{"r":50}' http://127.0.0.1:8080/api/v1/control/vel_l
curl -X POST -H 'Content-Type: application/json' -d '{"r":50}' http://127.0.0.1:8080/api/v1/control/cp
curl -X POST -H 'Content-Type: application/json' -d '{"load":1.2,"x":0,"y":0,"z":50}' http://127.0.0.1:8080/api/v1/control/set_payload
curl -X POST -H 'Content-Type: application/json' -d '{"distance":10}' http://127.0.0.1:8080/api/v1/control/set_back_distance
```

### 6.4 运动控制接口

```bash
curl -X POST -H 'Content-Type: application/json' -d '{"mode":false,"a":300,"b":0,"c":200,"d":180,"e":0,"f":90,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/mov_j
curl -X POST -H 'Content-Type: application/json' -d '{"mode":false,"a":300,"b":0,"c":200,"d":180,"e":0,"f":90,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/mov_l
curl -X POST -H 'Content-Type: application/json' -d '{"a":1,"b":0,"c":0,"d":0,"e":0,"f":0,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/rel_joint_mov_j
curl -X POST -H 'Content-Type: application/json' -d '{"a":1,"b":0,"c":0,"d":0,"e":0,"f":0,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/rel_mov_j_tool
curl -X POST -H 'Content-Type: application/json' -d '{"a":1,"b":0,"c":0,"d":0,"e":0,"f":0,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/rel_mov_l_tool
curl -X POST -H 'Content-Type: application/json' -d '{"a":1,"b":0,"c":0,"d":0,"e":0,"f":0,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/rel_mov_j_user
curl -X POST -H 'Content-Type: application/json' -d '{"a":1,"b":0,"c":0,"d":0,"e":0,"f":0,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/rel_mov_l_user
curl -X POST -H 'Content-Type: application/json' -d '{"mode":false,"a":300,"b":0,"c":200,"d":180,"e":0,"f":90,"MDIS":[],"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/mov_j_io
curl -X POST -H 'Content-Type: application/json' -d '{"mode":false,"a":300,"b":0,"c":200,"d":180,"e":0,"f":90,"MDIS":[],"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/mov_l_io
curl -X POST -H 'Content-Type: application/json' -d '{"mode":false,"a":300,"b":0,"c":200,"d":180,"e":0,"f":90,"a2":300,"b2":20,"c2":200,"d2":180,"e2":0,"f2":90,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/arc
curl -X POST -H 'Content-Type: application/json' -d '{"mode":false,"a":300,"b":0,"c":200,"d":180,"e":0,"f":90,"a2":300,"b2":20,"c2":200,"d2":180,"e2":0,"f2":90,"count":1,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/circle
curl -X POST -H 'Content-Type: application/json' -d '{"axisID":"J1","paramValue":[]}' http://127.0.0.1:8080/api/v1/control/move_jog
curl -X POST -H 'Content-Type: application/json' -d '{"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/servo_j
curl -X POST -H 'Content-Type: application/json' -d '{"a":300,"b":0,"c":200,"d":180,"e":0,"f":90,"paramValue":[]}' http://127.0.0.1:8080/api/v1/control/servo_p
```

### 6.5 IO 控制接口

```bash
curl -X POST -H 'Content-Type: application/json' -d '{"index":1,"status":1,"time":0}' http://127.0.0.1:8080/api/v1/control/do
curl -X POST -H 'Content-Type: application/json' -d '{"index":1,"status":1}' http://127.0.0.1:8080/api/v1/control/do_instant
curl -X POST -H 'Content-Type: application/json' -d '{"index":1,"status":1}' http://127.0.0.1:8080/api/v1/control/tool_do
curl -X POST -H 'Content-Type: application/json' -d '{"index":1,"status":1}' http://127.0.0.1:8080/api/v1/control/tool_do_instant
curl -X POST -H 'Content-Type: application/json' -d '{"index":1,"value":5}' http://127.0.0.1:8080/api/v1/control/ao
curl -X POST -H 'Content-Type: application/json' -d '{"index":1,"value":5}' http://127.0.0.1:8080/api/v1/control/ao_instant
curl -X POST -H 'Content-Type: application/json' -d '{"args":[1,1,0,1]}' http://127.0.0.1:8080/api/v1/control/do_group
curl -X POST -H 'Content-Type: application/json' -d '{"address":1,"value":1}' http://127.0.0.1:8080/api/v1/control/set_output_bool
curl -X POST -H 'Content-Type: application/json' -d '{"address":1,"value":100}' http://127.0.0.1:8080/api/v1/control/set_output_int
curl -X POST -H 'Content-Type: application/json' -d '{"address":1,"value":1.5}' http://127.0.0.1:8080/api/v1/control/set_output_float
curl -X POST -H 'Content-Type: application/json' -d '{"index":0,"addr":0,"count":1,"valTab":"1"}' http://127.0.0.1:8080/api/v1/control/set_coils
curl -X POST -H 'Content-Type: application/json' -d '{"index":0,"addr":0,"count":1,"valTab":"100","valType":"U16"}' http://127.0.0.1:8080/api/v1/control/set_hold_regs
```

### 6.6 坐标、工具、脚本和 Modbus 接口

```bash
curl -X POST -H 'Content-Type: application/json' -d '{"index":0}' http://127.0.0.1:8080/api/v1/control/user
curl -X POST -H 'Content-Type: application/json' -d '{"index":0}' http://127.0.0.1:8080/api/v1/control/tool
curl -X POST -H 'Content-Type: application/json' -d '{"index":0,"value":"0,0,0,0,0,0"}' http://127.0.0.1:8080/api/v1/control/set_user
curl -X POST -H 'Content-Type: application/json' -d '{"index":0,"value":"0,0,0,0,0,0"}' http://127.0.0.1:8080/api/v1/control/set_tool
curl -X POST -H 'Content-Type: application/json' -d '{"mode":0,"type":0}' http://127.0.0.1:8080/api/v1/control/set_tool_mode
curl -X POST -H 'Content-Type: application/json' -d '{"status":1}' http://127.0.0.1:8080/api/v1/control/set_tool_power
curl -X POST -H 'Content-Type: application/json' -d '{"baudrate":115200,"parity":"N","stop":1,"identify":0}' http://127.0.0.1:8080/api/v1/control/set_tool_485
curl -X POST -H 'Content-Type: application/json' -d '{"projectName":"demo.lua"}' http://127.0.0.1:8080/api/v1/control/run_script
curl -X POST -H 'Content-Type: application/json' -d '{"traceName":"demo","paramValue":[]}' http://127.0.0.1:8080/api/v1/control/start_path
curl -X POST -H 'Content-Type: application/json' -d '{"command":"RobotMode()"}' http://127.0.0.1:8080/api/v1/control/tcp_dashboard
curl -X POST -H 'Content-Type: application/json' -d '{"ip":"192.168.1.10","port":502,"slave_id":1,"isRTU":0}' http://127.0.0.1:8080/api/v1/control/modbus_create
curl -X POST -H 'Content-Type: application/json' -d '{"slave_id":1,"baud":115200,"parity":"N","data_bit":8,"stop_bit":1}' http://127.0.0.1:8080/api/v1/control/modbus_rtu_create
curl -X POST -H 'Content-Type: application/json' -d '{"index":0}' http://127.0.0.1:8080/api/v1/control/modbus_close
```

以上命令中的示例坐标、IO 地址、寄存器地址和 Modbus 参数仅用于展示请求格式，实际值必须根据机器人型号、控制器配置和现场安全要求填写。
