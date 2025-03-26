import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_BATTERY_LEVEL,
    CONF_MAX_TEMPERATURE,
    CONF_MIN_TEMPERATURE,
    CONF_POWER,
    CONF_ENERGY,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_DISTANCE,
    STATE_CLASS_MEASUREMENT,
    UNIT_EMPTY,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_PERCENT,
    UNIT_CELSIUS,
    UNIT_WATT,
    UNIT_WATT_HOURS,
    UNIT_KILOMETER,
    UNIT_MINUTE,
    ICON_FLASH,
    ICON_PERCENT,
    ICON_COUNTER,
    ICON_THERMOMETER,
    ICON_GAUGE,
)
from . import DalyHkmsBmsComponent, CONF_DALY_HKMS_BMS_ID

CONF_MAX_CELL_VOLTAGE = "max_cell_voltage"
CONF_MAX_CELL_VOLTAGE_NUMBER = "max_cell_voltage_number"
CONF_MIN_CELL_VOLTAGE = "min_cell_voltage"
CONF_MIN_CELL_VOLTAGE_NUMBER = "min_cell_voltage_number"
CONF_DELTA_CELL_VOLTAGE = "delta_cell_voltage"
CONF_MAX_TEMPERATURE_PROBE_NUMBER = "max_temperature_probe_number"
CONF_MIN_TEMPERATURE_PROBE_NUMBER = "min_temperature_probe_number"
CONF_CELLS_NUMBER = "cells_number"
CONF_TEMPS_NUMBER = "temps_number"

CONF_REMAINING_CAPACITY = "remaining_capacity"
CONF_CYCLES = "cycles"
CONF_BALANCE_CURRENT = "balance_current"

CONF_REMAINING_MILEAGE = "remaining_mileage"
CONF_REMAINING_CHARGING_TIME = "remaining_charging_time"

CONF_TEMPERATURE_MOS = "temperature_mos"
CONF_TEMPERATURE_BOARD = "temperature_board"

ICON_CURRENT_DC = "mdi:current-dc"
ICON_BATTERY_OUTLINE = "mdi:battery-outline"
ICON_BATTERY_CLOCK = "mdi:battery-clock"
ICON_THERMOMETER_CHEVRON_UP = "mdi:thermometer-chevron-up"
ICON_THERMOMETER_CHEVRON_DOWN = "mdi:thermometer-chevron-down"
ICON_CAR_BATTERY = "mdi:car-battery"
ICON_MAP_MARKER_DISTANCE = "mdi:map-marker-distance"
ICON_SCALE_BALANCE = "mdi:scale-balance"

UNIT_AMPERE_HOUR = "Ah"

MAX_CELL_NUMBER = 48
MAX_TEMP_NUMBER = 8

TYPES = [
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_BATTERY_LEVEL,
    CONF_MAX_CELL_VOLTAGE,
    CONF_MAX_CELL_VOLTAGE_NUMBER,
    CONF_MIN_CELL_VOLTAGE,
    CONF_MIN_CELL_VOLTAGE_NUMBER,
    CONF_DELTA_CELL_VOLTAGE,
    CONF_MAX_TEMPERATURE,
    CONF_MAX_TEMPERATURE_PROBE_NUMBER,
    CONF_MIN_TEMPERATURE,
    CONF_MIN_TEMPERATURE_PROBE_NUMBER,
    CONF_CELLS_NUMBER,
    CONF_TEMPS_NUMBER,
    CONF_REMAINING_CAPACITY,
    CONF_CYCLES,
    CONF_BALANCE_CURRENT,
    CONF_POWER,
    CONF_ENERGY,
    CONF_TEMPERATURE_MOS,
    CONF_TEMPERATURE_BOARD,
    CONF_REMAINING_MILEAGE,
    CONF_REMAINING_CHARGING_TIME,
    # Cell voltages and temperatures are handled by loops below
]

TEMPERATURE_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    icon=ICON_THERMOMETER,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

CELL_VOLTAGE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
    icon=ICON_FLASH,
    accuracy_decimals=3,
)


def get_cell_voltage_key(cell):
    return f"cell_{cell}_voltage"


def get_cell_voltages_schema():
    schema_obj = {}
    for i in range(1, MAX_CELL_NUMBER + 1):
        schema_obj[cv.Optional(get_cell_voltage_key(i))] = CELL_VOLTAGE_SCHEMA
    return cv.Schema(schema_obj)


def get_temperature_sensor_key(temp):
    return f"temperature_{temp}"


def get_temperature_sensors_schema():
    schema_obj = {}
    for i in range(1, MAX_TEMP_NUMBER + 1):
        schema_obj[cv.Optional(get_temperature_sensor_key(i))] = TEMPERATURE_SENSOR_SCHEMA
    return cv.Schema(schema_obj)


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_DALY_HKMS_BMS_ID): cv.use_id(DalyHkmsBmsComponent),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                icon=ICON_CURRENT_DC,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_PERCENT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_CELL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                icon=ICON_FLASH,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_CELL_VOLTAGE_NUMBER): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_CELL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                icon=ICON_FLASH,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_CELL_VOLTAGE_NUMBER): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DELTA_CELL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                icon=ICON_FLASH,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                icon=ICON_THERMOMETER_CHEVRON_UP,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_TEMPERATURE_PROBE_NUMBER): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                icon=ICON_THERMOMETER_CHEVRON_DOWN,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_TEMPERATURE_PROBE_NUMBER): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_REMAINING_CAPACITY): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE_HOUR,
                icon=ICON_GAUGE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CYCLES): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BALANCE_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                icon=ICON_SCALE_BALANCE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CELLS_NUMBER): sensor.sensor_schema(
                icon=ICON_COUNTER,
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_TEMPS_NUMBER): sensor.sensor_schema(
                icon=ICON_COUNTER,
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ENERGY): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT_HOURS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_MOS): TEMPERATURE_SENSOR_SCHEMA,
            cv.Optional(CONF_TEMPERATURE_BOARD): TEMPERATURE_SENSOR_SCHEMA,
            cv.Optional(CONF_REMAINING_MILEAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_KILOMETER,
                icon=ICON_MAP_MARKER_DISTANCE,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_DISTANCE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_REMAINING_CHARGING_TIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_MINUTE,
                icon=ICON_BATTERY_CLOCK,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_DURATION,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(get_cell_voltages_schema())
    .extend(get_temperature_sensors_schema())
    .extend(cv.COMPONENT_SCHEMA)
)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        sens = await sensor.new_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_sensor")(sens))


async def setup_cell_voltage_conf(config, cell, hub):
    key = get_cell_voltage_key(cell)
    if sensor_config := config.get(key):
        sens = await sensor.new_sensor(sensor_config)
        cg.add(hub.set_cell_voltage_sensor(cell, sens))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_DALY_HKMS_BMS_ID])
    for i in range(1, MAX_CELL_NUMBER + 1):
        await setup_cell_voltage_conf(config, i, hub)
    for i in range(1, MAX_TEMP_NUMBER + 1):
        await setup_cell_voltage_conf(config, get_temperature_sensor_key(i), hub)
    for key in TYPES:
        await setup_conf(config, key, hub)
