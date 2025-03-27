#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../daly_hkms_bms.h"

namespace esphome {
namespace daly_hkms_bms {

class DalyHkmsBmsSwitch : public switch_::Switch, public Component, public DalyHkmsBmsInput {
 public:
  void set_parent(DalyHkmsBmsComponent *parent) {
    this->parent_ = parent;
  };
  void set_reg_addr(uint16_t address) {
    this->address_ = address;
  };

  uint16_t get_reg_addr() override {
    return this->address_;
  };

  void handle_update(uint16_t value) override {
    this->publish_state(value > 0);
  };

 protected:
  DalyHkmsBmsComponent *parent_;
  uint16_t address_;

  void write_state(bool state) override {
    this->parent_->write_register(this->address_, state ? 1 : 0);
  };
};

}  // namespace daly_hkms_bms
}  // namespace esphome
