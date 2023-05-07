#!/bin/bash
g++ main.cpp -o alert_service `pkg-config --cflags --libs gtkmm-3.0 gstreamer-1.0 gstreamermm-1.0 libcurl jsoncpp` -pthread -std=c++11
