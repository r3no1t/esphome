#include "sht25.h"
#include "esphome/core/log.h"
#include <stdlib.h>

namespace esphome {
namespace sht25 {

namespace {
const char *TAG = "sht25";

constexpr uint8_t SHT25_COMMAND_START_READ_TEMP = 0xF3;
constexpr uint8_t SHT25_COMMAND_START_READ_HUMID = 0xF5;
constexpr uint8_t SHT25_COMMAND_SOFT_RESET = 0xFE;

constexpr uint8_t SHT25_COMMAND_READ_IDENT_1 = 0xFA;
constexpr uint8_t SHT25_COMMAND_READ_IDENT_2 = 0xFC;
constexpr uint8_t SHT25_MEMORY_ADDR_IDENT_1 = 0x0F;
constexpr uint8_t SHT25_MEMORY_ADDR_IDENT_2 = 0xC9;

constexpr uint32_t MAX_RETRY = 20;
}

void SHT25Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up SHT25...");
  std::string serial_number;
  if (!this->read_serial_number_(serial_number)) {
    this->mark_failed();
    return;
  }
  ESP_LOGV(TAG, "    Serial Number: %s", serial_number.c_str());
}

void SHT25Component::dump_config() {
  ESP_LOGCONFIG(TAG, "SHT25:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with SHT25 failed!");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
}

float SHT25Component::get_setup_priority() const {
  return setup_priority::DATA;
}

void SHT25Component::update() {
  uint8_t raw_data[3];

  switch(this->sht25_state_) {

  case SHT25State::READ_TEMP:
    this->retry_count_ = 0;
    if(!this->write_bytes(SHT25_COMMAND_START_READ_TEMP, nullptr, 0)) {
      ESP_LOGE(TAG, "sending start READ_TEMP failed");
      this->status_set_warning();
    }
    this->sht25_state_ = SHT25State::WAIT_TEMP;
    this->set_timeout("update", 100, [this]() { this->update(); });
    break;

  case SHT25State::WAIT_TEMP:
    if(this->read_bytes_raw(raw_data, sizeof(raw_data))) {
      if(this->check_crc_(raw_data, 2, raw_data[2])) {
        this->temperature_ = this->calc_temperature_(raw_data);
        this->sht25_state_ = SHT25State::READ_HUMID;
      }
      else {
        ESP_LOGE(TAG, "temp crc failed");
        this->status_set_warning();
      }
    }
    else if(++this->retry_count_ > MAX_RETRY) {
      ESP_LOGE(TAG, "temp retry failed");
      this->retry_count_ = 0;
      this->status_set_warning();
    }
    this->set_timeout("update", 100, [this]() { this->update(); });
    break;

  case SHT25State::READ_HUMID:
    this->retry_count_ = 0;
    if(!this->write_bytes(SHT25_COMMAND_START_READ_HUMID, nullptr, 0)) {
      ESP_LOGE(TAG, "sending start READ_HUMID failed");
      this->status_set_warning();
    }
    this->sht25_state_ = SHT25State::WAIT_HUMID;
    this->set_timeout("update", 50, [this]() { this->update(); });
    break;

  case SHT25State::WAIT_HUMID:
    if(this->read_bytes_raw(raw_data, sizeof(raw_data))) {
      if(this->check_crc_(raw_data, 2, raw_data[2])) {
        this->humidity_ = this->calc_humidity_(raw_data);
        this->sht25_state_ = SHT25State::READ_TEMP;
        ESP_LOGD(TAG, "Got temperature=%.2f°C humidity=%.2f%%", this->temperature_, this->humidity_);
        if (this->temperature_sensor_ != nullptr)
          this->temperature_sensor_->publish_state(this->temperature_);
        if (this->humidity_sensor_ != nullptr)
          this->humidity_sensor_->publish_state(this->humidity_);
      }
    }
    else {
      if(++this->retry_count_ > MAX_RETRY) {
        ESP_LOGE(TAG, "humid retry failed");
        this->retry_count_ = 0;
        this->status_set_warning();
      }
      this->set_timeout("update", 50, [this]() { this->update(); });
    }
    break;

  default:
    break;
  }

  if (this->status_has_warning() && (this->sht25_state_ != SHT25State::IN_RESET)) {
    ESP_LOGD(TAG, "Trying to reset the sensor.");
    this->sht25_state_ = SHT25State::IN_RESET;
    this->write_bytes(SHT25_COMMAND_SOFT_RESET, nullptr, 0);
    this->set_timeout("update", 50, [this]() {
      this->status_clear_warning();
      this->sht25_state_ = SHT25State::READ_TEMP;
      this->update();
    });
  }
}

bool SHT25Component::read_serial_number_(std::string& serial_number) {
  uint8_t ident_code1[8];
  uint8_t ident_code2[6];
  char tmp_buf[20];

  bool success = this->write_byte(SHT25_COMMAND_READ_IDENT_1, SHT25_MEMORY_ADDR_IDENT_1);
  success = success && this->read_bytes_raw(ident_code1, sizeof(ident_code1));
  success = success && this->write_byte(SHT25_COMMAND_READ_IDENT_2, SHT25_MEMORY_ADDR_IDENT_2);
  success = success && this->read_bytes_raw(ident_code2, sizeof(ident_code2));

  if(success) {
    serial_number.append(itoa(ident_code2[4], tmp_buf, 16)); // add SNA_0
    serial_number.append(itoa(ident_code2[3], tmp_buf, 16)); // add SNA_1
    serial_number.append(itoa(ident_code1[6], tmp_buf, 16)); // add SNB_0
    serial_number.append(itoa(ident_code1[4], tmp_buf, 16)); // add SNB_1
    serial_number.append(itoa(ident_code1[2], tmp_buf, 16)); // add SNB_2
    serial_number.append(itoa(ident_code1[0], tmp_buf, 16)); // add SNB_3
    serial_number.append(itoa(ident_code2[1], tmp_buf, 16)); // add SNC_0
    serial_number.append(itoa(ident_code2[0], tmp_buf, 16)); // add SNC_1
  }
  return success;
}

bool SHT25Component::check_crc_(const uint8_t* data, uint8_t len, uint8_t checksum) const {
  constexpr uint16_t POLYNOMIAL = 0x131;  //P(x)=x^8+x^5+x^4+1 = 100110001
  uint8_t crc = 0;

  //calculates 8-Bit checksum with given polynomial
  for (uint8_t i = 0; i < len; ++i) {
    crc ^= (data[i]);
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (crc & 0x80)
        crc = (crc << 1) ^ POLYNOMIAL;
      else
        crc = (crc << 1);
    }
  }
  return (crc == checksum);
}

float SHT25Component::calc_temperature_(const uint8_t* raw_temp) const {
  uint16_t temp = (raw_temp[0] << 8) + raw_temp[1];

  temp &= ~0x0003; // clear bits [1..0] (status bits)
  return -46.85f + 175.72f / 65536.0f * (float)temp; //T= -46.85 + 175.72 * ST/2^16
}

float SHT25Component::calc_humidity_(const uint8_t* raw_humid) const {
  uint16_t humid = (raw_humid[0] << 8) + raw_humid[1];

  humid &= ~0x0003; // clear bits [1..0] (status bits)
  return -6.0 + 125.0/65536 * (float)humid; // RH= -6 + 125 * SRH/2^16
}


}  // namespace sht25
}  // namespace esphome
