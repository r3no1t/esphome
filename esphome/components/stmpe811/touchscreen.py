import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import touchscreen, spi
from esphome.const import CONF_ID, CONF_BACKLIGHT_TIMEOUT, CONF_LIMIT_X_MIN, \
    CONF_LIMIT_X_MAX, CONF_LIMIT_Y_MIN, CONF_LIMIT_Y_MAX

DEPENDENCIES = ['spi']

stmpe811_ns = cg.esphome_ns.namespace('stmpe811')
STMPE811Component = stmpe811_ns.class_('STMPE811Component', cg.Component, touchscreen.Touchscreen, spi.SPIDevice)


CONFIG_SCHEMA = cv.All(touchscreen.TOUCHSCREEN_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(STMPE811Component),
    cv.Optional(CONF_BACKLIGHT_TIMEOUT, default='0min'): cv.positive_time_period_seconds,
    cv.Optional(CONF_LIMIT_X_MIN, default=460): cv.int_range(min=1, max=10000),
    cv.Optional(CONF_LIMIT_X_MAX, default=3830): cv.int_range(min=1, max=10000),
    cv.Optional(CONF_LIMIT_Y_MIN, default=250): cv.int_range(min=1, max=10000),
    cv.Optional(CONF_LIMIT_Y_MAX, default=3810): cv.int_range(min=1, max=10000),
}).extend(cv.COMPONENT_SCHEMA).extend(spi.SPI_DEVICE_SCHEMA))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield touchscreen.register_touchscreen(var, config)
    yield spi.register_spi_device(var, config)
    cg.add(var.set_backlight_timeout(config[CONF_BACKLIGHT_TIMEOUT]))
    cg.add(var.set_limit_x_min(config[CONF_LIMIT_X_MIN]))
    cg.add(var.set_limit_x_max(config[CONF_LIMIT_X_MAX]))
    cg.add(var.set_limit_y_min(config[CONF_LIMIT_Y_MIN]))
    cg.add(var.set_limit_y_max(config[CONF_LIMIT_Y_MAX]))

