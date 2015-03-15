#!/bin/bash

bwa index ref.fa
bwa aln -f left.fq.gz.sai ref.fa left.fq.gz
bwa aln -f right.fq.gz.sai ref.fa right.fq.gz
bwa sampe ref.fa left.fq.gz.sai right.fq.gz.sai left.fq.gz right.fq.gz \
      | samtools view -Sb - | samtools sort - simulated
samtools index simulated.bam

git add simulated.bam simulated.bam.bai
