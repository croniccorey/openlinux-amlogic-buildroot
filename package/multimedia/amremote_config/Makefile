FILE_DIR=amremote_config


DEPENDS=

CONFIG_FLAGS=

TAR_DIR=${PKG_BUILD_DIR}/${FILE_DIR}

P_FILE_F=${PKG_DIR}/${P_FILE}
BUILD_DIR=${PKG_BUILD_DIR}/${FILE_DIR}

.PHONY:configure	all config  before_cmd install clean distclean


all:before_cmd config  
	make -C ${BUILD_DIR} CC=${HOST_GCC}
	make install
configure:config


config:get_file

get_file:
	@if [ ! -d "${TAR_DIR}" ]; then \
		mkdir ${TAR_DIR}; \
	fi;
	cp -rf ${PKG_TOP_DIR}/${FILE_DIR}/src/* ${PKG_BUILD_DIR}/${FILE_DIR}
	

install:
	@if [ ! -d "${PREFIX}/bin" ]; then\
		mkdir -p ${PREFIX}/bin  ; \
	fi;
	cp $(BUILD_DIR)/amremote_config  ${PREFIX}/bin; 
	@if [ ! -d "${PREFIX}/etc" ]; then\
		mkdir -p ${PREFIX}/etc  ; \
	fi;
	cp $(BUILD_DIR)/remote.conf  ${PREFIX}/etc; 
	

distclean:clean

clean:
	-rm ${PREFIX}/bin/amremote_config -rf
	 -rm $(BUILD_DIR)/*.o -rf 


before_cmd:depends

depends:
	@if [   "${DEPENDS}" != "" ]; then \
                cd ${PKG_TOP_DIR};make lib_depends TOP_DEPENDS="${DEPENDS}";    \
        fi
#include $(MAKEFILES_DIR)/package.rules

