#include "ili9341_display.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace ili9341 {

static const char *TAG = "ili9341";

void ILI9341Display::setup_pins_() {
  this->init_internal_(this->get_buffer_length_());
  this->dc_pin_->setup();  // OUTPUT
  this->dc_pin_->digital_write(false);
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();  // OUTPUT
    this->reset_pin_->digital_write(true);
  }
  if (this->led_pin_ != nullptr) {
    this->led_pin_->setup();
    this->led_pin_->digital_write(true);
  }
  if (this->cs_pin_ != nullptr) {
    this->cs_pin_->setup();
    this->cs_pin_->digital_write(true);
  }
  if (this->rd_pin_ != nullptr) {
    this->rd_pin_->setup();
    this->rd_pin_->digital_write(true);
  }
  if (this->wr_pin_ != nullptr) {
    this->wr_pin_->setup();
    this->wr_pin_->digital_write(true);
  }
  if (this->data0_pin_ != nullptr) {
    this->data0_pin_->setup();
    this->data0_pin_->digital_write(false);
  }
  if (this->data1_pin_ != nullptr) {
    this->data1_pin_->setup();
    this->data1_pin_->digital_write(false);
  }
  if (this->data2_pin_ != nullptr) {
    this->data2_pin_->setup();
    this->data2_pin_->digital_write(false);
  }
  if (this->data3_pin_ != nullptr) {
    this->data3_pin_->setup();
    this->data3_pin_->digital_write(false);
  }
  if (this->data4_pin_ != nullptr) {
    this->data4_pin_->setup();
    this->data4_pin_->digital_write(false);
  }
  if (this->data5_pin_ != nullptr) {
    this->data5_pin_->setup();
    this->data5_pin_->digital_write(false);
  }
  if (this->data6_pin_ != nullptr) {
    this->data6_pin_->setup();
    this->data6_pin_->digital_write(false);
  }
  if (this->data7_pin_ != nullptr) {
    this->data7_pin_->setup();
    this->data7_pin_->digital_write(false);
  }
  if (this->data8_pin_ != nullptr) {
    this->data8_pin_->setup();
    this->data8_pin_->digital_write(false);
  }
  if (this->data9_pin_ != nullptr) {
    this->data9_pin_->setup();
    this->data9_pin_->digital_write(false);
  }
  if (this->data10_pin_ != nullptr) {
    this->data10_pin_->setup();
    this->data10_pin_->digital_write(false);
  }
  if (this->data11_pin_ != nullptr) {
    this->data11_pin_->setup();
    this->data11_pin_->digital_write(false);
  }
  if (this->data12_pin_ != nullptr) {
    this->data12_pin_->setup();
    this->data12_pin_->digital_write(false);
  }
  if (this->data13_pin_ != nullptr) {
    this->data13_pin_->setup();
    this->data13_pin_->digital_write(false);
  }
  if (this->data14_pin_ != nullptr) {
    this->data14_pin_->setup();
    this->data14_pin_->digital_write(false);
  }
  if (this->data15_pin_ != nullptr) {
    this->data15_pin_->setup();
    this->data15_pin_->digital_write(false);
  }

  // TODO support serial and parallel
//  this->spi_setup();

  this->reset_();
}

void ILI9341Display::dump_config() {
  LOG_DISPLAY("", "ili9341", this);
  ESP_LOGCONFIG(TAG, "  Width: %d, Height: %d,  Rotation: %d", this->width_, this->height_, this->rotation_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Busy Pin: ", this->busy_pin_);
  LOG_PIN("  Backlight Pin: ", this->led_pin_);
  LOG_UPDATE_INTERVAL(this);
}

float ILI9341Display::get_setup_priority() const { return setup_priority::PROCESSOR; }
void ILI9341Display::command(uint8_t value) {
  this->start_command_();
  this->write_byte(value);
  this->end_command_();
}

void ILI9341Display::reset_() {
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->digital_write(false);
    delay(10);
    this->reset_pin_->digital_write(true);
    delay(10);
  }
}

void ILI9341Display::data(uint8_t value) {
  this->start_data_();
  this->write_byte(value);
  this->end_data_();
}

void ILI9341Display::send_command(uint8_t command_byte, const uint8_t *data_bytes, uint8_t num_data_bytes) {
  this->command(command_byte);  // Send the command byte
  this->start_data_();
  for (int i = 0; i < num_data_bytes; i++) {
    this->write_byte(*data_bytes);  // Send the data bytes
    data_bytes++;
  }
  this->end_data_();
}

uint8_t ILI9341Display::read_command(uint8_t command_byte, uint8_t index) {
  uint8_t data = 0x10 + index;
  this->send_command(0xD9, &data, 1);  // Set Index Register
  uint8_t result;
  this->start_command_();
  this->write_byte(command_byte);
  this->start_data_();
  do {
    result = this->read_byte();
  } while (index--);
  this->end_data_();
  return result;
}

void ILI9341Display::loop() {
  if (this->repaint_) {
    this->repaint_ = false;
    this->display_();
  }
}

void ILI9341Display::update() {
  this->do_update_();
  this->display_();
}

void ILI9341Display::display_() {
  //we will only update the changed window to the display
  int w = this->x_high_ - this->x_low_ + 1;
  int h = this->y_high_ - this->y_low_ + 1;
  int x_h = this->x_high_;
  int x_l = this->x_low_;
  int y_h = this->y_high_;
  int y_l = this->y_low_;

  set_addr_window_(this->x_low_,this->y_low_,w, h);
  this->start_data_();
  uint32_t start_pos = ((this->y_low_ * this->width_) + x_low_);
  for (uint16_t row = 0; row < h; row++) {
    for(uint16_t col = 0; col < w; col++) {
      uint32_t pos = start_pos + (row * width_) + col;

      uint16_t color = convert_to_16bit_color_(buffer_[pos]);
      this->write_byte(color >> 8);
      this->write_byte(color);  
    }
  }
  this->end_data_();

  if (x_h == this->x_high_ && x_l == this->x_low_ &&  y_h == this->y_high_ && y_l == this->y_low_) {
    //invalidate watermarks
    this->x_low_ = this->width_;
    this->y_low_ = this->height_;
    this->x_high_ = 0;
    this->y_high_ = 0;
  }
}

uint16_t ILI9341Display::convert_to_16bit_color_(uint8_t color_8bit) {
    int r = color_8bit >> 5;
    int g = (color_8bit >> 2 )& 0x07;
    int b = color_8bit & 0x03;
    uint16_t color = (r * 0x04) << 11;
    color |= (g * 0x09) << 5;
    color |= (b * 0x0A);
    
    return color;
}

uint8_t ILI9341Display::convert_to_8bit_color_(uint16_t color_16bit) {
  //convert 16bit color to 8 bit buffer
  uint8_t r = color_16bit >> 11;
  uint8_t g = (color_16bit >> 5 ) &  0x3F;
  uint8_t b = color_16bit & 0x1F;

  return((b / 0x0A) | ((g / 0x09) << 2) | ((r / 0x04) << 5));
}

/**
 * do nothing.
 * we need this function het to override the default behaviour.
 * Otherwise the buffer is cleared at every update
 * */
void ILI9341Display::fill(int color) {
  //do nothing.
}

void ILI9341Display::fill_internal_(int color) {
  this->set_addr_window_(0, 0, this->get_width_internal(), this->get_height_internal());
  this->start_data_();
  for (uint32_t i = 0; i < (this->get_width_internal()) * (this->get_height_internal()); i++) {
    this->write_byte(color >> 8);
    this->write_byte(color);
  }
  this->end_data_();
  for (uint32_t i = 0; i < (this->get_width_internal() * this->get_height_internal()); ++i) buffer_[i] = 0;
}

void HOT ILI9341Display::draw_absolute_pixel_internal(int x, int y, int color) {
  if (x > this->get_width_internal() || x < 0 || y > this->get_height_internal() || y < 0)
    return;

  // low and high watermark may speed up drawing from buffer
  this->x_low_ = (x < this->x_low_) ? x : this->x_low_;
  this->y_low_ = (y < this->y_low_) ? y : this->y_low_;  
  this->x_high_ = (x > this->x_high_) ? x : this->x_high_;
  this->y_high_ = (y > this->y_high_) ? y : this->y_high_;

  uint32_t pos = (y*width_) + x;
  buffer_[pos] = convert_to_8bit_color_(color);
}

// should return the total size: return this->get_width_internal() * this->get_height_internal() * 2 // 16bit color
// values per bit is huge
uint32_t ILI9341Display::get_buffer_length_() { return this->get_width_internal() * this->get_height_internal(); }

void ILI9341Display::start_command_() {
  this->dc_pin_->digital_write(false);
  this->enable();
}

void ILI9341Display::end_command_() { this->disable(); }
void ILI9341Display::start_data_() {
  this->dc_pin_->digital_write(true);
  this->enable();
}
void ILI9341Display::end_data_() { this->disable(); }

void ILI9341Display::init_lcd_(const uint8_t *init_cmd) {
  uint8_t cmd, x, num_args;
  const uint8_t *addr = init_cmd;
  while ((cmd = pgm_read_byte(addr++)) > 0) {
    x = pgm_read_byte(addr++);
    num_args = x & 0x7F;
    send_command(cmd, addr, num_args);
    addr += num_args;
    if (x & 0x80)
      delay(150); // NOLINT
  }
}

void ILI9341Display::set_addr_window_(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h) {
  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
  this->command(ILI9341_CASET);  // Column address set
  this->start_data_();
  this->write_byte(x1 >> 8);
  this->write_byte(x1);
  this->write_byte(x2 >> 8);
  this->write_byte(x2);
  this->end_data_();
  this->command(ILI9341_PASET);  // Row address set
  this->start_data_();
  this->write_byte(y1 >> 8);
  this->write_byte(y1);
  this->write_byte(y2 >> 8);
  this->write_byte(y2);
  this->end_data_();
  this->command(ILI9341_RAMWR);  // Write to RAM
}

void ILI9341Display::invert_display_(bool invert) { this->command(invert ? ILI9341_INVON : ILI9341_INVOFF); }

int ILI9341Display::get_width_internal() { return this->width_; }
int ILI9341Display::get_height_internal() { return this->height_; }

//   M5Stack display
void ILI9341_M5Stack::initialize() {
  this->init_lcd_(INITCMD_M5STACK);
  this->width_ = 320;
  this->height_ = 240;
  this->fill_internal_(BLACK);
}

//   24_TFT display
void ILI9341_24_TFT::initialize() {
  this->init_lcd_(INITCMD_TFT);
  this->width_ = 240;
  this->height_ = 320;
  this->fill_internal_(BLACK);
}

//   24_TFT display landscape
void ILI9342_24_TFT_LANDSCAPE::initialize() {
  this->init_lcd_(initcmd_tft);
  this->width_ = 320;
  this->height_ = 240;
  this->fill_internal_(0x001F);
}

void ILI9341Display::enable() {
  this->cs_pin_->digital_write(false);
}
void ILI9341Display::disable() {
  this->cs_pin_->digital_write(true);
}

}  // namespace ili9341
}  // namespace esphome
