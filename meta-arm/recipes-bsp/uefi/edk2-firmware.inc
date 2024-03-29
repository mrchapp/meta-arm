SUMMARY = "UEFI EDK2 Firmware"
DESCRIPTION = "UEFI EDK2 Firmware for Arm reference platforms"
HOMEPAGE = "https://github.com/tianocore/edk2"
LICENSE = "BSD-2-Clause-Patent"

PROVIDES = "virtual/bootloader"

# EDK2
LIC_FILES_CHKSUM = "file://License.txt;md5=2b415520383f7964e96700ae12b4570a"
# EDK2 Platforms
LIC_FILES_CHKSUM += "file://edk2-platforms/License.txt;md5=2b415520383f7964e96700ae12b4570a"

# These can be overridden as needed
EDK2_SRC_URI = "gitsm://github.com/tianocore/edk2.git;branch=master;protocol=https"
EDK2_PLATFORMS_SRC_URI = "git://github.com/tianocore/edk2-platforms.git;branch=master;protocol=https"

SRC_URI = "\
    ${EDK2_SRC_URI};name=edk2;destsuffix=edk2;nobranch=1 \
    ${EDK2_PLATFORMS_SRC_URI};name=edk2-platforms;destsuffix=edk2/edk2-platforms;nobranch=1 \
    file://unaligned.patch \
    file://0001-Basetools-turn-off-gcc12-warning.patch \
    file://0002-BaseTools-fix-gcc12-warning.patch \
    file://0003-BaseTools-fix-gcc12-warning.patch \
"

SRCREV_FORMAT         = "edk2_edk2-platforms"
UPSTREAM_CHECK_GITTAGREGEX = "^edk2-stable(?P<pver>\d+)$"

COMPATIBLE_MACHINE ?= "invalid"
PACKAGE_ARCH = "${MACHINE_ARCH}"

EDK2_BUILD_RELEASE = "1"
# Release builds don't work with clang yet:
# https://bugzilla.tianocore.org/show_bug.cgi?id=3758.
# Can be removed in 202202 (c9b7c6e)
EDK2_BUILD_RELEASE:toolchain-clang = "0"

EDK2_PLATFORM      = "unset"
# build --platform
EDK2_PLATFORM_DSC  = "unset"
EDK2_BIN_NAME      = ""
# build --arch
EDK2_ARCH         ?= "unset"
EDK2_ARCH:arm      = "ARM"
EDK2_ARCH:aarch64  = "AARCH64"
EDK2_ARCH:x86      = "IA32"
EDK2_ARCH:x86-64   = "X64"
EDK2_ARCH:riscv64  = "RISCV64"

# Extra arguments passed to build
EDK2_EXTRA_BUILD   = ""

# build --buildtarget
EDK2_BUILD_MODE ?= "${@bb.utils.contains('EDK2_BUILD_RELEASE', '1', 'RELEASE', 'DEBUG', d)}"

# Baremetal, no need for a C library
DEPENDS:remove = "virtual/${TARGET_PREFIX}compilerlibs virtual/libc"
DEPENDS += "util-linux-native iasl-native"

inherit deploy

S = "${WORKDIR}/edk2"
B = "${WORKDIR}/build"

LDFLAGS[unexport] = "1"

do_configure[cleandirs] += "${B}"

# Set variables as per envsetup
export PACKAGES_PATH       = "${S}:${S}/edk2-platforms"
export WORKSPACE           = "${B}"
export EDK_TOOLS_PATH      = "${S}/BaseTools"
export PYTHON_COMMAND      = "python3"
export CONF_PATH           = "${S}/Conf"

export BTOOLS_PATH = "${EDK_TOOLS_PATH}/BinWrappers/PosixLike"

EDK_COMPILER ?= "GCC5"
export GCC5_AARCH64_PREFIX = "${TARGET_PREFIX}"

EDK_COMPILER:toolchain-clang = "CLANG38"
export CLANG38_AARCH64_PREFIX = "${TARGET_PREFIX}"

do_configure:prepend() {
    sed -i -e "s#-target ${HOST_ARCH}-linux-gnu*#-target ${HOST_SYS}#" ${S}/BaseTools/Conf/tools_def.template
}

do_compile() {
    sed -i -e 's:-I \.\.:-I \.\. ${BUILD_CFLAGS} :' ${EDK_TOOLS_PATH}/Source/C/Makefiles/header.makefile
    sed -i -e 's: -luuid: -luuid ${BUILD_LDFLAGS}:g' ${EDK_TOOLS_PATH}/Source/C/*/GNUmakefile

    # Copy the templates as we don't run envsetup
    cp ${EDK_TOOLS_PATH}/Conf/build_rule.template ${S}/Conf/build_rule.txt
    cp ${EDK_TOOLS_PATH}/Conf/tools_def.template ${S}/Conf/tools_def.txt
    cp ${EDK_TOOLS_PATH}/Conf/target.template ${S}/Conf/target.txt

    # Build basetools
    oe_runmake -C ${S}/BaseTools

    PATH="${WORKSPACE}:${BTOOLS_PATH}:$PATH" \
    build \
       --arch "${EDK2_ARCH}" \
       --buildtarget ${EDK2_BUILD_MODE} \
       --tagname ${EDK_COMPILER} \
       --platform ${EDK2_PLATFORM_DSC} \
       ${@oe.utils.parallel_make_argument(d, "-n %d")} \
       ${EDK2_EXTRA_BUILD}
}

do_install() {
    install -d ${D}/firmware
    install ${B}/Build/${EDK2_PLATFORM}/${EDK2_BUILD_MODE}_${EDK_COMPILER}/FV/${EDK2_BIN_NAME} ${D}/firmware/uefi.bin
}

FILES:${PN} = "/firmware"
SYSROOT_DIRS += "/firmware"
# Skip QA check for relocations in .text of elf binaries
INSANE_SKIP:${PN} = "textrel"

do_deploy() {
    # Copy the images to deploy directory
    cp -rf ${D}/firmware/* ${DEPLOYDIR}/
}
addtask deploy after do_install
