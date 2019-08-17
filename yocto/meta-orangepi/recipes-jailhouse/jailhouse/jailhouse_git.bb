require jailhouse.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-git:"

SRC_URI = "git://github.com/siemens/jailhouse.git;protocol=git \
	  file://no-kbuild-of-tools.patch \
	  file://tools-makefile.patch \
          file://tools-makefile-man-pages.patch"


#SRCREV = "${AUTOREV}"
SRCREV = "f596aa7355bc2134650544bdf1e13f1f55d3f2fc"
PV = "0.10-git${SRCPV}"

CELLS = ""

#DEFAULT_PREFERENCE = "-1"
