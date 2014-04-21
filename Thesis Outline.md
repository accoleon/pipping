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
A short write up of the 16S pipeline, specifically involving the preprocessing
portions 

<!--
TODO:  Write about db schema and rationale, the data flow
-->

2. Database Management, Schema
------------------------------
Talk about the db schema, rationale.

3. Compression Scheme and Rationale
-----------------------------------
Short description of the compression scheme (combining sequence and quality lines)
and file size savings

4. Streaming (Trimmomatic) technique and rationale
--------------------------------------------------
Description of the streaming technique (UNIX Pipes) and how it interfaces with
Trimmomatic.

5. Performance Benchmarks using various sample sizes
----------------------------------------------------
Show insert speed, normalization speed, streaming speed (to trimmomatic)

6. Conclusion and future work
-----------------------------
Based on performance benchmarks and (user feedback?), tool is usable. More work 
required to make it work with other tools (bowtie, rest of pipeline). 