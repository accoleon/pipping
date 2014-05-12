Title: Acceleration of 16S sequencing data management using SQLite
==================================================================

Abstract
--------
The 16S Ribosomal RNA Sequencing is used extensively in analzying bacterial
phylogeny and taxonomy. This project attempts to streamline the 16S sequencing
pipeline using local file databases to replace multiple flat sequence files used
in the pipeline, to ease logistical burdens on the researcher and enable 
greater meta-analysis and accountability of experiments.

1. Introduction 
---------------
The project utilized computational methods such as 16S Ribosomal Profiling as a proxy for species identity. The 16S small subunit of bacterial ribosomes are highly conserved, which means that the differences within the 16S RNA profiles can be used as an analogue for species identity. Since the 16S gene contains both highly conserved and highly variable regions all interspersed together,
The highly conserved regions can be amplified using PCR (Polymerase chain reaction), while the variable regions are used to classify the organism. HTS (High Throughput Sequencing) using Illumina sequencers are then used to sequence the PCR products from the prior step.

The output from HTS forms the start of the computational pipeline. In the preprocessing stage, poor quality reads are filtered and trimmed. Chimeras and non-bacterial 16S reads are then removed. The pipeline then attempts to produce phylogenetic classification through the use of RDP classifiers and sequence similarity (i.e. OTU Analysis). By comparing the filtered output from HTS against the already existing taxonomies of over 2 million species, the genus, family, and order of the sample can be determined.

The RDP classifier can fail for various reasons: the majority of bacterial species have not been identified or sequenced, the 16S reads are usually too short for accurate classification. In those cases, groups of highly similar sequences are grouped into OTUs (Operational Taxonomic Unit) that can be analyzed for
quantitative differences in communities between the sequenced samples.

The project utilizes a computational approach due to the large amount of raw sequencing data that is created from the PCR amplification process. The variability of results is further affected by the OTU Analysis stage, where prior parameters can affect the analysis outcome. Using a computational approach would allow the change of various parameters, in order to quantitate the effects of those parameter changes.

The majority of the pipeline is executed on the University of Oregon’s ACISS High-Performance Supercomputer Cluster, and utilizes PBS scripts (normal shell scripts with extra variables defined to manage job resources) to execute the different stages of the pipeline.

The FASTQ output produced by the Illumina sequencer is run through the preprocessing stage of filtering, trimming, and demultiplexing. The PBS script for the preprocessing stage calls on the Demultiplexer, a python script written by Rodger Voelker, which removes the primer attached to the sequences during the amplification process, and attaches barcodes (signifying sample origin) to both ends of the paired-end reads in the FASTQ file. After demultiplexing, the pipeline proceeds to use Trimmomatic v0.32, an open source tool to trim poor quality feeds from the reads. It uses a sliding window trimming, that cuts out sequences when the average quality within a window falls below a certain threshold. Finally, Bowtie 2.1.0 is used to align the reads to existing mitochondrial and phiX sequences and to remove them.

2. Database Management, Schema
------------------------------
Talk about the db schema, rationale.



3. Compression Scheme and Rationale
-----------------------------------
Short description of the compression scheme (combining sequence and quality lines)
and file size savings

Majority of space usage within sequence/quality lines:
@HWI-ST0747:277:D1M96ACXX:6:1101:1232:2090 1:N:0:
GGATAGTACTAGGGTATCTAATCCTGTTTGCTCCCCACGCTTTCGCACCTCAGCGTCAGTATCGAGCCAGTGAGCCGCCTTCGCCACTGGTGTTCCTCCGAATATCTACGAATTTCACTGCTACACGCGGAATTCCATCCCCCTCTACCGT
+
ACCCFFDDFH#FHHIGIJJFJJJJJJJJJJJIIJJJGIJJGIJJJIGIJJJIIIIJJJJHHHHHFFFDCEACCDDCDDD@BDDDDBDDDDDDDDDDDDDBBB@BDEEACDDDDDDDEDDCCDDDADBBDDDDDDDDECCBDDDB@9@AA<<

ATCGN #$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJ

* 8-bit char can hold 256 (28) values
* 5 bases require 4 (22) to 8 (23) values
* 42 qualities require 32 (25) to 64 (26) values

A: 0-49
T: 50-99
C: 100-149
G: 150-199
N: 200-255

* 50% compression
* Fast compression/decompression
* Lossless

4. Streaming (Trimmomatic) technique and rationale
--------------------------------------------------
Description of the streaming technique (UNIX Pipes) and how it interfaces with
Trimmomatic.

Original data in compressed form, tool wants data in certain format. 

* Streaming Data with Named Pipes
	* Tool-specific adapters
* Demultiplexer, Trimmomatic, Bowtie
* No intermediate files Virtual files on filesystem

1. Request data
2. Transforms data into streams of FASTQ data
3. Feed data into tool using pipes
4. Receive feedback from tool
5. Transform feedback into deltas
6. Store data into SQLite

* No intermediate files
* No disk I/O

5. Performance Benchmarks using various sample sizes
----------------------------------------------------
Show insert speed, normalization speed, streaming speed (to trimmomatic)

6. Conclusion and future work
-----------------------------
Based on performance benchmarks and (user feedback?), tool is usable. More work 
required to make it work with other tools (bowtie, rest of pipeline). 