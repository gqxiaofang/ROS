// Copyright (c) 2014, Pepperl+Fuchs GmbH, Mannheim
// Copyright (c) 2014, Denis Dillenberger
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or
//   other materials provided with the distribution.
//
// * Neither the name of Pepperl+Fuchs nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "r2000_node.h"
#include <sensor_msgs/LaserScan.h>
#include <pepperl_fuchs_r2000/r2000_driver.h>

namespace pepperl_fuchs
{

//-----------------------------------------------------------------------------
R2000Node::R2000Node()
    : nh_("~"), is_time_synced_(false), timediff_(0.), is_connected_(false), driver_(nullptr)
{
    ros::NodeHandle nh;

    // Reading and checking parameters-
    //-------------------------------------------------------------------------
    nh_.param("frame_id", frame_id_, std::string("laser"));
    nh_.param("scanner_ip", scanner_ip_, std::string("192.168.0.10"));
    nh_.param("scan_frequency", scan_frequency_, 20.0f);
    nh_.param("samples_per_scan", samples_per_scan_, 4200);
    nh_.param("angle_min", angle_min_, -M_PI);
    nh_.param("angle_max", angle_max_, M_PI);

    // Declare publisher and create timer
    //-------------------------------------------------------------------------
    scan_publisher_ = nh.advertise<sensor_msgs::LaserScan>("scan", 20);
    cmd_subscriber_ = nh.subscribe("control_command", 5, &R2000Node::cmdMsgCallback, this);
    // get_scan_data_timer_ = nh_.createTimer(
    //     ros::Duration(1 / 1000),
    //     &R2000Node::getScanData,
    //     this);
}

//-----------------------------------------------------------------------------
bool R2000Node::connect()
{
    if (scanner_ip_ == "")
    {
        ROS_ERROR("IP of laser range finder not set!");
        return false;
    }

    if (nullptr != driver_)
    {
        delete driver_;
    }

    // Connecting to laser range finder
    //-------------------------------------------------------------------------
    driver_ = new R2000Driver();
    ROS_INFO_STREAM("Connecting to scanner at " << scanner_ip_ << " ... ");

    if (!driver_->connect(scanner_ip_, 80))
    {
        ROS_ERROR_STREAM("Connection to scanner at " << scanner_ip_ << " failed!");
        return false;
    }

    ROS_INFO_STREAM("Connected to scanner at " << scanner_ip_ << " ... ");

    // Setting, reading and displaying parameters
    //-------------------------------------------------------------------------
    if (!driver_->setScanFrequency(scan_frequency_))
    {
        ROS_WARN_STREAM("Set scan_frequency failed!" << scan_frequency_);
    }

    if (!driver_->setSamplesPerScan(samples_per_scan_))
    {
        ROS_WARN_STREAM("Set sample_per_scan failed!" << samples_per_scan_);
    }

    auto params = driver_->getParameters();

    ROS_INFO_STREAM("Current scanner settings:");
    ROS_INFO_STREAM("============================================================");

    for (const auto &p : params)
    {
        ROS_INFO_STREAM(p.first << " : " << p.second);
    }

    // Start capturing scanner data
    //-------------------------------------------------------------------------
    ROS_INFO_STREAM("Starting capturing: ");

    if (driver_->startCapturingUDP())
    {
        ROS_INFO_STREAM("OK");
    }
    else
    {
        ROS_ERROR_STREAM("FAILED!");
        return false;
    }

    is_connected_ = true;
    return true;
}

//-----------------------------------------------------------------------------
void R2000Node::getScanData()
{
    // ROS_INFO_STREAM("Get scan_data.");
    if (!driver_->isCapturing())
    {
        ROS_INFO_STREAM("ERROR: Laser range finder disconnected. Trying to reconnect...");

        while (!connect())
        {
            ROS_INFO_STREAM("ERROR: Reconnect failed. Trying again in 2 seconds...");
            usleep((2 * 1000000));
        }
    }

    auto scandata = driver_->getFullScan();

    if (scandata.amplitude_data.empty() || scandata.distance_data.empty())
    {
        // ROS_WARN_STREAM("No data...");
        return;
    }

    sensor_msgs::LaserScan scanmsg;
    scanmsg.header.frame_id = frame_id_;
    scanmsg.header.stamp = ros::Time::now() - ros::Duration(0.06);
    scanmsg.angle_min = angle_min_;
    scanmsg.angle_max = angle_max_;
    scanmsg.angle_increment = 2 * M_PI / float(scandata.distance_data.size());
    scanmsg.scan_time = 1 / std::atof(driver_->getParametersCached().at("scan_frequency").c_str());
    scanmsg.time_increment = scanmsg.scan_time / samples_per_scan_;
    scanmsg.range_min = std::atof(driver_->getParametersCached().at("radial_range_min").c_str());
    scanmsg.range_max = std::atof(driver_->getParametersCached().at("radial_range_max").c_str());

    if (scandata.distance_data.size() != scandata.amplitude_data.size())
    {
        ROS_ERROR("distance size doesn't match amplitude size!");
        return;
    }

    auto start = static_cast<std::size_t>(std::ceil((angle_min_ + M_PI) / scanmsg.angle_increment));
    auto end = static_cast<std::size_t>(std::floor((angle_max_ + M_PI) / scanmsg.angle_increment));

    scanmsg.ranges.resize(end - start + 1);
    scanmsg.intensities.resize(end - start + 1);

    for (std::size_t i = 0; i < end - start + 1; i++)
    {
        // auto j = i + (scandata.distance_data.size() - size) / 2;
        scanmsg.ranges[i] = float(scandata.distance_data[start + i]) / 1000.0f;
        scanmsg.intensities[i] = scandata.amplitude_data[start + i];
    }

    scan_publisher_.publish(scanmsg);
}

//-----------------------------------------------------------------------------
void R2000Node::cmdMsgCallback(const std_msgs::StringConstPtr &msg)
{
    const std::string &cmd = msg->data;
    static const std::string set_scan_frequency_cmd("set scan_frequency=");
    static const std::string set_samples_per_scan_cmd("set samples_per_scan=");

    // Setting of scan_frequency
    //-------------------------------------------------------------------------
    if (cmd.substr(0, set_scan_frequency_cmd.size()) == set_scan_frequency_cmd)
    {
        std::string value = cmd.substr(set_scan_frequency_cmd.size());
        int frequency = std::atoi(value.c_str());
        if (frequency >= 10 && frequency <= 50)
        {
            scan_frequency_ = frequency;
            driver_->setScanFrequency(frequency);
        }
    }

    // Setting of samples_per_scan
    //-------------------------------------------------------------------------
    if (cmd.substr(0, set_samples_per_scan_cmd.size()) == set_samples_per_scan_cmd)
    {
        std::string value = cmd.substr(set_samples_per_scan_cmd.size());
        int samples = std::atoi(value.c_str());
        if (samples >= 72 && samples <= 25200)
        {
            samples_per_scan_ = samples;
            driver_->setSamplesPerScan(samples);
        }
    }
}

//-----------------------------------------------------------------------------
} // namespace pepperl_fuchs

int main(int argc, char **argv)
{
    ros::init(argc, argv, "r2000_node", ros::init_options::AnonymousName);
    pepperl_fuchs::R2000Node r2000_node;

    while (ros::ok() && !r2000_node.connect())
    {
        ros::Duration(0.5).sleep();
    }

    while (ros::ok() && r2000_node.isConnected())
    {
        r2000_node.getScanData();
    }

    ros::spin();

    return 0;
}
