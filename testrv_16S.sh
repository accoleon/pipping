#!/bin/bash -l

#PBS -N rv_preproc
#PBS -q generic
#PBS -l nodes=1:ppn=6
#PBS -l walltime=24:00:00
#PBS -d /home6/rvoelker/anna/trimmed
#PBS -M rvoelker@uoregon.edu
#PBS -m bea

# Test file modified by kevin for pip pipeline

#demultiplex and trim variable spacer
#module load python/2.7.3
#cd /home6/rvoelker/anna/trimmed
# SEQ1=kt1_100K.1.fastq
# SEQ2=kt1_100K.2.fastq
# AUX=kt1_barcodes.txt
OUTPREF=data/kt1_100K

# deMultiPlexOffsetSeqs.py -p 6 -a $AUX -s1 $SEQ1 -s2 $SEQ2 -o $OUTPREF

# #trim using trimmomatic
# module load java
TRIM_IN1=$OUTPREF.1.fastq
TRIM_IN2=$OUTPREF.2.fastq
TRIM_OUT_PAIRED1=$OUTPREF.trim.paired.1.fastq
TRIM_OUT_PAIRED2=$OUTPREF.trim.paired.2.fastq
TRIM_OUT_UNPAIRED1=$OUTPREF.trim.unpaired.1.fastq
TRIM_OUT_UNPAIRED2=$OUTPREF.trim.unpaired.2.fastq
TRIM_LOG=trimming_log.txt
JAR_PATH=Trimmomatic-0.32/trimmomatic-0.32.jar
CLIP=ILLUMINACLIP:Trimmomatic-0.32/adapters/TruSeq2-PE.fa:2:30:10:3:TRUE
MAXINFO=MAXINFO:140:0.9
MINLEN=MINLEN:140
CROP=CROP:140

java -jar $JAR_PATH PE -phred33 -threads 6 -trimlog $TRIM_LOG $TRIM_IN1 $TRIM_IN2 $TRIM_OUT_PAIRED1 $TRIM_OUT_UNPAIRED1 $TRIM_OUT_PAIRED2 $TRIM_OUT_UNPAIRED2 $CLIP $MAXINFO $MINLEN $CROP
# 
# #bowtie filter to remove mitochondrial and phiX sequences
# module load bowtie/2.1.0
# BWT_IN1=$TRIM_OUT_PAIRED1
# BWT_IN2=$TRIM_OUT_PAIRED2
# INDEXES=/home6/rvoelker/ref_seqs/bowtie2indxes/danRerChrMphiX/danRer_chrM_phiX
# FILTERED=$OUTPREF.trim.paired.filtered.fastq
# SAMOUT=filter_alignment.sam
# 
# bowtie2 -5 16 --phred33 --fr --un-conc $FILTERED --threads 6 -x $INDEXES -1 $BWT_IN1 -2 $BWT_IN2 -S $SAMOUT
# rm $SAMOUT
# 
# 
