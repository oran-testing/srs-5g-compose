/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/e1/common/e1_common.h"
#include "srsgnb/f1ap/cu_cp/f1ap_cu.h"
#include "srsgnb/f1u/cu_up/f1u_gateway.h"
#include "srsgnb/support/executors/task_executor.h"
#include "srsgnb/support/io_broker/io_broker.h"

namespace srsgnb {
namespace srs_cu_up {

/// Configuration passed to CU-UP.
struct cu_up_configuration {
  task_executor*       cu_up_executor = nullptr;
  e1_message_notifier* e1_notifier    = nullptr; ///< Callback for incoming E1 messages.
  f1u_cu_up_gateway*   f1u_gateway    = nullptr;
  io_broker*           epoll_broker   = nullptr; ///< IO broker to receive messages from a network gateway
  // TODO: Refactor to use UPF IP that we get from E1
  std::string upf_addr      = "0.0.0.0";   ///< IP address of UPF for NG-U connection.
  std::string gtp_bind_addr = "127.0.1.1"; ///< Local IP address to bind for GTP connection.

  unsigned    cu_up_id   = 0;
  std::string cu_up_name = "srs_cu_up_01";
  std::string plmn; /// Full PLMN as string (without possible filler digit) e.g. "00101"
};

} // namespace srs_cu_up
} // namespace srsgnb
