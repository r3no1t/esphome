import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome.const import CONF_ID, CONF_ROTATION, CONF_PIXEL_X, CONF_PIXEL_Y
from esphome.core import coroutine, coroutine_with_priority

IS_PLATFORM_COMPONENT = True

touchscreen_ns = cg.esphome_ns.namespace('touchscreen')
Touchscreen = touchscreen_ns.class_('Touchscreen')

TOUCHSCREEN_ROTATIONS = {
    0: touchscreen_ns.TOUCHSCREEN_ROTATION_0_DEGREES,
    90: touchscreen_ns.TOUCHSCREEN_ROTATION_90_DEGREES,
    180: touchscreen_ns.TOUCHSCREEN_ROTATION_180_DEGREES,
    270: touchscreen_ns.TOUCHSCREEN_ROTATION_270_DEGREES,
}


def validate_rotation(value):
    value = cv.string(value)
    if value.endswith("°"):
        value = value[:-1]
    return cv.enum(TOUCHSCREEN_ROTATIONS, int=True)(value)


TOUCHSCREEN_SCHEMA = cv.Schema({
    cv.Required(CONF_ROTATION): validate_rotation,
    cv.Required(CONF_PIXEL_X): cv.int_range(min=1, max=10000),
    cv.Required(CONF_PIXEL_Y): cv.int_range(min=1, max=10000),
})


@coroutine
def setup_touchscreen_core_(var, config):
    cg.add(var.set_rotation(TOUCHSCREEN_ROTATIONS[config[CONF_ROTATION]]))
    cg.add(var.set_pixel_x(config[CONF_PIXEL_X]))
    cg.add(var.set_pixel_y(config[CONF_PIXEL_Y]))

@coroutine
def register_touchscreen(var, config):
    yield setup_touchscreen_core_(var, config)

@coroutine_with_priority(100.0)
def to_code(config):
    cg.add_global(touchscreen_ns.using)
