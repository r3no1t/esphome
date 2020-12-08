import logging

from esphome import core
from esphome.components import display, font
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_FILE, CONF_ID, CONF_RESIZE, CONF_NUM_COLOR_BITS, \
      CONF_FROM, CONF_TO
from esphome.core import CORE, HexInt
_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ['display']
MULTI_CONF = True


def validate_colormapping(value):
    if isinstance(value, dict):
        return cv.Schema({
            cv.Required(CONF_FROM): cv.int_range(min=0, max=15),
            cv.Required(CONF_TO): cv.color_rgb,
        })(value)
    value = cv.string(value)
    if '->' not in value:
        raise cv.Invalid("Datapoint mapping must contain '->'")
    a, b = value.split('->', 1)
    a, b = a.strip(), b.strip()
    return validate_colormapping({
        CONF_FROM: int(a),
        CONF_TO: cv.color_rgb(b)
    })


CONF_RAW_DATA_ID = 'raw_data_id'

IMAGE_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(display.Image),
    cv.Required(CONF_FILE): cv.file_,
    cv.Optional(CONF_RESIZE): cv.dimensions,
    cv.GenerateID(CONF_RAW_DATA_ID): cv.declare_id(cg.uint8),
    cv.Optional(CONF_NUM_COLOR_BITS, default=1): cv.int_range(min=1, max=8),
})

CONFIG_SCHEMA = cv.All(font.validate_pillow_installed, IMAGE_SCHEMA)


def to_code(config):
    from PIL import Image

    path = CORE.relative_config_path(config[CONF_FILE])
    try:
        image = Image.open(path)
    except Exception as e:
        raise core.EsphomeError(f"Could not load image file {path}: {e}")

    if CONF_RESIZE in config:
        image.thumbnail(config[CONF_RESIZE])

    if config[CONF_NUM_COLOR_BITS] > 1:
        image = image.convert('RGB', dither=Image.NONE)
        width, height = image.size
        data = [0 for _ in range((height * width * config[CONF_NUM_COLOR_BITS] +
                                  config[CONF_NUM_COLOR_BITS]) // 8)]
        colors = list()
        for y in range(height):
            for x in range(width):
                rgb = image.getpixel((x, y))
                rgb = (((rgb[0] >> 3) & 0x1F) << 11) + (((rgb[1] >> 2) & 0x3F) << 6) + \
                      ((rgb[2] >> 3) & 0x1F)
                if rgb not in colors:
                    colors.append(rgb)
                rgb_id = 0
                for color in colors:
                    if color == rgb:
                        break
                    rgb_id += 1
                bit_pos = (y * width + x) * config[CONF_NUM_COLOR_BITS]
                byte_pos = bit_pos // 8
                data[byte_pos] |= ((rgb_id & (2 ** config[CONF_NUM_COLOR_BITS] - 1)) <<
                                   (bit_pos % 8)) & 0xFF
                if ((bit_pos % 8) + config[CONF_NUM_COLOR_BITS]) > 8:
                    data[byte_pos+1] |= ((rgb_id & (2 ** config[CONF_NUM_COLOR_BITS] - 1)) >>
                                         (8 - bit_pos % 8)) & 0xFF

        if len(colors) > (1 << config[CONF_NUM_COLOR_BITS]):
            _LOGGER.warning("The number of colors in file %s exceeds the num_color_bits range, "
                            "some colors will be missing", config[CONF_FILE])
            colors = colors[:(1 << config[CONF_NUM_COLOR_BITS])]
    else:
        image = image.convert('1', dither=Image.NONE)
        width, height = image.size
        width8 = ((width + 7) // 8) * 8
        data = [0 for _ in range(height * width8 // 8)]
        for y in range(height):
            for x in range(width):
                if image.getpixel((x, y)):
                    continue
                pos = x + y * width8
                data[pos // 8] |= 0x80 >> (pos % 8)

    if width > 500 or height > 500:
        _LOGGER.warning("The image you requested is very big. Please consider using the resize "
                        "parameter")

    rhs = [HexInt(x) for x in data]
    prog_arr = cg.progmem_array(config[CONF_RAW_DATA_ID], rhs)
    var = cg.new_Pvariable(config[CONF_ID], prog_arr, width, height)
    if config[CONF_NUM_COLOR_BITS] > 1:
        cg.add(var.set_colors(config[CONF_NUM_COLOR_BITS], colors))
