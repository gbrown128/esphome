// Implementation based on:
//  - DHT12 ESPHome component, which is based on:
//  - ESPEasy: https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P034_DHT12.ino
//  - DHT12_sensor_library: https://github.com/xreef/DHT12_sensor_library/blob/master/DHT12.cpp

#include "cc2d.h"
#include "esphome/core/log.h"

namespace esphome {
namespace cc2d {

static const char *const TAG = "cc2d";

void CC2DComponent::update() {
  uint8_t data[4];
  if (!this->read_data_(data)) {
    this->status_set_warning();
    return;
  }
  const uint16_t raw_temperature = uint16_t(data[2] << 6) | ((data[3] & 0xFC)>>2);
  float temperature = (165*raw_temperature/16384.0)-40.0;

  const uint16_t raw_humidity = uint16_t(data[0]&0x3F) << 8 | data[1];
  float humidity = 100*raw_humidity/16384.0;

  ESP_LOGD(TAG, "Got temperature=%.2f°C humidity=%.2f%%", temperature, humidity);
  if (this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(temperature);
  if (this->humidity_sensor_ != nullptr)
    this->humidity_sensor_->publish_state(humidity);
  this->status_clear_warning();
}
void CC2DComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CC2D...");
  uint8_t data[4];
  if (!this->read_data_(data)) {
    this->mark_failed();
    return;
  }
}
void CC2DComponent::dump_config() {
  ESP_LOGD(TAG, "CC2D:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with CC2D failed!");
  }
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
}
float CC2DComponent::get_setup_priority() const { return setup_priority::DATA; }
bool CC2DComponent::read_data_(uint8_t *data) {
  if (!this->read_bytes(0, data, 4)) {
    ESP_LOGW(TAG, "Updating CC2D failed!");
    return false;
  }

  switch(data[0] & 0xC0){
    case 1:
      ESP_LOGW(TAG, "CC2D reported data is stale");
      return false;
    break;;
    case 2:
      ESP_LOGW(TAG, "CC2D reports it is in command mode.");
      return false;
    break;;
    case 3:
      ESP_LOGW(TAG, "CC2D invald status report.");
      return false;
    break;;
  }

  return true;
}

}  // namespace cc2d
}  // namespace esphome
