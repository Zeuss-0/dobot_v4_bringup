# Dobot V4 HTTP API 设计

## 目标

保留 `dobot_v4_bringup` 现有 ROS service 和 topic，同时在同一 ROS 节点中增加 HTTP 网关，使外部系统可以通过 HTTP 请求访问机器人信息和控制接口。HTTP 接口统一返回 JSON。

## 运行方式

- HTTP server 与 `CRRobot` 使用同一进程和机器人连接。
- 默认监听端口：`8080`。
- 端口通过 ROS 私有参数配置：`~http_port`。
- HTTP 层调用现有 ROS service 或共享的机器人控制逻辑，不复制 TCP 协议实现。
- ROS service 保持原有名称、参数和行为不变。

## 路由

```text
GET  /api/v1/health
GET  /api/v1/info/<name>
POST /api/v1/control/<name>
```

接口名使用小写下划线命名，例如：

```text
GET  /api/v1/info/pose
GET  /api/v1/info/robot_mode
POST /api/v1/control/enable_robot
POST /api/v1/control/mov_j
POST /api/v1/control/emergency_stop
```

## JSON 响应

成功响应：

```json
{
  "success": true,
  "code": 0,
  "message": "ok",
  "data": {}
}
```

失败响应：

```json
{
  "success": false,
  "code": 1001,
  "message": "robot service call failed",
  "data": null
}
```

约定：

- HTTP 2xx 表示请求已被正确处理；机器人返回失败时使用 `success: false` 和业务错误码。
- 400：JSON 参数缺失、类型错误或值非法。
- 404：接口不存在。
- 409：机器人当前状态不允许执行操作。
- 500：ROS service 或机器人连接异常。
- `data` 保存 ROS service response 中除 `res` 外的返回字段；`res` 统一映射到 `code`，同时保留在 `data.res`。

## 当前已实现的信息接口

当前代码已经实现：

- `GET /api/v1/health`
- `GET /api/v1/info/joint_state`
- `GET /api/v1/info/tool_pose`

其余信息接口按下方清单继续接入时，路由命名和 JSON 映射保持一致。

## 信息接口规划

包括：

- `GetStartPose`、`GetPose`、`GetAngle`、`PositiveKin`、`InverseKin`
- `RobotMode`、`GetErrorID`、`GetCurrentCommandId`
- `DI`、`DIGroup`、`AI`、`ToolDI`、`ToolAI`
- `GetDO`、`GetDOGroup`、`GetAO`
- `GetInputBool`、`GetInputInt`、`GetInputFloat`
- `GetOutputBool`、`GetOutputInt`、`GetOutputFloat`
- `GetInBits`、`GetInRegs`、`GetCoils`、`GetHoldRegs`
- `CalcTool`、`CalcUser`
- `/dobot_v4_bringup/msg/RobotStatus` 和 `/dobot_v4_bringup/msg/ToolVectorActual` 的健康/状态汇总

## 当前已实现的控制接口

当前代码已经实现：

- `POST /api/v1/control/enable`
- `POST /api/v1/control/disable`
- `POST /api/v1/control/clear_error`
- `POST /api/v1/control/stop`
- `POST /api/v1/control/pause`
- `POST /api/v1/control/continue`
- `POST /api/v1/control/power_on`
- `POST /api/v1/control/emergency_stop`，请求体可传 `{ "value": 1 }`

这些接口通过 ROS service client 调用现有服务，不改变原有机器人控制回调。

## 控制接口规划

包括：

- `EnableRobot`、`DisableRobot`、`PowerOn`、`ClearError`
- `MovJ`、`MovL`、`MovJIO`、`MovLIO`
- `RelJointMovJ`、`RelMovJTool`、`RelMovLTool`、`RelMovJUser`、`RelMovLUser`
- `Arc`、`Circle`、`MoveJog`、`StopMoveJog`、`ServoJ`、`ServoP`
- `Stop`、`Pause`、`Continue`、`EmergencyStop`
- `SpeedFactor`、`AccJ`、`AccL`、`VelJ`、`VelL`、`CP`
- `DO`、`DOInstant`、`ToolDO`、`ToolDOInstant`、`AO`、`AOInstant`
- `SetCoils`、`SetHoldRegs`、`SetOutputBool`、`SetOutputInt`、`SetOutputFloat`
- `User`、`Tool`、`SetUser`、`SetTool`、`SetPayload`
- `SetCollisionLevel`、`EnableSafeSkin`、`SetSafeSkin`、`SetSafeWallEnable`
- `SetTool485`、`SetToolPower`、`SetToolMode`、`SetBackDistance`、`SetPostCollisionMode`
- `BrakeControl`、`StartDrag`、`StopDrag`、`DragSensivity`
- `StartPath`、`RunScript`
- `ModbusRTUCreate`、`ModbusCreate`、`ModbusClose`
- `TcpDashboard`

## 参数映射

HTTP body 使用 JSON 对象，字段名与对应 `.srv` request 字段保持一致。例如 `MovJ.srv`：

```json
{
  "mode": false,
  "a": 100.0,
  "b": 0.0,
  "c": 200.0,
  "d": 0.0,
  "e": 0.0,
  "f": 0.0,
  "paramValue": []
}
```

参数转换在 HTTP 层完成，ROS service 回调和机器人底层协议不改动。

## 并发与安全

- 同一机器人控制对象的控制请求串行化，避免同时执行运动、急停和 IO 操作。
- 信息查询可以并发，但需要保护共享的机器人状态。
- HTTP server 绑定地址和端口可配置。
- 初始版本不引入认证；部署在非可信网络时必须通过反向代理或网络策略限制访问。
- 急停、运动和脚本执行接口记录请求日志，日志不记录敏感凭据。

## 验证标准

- 现有 ROS service 和原有启动方式不受影响。
- `GET /api/v1/health` 能返回 ROS 节点和机器人连接状态。
- 每个现有 service 至少有一个对应 HTTP 路由。
- 参数错误返回 400，未知路由返回 404，ROS/机器人调用失败返回结构化错误 JSON。
- 至少验证一个信息接口、一个运动控制接口、一个 IO 接口和一个错误场景。
