/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/ran/s_nssai.h"

namespace srsran {
/// O-RAN.WG3.E2SM-RC-R003-v3.00 Section 8.4.3.6
struct rrm_policy_member {
  std::string plmn_id;
  s_nssai_t   s_nssai;
};

struct rrm_policy_ratio_group {
  // used to identify the group to which the policy is applied.
  rrm_policy_member pol_member;
  // sets the minimum percentage of PRBs to be allocated to this group.
  int               min_PRB_policy_ratio;
  // sets the maximum percentage of PRBs to be allocated to this group.
  int               max_PRB_policy_ratio;
  // sets the percentage of PRBs to be allocated to this group.
  int               ded_PRB_policy_ratio;
};

} // namespace srsran
