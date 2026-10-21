# dobot_v4_bringup

ROS 1（catkin）功能包：通过 TCP/IP 连接越疆 Dobot V4 六轴控制器，发布关节与末端状态，并提供 Dashboard 服务和 `FollowJointTrajectory` 动作服务器。

ROS 1 (catkin) package that connects to a Dobot V4 6-axis controller over TCP/IP, publishes joint and tool state, and exposes Dashboard services plus a `FollowJointTrajectory` action server.

本仓库是通信功能包 `dobot_v4_bringup`，衍生自 [Dobot-Arm/TCP-IP-ROS-6AXis](https://github.com/Dobot-Arm/TCP-IP-ROS-6AXis)。MoveIt、Gazebo 与 URDF 在上游完整仓库中。

This repository is the `dobot_v4_bringup` communication package, derived from [Dobot-Arm/TCP-IP-ROS-6AXis](https://github.com/Dobot-Arm/TCP-IP-ROS-6AXis). MoveIt, Gazebo, and the URDF packages live in that upstream tree.

运动类服务和轨迹动作会驱动真实机械臂。启动前确认工作空间清空，控制器已切到 TCP 模式，并且本机可以 ping 通控制器。

Motion services and the trajectory action command a real arm. Before launch, keep the workspace clear, put the controller in TCP mode, and confirm this computer can ping the controller.

## 依赖 / Dependencies

- Ubuntu 上的 ROS 1（Kinetic、Melodic 或 Noetic）与 catkin。源码按 C++14 编译。
- ROS 1 on Ubuntu (Kinetic, Melodic, or Noetic) with catkin. Sources compile as C++14.

`package.xml` 声明了 `roscpp`、`std_msgs`、`message_generation`、`message_runtime`。源码还直接包含 `actionlib`、`control_msgs`、`sensor_msgs`、`trajectory_msgs`，`CMakeLists.txt` 会查找 Boost.System。`nlohmann/json` 已放在 `include/nlohmann/`，无需单独安装。

`package.xml` declares `roscpp`, `std_msgs`, `message_generation`, and `message_runtime`. The sources also include `actionlib`, `control_msgs`, `sensor_msgs`, and `trajectory_msgs`, and `CMakeLists.txt` looks up Boost.System. `nlohmann/json` is vendored under `include/nlohmann/` and does not need a separate install.

```bash
sudo apt-get install \
  ros-$ROS_DISTRO-roscpp \
  ros-$ROS_DISTRO-std-msgs \
  ros-$ROS_DISTRO-message-generation \
  ros-$ROS_DISTRO-message-runtime \
  ros-$ROS_DISTRO-actionlib \
  ros-$ROS_DISTRO-control-msgs \
  ros-$ROS_DISTRO-sensor-msgs \
  ros-$ROS_DISTRO-trajectory-msgs \
  libboost-system-dev
```

## 环境变量与参数 / Environment and parameters

### `DOBOT_TYPE`（启动前必须设置 / required before launch）

`launch/bringup_v4.launch` 用 `$(env DOBOT_TYPE)` 拼节点名。变量未导出时，`roslaunch` 会报错退出。这里的字符串只作为名字前缀，本包不据此加载 URDF。

`launch/bringup_v4.launch` builds the node name with `$(env DOBOT_TYPE)`. `roslaunch` exits with an error when the variable is unset. The string is only a name prefix; this package does not load a URDF from it.

与上游说明一致的常用取值：`cr3`、`cr5`、`cr10`、`cr16`、`me6`、`nova2`、`nova5`。

Values used in the upstream docs: `cr3`, `cr5`, `cr10`, `cr16`, `me6`, `nova2`, `nova5`.

```bash
export DOBOT_TYPE=cr5
```

| 名称 / Name | 规则 / Rule |
| --- | --- |
| 节点名 / node name | `$(arg robotName)$(env DOBOT_TYPE)_robot`。`robotName` 默认为空，故 `DOBOT_TYPE=cr5` 时节点为 `cr5_robot`。 |
| 动作名 / action name | `/<DOBOT_TYPE>_robot/joint_controller/follow_joint_trajectory`。节点进程内若读不到该变量，动作名回退为 `/cr5_robot/joint_controller/follow_joint_trajectory`。经本 launch 启动时变量已经存在。 |

### 机器人 IP / Robot IP

Launch 参数 `robotIp` 默认 **`192.168.1.80`**，写入节点私有参数 `robot_ip_address`。代码在参数缺失时同样回退到 `192.168.1.80`。

Launch argument `robotIp` defaults to **`192.168.1.80`** and is stored as the private parameter `robot_ip_address`. The node uses the same address when that parameter is absent.

节点连接两个 TCP 端口：

The node opens two TCP ports:

| 端口 / Port | 用途 / Role |
| --- | --- |
| 29999 | Dashboard。下发设置与运动指令，同一时间通常只接受一个客户端，且要求控制器处于 TCP 模式。 |
| 30004 | 实时反馈（关节角、末端位姿等）。 |

控制器有线 LAN1 常见地址是 `192.168.5.1`，LAN2 常见 `192.168.100.1`。与本仓库默认值不一致时，在启动命令里覆盖 `robotIp`。

Wired LAN1 on the controller is often `192.168.5.1`, and LAN2 is often `192.168.100.1`. Pass `robotIp` on the launch command when the controller is not at this repo's default.

### Launch 写入的私有参数 / Private parameters set by the launch file

| 参数 / Parameter | Launch 默认 / Launch default | 说明 / Meaning |
| --- | --- | --- |
| `robot_ip_address` | `192.168.1.80`（来自 `robotIp`） | 控制器 IP。 |
| `trajectory_duration` | `0.1` | 轨迹执行定时器周期（秒）。参数缺失时节点使用 `0.3`。 |
| `num_nodes` | `1` | 大于 1 时，服务名中插入 `robot_node_name`。 |
| `robot_node_name` | 空（来自 `robotName`） | 多机时的名字前缀。`num_nodes` 为 1 时不参与服务名。 |
| `joint_publish_rate` | `10` | Launch 会设置此项。节点实际读取的发布频率参数是 `JointStatePublishRate`（未设置时为 10 Hz）。 |

## 编译与启动 / Build and launch

将本仓库作为 catkin 包放进工作空间（目录内已有 `package.xml`）：

Clone this repository into a catkin workspace as a package (it already contains `package.xml`):

```bash
mkdir -p ~/catkin_ws/src
cd ~/catkin_ws/src
git clone https://github.com/Zeuss-0/dobot_v4_bringup.git
cd ~/catkin_ws
catkin_make
source devel/setup.bash

export DOBOT_TYPE=cr5
roslaunch dobot_v4_bringup bringup_v4.launch robotIp:=192.168.1.80
```

控制器地址就是默认的 `192.168.1.80` 时，可以省略 `robotIp:=192.168.1.80`。

Omit `robotIp:=192.168.1.80` when the controller is already at that default address.

多机时，把 `bringup_v4.launch` 里的 `num_nodes` 改为大于 1，再为每台控制器单独启动一次，并用 `robotName` 区分。服务名变为 `/dobot_v4_bringup/<robotName>/srv/...`，节点名变为 `<robotName><DOBOT_TYPE>_robot`。

For more than one arm, set `num_nodes` in `bringup_v4.launch` to a value greater than 1, launch once per controller, and distinguish them with `robotName`. Service names become `/dobot_v4_bringup/<robotName>/srv/...`, and the node name becomes `<robotName><DOBOT_TYPE>_robot`.

```bash
roslaunch dobot_v4_bringup bringup_v4.launch robotIp:=192.168.1.80 robotName:=robotA
```

## 启动后的接口 / Interfaces after launch

`num_nodes` 为 1 时：

With `num_nodes` left at 1:

| 接口 / Interface | 类型 / Type | 内容 / Contents |
| --- | --- | --- |
| `/joint_states` | `sensor_msgs/JointState` | `joint1`–`joint6`，单位弧度。 |
| `/dobot_v4_bringup/msg/RobotStatus` | `dobot_v4_bringup/RobotStatus` | `is_enable`、`is_connected`。 |
| `/dobot_v4_bringup/msg/ToolVectorActual` | `dobot_v4_bringup/ToolVectorActual` | `x y z rx ry rz`。 |
| `/dobot_v4_bringup/srv/*` | `rosservice` | EnableRobot、MovJ、MovL 等 Dashboard 服务。以 `rosservice list` 为准。 |
| `/<DOBOT_TYPE>_robot/joint_controller/follow_joint_trajectory` | `control_msgs/FollowJointTrajectory` | 轨迹动作服务器。 |

## 许可 / License

本包以 MIT 发布，正文见 [LICENSE](LICENSE)，与 `package.xml` 中的 `<license>MIT</license>` 一致。版权声明为 Copyright (c) 2023 Dobot。

This package is released under the MIT License in [LICENSE](LICENSE), matching `<license>MIT</license>` in `package.xml`. The copyright line is Copyright (c) 2023 Dobot.

第三方组件（含 `include/nlohmann/` 中的 nlohmann/json）以及相对上游仓库的归属说明见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。

Third-party components, including nlohmann/json under `include/nlohmann/`, and the attribution relative to the upstream repository are listed in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
