import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_STATUS
from . import DalyHkmsBmsComponent, CONF_DALY_HKMS_BMS_ID

CONF_ALERTS = "alerts"

ICON_CAR_BATTERY = "mdi:car-battery"
ICON_BATTERY_ALERT = "mdi:battery-alert"

TYPES = [
    CONF_STATUS,
    CONF_ALERTS,
]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_DALY_HKMS_BMS_ID): cv.use_id(DalyHkmsBmsComponent),
            cv.Optional(CONF_STATUS): text_sensor.text_sensor_schema(
                icon=ICON_CAR_BATTERY
            ),
            cv.Optional(CONF_ALERTS): text_sensor.text_sensor_schema(
                icon=ICON_BATTERY_ALERT
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        sens = await text_sensor.new_text_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_DALY_HKMS_BMS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
    if CONF_ALERTS in config:
        cg.add_define("USE_TEXT_ALERTS")
