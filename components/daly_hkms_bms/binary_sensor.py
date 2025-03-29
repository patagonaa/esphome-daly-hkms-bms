import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import DalyHkmsBmsComponent, CONF_DALY_HKMS_BMS_ID, MAX_CELL_NUMBER

CONF_CHARGING_MOS_ENABLED = "charging_mos_enabled"
CONF_DISCHARGING_MOS_ENABLED = "discharging_mos_enabled"
CONF_PRECHARGING_MOS_ENABLED = "precharging_mos_enabled"
CONF_BALANCING_ACTIVE = "balancing_active"

CONF_ERROR_CHARGE_MOS_OVERTEMPERATURE = "error_charge_mos_overtemperature"
CONF_ERROR_CHARGE_MOS_TEMPERATURE_DETECT = "error_charge_mos_temperature_detect"
CONF_ERROR_DISCHARGE_MOS_OVERTEMPERATURE = "error_discharge_mos_overtemperature"
CONF_ERROR_DISCHARGE_MOS_TEMPERATURE_DETECT = "error_discharge_mos_temperature_detect"
CONF_ERROR_SHORT_CIRCUIT = "error_short_circuit"
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
    CONF_ERROR_CHARGE_MOS_OVERTEMPERATURE,
    CONF_ERROR_CHARGE_MOS_TEMPERATURE_DETECT,
    CONF_ERROR_DISCHARGE_MOS_OVERTEMPERATURE,
    CONF_ERROR_DISCHARGE_MOS_TEMPERATURE_DETECT,
    CONF_ERROR_SHORT_CIRCUIT,
    CONF_HAS_ERRORS,
]

def get_cell_balancing_key(cell):
    return f"cell_{cell}_balancing"

CELL_BALANCING_SCHEMA = binary_sensor.binary_sensor_schema(icon=ICON_SCALE_BALANCE)

def get_cell_balancing_schema():
    schema_obj = {}
    for i in range(1, MAX_CELL_NUMBER + 1):
        schema_obj[cv.Optional(get_cell_balancing_key(i))] = CELL_BALANCING_SCHEMA
    return cv.Schema(schema_obj)

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
            cv.Optional(CONF_ERROR_CHARGE_MOS_OVERTEMPERATURE): binary_sensor.binary_sensor_schema(icon=ICON_BATTERY_ALERT),
            cv.Optional(CONF_ERROR_CHARGE_MOS_TEMPERATURE_DETECT): binary_sensor.binary_sensor_schema(icon=ICON_BATTERY_ALERT),
            cv.Optional(CONF_ERROR_DISCHARGE_MOS_OVERTEMPERATURE): binary_sensor.binary_sensor_schema(icon=ICON_BATTERY_ALERT),
            cv.Optional(CONF_ERROR_DISCHARGE_MOS_TEMPERATURE_DETECT): binary_sensor.binary_sensor_schema(icon=ICON_BATTERY_ALERT),
            cv.Optional(CONF_ERROR_SHORT_CIRCUIT): binary_sensor.binary_sensor_schema(icon=ICON_BATTERY_ALERT),
            cv.Optional(CONF_HAS_ERRORS): binary_sensor.binary_sensor_schema(icon=ICON_BATTERY_ALERT),
        }
    )
    .extend(get_cell_balancing_schema())
    .extend(cv.COMPONENT_SCHEMA)
)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await binary_sensor.new_binary_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_binary_sensor")(var))

async def setup_cell_balancing_conf(config, cell, hub):
    key = get_cell_balancing_key(cell)
    if sensor_config := config.get(key):
        sens = await binary_sensor.new_binary_sensor(sensor_config)
        cg.add(hub.set_cell_balancing_sensor(cell, sens))

async def to_code(config):
    hub = await cg.get_variable(config[CONF_DALY_HKMS_BMS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
    for i in range(1, MAX_CELL_NUMBER + 1):
        await setup_cell_balancing_conf(config, i, hub)
