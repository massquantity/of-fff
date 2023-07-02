#include "buffer/lru_k_replacer.h"
#include "common/exception.h"
#include "fmt/format.h"

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : max_num_frames_(num_frames), k_(k) {}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);

  bool has_inf = false;
  size_t earliest_backward_k = 0;
  size_t earliest_fewer_k = 0;
  frame_id_t evict_id = -1;
  for (const auto &[fid, node] : node_store_) {
    if (!node.IsEvictable() || node.IsEmpty()) {
      continue;
    }
    if (node.Size() < k_) {
      has_inf = true;
      size_t earliest_in_node = node.EarliestTime();
      if (earliest_fewer_k == 0 || earliest_in_node < earliest_fewer_k) {
        earliest_fewer_k = earliest_in_node;
        evict_id = fid;
      }
    } else if (!has_inf && node.Size() >= k_) {
      size_t last_k_time = node.LastKTime(static_cast<int>(k_));
      if (earliest_backward_k == 0 || last_k_time < earliest_backward_k) {
        earliest_backward_k = last_k_time;
        evict_id = fid;
      }
    }
  }

  bool evict_success = false;
  if (evict_id != -1) {
    evict_success = true;
    replacer_size_--;
    node_store_[evict_id].ClearNode();
    *frame_id = evict_id;
  }
  return evict_success;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) {
  std::scoped_lock<std::mutex> lock(latch_);

  CheckFrameId(frame_id);
  // auto now = std::chrono::high_resolution_clock::now();
  // size_t cur_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
  if (node_store_.find(frame_id) == node_store_.end()) {
    node_store_[frame_id] = LRUKNode(k_, frame_id);
  }
  current_timestamp_++;
  node_store_[frame_id].AddHistory(current_timestamp_);
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  std::scoped_lock<std::mutex> lock(latch_);

  CheckFrameId(frame_id);
  if (node_store_.find(frame_id) == node_store_.end() || node_store_[frame_id].IsEmpty()) {
    return;
  }
  LRUKNode &node = node_store_[frame_id];
  if (set_evictable && !node.IsEvictable()) {
    replacer_size_++;
    node.SetEvictable(set_evictable);
  } else if (!set_evictable && node.IsEvictable()) {
    replacer_size_--;
    node.SetEvictable(set_evictable);
  }
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);

  if (node_store_.find(frame_id) != node_store_.end() && !node_store_[frame_id].IsEmpty()) {
    LRUKNode &node = node_store_[frame_id];
    if (!node.IsEvictable()) {
      throw bustub::Exception(fmt::format("frame {} is non-evictable.", frame_id));
    }
    node.ClearNode();
    replacer_size_--;
  }
}

auto LRUKReplacer::Size() -> size_t { return replacer_size_; }

}  // namespace bustub
