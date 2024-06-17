/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "lib/scheduler/logging/scheduler_metrics_handler.h"
#include "srsran/support/test_utils.h"
#include <gtest/gtest.h>

using namespace srsran;

class test_scheduler_ue_metrics_notifier : public scheduler_metrics_notifier
{
public:
  scheduler_cell_metrics last_report;

  void report_metrics(const scheduler_cell_metrics& metrics) override { last_report = metrics; }
};

class scheduler_metrics_handler_tester : public ::testing::Test
{
protected:
  scheduler_metrics_handler_tester(
      std::chrono::milliseconds period = std::chrono::milliseconds{test_rgen::uniform_int<unsigned>(2, 100)}) :
    report_period(period), metrics(period, metrics_notif)
  {
    metrics.handle_ue_creation(test_ue_index, to_rnti(0x4601), pci_t{0}, nof_prbs);
  }

  void run_slot(const sched_result& sched_res, std::chrono::microseconds latency = std::chrono::microseconds{0})
  {
    metrics.push_result(next_sl_tx, sched_res, latency);
    ++next_sl_tx;
    slot_count++;
  }

  void get_next_metric()
  {
    metrics_notif.last_report = {};
    sched_result sched_res;
    while (metrics_notif.last_report.ue_metrics.empty()) {
      run_slot(sched_res);
    }
  }

  std::chrono::milliseconds          report_period;
  test_scheduler_ue_metrics_notifier metrics_notif;
  scheduler_metrics_handler          metrics;
  du_ue_index_t test_ue_index = to_du_ue_index(test_rgen::uniform_int<unsigned>(0, MAX_NOF_DU_UES - 1));

  slot_point next_sl_tx{0, test_rgen::uniform_int<unsigned>(0, 10239)};
  unsigned   slot_count = 0;
  unsigned   nof_prbs   = 100;
};

TEST_F(scheduler_metrics_handler_tester, metrics_sent_with_defined_periodicity)
{
  unsigned nof_reports = test_rgen::uniform_int<unsigned>(1, 10);
  ASSERT_TRUE(metrics_notif.last_report.ue_metrics.empty());
  for (unsigned i = 0; i != nof_reports; ++i) {
    get_next_metric();
    ASSERT_EQ(metrics_notif.last_report.ue_metrics.size(), 1);
    ASSERT_EQ(slot_count, report_period.count() * (i + 1));

    ASSERT_EQ(metrics_notif.last_report.ue_metrics[0].rnti, to_rnti(0x4601));
  }
}

TEST_F(scheduler_metrics_handler_tester, when_no_events_took_place_then_metrics_are_zero)
{
  this->get_next_metric();
  scheduler_ue_metrics ue_metrics = metrics_notif.last_report.ue_metrics[0];

  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.dl_brate_kbps, 0);
  ASSERT_EQ(ue_metrics.ul_brate_kbps, 0);
  ASSERT_EQ(ue_metrics.dl_mcs, 0);
  ASSERT_EQ(ue_metrics.ul_mcs, 0);
  ASSERT_EQ(ue_metrics.dl_nof_ok, 0);
  ASSERT_EQ(ue_metrics.dl_nof_nok, 0);
  ASSERT_EQ(ue_metrics.ul_nof_ok, 0);
  ASSERT_EQ(ue_metrics.ul_nof_nok, 0);
  ASSERT_EQ(ue_metrics.pusch_snr_db, 0);
  ASSERT_EQ(ue_metrics.pusch_rsrp_db, -std::numeric_limits<float>::infinity());
  ASSERT_EQ(ue_metrics.pucch_snr_db, 0);
  ASSERT_EQ(ue_metrics.bsr, 0);

  // Check that the CQI and RI statistics have no observations.
  ASSERT_EQ(ue_metrics.cqi_stats.get_nof_observations(), 0);
  ASSERT_EQ(ue_metrics.ri_stats.get_nof_observations(), 0);
}

TEST_F(scheduler_metrics_handler_tester, compute_nof_dl_oks_and_noks)
{
  unsigned nof_acks  = test_rgen::uniform_int<unsigned>(1, 100);
  unsigned nof_nacks = test_rgen::uniform_int<unsigned>(1, 100);

  for (unsigned i = 0; i != nof_acks; ++i) {
    metrics.handle_dl_harq_ack(test_ue_index, true, units::bytes{1});
  }
  for (unsigned i = 0; i != nof_nacks; ++i) {
    metrics.handle_dl_harq_ack(test_ue_index, false, units::bytes{1});
  }

  this->get_next_metric();
  scheduler_ue_metrics ue_metrics = metrics_notif.last_report.ue_metrics[0];
  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.dl_nof_ok, nof_acks);
  ASSERT_EQ(ue_metrics.dl_nof_nok, nof_nacks);

  this->get_next_metric();
  ue_metrics = metrics_notif.last_report.ue_metrics[0];
  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.dl_nof_ok, 0) << "Nof DL OKs not reset";
  ASSERT_EQ(ue_metrics.dl_nof_nok, 0) << "Nof DL NOKs not reset";
}

TEST_F(scheduler_metrics_handler_tester, compute_nof_ul_oks_and_noks)
{
  unsigned nof_acks  = test_rgen::uniform_int<unsigned>(1, 100);
  unsigned nof_nacks = test_rgen::uniform_int<unsigned>(1, 100);

  ul_crc_pdu_indication crc_pdu;
  crc_pdu.rnti           = to_rnti(0x4601);
  crc_pdu.ue_index       = test_ue_index;
  crc_pdu.tb_crc_success = true;
  for (unsigned i = 0; i != nof_acks; ++i) {
    metrics.handle_crc_indication(crc_pdu, units::bytes{1});
  }
  crc_pdu.tb_crc_success = false;
  for (unsigned i = 0; i != nof_nacks; ++i) {
    metrics.handle_crc_indication(crc_pdu, units::bytes{1});
  }

  this->get_next_metric();
  scheduler_ue_metrics ue_metrics = metrics_notif.last_report.ue_metrics[0];
  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.pci, pci_t{0});
  ASSERT_EQ(ue_metrics.ul_nof_ok, nof_acks);
  ASSERT_EQ(ue_metrics.ul_nof_nok, nof_nacks);

  this->get_next_metric();
  ue_metrics = metrics_notif.last_report.ue_metrics[0];
  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.ul_nof_ok, 0) << "Nof DL OKs not reset";
  ASSERT_EQ(ue_metrics.ul_nof_nok, 0) << "Nof DL NOKs not reset";
}

TEST_F(scheduler_metrics_handler_tester, compute_mcs)
{
  sch_mcs_index dl_mcs{test_rgen::uniform_int<uint8_t>(1, 28)};
  sch_mcs_index ul_mcs{test_rgen::uniform_int<uint8_t>(1, 28)};

  sched_result  res;
  dl_msg_alloc& dl_msg  = res.dl.ue_grants.emplace_back();
  dl_msg.pdsch_cfg.rnti = to_rnti(0x4601);
  auto&          cw     = dl_msg.pdsch_cfg.codewords.emplace_back();
  ul_sched_info& ul_msg = res.ul.puschs.emplace_back();
  ul_msg.pusch_cfg.rnti = to_rnti(0x4601);

  for (unsigned i = 0; i != report_period.count(); ++i) {
    cw.mcs_index               = dl_mcs;
    ul_msg.pusch_cfg.mcs_index = ul_mcs;
    run_slot(res);
  }

  scheduler_ue_metrics ue_metrics = metrics_notif.last_report.ue_metrics[0];
  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.dl_mcs, dl_mcs);
  ASSERT_EQ(ue_metrics.ul_mcs, ul_mcs);

  this->get_next_metric();
  ue_metrics = metrics_notif.last_report.ue_metrics[0];
  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.dl_mcs, 0) << "DL MCS not reset";
  ASSERT_EQ(ue_metrics.ul_mcs, 0) << "UL MCS not reset";
}

TEST_F(scheduler_metrics_handler_tester, compute_bitrate)
{
  units::bytes dl_tbs{test_rgen::uniform_int<unsigned>(1, 1000)};
  units::bytes ul_tbs{test_rgen::uniform_int<unsigned>(1, 1000)};

  // Slot with bytes ACKed.
  metrics.handle_dl_harq_ack(test_ue_index, true, dl_tbs);
  ul_crc_pdu_indication crc_pdu;
  crc_pdu.rnti           = to_rnti(0x4601);
  crc_pdu.ue_index       = test_ue_index;
  crc_pdu.tb_crc_success = true;
  metrics.handle_crc_indication(crc_pdu, ul_tbs);

  this->get_next_metric();
  scheduler_ue_metrics ue_metrics = metrics_notif.last_report.ue_metrics[0];
  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.pci, pci_t{0});
  ASSERT_EQ(ue_metrics.dl_brate_kbps, static_cast<double>(dl_tbs.value() * 8) / report_period.count());
  ASSERT_EQ(ue_metrics.ul_brate_kbps, static_cast<double>(ul_tbs.value() * 8) / report_period.count());

  // Slot with no ACKs.
  this->get_next_metric();
  ue_metrics = metrics_notif.last_report.ue_metrics[0];
  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.dl_brate_kbps, 0) << "DL bitrate not reset";
  ASSERT_EQ(ue_metrics.ul_brate_kbps, 0) << "UL bitrate not reset";

  // Slot with bytes NACKed.
  metrics.handle_dl_harq_ack(test_ue_index, false, dl_tbs);
  crc_pdu.rnti           = to_rnti(0x4601);
  crc_pdu.ue_index       = test_ue_index;
  crc_pdu.tb_crc_success = false;
  metrics.handle_crc_indication(crc_pdu, ul_tbs);

  this->get_next_metric();
  ue_metrics = metrics_notif.last_report.ue_metrics[0];
  ASSERT_EQ(ue_metrics.rnti, to_rnti(0x4601));
  ASSERT_EQ(ue_metrics.dl_brate_kbps, 0) << "NACKs should not count for bitrate";
  ASSERT_EQ(ue_metrics.ul_brate_kbps, 0) << "NACKs should not count for bitrate";
}

TEST_F(scheduler_metrics_handler_tester, compute_latency_metric)
{
  using usecs                = std::chrono::microseconds;
  std::vector<usecs> samples = {usecs{10}, usecs{50}, usecs{150}, usecs{300}, usecs{500}};
  samples.resize(report_period.count());

  sched_result sched_res;
  for (unsigned i = 0; i != samples.size(); ++i) {
    ASSERT_TRUE(metrics_notif.last_report.ue_metrics.empty());
    this->run_slot(sched_res, samples[i]);
  }
  ASSERT_FALSE(metrics_notif.last_report.ue_metrics.empty());
  scheduler_cell_metrics cell_metrics = metrics_notif.last_report;

  unsigned expected_avg = std::accumulate(samples.begin(), samples.end(), usecs{0}).count() / report_period.count();
  ASSERT_EQ(cell_metrics.average_decision_latency.count(), expected_avg);

  for (unsigned i = 0; i != cell_metrics.latency_histogram.size(); ++i) {
    unsigned expected = std::count_if(samples.begin(), samples.end(), [i](usecs u) {
      unsigned bin = u.count() / scheduler_cell_metrics::nof_usec_per_bin;
      bin          = std::min(bin, scheduler_cell_metrics::latency_hist_bins - 1);
      return bin == i;
    });
    ASSERT_EQ(cell_metrics.latency_histogram[i], expected);
  }
}

TEST_F(scheduler_metrics_handler_tester, compute_error_indications)
{
  metrics.handle_error_indication();
  metrics.handle_error_indication();
  this->get_next_metric();
  ASSERT_EQ(metrics_notif.last_report.nof_error_indications, 2);
  this->get_next_metric();
  ASSERT_EQ(metrics_notif.last_report.nof_error_indications, 0);
}