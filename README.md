# pcanflash
Linux flash tool for PCAN Routers

This tool can be used to flash PEAK System PCAN Routers as an alternative to the PCAN-Flash Windows application.

As it is possible to build firmware images for the PCAN Routers with Linux this pcanflash tool was the missing piece to provide a complete toolchain for Linux environments:
http://www.peak-system.com/forum/viewtopic.php?f=34&t=1112

See announcement at http://www.peak-system.com/forum/viewtopic.php?f=34&t=1818

This programm is currently tested with the PCAN Router (pro) and PCAN Router DR. Other PCAN hardware may follow soon. Stay tuned :-)

# REMARK

As there's no flow control when a flash block is transferred the tx-queue-len should be extendend. The default queue length for Linux is 10 frames - the PEAK Linux driver v8.1 sets the queue length to 50 frames.

A tx queue length of 500 CAN frames is recommended to run 'pcanflash' which can be set by the 'ip' tool from the iproute2 package or by sysfs:

- ip link set can0 txqueuelen 500

- echo 500 > /sys/class/net/can0/tx_queue_len

E.g.

ip link set can0 up type can bitrate 500000

ip link set can0 txqueuelen 500
