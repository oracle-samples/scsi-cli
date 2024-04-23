# Copyright (c) 2024, Oracle and/or its affiliates.
#
# The Universal Permissive License (UPL), Version 1.0
#
# Subject to the condition set forth below, permission is hereby granted to any
# person obtaining a copy of this software, associated documentation and/or data
# (collectively the "Software"), free of charge and under any and all copyright
# rights in the Software, and any and all patent rights owned or freely
# licensable by each licensor hereunder covering either (i) the unmodified
# Software as contributed to or provided by such licensor, or (ii) the Larger
# Works (as defined below), to deal in both
#
# (a) the Software, and
# (b) any piece of software and/or hardware listed in the
#     lrgrwrks.txt file if one is included with the Software (each a "Larger
#     Work" to which the Software is contributed by such licensors),
#
# without restriction, including without limitation the rights to copy, create
# derivative works of, display, perform, and distribute the Software and make,
# use, sell, offer for sale, import, export, have made, and have sold the
# Software and the Larger Work(s), and to sublicense the foregoing rights on
# either these or other terms.
#
# This license is subject to the following condition:
# The above copyright notice and either this complete permission notice or at
# a minimum a reference to the UPL must be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

TARGET = scsi-cli
VERSION=$(shell grep 'VERSION' scsi.h | sed s/\"//g | awk '{print($3)}')
PKG_CONFIG ?= pkg-config
LIBS = -lm
CC = gcc
CFLAGS ?= -O2 -Wall -Wextra -Werror -g
DEST_DIR ?=
SBIN_DIR = /sbin
ETC_DIR = /etc

UDEVDIR := $(shell $(PKG_CONFIG) --variable=udevdir udev 2>/dev/null)

ifndef UDEVDIR
 UDEVDIR = /lib/udev
endif

RULES_DIR ?= $(ETC_DIR)/udev/rules.d
SYSTEMD_DIR =/lib/systemd/system
INSTALL	= /usr/bin/install
MAN_DIR = /usr/share/man
INSTALL_DIR = /usr/local/bin

.PHONY: default all clean

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

$(RULES_GEN): %.rules: %.rules.template
	$(SED) -e 's:@SBIN_DIR@:$(SBIN_DIR):' $? > $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

default: help

all: clean
	@echo ''
	@echo ' Building $(TARGET) '
	@echo ' ===================='
	@echo ''
	$(MAKE) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

$(RULES_DEST): $(DEST_DIR)$(RULES_DIR)/%: %
	$(INSTALL) -m 644 $? $@
help:
	@echo ''
	@echo ' Posible targets: '
	@echo ' ================='
	@echo ''
	@echo ' all 		- Build all $(TARGET)'
	@echo ' clean 		- clean up object files'
	@echo ' udev		- Generate udev rule '
	@echo ' install_udev	- Install udev rules to $(RULES_DIR)'
	@echo ' install 	- Install $(TARGET) binary'
	@echo ' uninstall 	- Uninstall $(TARGET)'
	@echo ''

clean:
	@echo ''
	@echo ' Clean $(TARGET)  '
	@echo ' ================='
	@echo ''
	-rm -f *.o
	-rm -f $(TARGET)
	@echo ''

udev:	$(RULES_GEN)

install_udev: $(RULES_DEST)
#	$(INSTALL) -m 644 ../etc/udev/rules.d/99-scsi-alias.rules $(UDEV_DIR)

install: all
	@echo ''
	@echo ' Installing $(TARGET)  '
	@echo ' ======================'
	@echo ''
	$(INSTALL) -m 755  $(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo ''

uninstall:
	@echo ''
	@echo ' Remove $(TARGET)      '
	@echo ' ======================'
	@echo ''
	rm -f $(INSTALL_DIR)/$(TARGET)
	@echo ''
