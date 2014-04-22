# FASTQ sequence class

class FASTQ:
	"""
	An object of the FASTQ class represents a single sequence read from a FASTQ
	format sequence file.  Objects may be compressed or uncompressed.  To make an
	uncompressed sequence pass the object 4 lines of text read from a file; the
	constructor will split the lines and save the 1st line as a defline attribute,
	the 2nd as the sequence, and the 4th as the quality string.  To compress a
	sequence call the pack method to compress both the sequence and string into a
	single byte array.  The sequence and defline attributes will be replaced by a
	the byte array, which is called 'blob'.  To uncompress a packed object call
	unpack, which restores the sequence and quality attributes.  A new object can
	also be created by passing a blob (e.g. a byte string read from a database) to
	the constructor.
	"""
	def __init__(self, arg):
		if isinstance(arg, str):
			self._def, self._seq, dummy, self._qual = arg[1:].split('\n')
		else:
			self._def = ''
			self._blob = arg
	
	def defline(self):
		"Return the 'defline' string for this object."
		return self._def
	
	def sequence(self):
		"Return the sequence string for this object, or None if the object is compressed."
		return self._seq if hasattr(self, '_seq') else None
	
	def quality(self):
		"Return the quality string for this object, or None if the object is compressed."
		return self._qual if hasattr(self, '_qual') else None
	
	def blob(self):
		"Return the packed byte string if the object is compressed, otherwise None."
		return self._blob if hasattr(self, '_blob') else None
		
	int2char = 'ACTGN'
	char2int = {'A': 0, 'C':1, 'T':2, 'G':3, 'N':4}
	qbase = ord('#')
	
	def pack(self):
		"Use a lossless encoding scheme to compress the sequence and quality strings into a single byte array."
		self._blob = bytearray()
		for i in range(len(self._seq)):
			self._blob.append(50*FASTQ.char2int[self._seq[i]] + ord(self._qual[i])-FASTQ.qbase)
		self._blob = bytes(self._blob)
		delattr(self, '_seq')
		delattr(self, '_qual')
		
	def unpack(self):
		"Uncompress a compressed object."
		self._seq = ''
		self._qual = ''
		if self._blob is not None:
			for byte in self._blob:
				s, q = divmod(byte, 50)
				self._seq += FASTQ.int2char[s]
				self._qual += chr(FASTQ.qbase + q)
		delattr(self, '_blob')
	
	def __repr__(self):
		"""
		The representation of an unpacked sequence is a 4-line string suitable for printing in 
		a FASTQ file.
		"""
		a = ['@' + self._def]
		b = [self._seq, '+', self._qual] if hasattr(self, '_seq') else [str(self._blob)]
		return '\n'.join(a+b)
	
	@staticmethod
	def read(f):
		"Read 4 lines from a FASTQ format data file, return an uncompressed FASTQ object."
		res = f.readline()
		if len(res) > 0:
			for i in range(3):
				res += f.readline()
			return FASTQ(res.strip())
		else:
			return None
		

