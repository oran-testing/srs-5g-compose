/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "initial_cu_up_processor_setup_routine.h"

using namespace srsran;
using namespace srs_cu_cp;

initial_cu_up_processor_setup_routine::initial_cu_up_processor_setup_routine(
    cu_up_processor_context&                   context_,
    cu_up_processor_e1ap_control_notifier&     e1ap_conn_notifier_,
    cu_up_processor_cu_up_management_notifier& cu_cp_notifier_) :
  context(context_), e1ap_conn_notifier(e1ap_conn_notifier_), cu_cp_notifier(cu_cp_notifier_)
{
}

void initial_cu_up_processor_setup_routine::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  // Initiate CU-CP E1 Setup.
  CORO_AWAIT_VALUE(response, start_cu_cp_e1_setup_request());

  // Handle CU-CP E1 setup result.
  if (response.success) {
    // In case of CU-CP E1 Setup Response.
    handle_cu_cp_e1_setup_response(response);
  } else {
    cu_cp_notifier.on_cu_up_remove_request_received(context.cu_up_index);
  }

  CORO_RETURN();
}

async_task<cu_cp_e1_setup_response> initial_cu_up_processor_setup_routine::start_cu_cp_e1_setup_request()
{
  // Prepare request to send to CU-CP E1 Setup Request message.
  cu_cp_e1_setup_request request = {};
  request.gnb_cu_cp_name         = context.cu_cp_name;

  // Initiate CU-CP E1 Setup Request.
  return e1ap_conn_notifier.on_cu_cp_e1_setup_request(request);
}

void initial_cu_up_processor_setup_routine::handle_cu_cp_e1_setup_response(const cu_cp_e1_setup_response& resp)
{
  if (resp.gnb_cu_up_name.has_value()) {
    context.cu_up_name = resp.gnb_cu_up_name.value();
  }
  context.id = resp.gnb_cu_up_id.value();

  // TODO: handle response
}
