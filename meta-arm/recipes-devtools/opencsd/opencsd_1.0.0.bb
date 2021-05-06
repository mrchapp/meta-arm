SUMMARY = "OpenCSD - An open source CoreSight(tm) Trace Decode library"
HOMEPAGE = "https://github.com/Linaro/OpenCSD"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=ad8cb685eb324d2fa2530b985a43f3e5"

SRC_URI = "git://github.com/Linaro/OpenCSD;protocol=https"
SRCREV = "01d44a34f8fc057f4b041c01f8d9502d77fe612f"

S = "${WORKDIR}/git"

COMPATIBLE_HOST = "(i.86|x86_64|arm|aarch64).*-linux"

EXTRA_OEMAKE = "ARCH='${TARGET_ARCH}' \
                CROSS_COMPILE='${TARGET_SYS}-' \
                CC='${CC}' \
                CXX='${CXX}' \
                LIB='${AR}' \
                LINKER='${CXX}' \
                LINUX64=1 \
                DEBUG=1 \
                PREFIX=${D}/usr \
                INSTALL_BIN_DIR=${D}${bindir} \
                INSTALL_INCLUDE_DIR=${D}${includedir} \
                INSTALL_LIB_DIR=${D}${libdir} \
                INSTALL_MAN_DIR=${D}${mandir}/man1 \
                "

do_compile() {
    oe_runmake -C ${S}/decoder/build/linux
}

do_install() {
    # Remove mkdir when upgrading (https://github.com/Linaro/OpenCSD/pull/36)
    mkdir --parents ${D}${mandir}/man1
    oe_runmake -C ${S}/decoder/build/linux install install_man
}

BBCLASSEXTEND = "native"