import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from . import DalyHkmsBmsComponent, CONF_DALY_HKMS_BMS_ID

CONF_CHARGING_MOS_ENABLED = "charging_mos_enabled"
CONF_DISCHARGING_MOS_ENABLED = "discharging_mos_enabled"
CONF_PRECHARGING_MOS_ENABLED = "precharging_mos_enabled"
CONF_BALANCING_ACTIVE = "balancing_active"
CONF_HAS_ERRORS = "has_errors"

ICON_BATTERY_ARROW_UP = "mdi:battery-arrow-up"
ICON_BATTERY_ARROW_DOWN = "mdi:battery-arrow-down"
ICON_SLOPE_UPHILL = "mdi:slope-uphill"
ICON_SCALE_BALANCE = "mdi:scale-balance"
ICON_BATTERY_ALERT = "mdi:battery-alert"

TYPES = [
    CONF_CHARGING_MOS_ENABLED,
    CONF_DISCHARGING_MOS_ENABLED,
    CONF_PRECHARGING_MOS_ENABLED,
    CONF_BALANCING_ACTIVE,
    CONF_HAS_ERRORS,
]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_DALY_HKMS_BMS_ID): cv.use_id(DalyHkmsBmsComponent),
            cv.Optional(
                CONF_CHARGING_MOS_ENABLED
            ): binary_sensor.binary_sensor_schema(icon=ICON_BATTERY_ARROW_UP),
            cv.Optional(
                CONF_DISCHARGING_MOS_ENABLED
            ): binary_sensor.binary_sensor_schema(icon=ICON_BATTERY_ARROW_DOWN),
            cv.Optional(
                CONF_PRECHARGING_MOS_ENABLED
            ): binary_sensor.binary_sensor_schema(icon=ICON_SLOPE_UPHILL),
            cv.Optional(CONF_BALANCING_ACTIVE): binary_sensor.binary_sensor_schema(icon=ICON_SCALE_BALANCE),
            cv.Optional(CONF_HAS_ERRORS): binary_sensor.binary_sensor_schema(icon=ICON_BATTERY_ALERT),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await binary_sensor.new_binary_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_binary_sensor")(var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_DALY_HKMS_BMS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
