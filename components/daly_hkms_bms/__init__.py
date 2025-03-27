import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import modbus
from esphome.const import CONF_ID, CONF_ADDRESS

CODEOWNERS = ["@patagonaa"]
MULTI_CONF = True
DEPENDENCIES = ["modbus"]

CONF_DALY_HKMS_BMS_ID = "daly_hkms_bms_id"
CONF_UPDATE_INTERVAL_FAST = "update_interval_fast"

daly_hkms_bms = cg.esphome_ns.namespace("daly_hkms_bms")
DalyHkmsBmsComponent = daly_hkms_bms.class_(
    "DalyHkmsBmsComponent", cg.PollingComponent, modbus.ModbusDevice
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DalyHkmsBmsComponent),
        cv.GenerateID(modbus.CONF_MODBUS_ID): cv.use_id(modbus.Modbus),
        cv.Optional(CONF_ADDRESS, default=1): cv.positive_int,
        cv.Optional(CONF_UPDATE_INTERVAL_FAST): cv.update_interval,
    }
).extend(cv.polling_component_schema("30s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await modbus.register_modbus_device(var, config)
    cg.add(var.set_daly_address(config[CONF_ADDRESS]))
    if update_interval_fast := config.get(CONF_UPDATE_INTERVAL_FAST):
        cg.add(var.set_update_interval_fast(update_interval_fast))