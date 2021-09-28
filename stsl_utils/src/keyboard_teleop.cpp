#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <thread>
#include <ncurses.h>

namespace stsl_utils
{

class KeyboardTeleop : public rclcpp::Node
{
public:
  explicit KeyboardTeleop(const rclcpp::NodeOptions & options)
  : rclcpp::Node("keyboard_teleop", options)
  {
    key_capture_thread_ = std::thread([this]() {KeyCaptureThread();});
    publisher_ =
      create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", rclcpp::SystemDefaultsQoS());
    timer_ = create_wall_timer(std::chrono::duration<double>(0.1), [this]() {PublishMessage();});
  }

  ~KeyboardTeleop()
  {
    thread_interrupt_ = true;
    if (key_capture_thread_.joinable()) {
      key_capture_thread_.join();
    }
  }

private:
  std::unordered_map<int, std::pair<double, double>> mapping_ = {
    {'i', {1, 0}},
    {'k', {0, 0}},
    {',', {-1, 0}},
    {'j', {0, 1}},
    {'l', {0, -1}},
    {'u', {1, 1}},
    {'o', {1, -1}},
    {'m', {-1, 1}},
    {'.', {-1, -1}}
  };
  double linear_speed_ = 0.5;
  double angular_speed_ = 1.0;
  std::atomic_bool thread_interrupt_{false};
  std::thread key_capture_thread_;
  std::mutex msg_mutex_;
  geometry_msgs::msg::Twist msg_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;

  void PublishMessage()
  {
    std::lock_guard<std::mutex> lock(msg_mutex_);
    publisher_->publish(msg_);
  }

  void KeyCaptureThread()
  {
    initscr();
    noecho();
    cbreak();
    timeout(10);
    auto menu_win = newwin(40, 40, 0, 0);
    PrintInstructions();
    while (!thread_interrupt_) {
      const auto key = getch();
      switch (key) {
        case ERR:
          // timeout
          break;
        case 'q':
          linear_speed_ *= 1.1;
          angular_speed_ *= 1.1;
          PrintSpeeds();
          break;
        case 'z':
          linear_speed_ *= 0.9;
          angular_speed_ *= 0.9;
          PrintSpeeds();
          break;
        case 'w':
          linear_speed_ *= 1.1;
          PrintSpeeds();
          break;
        case 'x':
          linear_speed_ *= 0.9;
          PrintSpeeds();
          break;
        case 'e':
          angular_speed_ *= 1.1;
          PrintSpeeds();
          break;
        case 'c':
          angular_speed_ *= 0.9;
          PrintSpeeds();
          break;
        default:
          {
            std::lock_guard<std::mutex> lock(msg_mutex_);
            msg_ = BuildMsg(mapping_[key]);
            break;
          }
      }
    }
    endwin();
  }

  geometry_msgs::msg::Twist BuildMsg(const std::pair<double, double> & speed_factors)
  {
    geometry_msgs::msg::Twist msg;
    msg.linear.x = speed_factors.first * linear_speed_;
    msg.angular.z = speed_factors.second * angular_speed_;
    return msg;
  }

  void PrintInstructions()
  {
    const auto instructions_str =
      R"(
Use these keys to move around:
u i o
j k l
m , .

q/z : increase/decrease max speeds by 10%
w/x : increase/decrease only linear speed by 10%
e/c : increase/decrease only angular speed by 10%

Press ctrl+c to quit
)";
    mvprintw(0, 0, instructions_str);
    refresh();
  }

  void PrintSpeeds()
  {
    std::cout << "currently:\tspeed " << linear_speed_ << "\tturn " << angular_speed_ << "\r\n";
  }

};

}

RCLCPP_COMPONENTS_REGISTER_NODE(stsl_utils::KeyboardTeleop)
