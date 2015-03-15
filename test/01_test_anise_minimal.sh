#!/bin/bash

set -e
set -x

# A very minimalistic and simple test for the ANISE program.
#
# Usage: 01_test_anise_minimal.sh path/to/anise

# ---------------------------------------------------------------------------
# Create temporary directory
# ---------------------------------------------------------------------------

SCRATCH=$(mktemp -d)
function cleanup {
    echo rm -rf "$SCRATCH"
}
trap cleanup EXIT
echo -e "SCRATCH:\t${SCRATCH}"

# ---------------------------------------------------------------------------
# Check args, set path variables
# ---------------------------------------------------------------------------

ANISE=$1
test -f "$ANISE" || { echo "Program $ANISE not found" 1>&2; exit 1; }
echo -e "anise executable:\t${ANISE}" 1>&2

THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
IN_REF=${THIS_DIR}/01/ref.fa
IN_BAM=${THIS_DIR}/01/simulated.bam
IN_VCF=${THIS_DIR}/01/basil.vcf
OUT_FA=${SCRATCH}/anise.fa
OUT_SAM=${SCRATCH}/anise.sam

${ANISE} \
    -ir ${IN_REF} \
    -iv ${IN_VCF} \
    -im ${IN_BAM} \
    -of ${OUT_FA} \
    -om ${OUT_SAM} \
    1>${SCRATCH}/stdout.txt \
    2>${SCRATCH}/stderr.txt

# ---------------------------------------------------------------------------
# Compare the output VCF to the reference one.
# ---------------------------------------------------------------------------

diff ${THIS_DIR}/01/anise.fa ${OUT_FA}
diff ${THIS_DIR}/01/anise.sam ${OUT_SAM}
