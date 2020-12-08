import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation, core
from esphome.components import display
from esphome.core import CORE, coroutine, coroutine_with_priority
from esphome.const import CONF_FILE, CONF_ID, CONF_TYPE, CONF_POSITION, CONF_CONTAINER, CONF_CONTAINERS, \
        CONF_LABELS, CONF_ICONS, CONF_IMAGE, CONF_BUTTONS, CONF_FONT, CONF_DEFAULT, CONF_BACKGROUND, \
        CONF_BORDER, CONF_TOP, CONF_BOTTOM, CONF_LEFT, CONF_RIGHT, CONF_TAB_NAME, CONF_ON_PRESS, CONF_TRIGGER_ID, \
        CONF_ON_RELEASE, CONF_ON_CLICK, CONF_MIN_LENGTH, CONF_MAX_LENGTH, CONF_IMAGE_BUTTON, CONF_IMAGE_BACKGROUND, \
        CONF_DEFAULT_VALUE, CONF_SLIDERS, CONF_COLOR, CONF_IMAGE_BUTTON_PRESSED, CONF_IMAGE_SELECTED, \
        CONF_IMAGE_PRESSED

DEPENDENCIES = ['display']
MULTI_CONF = True

display_ns = cg.esphome_ns.namespace('display')
ContainerType = display_ns.enum('ContainerType')
CONTAINER_TYPE = {
    'PLAIN': ContainerType.PLAIN,
    'TABBED_TOP': ContainerType.TABBED_TOP,
    'TABBED_BOTTOM': ContainerType.TABBED_BOTTOM,
}

# Triggers
PressTrigger = display_ns.class_('PressTrigger', automation.Trigger.template())
ReleaseTrigger = display_ns.class_('ReleaseTrigger', automation.Trigger.template())
ClickTrigger = display_ns.class_('ClickTrigger', automation.Trigger.template())

CONFIG_SCHEMA = cv.All(cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(display.Container),
    cv.Required(CONF_POSITION): cv.position,
    cv.Optional(CONF_TYPE, default='PLAIN'): cv.enum(CONTAINER_TYPE, upper=True),  
    cv.Optional(CONF_FONT): cv.use_id(display.Font), 
    cv.Optional(CONF_COLOR): cv.color_rgb, 
    cv.Optional(CONF_BACKGROUND): cv.color_rgb,    
    cv.Optional(CONF_BORDER): cv.All(cv.Schema({
        cv.Optional(CONF_TOP): cv.color_rgb,
        cv.Optional(CONF_BOTTOM): cv.color_rgb,
        cv.Optional(CONF_LEFT): cv.color_rgb,
        cv.Optional(CONF_RIGHT): cv.color_rgb,
    })),
    cv.Optional(CONF_CONTAINERS): cv.All(cv.ensure_list({
        cv.Required(CONF_CONTAINER): cv.use_id(display.Container),
        cv.Optional(CONF_IMAGE): cv.use_id(display.Image),
        cv.Optional(CONF_IMAGE_SELECTED): cv.use_id(display.Image),
        cv.Optional(CONF_TAB_NAME): cv.string,
    }), cv.Length(min=1)),
    cv.Optional(CONF_LABELS): cv.All(cv.ensure_list({
        cv.Required(CONF_ID): cv.declare_id(display.Label),
        cv.Required(CONF_POSITION): cv.position,
        cv.Required(CONF_FONT): cv.use_id(display.Font),
        cv.Required(CONF_COLOR): cv.color_rgb, 
        cv.Optional(CONF_DEFAULT): cv.string,
    }), cv.Length(min=1)),
    cv.Optional(CONF_ICONS): cv.All(cv.ensure_list({
        cv.Required(CONF_ID): cv.declare_id(display.Icon),
        cv.Required(CONF_IMAGE): cv.use_id(display.Image),
        cv.Required(CONF_POSITION): cv.position,
    }), cv.Length(min=1)),
    cv.Optional(CONF_BUTTONS): cv.All(cv.ensure_list({
        cv.Required(CONF_ID): cv.declare_id(display.Button),
        cv.Optional(CONF_IMAGE): cv.use_id(display.Image),
        cv.Optional(CONF_IMAGE_PRESSED): cv.use_id(display.Image),
        cv.Required(CONF_POSITION): cv.position,
        cv.Optional(CONF_DEFAULT): cv.string,
        cv.Optional(CONF_ON_PRESS): automation.validate_automation({
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PressTrigger),
        }),
        cv.Optional(CONF_ON_RELEASE): automation.validate_automation({
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ReleaseTrigger),
        }),
        cv.Optional(CONF_ON_CLICK): automation.validate_automation({
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ClickTrigger),
            cv.Optional(CONF_MIN_LENGTH, default='50ms'): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_MAX_LENGTH, default='350ms'): cv.positive_time_period_milliseconds,
        }), 
    }), cv.Length(min=1)),    
    cv.Optional(CONF_SLIDERS): cv.All(cv.ensure_list({
        cv.Required(CONF_ID): cv.declare_id(display.Slider),
        cv.Required(CONF_POSITION): cv.position,
        cv.Required(CONF_IMAGE_BUTTON): cv.use_id(display.Image),
        cv.Required(CONF_IMAGE_BUTTON_PRESSED): cv.use_id(display.Image),
        cv.Required(CONF_IMAGE_BACKGROUND): cv.use_id(display.Image),
        cv.Required(CONF_DEFAULT_VALUE): cv.int_range(min=0, max=100),
        cv.Optional(CONF_ON_PRESS): automation.validate_automation({
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PressTrigger),
        }),
        cv.Optional(CONF_ON_RELEASE): automation.validate_automation({
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ReleaseTrigger),
        }),
        cv.Optional(CONF_ON_CLICK): automation.validate_automation({
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ClickTrigger),
            cv.Optional(CONF_MIN_LENGTH, default='50ms'): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_MAX_LENGTH, default='350ms'): cv.positive_time_period_milliseconds,
        }),         
    }), cv.Length(min=1)),    
}))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_position(config[CONF_POSITION]))
    cg.add(var.set_type(config[CONF_TYPE]))
    if CONF_FONT in config:
        font = yield cg.get_variable(config[CONF_FONT])
        cg.add(var.set_font(font))
    if CONF_COLOR in config:
        cg.add(var.set_color(config[CONF_COLOR]))
    if CONF_BACKGROUND in config:
        cg.add(var.set_background(config[CONF_BACKGROUND]))
    if CONF_BORDER in config:
        if CONF_TOP in config[CONF_BORDER]:
            cg.add(var.set_border_top(config[CONF_BORDER][CONF_TOP]))
        if CONF_BOTTOM in config[CONF_BORDER]:
            cg.add(var.set_border_bottom(config[CONF_BORDER][CONF_BOTTOM]))
        if CONF_LEFT in config[CONF_BORDER]:
            cg.add(var.set_border_left(config[CONF_BORDER][CONF_LEFT]))
        if CONF_RIGHT in config[CONF_BORDER]:
            cg.add(var.set_border_right(config[CONF_BORDER][CONF_RIGHT]))
    if CONF_CONTAINERS in config:
        for conf in config[CONF_CONTAINERS]:
            container = yield cg.get_variable(conf[CONF_CONTAINER]) 
            if CONF_IMAGE in conf:
                image = yield cg.get_variable(conf[CONF_IMAGE])        
                cg.add(container.set_image(image))
            if CONF_IMAGE_SELECTED in conf:
                image = yield cg.get_variable(conf[CONF_IMAGE_SELECTED])        
                cg.add(container.set_image_selected(image))
            if CONF_TAB_NAME in conf:
                cg.add(container.set_tab_name(conf[CONF_TAB_NAME]))
            if config[CONF_TYPE] != 'PLAIN':
              cg.add(var.add_container(container))
            else:
              cg.add(var.add_container_child(container))
    if CONF_LABELS in config:
        for conf in config[CONF_LABELS]:
            label = cg.new_Pvariable(conf[CONF_ID])
            cg.add(label.set_position(conf[CONF_POSITION]))
            font = yield cg.get_variable(conf[CONF_FONT])
            cg.add(label.set_font(font))
            cg.add(label.set_color(conf[CONF_COLOR]))
            if CONF_DEFAULT in conf:
                cg.add(label.set_text(conf[CONF_DEFAULT]))
            cg.add(var.add_child(label))
    if CONF_ICONS in config:
        for conf in config[CONF_ICONS]:
            icon = cg.new_Pvariable(conf[CONF_ID])
            cg.add(icon.set_position(conf[CONF_POSITION]))
            image = yield cg.get_variable(conf[CONF_IMAGE])
            cg.add(icon.set_image(image))
            cg.add(var.add_child(icon))
    if CONF_BUTTONS in config:
        for conf in config[CONF_BUTTONS]:
            button = cg.new_Pvariable(conf[CONF_ID])
            cg.add(button.set_position(conf[CONF_POSITION]))
            if CONF_IMAGE in conf:
                image = yield cg.get_variable(conf[CONF_IMAGE])
                cg.add(button.set_image(image))
            if CONF_IMAGE_PRESSED in conf:
                image = yield cg.get_variable(conf[CONF_IMAGE_PRESSED])
                cg.add(button.set_image_pressed(image))
            if CONF_DEFAULT in conf:
                cg.add(button.set_text(conf[CONF_DEFAULT]))
            for c in conf.get(CONF_ON_PRESS, []):
                trigger = cg.new_Pvariable(c[CONF_TRIGGER_ID], button)
                yield automation.build_automation(trigger, [], c)
            for c in conf.get(CONF_ON_RELEASE, []):
                trigger = cg.new_Pvariable(c[CONF_TRIGGER_ID], button)
                yield automation.build_automation(trigger, [], c)
            for c in conf.get(CONF_ON_CLICK, []):
                trigger = cg.new_Pvariable(c[CONF_TRIGGER_ID], button,
                                           c[CONF_MIN_LENGTH], c[CONF_MAX_LENGTH])
                yield automation.build_automation(trigger, [], c)                
            cg.add(var.add_child(button))

    if CONF_SLIDERS in config:
        for conf in config[CONF_SLIDERS]:
            slider = cg.new_Pvariable(conf[CONF_ID])
            cg.add(slider.set_position(conf[CONF_POSITION]))
            image = yield cg.get_variable(conf[CONF_IMAGE_BUTTON])
            cg.add(slider.set_image_button(image))
            image = yield cg.get_variable(conf[CONF_IMAGE_BUTTON_PRESSED])
            cg.add(slider.set_image_button_pressed(image))
            image = yield cg.get_variable(conf[CONF_IMAGE_BACKGROUND])
            cg.add(slider.set_image_background(image))
            cg.add(slider.set_value(conf[CONF_DEFAULT_VALUE]))
            for c in conf.get(CONF_ON_PRESS, []):
                trigger = cg.new_Pvariable(c[CONF_TRIGGER_ID], slider)
                yield automation.build_automation(trigger, [], c)
            for c in conf.get(CONF_ON_RELEASE, []):
                trigger = cg.new_Pvariable(c[CONF_TRIGGER_ID], slider)
                yield automation.build_automation(trigger, [], c)
            for c in conf.get(CONF_ON_CLICK, []):
                trigger = cg.new_Pvariable(c[CONF_TRIGGER_ID], slider,
                                           c[CONF_MIN_LENGTH], c[CONF_MAX_LENGTH])
                yield automation.build_automation(trigger, [], c)                
            cg.add(var.add_child(slider))
            
        