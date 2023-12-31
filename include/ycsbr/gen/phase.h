#pragma once

#include <memory>

#include "ycsbr/gen/chooser.h"
#include "ycsbr/gen/types.h"
#include "ycsbr/request.h"

namespace ycsbr {
namespace gen {

// Tracks the current state of a workload phase.
// This is meant for internal use only.
struct Phase {
  explicit Phase(PhaseID phase_id)
      : phase_id(phase_id),
        num_inserts(0),
        num_inserts_left(0),
        num_requests(0),
        num_requests_left(0),
        read_thres(0),
        rmw_thres(0),
        negativeread_thres(0),
        scan_thres(0),
        update_thres(0),
        delete_thres(0),         ///////////////////
        num_deletes(0),   //////////////////
        num_deletes_left(0),  ///////////////////
        max_scan_length(0) {}

  bool HasNext() const { return num_requests_left > 0; }

  void SetItemCount(const size_t item_count) {
    if (read_chooser != nullptr) {
      read_chooser->SetItemCount(item_count);
    }
    if (rmw_chooser != nullptr) {
      rmw_chooser->SetItemCount(item_count);
    }
    if (negativeread_chooser != nullptr) {
      negativeread_chooser->SetItemCount(item_count);
    }
    if (scan_chooser != nullptr) {
      scan_chooser->SetItemCount(item_count);
    }
    if (update_chooser != nullptr) {
      update_chooser->SetItemCount(item_count);
    }
    /////////////////////////////////
    if (delete_chooser != nullptr) {
      delete_chooser->SetItemCount(item_count);
    }
    /////////////////////////////////
  }

  void IncreaseItemCountBy(const size_t delta) {  
    if (read_chooser != nullptr) {
      read_chooser->IncreaseItemCountBy(delta);
    }
    if (rmw_chooser != nullptr) {
      rmw_chooser->IncreaseItemCountBy(delta);
    }
    if (negativeread_chooser != nullptr) {
      negativeread_chooser->IncreaseItemCountBy(delta);
    }
    if (scan_chooser != nullptr) {
      scan_chooser->IncreaseItemCountBy(delta);
    }
    if (update_chooser != nullptr) {
      update_chooser->IncreaseItemCountBy(delta);
    }
    /////////////////////////////////
    if (delete_chooser != nullptr) {
      delete_chooser->IncreaseItemCountBy(delta);
    }
    /////////////////////////////////
  }

  PhaseID phase_id;

  size_t num_inserts, num_inserts_left;
  size_t num_requests, num_requests_left;
  size_t num_deletes, num_deletes_left;   ///////////////////////////

  uint32_t read_thres, rmw_thres, negativeread_thres, scan_thres, update_thres;
  uint32_t delete_thres;              //////////////////////////////
  size_t max_scan_length;
  std::unique_ptr<Chooser> read_chooser;
  std::unique_ptr<Chooser> rmw_chooser;
  std::unique_ptr<Chooser> negativeread_chooser;
  std::unique_ptr<Chooser> scan_chooser;
  std::unique_ptr<Chooser> scan_length_chooser;
  std::unique_ptr<Chooser> update_chooser;
  std::unique_ptr<Chooser> delete_chooser;      ///////////////////////////
};

}  // namespace gen
}  // namespace ycsbr
