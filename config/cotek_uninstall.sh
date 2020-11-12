#!/bin/bash

# stop service
echo "123" | sudo -S service cotek_slam stop

# 卸载自启动
rosrun robot_upstart uninstall cotek_slam

# 卸载deb 文件
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-embedded
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-avoid
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-navigation
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-action
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-diagnostic
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-reflector-localizer
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-calibration
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-decision-maker
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-communicate
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-common
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-msgs
echo "123" | sudo -S dpkg --purge ros-kinetic-cotek-cmake