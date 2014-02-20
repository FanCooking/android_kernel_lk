# top level project rules for the msm8916 project
#
LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := msm8916

MODULES += app/aboot

DEBUG := 1

#DEFINES += WITH_DEBUG_DCC=1
DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1
DEFINES += DEVICE_TREE=1
#DEFINES += MMC_BOOT_BAM=1
#DEFINES += CRYPTO_BAM=1
DEFINES += SPMI_CORE_V2=1
DEFINES += ABOOT_IGNORE_BOOT_HEADER_ADDRS=1

DEFINES += ABOOT_FORCE_KERNEL_ADDR=0x80008000
DEFINES += ABOOT_FORCE_KERNEL64_ADDR=0x80080000
DEFINES += ABOOT_FORCE_RAMDISK_ADDR=0x82000000
DEFINES += ABOOT_FORCE_TAGS_ADDR=0x81E00000

#Disable thumb mode
ENABLE_THUMB := false

ENABLE_SDHCI_SUPPORT := 1

ifeq ($(ENABLE_SDHCI_SUPPORT),1)
DEFINES += MMC_SDHCI_SUPPORT=1
endif
