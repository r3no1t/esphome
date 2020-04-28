import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome import pins
from esphome.const import (CONF_PHASE_IN_PIN, CONF_PHASE_CUT_PIN, CONF_PHASE_DELAY, CONF_HW_TIMER_ID, CONF_ID)


dimmer_output_ns = cg.esphome_ns.namespace('dimmer_output')
DimmerOutputComponent = dimmer_output_ns.class_('DimmerOutputComponent', output.FloatOutput, cg.Component, cg.Nameable)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(DimmerOutputComponent),
    cv.Required(CONF_PHASE_IN_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_PHASE_CUT_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_PHASE_DELAY): cv.positive_time_period_microseconds,
    cv.Required(CONF_HW_TIMER_ID): cv.int_range(min=0, max=3),
}).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield output.register_output(var, config)

    phase_in = yield cg.gpio_pin_expression(config[CONF_PHASE_IN_PIN])
    cg.add(var.set_pin_phase_in(phase_in))
    phase_cut = yield cg.gpio_pin_expression(config[CONF_PHASE_CUT_PIN])
    cg.add(var.set_pin_phase_cut(phase_cut))
    cg.add(var.set_phase_delay(config[CONF_PHASE_DELAY]))
    cg.add(var.set_hw_timer_id(config[CONF_HW_TIMER_ID]))

