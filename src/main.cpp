/**
 ***********************************************************************************************************************
 *
 * @author ZhangRan
 * @date   2021/08/07
 *
 * <h2><center>&copy; COPYRIGHT 2021 Dobot CORPORATION</center></h2>
 *
 ***********************************************************************************************************************
 */

#include <ros/ros.h>
#include <ros/param.h>
#include <dobot_v4_bringup/cr5_v4_robot.h>
#include <sensor_msgs/JointState.h>
#include <dobot_v4_bringup/ToolVectorActual.h>
#include <dobot_v4_bringup/http_api_types.h>
#include <dobot_v4_bringup/http_server.h>
#include <dobot_v4_bringup/EnableRobot.h>
#include <dobot_v4_bringup/DisableRobot.h>
#include <dobot_v4_bringup/ClearError.h>
#include <dobot_v4_bringup/Stop.h>
#include <dobot_v4_bringup/Pause.h>
#include <dobot_v4_bringup/Continue.h>
#include <dobot_v4_bringup/PowerOn.h>
#include <dobot_v4_bringup/EmergencyStop.h>
#include <dobot_v4_bringup/RobotMode.h>
#include <dobot_v4_bringup/GetPose.h>
#include <dobot_v4_bringup/GetErrorID.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

template <typename Service>
bool callRobotService(const std::string& prefix, const std::string& name, Service& service)
{
    ros::NodeHandle nh;
    return nh.serviceClient<Service>(prefix + name).call(service);
}

int main(int argc, char* argv[])
{
    ros::init(argc, argv, "CRRobot");

    try
    {
        ros::NodeHandle node;
        ros::NodeHandle private_node("~");

        ros::AsyncSpinner async_spinner(1);
        async_spinner.start();

        sensor_msgs::JointState joint_state_msg;
        ros::Publisher joint_state_pub = private_node.advertise<sensor_msgs::JointState>("/joint_states", 100);
        dobot_v4_bringup::RobotStatus robot_status_msg;
        ros::Publisher robot_status_pub = private_node.advertise<dobot_v4_bringup::RobotStatus>("/dobot_v4_bringup/msg/RobotStatus", 100);

        dobot_v4_bringup::ToolVectorActual tool_vector_actual_msg;
        ros::Publisher tool_vector_pub =
            private_node.advertise<dobot_v4_bringup::ToolVectorActual>("/dobot_v4_bringup/msg/ToolVectorActual", 100);
        string z ="/";
        const char* robot_type = getenv("DOBOT_TYPE");
        string a = robot_type == nullptr ? "cr5" : robot_type;
        string b = "_robot/joint_controller/follow_joint_trajectory";
        string ss =  z + a + b ;
        for (uint32_t i = 0; i < 6; i++)
        {
            joint_state_msg.position.push_back(0.0);
            joint_state_msg.name.push_back(std::string("joint") + std::to_string(i + 1));
        }

        CRRobot robot(private_node, ss);

        dobot_v4_bringup::HttpServer http_server;
        const int http_port = private_node.param("http_port", 8080);
        const std::string service_prefix = "/dobot_v4_bringup/srv/";
        auto call_service = [&service_prefix](const std::string& name, auto& service) {
            return callRobotService(service_prefix, name, service);
        };
        http_server.registerGet("/api/v1/health", [&robot](const nlohmann::json&) {
            return dobot_v4_bringup::makeSuccess(
                0, "ok", {{"ros_ok", ros::ok()}, {"robot_connected", robot.isConnected()},
                          {"robot_enabled", robot.isEnable()}});
        });

        http_server.registerGet("/api/v1/info/joint_state", [&robot](const nlohmann::json&) {
            double joints[6]{};
            robot.getJointState(joints);
            return dobot_v4_bringup::makeSuccess(
                0, "ok", { {"joints", {joints[0], joints[1], joints[2], joints[3], joints[4], joints[5]}} });
        });
        http_server.registerGet("/api/v1/info/tool_pose", [&robot](const nlohmann::json&) {
            double pose[6]{};
            robot.getToolVectorActual(pose);
            return dobot_v4_bringup::makeSuccess(
                0, "ok", { {"pose", {pose[0], pose[1], pose[2], pose[3], pose[4], pose[5]}} });
        });

        http_server.registerPost("/api/v1/control/enable", [&call_service](const nlohmann::json&) {
            dobot_v4_bringup::EnableRobot srv;
            if (!call_service("EnableRobot", srv)) return dobot_v4_bringup::makeError(503, "ROS service unavailable");
            return dobot_v4_bringup::makeSuccess(srv.response.res, "EnableRobot completed", {});
        });
        http_server.registerPost("/api/v1/control/disable", [&call_service](const nlohmann::json&) {
            dobot_v4_bringup::DisableRobot srv;
            if (!call_service("DisableRobot", srv)) return dobot_v4_bringup::makeError(503, "ROS service unavailable");
            return dobot_v4_bringup::makeSuccess(srv.response.res, "DisableRobot completed", {});
        });
        http_server.registerPost("/api/v1/control/clear_error", [&call_service](const nlohmann::json&) {
            dobot_v4_bringup::ClearError srv;
            if (!call_service("ClearError", srv)) return dobot_v4_bringup::makeError(503, "ROS service unavailable");
            return dobot_v4_bringup::makeSuccess(srv.response.res, "ClearError completed", {});
        });
        http_server.registerPost("/api/v1/control/stop", [&call_service](const nlohmann::json&) {
            dobot_v4_bringup::Stop srv;
            if (!call_service("Stop", srv)) return dobot_v4_bringup::makeError(503, "ROS service unavailable");
            return dobot_v4_bringup::makeSuccess(srv.response.res, "Stop completed", {});
        });
        http_server.registerPost("/api/v1/control/pause", [&call_service](const nlohmann::json&) {
            dobot_v4_bringup::Pause srv;
            if (!call_service("Pause", srv)) return dobot_v4_bringup::makeError(503, "ROS service unavailable");
            return dobot_v4_bringup::makeSuccess(srv.response.res, "Pause completed", {});
        });
        http_server.registerPost("/api/v1/control/continue", [&call_service](const nlohmann::json&) {
            dobot_v4_bringup::Continue srv;
            if (!call_service("Continue", srv)) return dobot_v4_bringup::makeError(503, "ROS service unavailable");
            return dobot_v4_bringup::makeSuccess(srv.response.res, "Continue completed", {});
        });
        http_server.registerPost("/api/v1/control/power_on", [&call_service](const nlohmann::json&) {
            dobot_v4_bringup::PowerOn srv;
            if (!call_service("PowerOn", srv)) return dobot_v4_bringup::makeError(503, "ROS service unavailable");
            return dobot_v4_bringup::makeSuccess(srv.response.res, "PowerOn completed", {});
        });
        http_server.registerPost("/api/v1/control/emergency_stop", [&call_service](const nlohmann::json& body) {
            dobot_v4_bringup::EmergencyStop srv;
            srv.request.value = body.value("value", 1);
            if (!call_service("EmergencyStop", srv)) return dobot_v4_bringup::makeError(503, "ROS service unavailable");
            return dobot_v4_bringup::makeSuccess(srv.response.res, "EmergencyStop completed", {});
        });
        if (!http_server.start(static_cast<std::uint16_t>(http_port))) {
            ROS_ERROR("Failed to start HTTP server on port %d", http_port);
            return -1;
        }

        double rate_vale = private_node.param("JointStatePublishRate", 10);

        robot.init();
        ros::Rate rate(rate_vale);
        double position[6];
        while (ros::ok())
        {
            //
            // publish joint state
            //
            robot.getJointState(position);
            joint_state_msg.header.stamp = ros::Time::now();
            joint_state_msg.header.frame_id = "dummy_link";
            for (uint32_t i = 0; i < 6; i++)
                joint_state_msg.position[i] = position[i];
            joint_state_pub.publish(joint_state_msg);

            double val[6];
            robot.getToolVectorActual(val);
            tool_vector_actual_msg.x = val[0];
            tool_vector_actual_msg.y = val[1];
            tool_vector_actual_msg.z = val[2];
            tool_vector_actual_msg.rx = val[3];
            tool_vector_actual_msg.ry = val[4];
            tool_vector_actual_msg.rz = val[5];
            tool_vector_pub.publish(tool_vector_actual_msg);

            //
            // publish robot status
            //
            robot_status_msg.is_enable = robot.isEnable();
            robot_status_msg.is_connected = robot.isConnected();
            robot_status_pub.publish(robot_status_msg);

            rate.sleep();
        }
    }
    catch (const std::exception& err)
    {
        ROS_ERROR("%s", err.what());
        return -1;
    }

    return 0;
}
