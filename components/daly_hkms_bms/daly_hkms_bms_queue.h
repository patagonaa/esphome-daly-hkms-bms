#pragma once

#include "esphome/core/defines.h"
#include "esphome/components/modbus/modbus.h"

#include <optional>
#include <unordered_map>
#include <deque>
#include <cstdint>

namespace esphome {
namespace daly_hkms_bms {

struct QueueItem
{
  uint16_t daly_address;
  uint8_t cmd;
  uint16_t register_address;
  uint16_t data;

  bool key_equals(const QueueItem &other) {
    return
      this->daly_address == other.daly_address &&
      this->cmd == other.cmd &&
      this->register_address == other.register_address;
  }
};

class DalyHkmsCommandQueue
{
 public:
  static DalyHkmsCommandQueue* get_for_modbus(const modbus::Modbus* modbus);
  void add_or_update(bool prioritize, const QueueItem &item_to_add);
  bool try_get_to_send(uint16_t daly_address, QueueItem* item);
  bool pop_pending(uint16_t daly_address, QueueItem* item);
 private:
  DalyHkmsCommandQueue() {}
  static std::unordered_map<const modbus::Modbus*, DalyHkmsCommandQueue*> instances_;
  std::deque<QueueItem> prio_queue_{};
  std::deque<QueueItem> non_prio_queue_{};
  std::optional<QueueItem> pending_item_{};
};

}  // namespace daly_hkms_bms
}  // namespace esphome
