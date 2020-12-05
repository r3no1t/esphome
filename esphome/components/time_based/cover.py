import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import binary_sensor, cover
from esphome.const import CONF_CLOSE_ACTION, CONF_CLOSE_DURATION, CONF_ID, CONF_OPEN_ACTION, \
    CONF_OPEN_DURATION, CONF_STOP_ACTION, CONF_ASSUMED_STATE, CONF_DIR_CHANGE_DELAY, \
    CONF_TILT_DURATION, CONF_PUBLISH_INTERVAL, CONF_MOTOR_STATUS

time_based_ns = cg.esphome_ns.namespace('time_based')
TimeBasedCover = time_based_ns.class_('TimeBasedCover', cover.Cover, cg.Component)

CONF_HAS_BUILT_IN_ENDSTOP = 'has_built_in_endstop'

CONFIG_SCHEMA = cover.COVER_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(TimeBasedCover),
    cv.Required(CONF_STOP_ACTION): automation.validate_automation(single=True),

    cv.Required(CONF_OPEN_ACTION): automation.validate_automation(single=True),
    cv.Required(CONF_OPEN_DURATION): cv.positive_time_period_milliseconds,

    cv.Required(CONF_CLOSE_ACTION): automation.validate_automation(single=True),
    cv.Required(CONF_CLOSE_DURATION): cv.positive_time_period_milliseconds,

    cv.Optional(CONF_MOTOR_STATUS): cv.use_id(binary_sensor.BinarySensor),

    cv.Optional(CONF_TILT_DURATION, default='0ms'): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_PUBLISH_INTERVAL, default='1000ms'): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_DIR_CHANGE_DELAY, default='0ms'): cv.positive_time_period_milliseconds,

    cv.Optional(CONF_HAS_BUILT_IN_ENDSTOP, default=False): cv.boolean,
    cv.Optional(CONF_ASSUMED_STATE, default=True): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield cover.register_cover(var, config)

    yield automation.build_automation(var.get_stop_trigger(), [], config[CONF_STOP_ACTION])

    cg.add(var.set_open_duration(config[CONF_OPEN_DURATION]))
    yield automation.build_automation(var.get_open_trigger(), [], config[CONF_OPEN_ACTION])

    cg.add(var.set_close_duration(config[CONF_CLOSE_DURATION]))
    yield automation.build_automation(var.get_close_trigger(), [], config[CONF_CLOSE_ACTION])

    if CONF_MOTOR_STATUS in config:
        motor_sensor = yield cg.get_variable(config[CONF_MOTOR_STATUS])
        cg.add(var.set_motor_sensor(motor_sensor))

    cg.add(var.set_tilt_duration(config[CONF_TILT_DURATION]))
    cg.add(var.set_publish_interval(config[CONF_PUBLISH_INTERVAL]))
    cg.add(var.set_dir_change_delay(config[CONF_DIR_CHANGE_DELAY]))

    cg.add(var.set_has_built_in_endstop(config[CONF_HAS_BUILT_IN_ENDSTOP]))
    cg.add(var.set_assumed_state(config[CONF_ASSUMED_STATE]))
