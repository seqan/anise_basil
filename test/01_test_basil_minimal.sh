#!/bin/bash

set -e
set -x

# A very minimalistic and simple test for the BASIL program.
#
# Usage: 01_test_basil_minimal.sh path/to/basil

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

BASIL=$1
test -f "$BASIL" || { echo "Program $BASIL not found" 1>&2; exit 1; }
echo -e "basil executable:\t${BASIL}" 1>&2

THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
IN_REF=${THIS_DIR}/01/ref.fa
IN_BAM=${THIS_DIR}/01/simulated.bam
OUT_VCF=${SCRATCH}/basil.vcf

${BASIL} \
    -ir ${IN_REF} \
    -im ${IN_BAM} \
    -ov ${OUT_VCF} \
    1>${SCRATCH}/stdout.txt \
    2>${SCRATCH}/stderr.txt

# ---------------------------------------------------------------------------
# Compare the output VCF to the reference one.
# ---------------------------------------------------------------------------

diff ${THIS_DIR}/01/basil.vcf ${OUT_BAM}
