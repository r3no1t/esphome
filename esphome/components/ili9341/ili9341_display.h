#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/display/display_buffer.h"
#include "ili9341_defines.h"
#include "ili9341_init.h"

namespace esphome {
namespace ili9341 {

enum ILI9341Model {
  M5STACK = 0,
  TFT_24,
  TFT_24_LANDSCAPE
};

class ILI9341Display : public PollingComponent,
                public display::DisplayBuffer,
                public spi::SPIDevice<spi::SPIComponent, spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW, spi::CLOCK_PHASE_LEADING,
                                      spi::DATA_RATE_16MHZ> {
 public:
  void set_dc_pin(GPIOPin *pin) { dc_pin_ = pin; }
  void set_cs_pin(GPIOPin *pin) { cs_pin_ = pin; }
  void set_rd_pin(GPIOPin *pin) { rd_pin_ = pin; }
  void set_wr_pin(GPIOPin *pin) { wr_pin_ = pin; }
  void set_data0_pin(GPIOPin *pin) { data0_pin_ = pin; }
  void set_data1_pin(GPIOPin *pin) { data1_pin_ = pin; }
  void set_data2_pin(GPIOPin *pin) { data2_pin_ = pin; }
  void set_data3_pin(GPIOPin *pin) { data3_pin_ = pin; }
  void set_data4_pin(GPIOPin *pin) { data4_pin_ = pin; }
  void set_data5_pin(GPIOPin *pin) { data5_pin_ = pin; }
  void set_data6_pin(GPIOPin *pin) { data6_pin_ = pin; }
  void set_data7_pin(GPIOPin *pin) { data7_pin_ = pin; }
  void set_data8_pin(GPIOPin *pin) { data8_pin_ = pin; }
  void set_data9_pin(GPIOPin *pin) { data9_pin_ = pin; }
  void set_data10_pin(GPIOPin *pin) { data10_pin_ = pin; }
  void set_data11_pin(GPIOPin *pin) { data11_pin_ = pin; }
  void set_data12_pin(GPIOPin *pin) { data12_pin_ = pin; }
  void set_data13_pin(GPIOPin *pin) { data13_pin_ = pin; }
  void set_data14_pin(GPIOPin *pin) { data14_pin_ = pin; }
  void set_data15_pin(GPIOPin *pin) { data15_pin_ = pin; }
  float get_setup_priority() const override;
  void set_reset_pin(GPIOPin *reset) { this->reset_pin_ = reset; }
  void set_led_pin(GPIOPin *led) { this->led_pin_ = led; }
  void set_model(ILI9341Model model) { this->model_ = model; }

  void command(uint8_t value);
  void data(uint8_t value);
  void send_command(uint8_t command_byte, const uint8_t *data_bytes, uint8_t num_data_bytes);
  uint8_t read_command(uint8_t command_byte, uint8_t index);
  virtual void initialize() = 0;

  void loop() override;
  void update() override;

  void fill(int color) override;

  void dump_config() override;
  void setup() override {
    this->setup_pins_();
    this->initialize();
  }

 protected:
  void draw_absolute_pixel_internal(int x, int y, int color) override;
  void setup_pins_();

  void enable();
  void disable();
  inline void write_byte(uint8_t value) {
    GPIO.out_w1tc = 4;
    if (value & 0x01) GPIO.out_w1ts = 0x00200000; else GPIO.out_w1tc = 0x00200000;
    if (value & 0x02) GPIO.out_w1ts = 0x00080000; else GPIO.out_w1tc = 0x00080000;
    if (value & 0x04) GPIO.out1_w1ts.val = 0x00000001; else GPIO.out1_w1tc.val = 0x00000001;
    if (value & 0x08) GPIO.out1_w1ts.val = 0x00000002; else GPIO.out1_w1tc.val = 0x00000002;
    if (value & 0x10) GPIO.out_w1ts = 0x02000000; else GPIO.out_w1tc = 0x02000000;
    if (value & 0x20) GPIO.out_w1ts = 0x04000000; else GPIO.out_w1tc = 0x04000000;
    if (value & 0x40) GPIO.out_w1ts = 0x08000000; else GPIO.out_w1tc = 0x08000000;
    if (value & 0x80) GPIO.out_w1ts = 0x00002000; else GPIO.out_w1tc = 0x00002000;
    GPIO.out_w1ts = 4;
  }

  void init_lcd_(const uint8_t *init_cmd);
  void set_addr_window_(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  void invert_display_(bool invert);
  void reset_();
  void fill_internal_(int color);
  void display_();
  uint16_t convert_to_16bit_color_(uint8_t color_8bit);
  uint8_t convert_to_8bit_color_(uint16_t color_16bit);

  ILI9341Model model_;
  int16_t width_{320};   ///< Display width as modified by current rotation
  int16_t height_{240};  ///< Display height as modified by current rotation
  uint16_t x_low_{0};
  uint16_t y_low_{0};
  uint16_t x_high_{0};
  uint16_t y_high_{0};

  uint32_t get_buffer_length_();
  int get_width_internal() override;
  int get_height_internal() override;

  void start_command_();
  void end_command_();
  void start_data_();
  void end_data_();

  GPIOPin *reset_pin_{nullptr};
  GPIOPin *led_pin_{nullptr};
  GPIOPin *dc_pin_;
  GPIOPin *cs_pin_;
  GPIOPin *rd_pin_;
  GPIOPin *wr_pin_;
  GPIOPin *data0_pin_;
  GPIOPin *data1_pin_;
  GPIOPin *data2_pin_;
  GPIOPin *data3_pin_;
  GPIOPin *data4_pin_;
  GPIOPin *data5_pin_;
  GPIOPin *data6_pin_;
  GPIOPin *data7_pin_;
  GPIOPin *data8_pin_;
  GPIOPin *data9_pin_;
  GPIOPin *data10_pin_;
  GPIOPin *data11_pin_;
  GPIOPin *data12_pin_;
  GPIOPin *data13_pin_;
  GPIOPin *data14_pin_;
  GPIOPin *data15_pin_;
  GPIOPin *busy_pin_{nullptr};
};

//-----------   M5Stack display --------------
class ILI9341_M5Stack : public ILI9341Display {
 public:
  void initialize() override;
};

//-----------   ILI9341_24_TFT display --------------
class ILI9341_24_TFT : public ILI9341Display {
 public:
  void initialize() override;
};

//-----------   ILI9341_24_TFT display --------------
class ILI9342_24_TFT_LANDSCAPE : public ILI9341Display {
 public:
  void initialize() override;
};
}  // namespace ili9341
}  // namespace esphome
