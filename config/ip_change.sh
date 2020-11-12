#!/bin/bash
# 用于更改网口2的ip, 执行完重启生效
echo "please input IPC ip:"
read IP
echo "please input IPC gateway:"
read GATEWAY
sleep 2

echo "# This file describes the network interfaces available on your system
# and how to activate them. For more information, see interfaces(5).

source /etc/network/interfaces.d/*

# The loopback network interface
auto lo
iface lo inet loopback

auto enp1s0
iface enp1s0 inet static
address 192.168.0.11
netmask 255.255.255.0
gateway 192.168.0.1

auto enp2s0
iface enp2s0 inet static
address $IP
netmask 255.255.255.0
gateway $GATEWAY

" > /etc/network/interfaces

echo "
############## ok! ################
"


