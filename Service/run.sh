#!/bin/bash

./dplService --instance_id Monitoring --port 22222 --num_threads 16 --dpl_config /tmp/adlearn/dpl_config.dps.xml --monitoring_config /var/txia12/tt/ALTOD_CR/lib/cpp/DataProxy/Service/cfg/Monitoring/monitoring.json 
