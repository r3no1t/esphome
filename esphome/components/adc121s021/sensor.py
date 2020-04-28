import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core, pins
from esphome.components import sensor
from esphome.const import CONF_CLK_PIN, CONF_DATA_PIN, CONF_CS_PIN, CONF_ID, CONF_HW_TIMER_ID, \
    CONF_SAMPLE_TIME, CONF_SAMPLE_DURATION, CONF_SAMPLE_PAUSE, CONF_FILTER_FREQUENCY, \
    UNIT_WATT, ICON_POWER

adc_ns = cg.esphome_ns.namespace('adc121s021')
ADCSensor = adc_ns.class_('ADCSensor', sensor.Sensor, cg.PollingComponent)


CONFIG_SCHEMA = sensor.sensor_schema(UNIT_WATT, ICON_POWER, 0).extend({
    cv.GenerateID(): cv.declare_id(ADCSensor),
    cv.Required(CONF_DATA_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_CLK_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_SAMPLE_TIME): cv.All(
        cv.positive_time_period_microseconds, cv.Range(
            min=core.TimePeriod(microseconds=10), max=core.TimePeriod(microseconds=1000000))),
    cv.Required(CONF_SAMPLE_DURATION): cv.All(
        cv.positive_time_period_milliseconds, cv.Range(
            min=core.TimePeriod(milliseconds=1), max=core.TimePeriod(milliseconds=60000))),
    cv.Required(CONF_SAMPLE_PAUSE): cv.All(
        cv.positive_time_period_milliseconds, cv.Range(
            min=core.TimePeriod(milliseconds=1), max=core.TimePeriod(milliseconds=60000))),
    cv.Required(CONF_FILTER_FREQUENCY): cv.All(cv.frequency, cv.Range(min=50, max=100000)),
    cv.Required(CONF_HW_TIMER_ID): cv.int_range(min=0, max=3),
}).extend(cv.polling_component_schema('60s'))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
    data_pin = yield cg.gpio_pin_expression(config[CONF_DATA_PIN])
    cg.add(var.set_data_pin(data_pin))
    clk_pin = yield cg.gpio_pin_expression(config[CONF_CLK_PIN])
    cg.add(var.set_clk_pin(clk_pin))
    cs_pin = yield cg.gpio_pin_expression(config[CONF_CS_PIN])
    cg.add(var.set_cs_pin(cs_pin))
    cg.add(var.set_hw_timer_id(config[CONF_HW_TIMER_ID]))
    cg.add(var.set_sample_time(config[CONF_SAMPLE_TIME]))
    cg.add(var.set_sample_duration(config[CONF_SAMPLE_DURATION]))
    cg.add(var.set_sample_pause(config[CONF_SAMPLE_PAUSE]))
    cg.add(var.set_filter_frequency(config[CONF_FILTER_FREQUENCY]))
    
    
    