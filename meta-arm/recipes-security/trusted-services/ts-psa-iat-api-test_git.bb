DESCRIPTION = "Initial Attestation PSA certification tests (psa-arch-test) for Trusted Services"

TS_ENV = "arm-linux"

require ts-psa-api-test-common_${PV}.inc

OECMAKE_SOURCEPATH = "${S}/deployments/psa-api-test/initial_attestation/${TS_ENV}"

PSA_TEST = "psa-iat-api-test"

# psa-arch-tests for INITIAL_ATTESTATION suite can't be built with pre-built qcbor
# Fetch qcbor sources as a temp work-around and pass PSA_TARGET_QCBOR to psa-arch-tests
SRC_URI += "git://github.com/laurencelundblade/QCBOR.git;name=psaqcbor;protocol=https;branch=master;destsuffix=git/psaqcbor \
            file://0012-PSA-TARGET-QCBOR.patch \
           "
SRCREV_psaqcbor = "42272e466a8472948bf8fca076d113b81b99f0e0"

EXTRA_OECMAKE += "-DPSA_TARGET_QCBOR=${WORKDIR}/git/psaqcbor \
                 "
