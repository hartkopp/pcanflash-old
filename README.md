# pcanflash
Linux flash tool for PCAN Routers

This tool can be used to flash PEAK System PCAN Routers as an alternative to the PCAN-Flash Windows application.

As it is possible to build firmware images for the PCAN Routers with Linux this pcanflash tool was the missing piece to provide a complete toolchain for Linux environments:
http://www.peak-system.com/forum/viewtopic.php?f=34&t=1112

This programm is currently tested with the PCAN Router IPEH-002210 with the binary files from the 1_ROUTING example.

See announcement at http://www.peak-system.com/forum/viewtopic.php?f=34&t=1818

So this software is at least BETA! Stay tuned :-)

# REMARK

As there's no flow control when a flash block is transferred the tx-queue-len should be extendend:

E.g.

ip link set can0 up type can bitrate 500000

ip link set can0 txqueuelen 500
