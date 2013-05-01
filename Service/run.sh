#!/bin/bash

./dplService --port 22222 --num_threads 16 --instance_id Monitoring --dpl_config ./monitoringtest/dpl_config.dps.xml --monitoring_config ./cfg/Monitoring/monitoring.json
