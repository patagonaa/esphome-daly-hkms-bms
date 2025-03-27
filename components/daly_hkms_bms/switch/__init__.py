import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
    CONF_ID,
)

from .. import DalyHkmsBmsComponent, daly_hkms_bms_ns, CONF_DALY_HKMS_BMS_ID

ICON_BATTERY_ARROW_UP = "mdi:battery-arrow-up"
ICON_BATTERY_ARROW_DOWN = "mdi:battery-arrow-down"

CONF_CHARGE_MOS = "charge_mos"
CONF_DISCHARGE_MOS = "discharge_mos"

DalyHkmsBmsSwitch = daly_hkms_bms_ns.class_(
    "DalyHkmsBmsSwitch", switch.Switch, cg.Component
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_DALY_HKMS_BMS_ID): cv.use_id(DalyHkmsBmsComponent),
            cv.Optional(CONF_CHARGE_MOS): switch.switch_schema(DalyHkmsBmsSwitch, default_restore_mode="DISABLED", icon=ICON_BATTERY_ARROW_UP),
            cv.Optional(CONF_DISCHARGE_MOS): switch.switch_schema(DalyHkmsBmsSwitch, default_restore_mode="DISABLED", icon=ICON_BATTERY_ARROW_DOWN),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_DALY_HKMS_BMS_ID])
    if CONF_CHARGE_MOS in config:
        conf = config[CONF_CHARGE_MOS]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await switch.register_switch(var, conf)
        cg.add(hub.register_input(var))
        cg.add(var.set_parent(hub))
        cg.add(var.set_reg_addr(0x121))
    if CONF_DISCHARGE_MOS in config:
        conf = config[CONF_DISCHARGE_MOS]
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await switch.register_switch(var, conf)
        cg.add(hub.register_input(var))
        cg.add(var.set_parent(hub))
        cg.add(var.set_reg_addr(0x122))
