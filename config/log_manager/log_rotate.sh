#!/bin/bash
echo "rotate smartGo_log"

. /opt/ros/kinetic/setup.sh
# get run_id of ROS if it is running, returns empty if not running
RUN_ID=$(rosparam get /run_id 2>/dev/null)

cd /home/cotek/smartGo_log
# delete logs  three days ago
for i in $(ls /home/cotek/smartGo_log | grep -v latest | grep -v cotek_slam.launch | grep -v cotek_slam.pid | grep -v cotek_slam.xacro); do
    if [ $i ] && [ $i != "$RUN_ID" ]; then
        cd /home/cotek/smartGo_log/$i
        for j in $(ls | find . -mtime +2) ]; do
            if [ "$j" != "." ] && [ "$j" != "]" ]; then
                echo "remove /home/cotek/smartGo_log/$i "
                rm -rf /home/cotek/smartGo_log/$i
                break
            fi
        done
    fi
done
# update log_dir 
# eg. 2019-11-4-15:33
cd /home/cotek/smartGo_log
RUN_ID=$(rosparam get /run_id 2>/dev/null)

for i in $(ls /home/cotek/smartGo_log | grep -v latest | grep -v cotek_slam.launch | grep -v cotek_slam.pid | grep -v cotek_slam.xacro); do
    echo $i
    if [ $i ] && [ $i != "$RUN_ID" ]; then
        file_name=$(ls -l $i --time-style=long-iso | awk '{print $6,$7;}' | sed -n '2p' | sed 's/ /-/')
        echo $file_name
        mv $i $file_name
    fi
done

echo "Finished !!!"
