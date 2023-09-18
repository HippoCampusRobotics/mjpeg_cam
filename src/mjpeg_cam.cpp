#include "mjpeg_cam/mjpeg_cam.hpp"

namespace mjpeg_cam {
MjpegCam::MjpegCam(const rclcpp::NodeOptions &_options)
    : rclcpp::Node("mjpeg_cam", _options) {
  if (_options.use_intra_process_comms() || true) {
    image_pub_ = create_publisher<sensor_msgs::msg::CompressedImage>(
        "image_raw/compressed", 10);
    info_pub_ =
        create_publisher<sensor_msgs::msg::CameraInfo>("camera_info", 10);
  }
  InitPublishers();
  InitServices();
  InitParams();
  InitFrameSizes();
  auto frame_size = frame_sizes_.at(std::clamp<std::size_t>(
      params_.discrete_size, 0, frame_sizes_.size() - 1));
  camera_ = std::make_shared<Device>(DeviceName(), frame_size.first,
                                     frame_size.second);
  RCLCPP_INFO(get_logger(), "Opened %s with size [%lu, %lu]",
              DeviceName().c_str(), camera_->GetWidth(), camera_->GetHeight());
  LogAvailableFrameSizes();
  InitCameraParams();

  capture_thread_ = std::thread{[this]() -> void {
    while (rclcpp::ok()) {
      sensor_msgs::msg::CameraInfo camera_info;
      auto img = camera_->Capture();
      if (img == nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }
      img->header.stamp = now();
      camera_info.header = img->header;
      camera_info.width = camera_->GetWidth();
      camera_info.height = camera_->GetHeight();
      image_pub_->publish(std::move(img));
      info_pub_->publish(camera_info);
    }
  }};
}

void MjpegCam::InitPublishers() {
  std::string topic;
  rclcpp::SensorDataQoS qos{};
  qos.keep_last(1);

  topic = "image_raw/compressed";
  image_pub_ = create_publisher<sensor_msgs::msg::CompressedImage>(topic, qos);

  topic = "camera_info";
  info_pub_ = create_publisher<sensor_msgs::msg::CameraInfo>(topic, qos);
}

void MjpegCam::InitServices() {
  std::string name;

  name = "~/set_defaults";
  set_defaults_service_ = create_service<std_srvs::srv::Trigger>(
      name,
      [this]([[maybe_unused]] const std_srvs::srv::Trigger::Request::SharedPtr
                 request,
             std_srvs::srv::Trigger::Response::SharedPtr response) {
        response->success = SetControlDefaults();
        if (!response->success) {
          response->message = "At least one control could not be set";
        }
        return response;
      });
}

void MjpegCam::InitFrameSizes() {
  auto camera = std::make_shared<Device>(DeviceName(), 0, 0);
  frame_sizes_ = camera->AvailableFrameSizes();
  camera.reset();
  if (frame_sizes_.size() <= 0) {
    throw std::runtime_error(
        "No discrete frame sizes available. Does the camera support MJPEG?");
  }
}

void MjpegCam::LogAvailableFormats() {
  std::string available_formats;
  available_formats = "Available formats:\n";
  std::vector<std::string> formats = camera_->AvailableFormats();
  for (auto const &format : formats) {
    available_formats += format + "\n";
  }
  RCLCPP_INFO_STREAM(get_logger(), available_formats);
}

void MjpegCam::LogAvailableFrameSizes() {
  std::string text = "\nFrame sizes:\n";
  auto sizes = camera_->AvailableFrameSizes();
  int i = 0;
  for (auto const &size : sizes) {
    text += std::to_string(i) + ": [" + std::to_string(size.first) + ", " +
            std::to_string(size.second) + "]\n";
    ++i;
  }
  RCLCPP_INFO_STREAM(get_logger(), text);
}

bool MjpegCam::SetControlDefaults() {
  if (!camera_) {
    RCLCPP_WARN(get_logger(),
                "Camera not initialized. Cannot set control defaults");
    return false;
  }
  bool success{true};
  for (auto const &control : camera_->Controls()) {
    if (!camera_->SetControlValue(control.id, control.default_value)) {
      RCLCPP_INFO(get_logger(), "Failed to set default [%s] = %d",
                   control.name.c_str(), control.default_value);
    }
  }
  return success;
}

}  // namespace mjpeg_cam

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(mjpeg_cam::MjpegCam)
