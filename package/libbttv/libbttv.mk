#############################################################
#
# libbttv
#
#############################################################
LIBBTTV_VERSION:=1.0.0
LIBBTTV_DIR=$(BUILD_DIR)/libbttv-$(LIBBTTV_VERSION)
LIBBTTV_SOURCE=src
LIBBTTV_SITE=.
LIBBTTV_DEPENDENCIES= libxfer Qt
#CXXFLAGS=-I$(BUILD_DIR)/qt-everywhere-opensource-src-4.6.2/include

$(LIBBTTV_DIR)/.unpacked:
	mkdir -p $(LIBBTTV_DIR)
	cp -arf ./package/libbttv/src/* $(LIBBTTV_DIR)
	touch $(LIBBTTV_DIR)/.unpacked

$(LIBBTTV_DIR)/.configured: $(LIBBTTV_DIR)/.unpacked
	(cd $(LIBBTTV_DIR); rm -rf config.cache; \
	    $(TARGET_CONFIGURE_OPTS) \
	    $(TARGET_CONFIGURE_ARGS) \
	    LDFLAGS="-L$(TARGET_DIR)/usr/lib -L$(BUILD_DIR)/qt-everywhere-opensource-src-4.6.2/lib" \
	    CXXFLAGS="-I$(BUILD_DIR)/qt-everywhere-opensource-src-4.6.2/include -I$(BUILD_DIR)/qt-everywhere-opensource-src-4.6.2/include/QtCore" \
	    ./configure \
	    --target=$(GNU_TARGET_NAME) \
	    --host=$(GNU_TARGET_NAME) \
	    --build=$(GNU_HOST_NAME) \
	    --prefix=/usr \
	    --sysconfdir=/etc \
	)
	touch $(LIBBTTV_DIR)/.configured

$(LIBBTTV_DIR)/libbttv: $(LIBBTTV_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(LIBBTTV_DIR) 
	touch -c $(LIBBTTV_DIR)/libbttv

$(TARGET_DIR)/lib/libbttv: $(LIBBTTV_DIR)/libbttv
	mkdir -p $(STAGING_DIR)/usr/include/bttv
	cp $(LIBBTTV_DIR)/include/* $(STAGING_DIR)/usr/include/bttv
	cp -a $(LIBBTTV_DIR)/.libs/libbttv.so* $(TARGET_DIR)/usr/lib
	cp -a $(LIBBTTV_DIR)/.libs/libbttv.so* $(STAGING_DIR)/usr/lib
	touch -c $(TARGET_DIR)/bin/manager

libbttv: libxfer opengl qt $(TARGET_DIR)/lib/libbttv

libbttv-source: $(DL_DIR)/$(LIBBTTV_SOURCE)

libbttv-clean:
	-$(MAKE) -C $(LIBBTTV_DIR) clean

libbttv-dirclean:
	rm -rf $(LIBBTTV_DIR)


#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_LIBBTTV),y)
TARGETS+=libbttv
endif
