#!/bin/bash

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

./pip -s trimmo &
java -jar $JAR_PATH PE -phred33 -trimlog $TRIM_LOG $TRIM_IN1 $TRIM_IN2 $TRIM_OUT_PAIRED1 $TRIM_OUT_UNPAIRED1 $TRIM_OUT_PAIRED2 $TRIM_OUT_UNPAIRED2 $CLIP $MAXINFO $MINLEN $CROP

#java -jar $JAR_PATH SE -threads 1 -phred33 -trimlog $TRIM_LOG $TRIM_IN1 $TRIM_OUT_UNPAIRED1 $CLIP $MAXINFO $MINLEN $CROP
