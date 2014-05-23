#############################################################
#
# amremote_config
#
#############################################################
AMREMOTE_CONFIG_VERSION:=0.9.9
AMREMOTE_CONFIG_DIR=$(BUILD_DIR)/amremote_config
AMREMOTE_CONFIG_SOURCE=${AMREMOTE_CONFIG-D}
AMREMOTE_CONFIG_SITE=.

export PREFIX=$(TARGET_DIR)/usr
export STAGING=$(STAGING_DIR)/usr

AMREMOTE_CONFIG_TARGET_BINARY=$(PREFIX)/bin/amremote_config

$(AMREMOTE_CONFIG_DIR)/.unpacked:
	-rm -rf $(AMREMOTE_CONFIG_DIR)
	mkdir -p $(AMREMOTE_CONFIG_DIR)
	cp -arf ./package/multimedia/amremote_config/src/* $(AMREMOTE_CONFIG_DIR)
	touch $@

$(AMREMOTE_CONFIG_TARGET_BINARY): $(AMREMOTE_CONFIG_DIR)/.unpacked
	$(MAKE) CC=$(TARGET_CC) -C $(AMREMOTE_CONFIG_DIR)
	install -m 755 $(AMREMOTE_CONFIG_DIR)/amremote_config $(AMREMOTE_CONFIG_TARGET_BINARY)

amremote_config: $(AMREMOTE_CONFIG_TARGET_BINARY)

amremote_config-clean:
	-$(MAKE) -C $(AMREMOTE_CONFIG_DIR) clean

amremote_config-dirclean:
	rm -rf $(AMREMOTE_CONFIG_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_AMREMOTE_CONFIG),y)
TARGETS+=amremote_config
endif
