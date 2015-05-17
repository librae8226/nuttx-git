// Select comparison method (default is "1"):
// NO_COMPARE 0
// COMPARE_FAST_MODIFIED 1 // Automatically detect fastest method. Only check sectors that needed to be reprogammed
// COMPARE_CRC_MODIFIED 2 // Compare via CRC (much faster than read back). Only check sectors that needed to be reprogammed
// COMPARE_MEM_MODIFIED 3 // Compare by explicitly reading back memory contents. Only check sectors that needed to be reprogammed
//exec SetSkipProgOnCRCMatch=3

// Select verification method:
// NO_VERIFY 0
// VERIFY_FAST_MODIFIED 1 // Automatically detect fastest method. Only check sectors that needed to be reprogammed
// VERIFY_CRC_MODIFIED 2 // Verify via CRC (much faster than read back). Only check sectors that needed to be reprogammed
// VERIFY_MEM_MODIFIED 3 // Verify by explicitly reading back memory contents. Only check sectors that needed to be reprogammed
// VERIFY_FAST_ALL 4 // Automatically detect fastest method
// VERIFY_CRC_ALL 5 // Verify via CRC (much faster than read back)
// VERIFY_MEM_ALL 6 // Verify via read back
// VERIFY_CHK_MODIFIED 7 // Verify via simple checksum (even faster than via CRC). Only check sectors that needed to be reprogammed
// VERIFY_CHK_ALL 8 // Verify via simple checksum (even faster than via CRC).
//exec SetVerifyDownload=6

device STM32f107VC
si 1
speed 4000
r
h
loadbin nuttx.bin,0x08010000
r
exit
