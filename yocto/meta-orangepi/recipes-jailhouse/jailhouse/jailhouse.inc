SUMMARY = "Jailhouse"
HOMEPAGE = "https://github.com/siemens/jailhouse"
SECTION = "jailhouse"
LICENSE = "GPL-2.0 & BSD-2-Clause"

LIC_FILES_CHKSUM = "file://COPYING;md5=9fa7f895f96bde2d47fd5b7d95b6ba4d \
                 file://tools/root-cell-config.c.tmpl;beginline=6;endline=33;md5=2825581c1666c44a17955dc574cfbfb3 \
                 file://include/jailhouse/hypercall.h;beginline=9;endline=36;md5=2825581c1666c44a17955dc574cfbfb3 \
                 file://include/jailhouse/cell-config.h;beginline=9;endline=36;md5=2825581c1666c44a17955dc574cfbfb3 \
                 file://include/arch/arm/asm/jailhouse_hypercall.h;beginline=9;endline=36;md5=2825581c1666c44a17955dc574cfbfb3 \
                 file://include/arch/arm64/asm/jailhouse_hypercall.h;beginline=9;endline=36;md5=2825581c1666c44a17955dc574cfbfb3 \
                 file://include/arch/x86/asm/jailhouse_hypercall.h;beginline=9;endline=36;md5=2825581c1666c44a17955dc574cfbfb3 \
                 file://driver/jailhouse.h;beginline=9;endline=36;md5=2825581c1666c44a17955dc574cfbfb3"

DEPENDS = "virtual/kernel make-native \
        python-mako-native python-mako \
        dtc-native "

S ="${WORKDIR}/git"

require jailhouse-defs.inc
inherit module pythonnative bash-completion setuptools kernel-arch

JH_CONFIG ?= ""
JH_CONFIG_x86 ?= "${S}/ci/jailhouse-config-x86.h"
JH_CONFIG_x86-64 ?= "${S}/ci/jailhouse-config-x86.h"
JH_CONFIG_arm ?= "${S}/ci/jailhouse-config-banana-pi.h"

do_configure() {
   if [ -d ${STAGING_DIR_HOST}/${CELLCONF_DIR} ]; 
   then
      cp "${STAGING_DIR_HOST}/${CELLCONF_DIR}/"*.c ${S}/configs/${ARCH}/
   fi

   if [ ! -z ${JH_CONFIG} ];
   then
      cp -av ${JH_CONFIG} ${S}/include/jailhouse/config.h
   fi
}

USER_SPACE_CFLAGS = '${CFLAGS} -DLIBEXECDIR=\\\"${libexecdir}\\\" \
		  -DJAILHOUSE_VERSION=\\\"$JAILHOUSE_VERSION\\\" \
		  -Wall -Wextra -Wmissing-declarations -Wmissing-prototypes -Werror \
		  -I../driver'

TOOLS_SRC_DIR = "${S}/tools"
TOOLS_OBJ_DIR = "${S}/tools"

do_compile() {
   unset LDFLAGS
   oe_runmake V=0 CC="${CC}" \
        CROSS_COMPILE=${TARGET_PREFIX} \
        KDIR=${STAGING_KERNEL_BUILDDIR}

   distutils_do_compile

   cd ${TOOLS_SRC_DIR}
   export JAILHOUSE_VERSION=$(cat ../VERSION)
   oe_runmake V=1 \
   	       CFLAGS="${USER_SPACE_CFLAGS}" \
               src=${TOOLS_SRC_DIR} obj=${TOOLS_OBJ_DIR} \
                ${TOOLS_OBJ_DIR}/jailhouse-cell-linux \
	        ${TOOLS_OBJ_DIR}/jailhouse-cell-stats \
	       ${TOOLS_OBJ_DIR}/jailhouse-config-collect \
               ${TOOLS_OBJ_DIR}/jailhouse.8 ${TOOLS_OBJ_DIR}/jailhouse-cell.8 \
               ${TOOLS_OBJ_DIR}/jailhouse-enable.8 \
               ${TOOLS_OBJ_DIR}/jailhouse
}

do_install() {
    oe_runmake V=1 \
	       LDFLAGS="" \
	       CC="${CC}" \
	       CROSS_COMPILE=${TARGET_PREFIX} \
	       KDIR=${STAGING_KERNEL_BUILDDIR} \
	       DESTDIR=${D} install

    distutils_do_install

    install -d ${D}${CELL_DIR}
    install ${B}/configs/${ARCH}/*.cell ${D}${CELL_DIR}/

    install -d ${D}${INMATES_DIR}
    install -d ${D}${INMATES_DIR}/tools/${ARCH}
    install ${B}/inmates/demos/${ARCH}/*.bin ${D}${INMATES_DIR}

    install -d ${D}${DTBS_DIR}
    install ${B}/configs/${ARCH}/dts/*.dtb ${D}${DTBS_DIR}/

    install -d ${D}${JH_DATADIR}/tools
    install ${B}/tools/jailhouse-cell-linux ${D}${JH_DATADIR}/tools
    install ${B}/tools/jailhouse-cell-stats ${D}${JH_DATADIR}/tools
    install ${B}/tools/jailhouse-config-collect ${D}${JH_DATADIR}/tools
    install ${B}/tools/jailhouse-config-create ${D}${JH_DATADIR}/tools
    install ${B}/tools/jailhouse-gcov-extract ${D}${JH_DATADIR}/tools
    install ${B}/tools/jailhouse-hardware-check ${D}${JH_DATADIR}/tools
    install ${B}/inmates/tools/${ARCH}/linux-loader.bin ${D}${INMATES_DIR}/tools/${ARCH}

    install -d ${D}${PYTHON_SITEPACKAGES_DIR}/pyjailhouse
    install -m 0644 ${B}/build${base_libdir}/pyjailhouse/*.py ${D}${PYTHON_SITEPACKAGES_DIR}/pyjailhouse
}

PACKAGE_BEFORE_PN = "kernel-module-jailhouse"
FILES_${PN} += "${base_libdir}/firmware ${libexecdir} ${sbindir} ${JH_DATADIR}"

#KERNEL_MODULE_AUTOLOAD += "jailhouse"
INSANE_SKIP_${PN} = "ldflags"

python __anonymous () {
    # Setup DEPENDS and RDEPENDS to included cells"
    cells = d.getVar('CELLS', True) or ""
    for cell in cells.split():
        d.appendVar('DEPENDS', ' ' + cell)
        d.appendVar('RDEPENDS_jailhouse', ' ' + cell)
}
