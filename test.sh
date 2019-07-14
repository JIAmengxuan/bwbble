#!/bin/sh
echo "TEST BEGIN" | tee omp_vs_tp_log.txt
for i in 1 2 4 6 8
do
	echo "-----------------------------------\nROUND $i BEGIN" | tee -a omp_vs_tp_log.txt
	echo "===================================\nTest OMP with $i Threads begin" | tee -a omp_vs_tp_log.txt
	mg-aligner/build/bwbble align -t $i test_data/GCA_000001405.28_GRCh38.p13_genomic.fna test_data/reads5000000.fq mg-aligner/build/output/reads5000000.aln | tee -a omp_vs_tp_log.txt
	echo "Test OMP end\n===================================" | tee -a omp_vs_tp_log.txt
	echo "===================================\nTest TP with $i Threads begin" | tee -a omp_vs_tp_log.txt
	mg-aligner/newBuild/bwbble align -t $i test_data/GCA_000001405.28_GRCh38.p13_genomic.fna test_data/reads5000000.fq mg-aligner/newBuild/output/reads5000000.aln | tee -a omp_vs_tp_log.txt
	echo "Test TP end\n===================================" | tee -a omp_vs_tp_log.txt
	echo "ROUND $i END\n-----------------------------------" | tee -a omp_vs_tp_log.txt
done
