#!/bin/bash

rm fastq.db
./pip -mn data/anna_500K.1.fastq data/anna_500K.2.fastq

# #trim using trimmomatic
# module load java
OUTPREF=pip-stream-trimmomatic
TRIM_IN1=$OUTPREF.1.fastq
TRIM_IN2=$OUTPREF.2.fastq
TRIM_OUT_PAIRED1=/dev/null
TRIM_OUT_PAIRED2=/dev/null
TRIM_OUT_UNPAIRED1=/dev/null
TRIM_OUT_UNPAIRED2=/dev/null
TRIM_LOG=$OUTPREF.log
JAR_PATH=Trimmomatic-0.32/trimmomatic-0.32.jar
CLIP=ILLUMINACLIP:Trimmomatic-0.32/adapters/TruSeq2-PE.fa:2:30:10:3:TRUE
MAXINFO=MAXINFO:140:0.9
MINLEN=MINLEN:140
CROP=CROP:140

#instruments -t "/Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/Resources/templates/Time Profiler.tracetemplate" -D ProfileResults /Users/junjiexu/pipping/pip -s trimmo &
./pip -s trimmo &
java -jar $JAR_PATH PE -phred33 -trimlog $TRIM_LOG $TRIM_IN1 $TRIM_IN2 $TRIM_OUT_PAIRED1 $TRIM_OUT_UNPAIRED1 $TRIM_OUT_PAIRED2 $TRIM_OUT_UNPAIRED2 $CLIP $MAXINFO $MINLEN $CROP
OUTPREF=data/anna_500K
TRIM_IN1=$OUTPREF.1.fastq
TRIM_IN2=$OUTPREF.2.fastq
TRIM_OUT_PAIRED1=$OUTPREF.1.pair.fastq
TRIM_OUT_PAIRED2=$OUTPREF.2.pair.fastq
TRIM_OUT_UNPAIRED1=$OUTPREF.1.unpair.fastq
TRIM_OUT_UNPAIRED2=$OUTPREF.2.unpair.fastq
TRIM_LOG=$OUTPREF.log
#time java -jar $JAR_PATH PE -phred33 -trimlog $TRIM_LOG $TRIM_IN1 $TRIM_IN2 $TRIM_OUT_PAIRED1 $TRIM_OUT_UNPAIRED1 $TRIM_OUT_PAIRED2 $TRIM_OUT_UNPAIRED2 $CLIP $MAXINFO $MINLEN $CROP
#java -jar $JAR_PATH SE -threads 1 -phred33 -trimlog $TRIM_LOG $TRIM_IN1 $TRIM_OUT_UNPAIRED1 $CLIP $MAXINFO $MINLEN $CROP

# results
# on Intel Core i7 quad core 2.7GHz , 256KB L2, 6MB L3, 16GB ram
# Apple SSD 512GB
# 1 thread
# pip streaming: 22.89s
# no pip: 46.43s
# 2 threads
# pip streaming: 23.58s
# no pip: 25.96s
# 4 threads
# pip streaming: 23.77s
# no pip: 26.05s
# 8 threads
# pip streaming: 23.21s
# no pip: 26.46s
# Conclusion: No performance loss when using pip streaming to Trimmomatic
# However, we save on file size and disk access - without pip, anna's test files
# resulted in:
# pair.1.fastq: 	161.1MB
#	pair.2.fastq: 	161.1MB
# unpair.1.fastq: 6MB
# unpair.2.fastq: 5.4MB
# .log:						77.3MB
# ---------------	410.9MB output
# original fastq:	371.4MB
# ---------------	782.3MB total

# pip uses:
#	start fastq.db	186.7MB
# after trim			206.5MB total
# Since pip only stores the deltas of the trim, in just 20mb we could store
# the exact same results that uncompressed took 410.9MB.