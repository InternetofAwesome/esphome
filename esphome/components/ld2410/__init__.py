from esphome import automation
from esphome.automation import maybe_simple_id
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_MODE, CONF_PASSWORD, CONF_THROTTLE, CONF_TIMEOUT

AUTO_LOAD = ["ld24xx"]
DEPENDENCIES = ["uart"]
CODEOWNERS = ["@sebcaps", "@regevbr"]
MULTI_CONF = True

ld2410_ns = cg.esphome_ns.namespace("ld2410")
LD2410Component = ld2410_ns.class_("LD2410Component", cg.Component, uart.UARTDevice)

CONF_LD2410_ID = "ld2410_id"
CONF_MAX_MOVE_DISTANCE = "max_move_distance"
CONF_MAX_STILL_DISTANCE = "max_still_distance"
CONF_MOVE_THRESHOLDS = [f"g{x}_move_threshold" for x in range(9)]
CONF_STILL_THRESHOLDS = [f"g{x}_still_threshold" for x in range(9)]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LD2410Component),
        cv.Optional(CONF_THROTTLE): cv.invalid(
            f"{CONF_THROTTLE} has been removed; use per-sensor filters, instead"
        ),
        cv.Optional(CONF_MAX_MOVE_DISTANCE): cv.invalid(
            f"The '{CONF_MAX_MOVE_DISTANCE}' option has been moved to the '{CONF_MAX_MOVE_DISTANCE}'"
            f" number component"
        ),
        cv.Optional(CONF_MAX_STILL_DISTANCE): cv.invalid(
            f"The '{CONF_MAX_STILL_DISTANCE}' option has been moved to the '{CONF_MAX_STILL_DISTANCE}'"
            f" number component"
        ),
        cv.Optional(CONF_TIMEOUT): cv.invalid(
            f"The '{CONF_TIMEOUT}' option has been moved to the '{CONF_TIMEOUT}'"
            f" number component"
        ),
    }
)

for i in range(9):
    CONFIG_SCHEMA = CONFIG_SCHEMA.extend(
        cv.Schema(
            {
                cv.Optional(CONF_MOVE_THRESHOLDS[i]): cv.invalid(
                    f"The '{CONF_MOVE_THRESHOLDS[i]}' option has been moved to the '{CONF_MOVE_THRESHOLDS[i]}'"
                    f" number component"
                ),
                cv.Optional(CONF_STILL_THRESHOLDS[i]): cv.invalid(
                    f"The '{CONF_STILL_THRESHOLDS[i]}' option has been moved to the '{CONF_STILL_THRESHOLDS[i]}'"
                    f" number component"
                ),
            }
        )
    )

CONFIG_SCHEMA = cv.All(
    CONFIG_SCHEMA.extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "ld2410",
    require_tx=True,
    require_rx=True,
    parity="NONE",
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)


CALIBRATION_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2410Component),
    }
)

CONF_CALIBRATION_DELAY = "delay_s"
CONF_CALIBRATION_SAMPLE = "sample_s"
CALIBRATION_MODES = ["Off", "Average", "Maximum", "Intelligent"]

START_CALIBRATION_ACTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(LD2410Component),
        cv.Optional(CONF_MODE): cv.one_of(*CALIBRATION_MODES),
        cv.Optional(CONF_CALIBRATION_DELAY): cv.templatable(cv.uint8_t),
        cv.Optional(CONF_CALIBRATION_SAMPLE): cv.templatable(cv.uint8_t),
    }
)

# Actions
BluetoothPasswordSetAction = ld2410_ns.class_(
    "BluetoothPasswordSetAction", automation.Action
)
StartCalibrationAction = ld2410_ns.class_("StartCalibrationAction", automation.Action)
ApplyCalibrationAction = ld2410_ns.class_("ApplyCalibrationAction", automation.Action)
DiscardCalibrationAction = ld2410_ns.class_(
    "DiscardCalibrationAction", automation.Action
)


BLUETOOTH_PASSWORD_SET_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(LD2410Component),
        cv.Required(CONF_PASSWORD): cv.templatable(cv.string_strict),
    }
)


@automation.register_action(
    "bluetooth_password.set",
    BluetoothPasswordSetAction,
    BLUETOOTH_PASSWORD_SET_SCHEMA,
    synchronous=True,
)
async def bluetooth_password_set_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_PASSWORD], args, cg.std_string)
    cg.add(var.set_password(template_))
    return var


@automation.register_action(
    "ld2410.start_calibration",
    StartCalibrationAction,
    START_CALIBRATION_ACTION_SCHEMA,
)
async def start_calibration_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    if mode_config := config.get(CONF_MODE):
        cg.add(var.set_mode(CALIBRATION_MODES.index(mode_config)))
    if delay_config := config.get(CONF_CALIBRATION_DELAY):
        template_ = await cg.templatable(delay_config, args, cg.uint8)
        cg.add(var.set_delay_s(template_))
    if sample_config := config.get(CONF_CALIBRATION_SAMPLE):
        template_ = await cg.templatable(sample_config, args, cg.uint8)
        cg.add(var.set_sample_s(template_))
    return var


@automation.register_action(
    "ld2410.apply_calibration",
    ApplyCalibrationAction,
    CALIBRATION_ACTION_SCHEMA,
)
async def apply_calibration_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    return var


@automation.register_action(
    "ld2410.discard_calibration",
    DiscardCalibrationAction,
    CALIBRATION_ACTION_SCHEMA,
)
async def discard_calibration_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    return var
