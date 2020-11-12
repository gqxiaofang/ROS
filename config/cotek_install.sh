#!/bin/bash

# 安装deb 文件
echo "123" | sudo -S dpkg -i software/cotek_msgs_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_cmake_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_common_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_embedded_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_avoid_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_navigation_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_action_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_diagnostic_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_reflector_localizer_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_calibration_1.0.0.deb 
echo "123" | sudo -S dpkg -i software/cotek_decision_maker_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_communicate_1.0.0.deb
echo "123" | sudo -S dpkg -i software/cotek_pepperl_fuchs_r2000_1.0.0.deb

. /opt/ros/kinetic/setup.sh

# stop service
echo "123" | sudo -S service cotek_slam stop

# move can lib 
echo "123" | sudo -S cp  usb_can/libusbcan.so /lib/
echo "123" | sudo -S cp  usb_can/usbcan.rules /etc/udev/rules.d/
udevadm control --reload

# update robot_upstart
cd /opt/ros/kinetic/share/
rosrun robot_upstart uninstall cotek_slam
rosrun robot_upstart install --job cotek_slam --user cotek --logdir /home/cotek/smartGo_log/ cotek_common/launch/cotek_slam.launch
echo "123" | sudo -S systemctl daemon-reload

# time to excute task
TASK_COMMAND1="/bin/sh /home/cotek/config/log_manager/log_rotate.sh"
TASK_COMMAND2="echo "123" | sudo -S rm -rf /var/log/syslog*"
# add crontab Task
CRONTAB_TASK1="00 10 * * * ${TASK_COMMAND1}"
CRONTAB_TASK2="00 10 * * * ${TASK_COMMAND2}"
CRONTAB_TASK3="00 15 * * * ${TASK_COMMAND1}"
CRONTAB_TASK4="00 15 * * * ${TASK_COMMAND2}"
# reserve crontab
CRONTAB_BAK_FILE="/home/cotek/config/crontab_bak"

# creat crontab task
echo 'Create crontab task...'
crontab -l >${CRONTAB_BAK_FILE} 2>/dev/null
sed -i "/.*log_rotate.sh/d" ${CRONTAB_BAK_FILE} # if task already exists, not creat task
sed -i "/.*syslog/d" ${CRONTAB_BAK_FILE} # if task already exists, not creat task
echo "${CRONTAB_TASK1}" >>${CRONTAB_BAK_FILE}
echo "${CRONTAB_TASK2}" >>${CRONTAB_BAK_FILE}
echo "${CRONTAB_TASK3}" >>${CRONTAB_BAK_FILE}
echo "${CRONTAB_TASK4}" >>${CRONTAB_BAK_FILE}
crontab ${CRONTAB_BAK_FILE}

# reboot cron service
echo "123" | sudo -S /etc/init.d/cron restart
echo "123" | sudo -S service cron restart

echo "Finished !!!"