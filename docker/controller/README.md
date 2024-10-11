# ORAN srsRAN gNB controller

## Overview

Our gNB controller software manages four main docker containers:
- Open5GS Core Network
- srsRAN generational Node B
- Python Metrics Server
- InfluxDB

## Message structure

Each message passed from UE to gNB controllers is as follows:

{
  "target": service to target (str)
  "action": action to perform (start | stop)
  "port": OPTIONAL port to use (int)
}

