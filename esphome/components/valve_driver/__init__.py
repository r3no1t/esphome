import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components.output import FloatOutput
from esphome.const import CONF_ID, CONF_CS1_PIN, CONF_CS2_PIN, CONF_CLK_PIN, CONF_DATA_PIN

valve_driver_ns = cg.esphome_ns.namespace('valve_driver')
ValveDriver = valve_driver_ns.class_('ValveDriver', cg.PollingComponent)

MULTI_CONF = True
CONFIG_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(ValveDriver),
    cv.Required(CONF_CS1_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_CS2_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_CLK_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_DATA_PIN): pins.gpio_output_pin_schema,
}).extend(cv.polling_component_schema('1s'))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    cs1_pin = yield cg.gpio_pin_expression(config[CONF_CS1_PIN])
    cg.add(var.set_pin_cs1(cs1_pin))
    cs2_pin = yield cg.gpio_pin_expression(config[CONF_CS2_PIN])
    cg.add(var.set_pin_cs2(cs2_pin))
    clk_pin = yield cg.gpio_pin_expression(config[CONF_CLK_PIN])
    cg.add(var.set_pin_clk(clk_pin))
    data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_PIN])
    cg.add(var.set_pin_data(data_pin))

