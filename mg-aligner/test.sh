#!/bin/sh
echo "===================================================" >> log.txt
echo "Test MP begin" >> log.txt
build/bwbble align -t 8 ../test_data/GCA_000001405.28_GRCh38.p13_genomic.fna ../test_data/reads5000000.fq build/output/reads5000000.aln >> log.txt
echo "Test TP begin" >> log.txt
newBuild/bwbble align -t 8 ../test_data/GCA_000001405.28_GRCh38.p13_genomic.fna ../test_data/reads5000000.fq newBuild/output/reads5000000.aln >> log.txt

