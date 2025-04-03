#include "daly_hkms_bms_queue.h"
#include "daly_hkms_bms.h"

namespace esphome {
namespace daly_hkms_bms {

static const char *const TAG = "daly_hkms_bms_queue";

DalyHkmsCommandQueue* DalyHkmsCommandQueue::get_for_modbus(const modbus::Modbus* modbus) {
  auto it = DalyHkmsCommandQueue::instances_.find(modbus);
  if (it != DalyHkmsCommandQueue::instances_.end()) {
    return it->second;
  }
  auto instance = new DalyHkmsCommandQueue();
  DalyHkmsCommandQueue::instances_[modbus] = instance;
  return instance;
}

void DalyHkmsCommandQueue::add_or_update(bool prioritize, const QueueItem &item_to_add) {
  auto comparer = [&](QueueItem x) -> bool { return x.key_equals(item_to_add); };

  // search prio queue for item with key and update
  auto it = std::find_if(prio_queue_.begin(), prio_queue_.end(), comparer);
  if (it != prio_queue_.end()) {
    it->data = item_to_add.data;
    return;
  }

  // search non prio queue for item with key and update
  it = std::find_if(non_prio_queue_.begin(), non_prio_queue_.end(), comparer);
  if (it != non_prio_queue_.end()) {
    it->data = item_to_add.data;
    return;
  }

  // not found in any queue -> add
  if (prioritize) {
    prio_queue_.push_back(item_to_add);
  } else {
    non_prio_queue_.push_back(item_to_add);
  }
}
bool DalyHkmsCommandQueue::try_get_to_send(uint16_t daly_address, QueueItem* item) {
  if (this->pending_item_.has_value()) {
    if (this->pending_item_.value().daly_address == daly_address) {
      ESP_LOGD(TAG, "Returning same queue item for %" PRIu16 " twice -> timeout?", daly_address);
      // pending device asks to send again -> return same item
      *item = this->pending_item_.value();
      return true;
    }
    return false;
  }

  if (!prio_queue_.empty()) {
    QueueItem first_item = prio_queue_.front();

    if (first_item.daly_address != daly_address)
      return false; // item is for another address

    *item = first_item;
    prio_queue_.pop_front();
    this->pending_item_ = *item;
    return true;
  }

  if (!non_prio_queue_.empty()) {
    QueueItem first_item = non_prio_queue_.front();

    if (first_item.daly_address != daly_address)
      return false; // item is for another address

    *item = first_item;
    non_prio_queue_.pop_front();
    this->pending_item_ = *item;
    return true;
  }

  return false;
}
bool DalyHkmsCommandQueue::pop_pending(uint16_t daly_address, QueueItem* item) {
  if (!this->pending_item_.has_value()) {
    return false;
  }

  if (this->pending_item_.value().daly_address != daly_address) {
    // response from other device -> keep waiting
    return false;
  }

  *item = this->pending_item_.value();
  this->pending_item_.reset();
  return true;
}

std::unordered_map<const modbus::Modbus*, DalyHkmsCommandQueue*> DalyHkmsCommandQueue::instances_;

}  // namespace daly_hkms_bms
}  // namespace esphome
