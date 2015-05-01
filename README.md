[![Build Status](https://travis-ci.org/seqan/anise_basil.svg?branch=master)](https://travis-ci.org/seqan/anise_basil)

# Anise & Basil

BASIL is a method to detect breakpoints for structural variants (including insertion breakpoints) from aligned paired HTS reads in BAM format.
ANISE is a method for the assembly of large insertions from paired reads in BAM format and a list candidate insert breakpoints as generated by BASIL.

## Quickstart

### Obtaining and Compiling

The following instructions explain how to obtain, compile, and use ANISE on a Linux and Mac Os X system.
You will need to install a C++ compiler with sufficient C++11 format (a fairly rent Linux distribution or copy of the Xcode developer tools should work) and [CMake].
Also, you have to install [Boost] (>= 1.51.0).

For obtaining the software, use the following instructions for getting ANISE/BASIL and SeqAn.

```
~ # git clone https://github.com/seqan/anise_basil.git
~ # cd anise_basil
anise_basil # git checkout develop
anise_basil # git submodule init
anise_basil # git submodule update --recursive
```

Then, compile the program.

```
anise_basil # cd build
build # cmake ..
build # make -j 4 anise basil
```

You can control the number of cores to use for compiling with the `-j` parameter to `make`, thus the code above gives an example for the compilation with 4 cores.
You can now have a look at the command line help for both programs:

```
# ./bin/basil -h
# ./bin/anise -h
```

### A First Example

The directory examples contains a reference file `ref.fa` and two reads files `left.fq.gz` and `right.fq.gz`.
The reference has a length of 10kb and the reads are sequenced from a donor that has a 2kb insertion into the reference.

The first step is to map the reads to the reference, e.g. using BWA, and to get a sorted and indexed BAM file, e.g. using Samtools.

```
# bwa index ref.fa
# bwa aln -f left.fq.gz.sai ref.fa left.fq.gz
# bwa aln -f right.fq.gz.sai ref.fa right.fq.gz
# bwa sampe ref.fa left.fq.gz.sai right.fq.gz.sai left.fq.gz right.fq.gz \
    | samtools view -Sb - | samtools sort - simulated
# samtools index simulated.bam
```

Next, use BASIL to analyze the BAM file for tentative insertion sites.

```
# basil -ir ref.fa -im simulated.bam -ov basil.vcf
...
# cat basil.vcf
##fileformat=VCFv4.1
##source=BASIL
##reference=ref.fa
##INFO=<ID=IMPRECISE,Number=0,Type=Flag,Description="Imprecise structural va...
##INFO=<ID=SVTYPE,Number=1,Type=String,Description="Type of structural varia...
##INFO=<ID=OEA_ONLY,Number=0,Type=Flag,Description="Breakpoint support by OE...
##ALT=<ID=INS,Description="Insertion of novel sequence">
##FORMAT=<ID=GSCORE,Number=1,Type=String,Description="Sum of Geometric score...
##FORMAT=<ID=CLEFT,Number=1,Type=String,Description="Clipped alignments supp...
##FORMAT=<ID=CRIGHT,Number=1,Type=String,Description="Clipped alignments sup...
##FORMAT=<ID=OEALEFT,Number=1,Type=String,Description="One-end anchored alig...
##FORMAT=<ID=OEARIGHT,Number=1,Type=String,Description="One-end anchored ali...
##contig=<ID=1,length=10000>
#CHROM  POS     ID      REF     ALT     QUAL    FILTER  INFO    FORMAT  indi...
1       5001    site_0  T       <INS>   .       PASS    IMPRECISE;SVTYPE=INS...
```

A shortdescription of the fields in the VCF file is given below in the section File Formats.

Usually, the next step is to filter the VCF to sites with a minimal support,
e.g. for 30x coverage:

```
# filter_basil.py -i basil.vcf -o basil.filtered.vcf --min-oea-each-side 10
```

This file now serves as the input to ANISE:

```
# anise -ir ref.fa -im simulated.bam -iv basil.filtered.vcf -of anise.fa
...
```

The result file now contains one assembled insert with some annotation in the FASTA meta line.

```
# cat anise.fa
>site_0_contig_0 REF=1 POS=5001 STEPS=6  ANCHORED_LEFT=yes ANCHORED_RIGHT=yes SPANNING=yes STOPPED=no_more_reads
GGGCTTCGCCTAGGGTCTCGGGAGAAATCTAGGGACCCCAATCTATTAGACGAACACGTCCAGGGCATGG
TCAGGTATACACCTTCCGACTAGACGTGTTCGAAGATTCGGGAAAATTACCTGAAGAGCCCCCGTAAGCC
GTAGTAGAAGAGGACACTTCATTTAAACAATACCGAAAAAGTGTCTTGGCAGACCGTATCTTCACAGGGC
CGAAGCACTTTTGGCAGGCTTATAAACGCCCAGAATGAAGCACTCGCCATAGGTGGAAACCTTTAAGCGA
CGCGGGGTGTGTCGGCCCTATCCCTTGCGCTTACAGACTTTATTTCTTCGTGAGGGAGTTGACCCATGCA
```

An easy to use tool for verifying this example is [EMBOSS Needle] which you can use to compare `anise.fa` with the actual insertion in `example/ins.fa`.

Note that BASIL will in general detect all kinds of breakpoints, e.g. for inversions on real-world data.
ANISE will try to assemble around these breakpoints but the assembly processes from the left and right will (generally) not meet.
A good heuristic is to only keep those ANISE contigs where the assembly process met.

This might exclude some sites where the assembly process broke, e.g. because of repeats, but it gives you a good start.
You can always look at the remaining ANISE contigs later.

You can use the script extract_spanning.awk to obtain the contigs marked as "spanning" by ANISE.

```
# extract_spanning anise.fa > anise.filtered.fa
```

Note that ANISE also assembles sequence left and right of the insert.
This can be used to anchor the inserted sequence to the reference using BLAT.
The executables blat and pslPretty are available from the [UCSC download site].

Let us now use BLAT to align the contig back to our reference.

```
# blat ref.fa anise.filtered.fa matches.psl
```

The file matches.psl only contains one line.
We can now visualize this alignment using pslPretty:

```
>site_0_contig_0:0+2592 of 2592 1:4716+5308 of 10000
GGGCTTCGCCTAGGGTCTCGGGAGAAATCTAGGGACCCCAATCTATTAGACGAACACGTC
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
GGGCTTCGCCTAGGGTCTCGGGAGAAATCTAGGGACCCCAATCTATTAGACGAACACGTC

CAGGGCATGGTCAGGTATACACCTTCCGACTAGACGTGTTCGAAGATTCGGGAAAATTAC
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
CAGGGCATGGTCAGGTATACACCTTCCGACTAGACGTGTTCGAAGATTCGGGAAAATTAC

CTGAAGAGCCCCCGTAAGCCGTAGTAGAAGAGGACACTTCATTTAAACAATACCGAAAAA
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
CTGAAGAGCCCCCGTAAGCCGTAGTAGAAGAGGACACTTCATTTAAACAATACCGAAAAA

GTGTCTTGGCAGACCGTATCTTCACAGGGCCGAAGCACTTTTGGCAGGCTTATAAACGCC
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
GTGTCTTGGCAGACCGTATCTTCACAGGGCCGAAGCACTTTTGGCAGGCTTATAAACGCC

CAGAATGAAGCACTCGCCATAGGTGGAAACCTTTAAGCGACGCGGGGTGT...TCAGGTT
||||||||||||||||||||||||||||||||||||||||||||               |
CAGAATGAAGCACTCGCCATAGGTGGAAACCTTTAAGCGACGCG-----2000------T

TGGGTCCGCGCAGCGCCAACGATTTCAACCGGGAGACGTTCGTTCATGATGAGAAGACGG
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
TGGGTCCGCGCAGCGCCAACGATTTCAACCGGGAGACGTTCGTTCATGATGAGAAGACGG

...
```

When using ANISE on real data, it might be useful to extract the best match for each query from the BLAT file by its BLAT score.
You can do this using the script `best_blat.py`.
`best_blat.py` computes the BLAT identity and score as computed on the BLAT web front-end; these values are not written out by command line blat.

Note that the script prepends the BLAT identity, query coverage, target coverage, and BLAT score to the first four columns of the output TSV file.

```
# best_blat.py -b matches.psl | column -t
#identity  query_coverage  target_coverage  blat_score  matches  mismatches...
95.9       22.8            5.9              591         592      0         ...
```
## File Formats

### BASIL VCF fields

A typical line in BASIL might look as follows.

```
1 5001 site_0 T <INS>   . PASS IMPRECISE;SVTYPE=INS GSCORE:CLEFT:CRIGHT:OEALEFT:OEARIGHT    46.4256:10:12:35:32
```

The first seven columns are as usually in VCF files (ref name, 1-based position, reference base, abbreviation for long insertion, no assigned quality, passing all filters, imprecise insertion SV).

The eigth column contains the names of the score values given in the ninth column:

* `GSCORE` Geometric mean of the sum of "1 + $score" for all of the following scores.
* `CLEFT` Number of clipping signatures supporting the site from the left side.
* `CRIGHT` Number of clipping signatures supporting the site from the right side.
* `OEALEFT` Number of OEA alignments supporting the site from the left.
* `OEARIGHT` Number of OEA alignmetns supproting the site from the right.

Generally, one should filter for a minimum support of OEA records on each side, e.g. a value of 10 makes sense for a 30x coverage and showed good results on simulated data.

For a ranking, GSCORE is a suitable measure but we did not develop any statistical model for BASIL matches and it is a mean of pseudocounts only.
It carries no statistically precise meaning.

### ANISE FASTA meta tags.

A typical FASTA line as written by ANISE looks as follows (we deliberately introduced a line break here for text wrapping).

```
>site_0_contig_0 REF=1 POS=5001 STEPS=6  ANCHORED_LEFT=yes ANCHORED_RIGHT=yes SPANNING=yes STOPPED=no_more_reads
```

This means that the contig with the name "site_0_contig_0" was generated for a site as called by BASIL on the reference named "1" at 1-based position 5,000 after 6 ANISE assembly steps.
The site is expected to be anchored on the left and right side, meaning that there should be at least two reads in the site that aligned on the forward strand in the input mapping and two reads that aligned on the reverse strand in the input mapping.

Also, ANISE could find a path from the left to the right end only using overlap and paired link information, i.e. the assembly did not stop without meeting.
Furthermore, the assembly was stopped since there were not sufficient reads mapped on this contig to continue assembly.
If the value of STOPPED was "too_many_reads", too many (thousands!) of reads mapped on the site which is an indicator for the site containing low-complexity regions or highly repetetive sequence.
ANISE gives up in this case.

## Frequently Asked Questions

* Q: ANISE does not start at step 1 and stops without sensible results.
* A: ANISE tries to continue where it left off earlier.  For this, it creates a directory with temporary file.  If the execution stops before the first assembly step, the directory might contain corrupted state. 
* Solution: Simply remove the temporary directory.  If the output file name is `output.fa`, the temporary directory will be called `output.fa.tmp`.

## References

* Holtgrewe, M., Kuchenbecker, L., & Reinert, K. (2015).
  [Methods for the Detection and Assembly of Novel Sequence in High-Throughput Sequencing Data].
  *Bioinformatics*, btv051.

## Contact

For questions, comments, or suggestions, please file a [GitHub issue] or an email to [Manuel Holtgrewe]

[Boost]: http://www.boost.org
[CMake]: http://www.cmake.org
[EMBOSS Needle]: https://www.ebi.ac.uk/Tools/psa/emboss_needle/nucleotide.html
[GitHub issue]: https://github.com/seqan/anise_basil/issues
[Lemon]: http://lemon.cs.elte.hu/trac/lemon
[Manuel Holtgrewe]: mailto:manuel.holtgrewe@fu-berlin.de
[Methods for the Detection and Assembly of Novel Sequence in High-Throughput Sequencing Data]: http://bioinformatics.oxfordjournals.org/content/early/2015/02/01/bioinformatics.btv051.short
[UCSC download site]: http://hgdownload.soe.ucsc.edu/downloads.html#source_downloads
