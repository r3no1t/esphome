import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display, spi
from esphome.const import CONF_DC_PIN, \
    CONF_ID, CONF_LAMBDA, CONF_MODEL, CONF_PAGES, CONF_RESET_PIN, \
    CONF_RD_PIN, CONF_WR_PIN, CONF_CS_PIN, CONF_DATA_0_PIN, \
    CONF_DATA_1_PIN, CONF_DATA_2_PIN, CONF_DATA_3_PIN, CONF_DATA_4_PIN, \
    CONF_DATA_5_PIN, CONF_DATA_6_PIN, CONF_DATA_7_PIN, CONF_DATA_8_PIN, \
    CONF_DATA_9_PIN, CONF_DATA_10_PIN, CONF_DATA_11_PIN, CONF_DATA_12_PIN, \
    CONF_DATA_13_PIN, CONF_DATA_14_PIN, CONF_DATA_15_PIN

DEPENDENCIES = ['spi']

CONF_LED_PIN = 'led_pin'

ili9341_ns = cg.esphome_ns.namespace('ili9341')
ili9341 = ili9341_ns.class_('ILI9341Display', cg.PollingComponent, spi.SPIDevice,
                            display.DisplayBuffer)
ILI9341M5Stack = ili9341_ns.class_('ILI9341_M5Stack', ili9341)
ILI9341_24_TFT = ili9341_ns.class_('ILI9341_24_TFT', ili9341)
ILI9342_24_TFT_LANDSCAPE = ili9341_ns.class_('ILI9342_24_TFT_LANDSCAPE', ili9341)

ILI9341Model = ili9341_ns.enum('ILI9341Model')

MODELS = {
    'M5STACK': ILI9341Model.M5STACK,
    'TFT_2.4': ILI9341Model.TFT_24,
    'TFT_2.4_LANDSCAPE': ILI9341Model.TFT_24_LANDSCAPE,
}

ILI9341_MODEL = cv.enum(MODELS, upper=True, space="_")

CONFIG_SCHEMA = cv.All(display.FULL_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(ili9341),
    cv.Required(CONF_MODEL): ILI9341_MODEL,
    cv.Required(CONF_DC_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_LED_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_CS_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_RD_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_WR_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_0_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_1_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_2_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_3_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_4_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_5_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_6_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_7_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_8_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_9_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_10_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_11_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_12_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_13_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_14_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DATA_15_PIN): pins.gpio_output_pin_schema,
}).extend(cv.polling_component_schema('1s')).extend(spi.SPI_DEVICE_SCHEMA),
                       cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA))


def to_code(config):
    if config[CONF_MODEL] == 'M5STACK':
        lcd_type = ILI9341M5Stack
    if config[CONF_MODEL] == 'TFT_2.4':
        lcd_type = ILI9341_24_TFT
    if config[CONF_MODEL] == 'TFT_2.4_LANDSCAPE':
        lcd_type = ILI9342_24_TFT_LANDSCAPE
    rhs = lcd_type.new()
    var = cg.Pvariable(config[CONF_ID], rhs, type=lcd_type)

    yield cg.register_component(var, config)
    yield display.register_display(var, config)
    yield spi.register_spi_device(var, config)
    cg.add(var.set_model(config[CONF_MODEL]))
    dc = yield cg.gpio_pin_expression(config[CONF_DC_PIN])
    cg.add(var.set_dc_pin(dc))

    if CONF_LAMBDA in config:
        lambda_ = yield cg.process_lambda(config[CONF_LAMBDA], [(display.DisplayBufferRef, 'it')],
                                          return_type=cg.void)
        cg.add(var.set_writer(lambda_))
    if CONF_RESET_PIN in config:
        reset = yield cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset))
    if CONF_LED_PIN in config:
        led_pin = yield cg.gpio_pin_expression(config[CONF_LED_PIN])
        cg.add(var.set_led_pin(led_pin))
    if CONF_CS_PIN in config:
        cs_pin = yield cg.gpio_pin_expression(config[CONF_CS_PIN])
        cg.add(var.set_cs_pin(cs_pin))
    if CONF_RD_PIN in config:
        rd_pin = yield cg.gpio_pin_expression(config[CONF_RD_PIN])
        cg.add(var.set_rd_pin(rd_pin))
    if CONF_WR_PIN in config:
        wr_pin = yield cg.gpio_pin_expression(config[CONF_WR_PIN])
        cg.add(var.set_wr_pin(wr_pin))
    if CONF_DATA_0_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_0_PIN])
        cg.add(var.set_data0_pin(data_pin))
    if CONF_DATA_1_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_1_PIN])
        cg.add(var.set_data1_pin(data_pin))
    if CONF_DATA_2_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_2_PIN])
        cg.add(var.set_data2_pin(data_pin))
    if CONF_DATA_3_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_3_PIN])
        cg.add(var.set_data3_pin(data_pin))
    if CONF_DATA_4_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_4_PIN])
        cg.add(var.set_data4_pin(data_pin))
    if CONF_DATA_5_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_5_PIN])
        cg.add(var.set_data5_pin(data_pin))
    if CONF_DATA_6_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_6_PIN])
        cg.add(var.set_data6_pin(data_pin))
    if CONF_DATA_7_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_7_PIN])
        cg.add(var.set_data7_pin(data_pin))
    if CONF_DATA_8_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_8_PIN])
        cg.add(var.set_data8_pin(data_pin))
    if CONF_DATA_9_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_9_PIN])
        cg.add(var.set_data9_pin(data_pin))
    if CONF_DATA_10_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_10_PIN])
        cg.add(var.set_data10_pin(data_pin))
    if CONF_DATA_11_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_11_PIN])
        cg.add(var.set_data11_pin(data_pin))
    if CONF_DATA_12_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_12_PIN])
        cg.add(var.set_data12_pin(data_pin))
    if CONF_DATA_13_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_13_PIN])
        cg.add(var.set_data13_pin(data_pin))
    if CONF_DATA_14_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_14_PIN])
        cg.add(var.set_data14_pin(data_pin))
    if CONF_DATA_15_PIN in config:
        data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_15_PIN])
        cg.add(var.set_data15_pin(data_pin))
        