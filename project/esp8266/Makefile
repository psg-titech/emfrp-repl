#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := emfrp-repl-esp8266
PP := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
EXTRA_COMPONENT_DIRS := $(PP)/../../src
CFLAGS := -D__ESP_8266__=1 -std=c11

include $(IDF_PATH)/make/project.mk
#COMPONENT_INCLUDES += $(PP)/../../include
