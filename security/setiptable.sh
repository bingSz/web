#!/bin/bash

iptables -A INPUT -p tcp --dport 79 -j DROP
iptables -A INPUT -p udp --dport 79 -j DROP
iptables -A INPUT -p tcp --dport 135 -j DROP
iptables -A INPUT -p udp --dport 135 -j DROP
iptables -A INPUT -p tcp --dport 137 -j DROP
iptables -A INPUT -p udp --dport 137 -j DROP
iptables -A INPUT -p tcp --dport 139 -j DROP
iptables -A INPUT -p udp --dport 139 -j DROP
iptables -A INPUT -p tcp --dport 161 -j DROP
iptables -A INPUT -p udp --dport 161 -j DROP
iptables -A INPUT -p tcp --dport 445 -j DROP
iptables -A INPUT -p udp --dport 445 -j DROP
iptables -A INPUT -p tcp --dport 593 -j DROP
iptables -A INPUT -p udp --dport 593 -j DROP
iptables -A INPUT -p tcp --dport 1025 -j DROP
iptables -A INPUT -p udp --dport 1025 -j DROP
iptables -A INPUT -p tcp --dport 3127 -j DROP
iptables -A INPUT -p udp --dport 3127 -j DROP
iptables -A INPUT -p tcp --dport 2475 -j DROP
iptables -A INPUT -p udp --dport 2475 -j DROP
iptables -A INPUT -p tcp --dport 3389 -j DROP
iptables -A INPUT -p udp --dport 3389 -j DROP
iptables -A INPUT -p tcp --dport 4899 -j DROP
iptables -A INPUT -p udp --dport 4899 -j DROP
iptables -A INPUT -p tcp --dport 6129 -j DROP
iptables -A INPUT -p udp --dport 6129 -j DROP
iptables -A INPUT -p tcp --dport 7626 -j DROP
iptables -A INPUT -p udp --dport 7626 -j DROP
iptables -A OUTPUT -p tcp --sport 2745 -j DROP
iptables -A OUTPUT -p udp --sport 2745 -j DROP
iptables -A OUTPUT -p tcp --sport 3127 -j DROP
iptables -A OUTPUT -p udp --sport 3127 -j DROP
iptables -A OUTPUT -p tcp --sport 6129 -j DROP
iptables -A OUTPUT -p udp --sport 6129 -j DROP