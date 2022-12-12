/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "radio_config_zmq_validator.h"
#include "fmt/format.h"
#include <regex>

using namespace srsgnb;

namespace {

bool validate_clock_sources(const radio_configuration::clock_sources& sources)
{
  if (sources.clock != radio_configuration::clock_sources::source::DEFAULT) {
    fmt::print("Only 'default' clock source is available.\n");
    return false;
  }

  if (sources.sync != radio_configuration::clock_sources::source::DEFAULT) {
    fmt::print("Only 'default' sync source is available.\n");
    return false;
  }

  return true;
}

bool validate_lo_freq(const radio_configuration::lo_frequency& lo_freq)
{
  if (!std::isnormal(lo_freq.center_frequency_hz)) {
    fmt::print("The center frequency must be non-zero, NAN nor infinite.\n");
    return false;
  }

  if (lo_freq.lo_frequency_hz != 0.0) {
    fmt::print("The custom LO frequency is not currently supported.\n");
    return false;
  }

  return true;
}

bool validate_channel_args(const std::string& channel_args)
{
  std::cmatch cmatch;
  std::regex  exp_tcp_socket(R"(^tcp:\/\/((([a-zA-Z0-9])\.*)*([a-zA-Z0-9])*|\*):[0-9]*$)");
  std::regex_match(channel_args.c_str(), cmatch, exp_tcp_socket);

  if (cmatch.empty()) {
    fmt::print("Channel arguments {} does not describe a valid TCP socket.\n", channel_args);
    return false;
  }

  return true;
}

bool validate_channel(const radio_configuration::channel& channel)
{
  if (!validate_lo_freq(channel.freq)) {
    return false;
  }

  if (std::isnan(channel.gain_dB) || std::isinf(channel.gain_dB)) {
    fmt::print("Channel gain must not be NAN nor infinite.\n");
    return false;
  }

  if (!validate_channel_args(channel.args)) {
    return false;
  }

  return true;
}

bool validate_stream(const radio_configuration::stream& stream)
{
  if (stream.channels.empty()) {
    fmt::print("Streams must contain at least one channel.\n");
    return false;
  }

  for (const radio_configuration::channel& channel : stream.channels) {
    if (!validate_channel(channel)) {
      return false;
    }
  }

  if (!stream.args.empty()) {
    fmt::print("Stream arguments are not currently supported.\n");
    return false;
  }

  return true;
}

bool validate_sampling_rate(double sampling_rate)
{
  if (!std::isnormal(sampling_rate)) {
    fmt::print("The sampling rate must be non-zero, NAN nor infinite.\n");
    return false;
  }

  if (sampling_rate < 0.0) {
    fmt::print("The sampling rate must be greater than zero.\n");
    return false;
  }

  return true;
}

bool validate_otw_format(radio_configuration::over_the_wire_format otw_format)
{
  if (otw_format != radio_configuration::over_the_wire_format::DEFAULT) {
    fmt::print("Only default OTW format is currently supported.\n");
    return false;
  }

  return true;
}

bool validate_radio_args(const std::string& radio_args)
{
  if (!radio_args.empty()) {
    fmt::print("Radio general arguments are not currently supported.\n");
    return false;
  }

  return true;
}

bool validate_log_level(std::string log_level)
{
  std::transform(log_level.begin(), log_level.end(), log_level.begin(), ::toupper);

  srslog::basic_levels level = srslog::str_to_basic_level(log_level);

  if (log_level != srslog::basic_level_to_string(level)) {
    fmt::print("Log level {} does not correspond to an actual logger level.\n", log_level);
    return false;
  }

  return true;
}

} // namespace

bool radio_config_zmq_config_validator::is_configuration_valid(const radio_configuration::radio& config) const
{
  if (!validate_clock_sources(config.clock)) {
    return false;
  }

  if (config.tx_streams.empty()) {
    fmt::print("At least one transmit stream must be available.\n");
    return false;
  }

  for (const radio_configuration::stream& tx_stream : config.tx_streams) {
    if (!validate_stream(tx_stream)) {
      return false;
    }
  }

  if (config.rx_streams.empty()) {
    fmt::print("At least one receive stream must be available.\n");
    return false;
  }

  for (const radio_configuration::stream& rx_stream : config.rx_streams) {
    if (!validate_stream(rx_stream)) {
      return false;
    }
  }

  if (!validate_sampling_rate(config.sampling_rate_hz)) {
    return false;
  }

  if (!validate_otw_format(config.otw_format)) {
    return false;
  }

  if (!validate_radio_args(config.args)) {
    return false;
  }

  if (!validate_log_level(config.log_level)) {
    return false;
  }

  return true;
}
