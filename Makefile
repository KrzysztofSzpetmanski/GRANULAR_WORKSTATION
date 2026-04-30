RACK_DIR ?= ../Rack-SDK
ifeq ("$(wildcard $(RACK_DIR)/plugin.mk)","")
RACK_DIR := /Users/lazuli/Documents/PROGRAMMING/TEENSY/KSZ_TEENSY_PLATFORMIO/Teensy_Chord_Gen/Rack-SDK
endif

PLUGIN_SLUG := GRANULAR_WORKSTATION
LOCAL_RACK_PLUGIN_DIR ?= $(HOME)/Library/Application Support/Rack2/plugins-mac-arm64
BIG_MAC_MOUNT_DIR ?= /Volumes/music
BIG_MAC_RACK_SUBDIR ?= Library/Application Support/Rack2/plugins-mac-arm64

LOCAL_DEPLOY_DIR := $(LOCAL_RACK_PLUGIN_DIR)/$(PLUGIN_SLUG)
BIG_MAC_DEPLOY_DIR := $(BIG_MAC_MOUNT_DIR)/$(BIG_MAC_RACK_SUBDIR)/$(PLUGIN_SLUG)

FLAGS += -std=c++17

SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/dsp/*.cpp)

DISTRIBUTABLES += res

include $(RACK_DIR)/plugin.mk

.PHONY: deploy-local deploy-big deploy-both

deploy-local: dist
	rsync -av --delete dist/$(PLUGIN_SLUG)/ "$(LOCAL_DEPLOY_DIR)/"

deploy-big: dist
	rsync -av --delete dist/$(PLUGIN_SLUG)/ "$(BIG_MAC_DEPLOY_DIR)/"

deploy-both: dist
	rsync -av --delete dist/$(PLUGIN_SLUG)/ "$(LOCAL_DEPLOY_DIR)/"
	rsync -av --delete dist/$(PLUGIN_SLUG)/ "$(BIG_MAC_DEPLOY_DIR)/"
