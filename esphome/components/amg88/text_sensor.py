import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, text_sensor
from esphome.const import CONF_ID, CONF_FRAME_RATE, CONF_MIN_CHANGE_PIXEL, \
      CONF_MIN_CHANGE_DEGREES, CONF_OFFSET_DEGREES, CONF_SEND_INITIAL_STATE

DEPENDENCIES = ['i2c']

amg88_ns = cg.esphome_ns.namespace('amg88')
AMG88Component = amg88_ns.class_('AMG88Component', text_sensor.TextSensor,
                                 cg.PollingComponent, i2c.I2CDevice)

AMG88FrameRate = amg88_ns.enum('AMG88FrameRate')
FRAME_RATE = {
    '10FPS': AMG88FrameRate.AMG8833_FRAME_RATE_10_FPS,
    '1FPS': AMG88FrameRate.AMG8833_FRAME_RATE_1_FPS,
}

CONFIG_SCHEMA = text_sensor.TEXT_SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(AMG88Component),
    cv.Optional(CONF_FRAME_RATE, default='10FPS'): cv.enum(FRAME_RATE, upper=True),
    cv.Optional(CONF_MIN_CHANGE_PIXEL, default=1): cv.int_range(min=1, max=64),
    cv.Optional(CONF_MIN_CHANGE_DEGREES, default=0.2): cv.float_range(min=0.2, max=100.0),
    cv.Optional(CONF_OFFSET_DEGREES, default=0): cv.int_range(min=-100, max=100),
    cv.Optional(CONF_SEND_INITIAL_STATE, default=True): cv.boolean,
}).extend(cv.polling_component_schema('1s')).extend(i2c.i2c_device_schema(0x69))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield i2c.register_i2c_device(var, config)
    yield text_sensor.register_text_sensor(var, config)
    cg.add(var.set_frame_rate(config[CONF_FRAME_RATE]))
    cg.add(var.set_min_change_pixel(config[CONF_MIN_CHANGE_PIXEL]))
    cg.add(var.set_min_change_degrees(config[CONF_MIN_CHANGE_DEGREES]))
    cg.add(var.set_offset_degrees(config[CONF_OFFSET_DEGREES]))
    cg.add(var.set_send_initial_state_(config[CONF_SEND_INITIAL_STATE]))
