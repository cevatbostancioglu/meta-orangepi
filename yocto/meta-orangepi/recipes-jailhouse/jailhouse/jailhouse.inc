#
# Jailhouse, a Linux-based partitioning hypervisor
#
# Copyright (c) Siemens AG, 2018
#
# Authors:
#  Jan Kiszka <jan.kiszka@siemens.com>
#
# SPDX-License-Identifier: MIT
#

DESCRIPTION = "Jailhouse partitioning hypervisor"

SUMMARY = "Jailhouse"
HOMEPAGE = "https://github.com/siemens/jailhouse"
SECTION = "jailhouse"

LICENSE = "GPL-2.0 & BSD-2-clause"

DEPENDS = "virtual/kernel make-native python-mako-native dtc-native python-mako"

inherit module pythonnative bash-completion kernel-arch

LIC_FILES_CHKSUM = "file://COPYING;md5=9fa7f895f96bde2d47fd5b7d95b6ba4d"

S = "${WORKDIR}/git"

JH_DATADIR ?= "${datadir}/jailhouse"
CELL_DIR ?= "${JH_DATADIR}/cells"
CELLCONF_DIR ?= "${JH_DATADIR}/configs"
INMATES_DIR ?= "${JH_DATADIR}/inmates"

JH_CONFIG ?= "${S}/ci/jailhouse-config-x86.h"
JH_CONFIG_x86 ?= "${S}/ci/jailhouse-config-x86.h"
JH_CONFIG_x86-64 ?= "${S}/ci/jailhouse-config-x86.h"
JH_CONFIG_arm ?= "${S}/ci/jailhouse-config-banana-pi.h"

SRC_URI = " \
    git://github.com/siemens/jailhouse;branch=next;protocol=git \
    file://0001-fix-makefiles.patch \
"

DEFAULT_PREFERENCE = "-1"

CELLS = ""

do_configure() {
   if [ -d ${STAGING_DIR_HOST}${CELLCONF_DIR} ]; 
   then
      cp "${STAGING_DIR_HOST}/${CELLCONF_DIR}/"*.c ${S}/configs/
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
   oe_runmake V=0 \
        CROSS_COMPILE=${TARGET_PREFIX} \
        KDIR=${STAGING_KERNEL_BUILDDIR}

   cd ${TOOLS_SRC_DIR}
   export JAILHOUSE_VERSION=$(cat ../VERSION)
   oe_runmake V=0 \
   	       CFLAGS="${USER_SPACE_CFLAGS}" \
               src=${TOOLS_SRC_DIR} obj=${TOOLS_OBJ_DIR} \
	       ${TOOLS_OBJ_DIR}/jailhouse-config-collect ${TOOLS_OBJ_DIR}/jailhouse
}

do_install() {
    oe_runmake \
	       CROSS_COMPILE=${TARGET_PREFIX} \
	       KDIR=${STAGING_KERNEL_BUILDDIR} \
	       DESTDIR=${D} install

    install -d ${D}${CELL_DIR}
    install ${B}/configs/*.cell ${D}${CELL_DIR}/

    install -d ${D}${INMATES_DIR}
    install ${B}/inmates/demos/${ARCH}/*.bin ${D}${INMATES_DIR}
}

PACKAGE_BEFORE_PN = "kernel-module-jailhouse"
FILES_${PN} = "${base_libdir}/firmware ${libexecdir} ${sbindir} ${JH_DATADIR}"

KERNEL_MODULE_AUTOLOAD += "jailhouse"

python __anonymous () {
    # Setup DEPENDS and RDEPENDS to included cells"
    cells = d.getVar('CELLS', True) or ""
    for cell in cells.split():
        d.appendVar('DEPENDS', ' ' + cell)
        d.appendVar('RDEPENDS_${PN}', ' ' + cell)
}
