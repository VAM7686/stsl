#include <memory>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32.hpp>
#include <std_msgs/msg/u_int64.hpp>

#include "motor.h"

using namespace std::chrono_literals;
using std::placeholders::_1;

class RobotInterfaceNode : public rclcpp::Node
{
public:
    RobotInterfaceNode() : Node("robot_interface_node")
    {
        left_encoder_publisher_ = create_publisher<std_msgs::msg::UInt64>("encoder_left", 1);
        right_encoder_publisher_ = create_publisher<std_msgs::msg::UInt64>("encoder_right", 1);
        center_line_publisher_ = create_publisher<std_msgs::msg::UInt64>("line_center", 1);
        side_line_publisher_ = create_publisher<std_msgs::msg::UInt64>("line_side", 1);
        ultrasonic_publisher_ = create_publisher<std_msgs::msg::Float32>("ultrasonic", 1);

        left_motor_subscriber_ = create_subscription<std_msgs::msg::Float32>("motor_left", 1, std::bind(&RobotInterfaceNode::leftMotorCallback, this, _1));
        right_motor_subscriber_ = create_subscription<std_msgs::msg::Float32>("motor_right", 1, std::bind(&RobotInterfaceNode::rightMotorCallback, this, _1));
    }

private:
    void leftMotorCallback(const std_msgs::msg::Float32::SharedPtr msg)
    {
        left_motor_.setPower(msg->data);
    }

    void rightMotorCallback(const std_msgs::msg::Float32::SharedPtr msg)
    {
        right_motor_.setPower(msg->data);
    }

    rclcpp::Publisher<std_msgs::msg::UInt64>::SharedPtr left_encoder_publisher_;
    rclcpp::Publisher<std_msgs::msg::UInt64>::SharedPtr right_encoder_publisher_;

    rclcpp::Publisher<std_msgs::msg::UInt64>::SharedPtr center_line_publisher_;
    rclcpp::Publisher<std_msgs::msg::UInt64>::SharedPtr side_line_publisher_;

    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr ultrasonic_publisher_;

    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr left_motor_subscriber_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr right_motor_subscriber_;

    Motor left_motor_{left_motor_parameters};
    Motor right_motor_{right_motor_parameters};
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RobotInterfaceNode>());
    rclcpp::shutdown();
    return 0;
}

