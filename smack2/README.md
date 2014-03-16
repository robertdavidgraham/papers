SMACK2

This is a project for testing Aho-Corasick pattern matching. The
purpose is to benchmark various algorithms on various CPUs for maximum
performance.

A typical way of running this is the following:

	bin\smack2 --benchmark --haystack kingjames.txt --needles patterns.txt

	---- SMACK/2 - benchmark program ----
	directory = C:\dev\papers\smack2
	CPU width = 32-bits
	CPU vendor = "GenuineIntel"
	CPU brand = "Intel(R) Core(TM) i7 CPU       X 980  @ 3.33GHz"
	CPU codename = "Westmere-EP" (0x206Cx)
	CPU info = type(0) family(6) model(44) stepping(2)

	--- low-level benchmarks ---
	c-ptr = 9.047-clocks
	c-idx = 9.007-clocks
	asm-ptr = 4.008-clocks
	asm-idx = 4.012-clocks

	-- smack-1 --
	bits/second = 6582.064-million
	clocks/byte = 4.061-clocks
	clockrate = 3.341-GHz

	haystack file = kingjames.txt
	file-size = 4452069
	needles = 6 patterns
	Found count = 39
	Search speed = 6.562-gbps (4.07-Hz/byte)
	CPU speed = 3.341-GHz

This tells us a few things. The `asm-ptr` and `asm-idx` tell us the MAXIMUM
THEORETICAL SPEED that any Aho-Corasick algorithm can run. That's because
at the heart of Aho-Corasick evaluation is something in C that looks like:

	row = table[row][column];

In x86 assembler, this translates to something like:

	mov esi, [eax + esi*4]

This assembly language instruction has a **latency** equal to a round-trip
through the L1 cache, which on modern x86 is 4 clock cycles. The `asm-idx`
benchmark confirms that the minimum latency is 4 clock cycles.

Therefore, when using Aho-Corasick to search data, it will take 4 clock
cycles per byte of input. In other words, given a 3.33 GHz CPU, the max
theoretical speed is 6.66 gbps per CPU.

That's what the later part of this tests. It takes the King James bible
and searches it for several patterns, one of which is "fornication", which
occurs 39 times. We search the bible and measure a rate of 6.562 gbps,
which is within 2% o the maximum theoretical speed.

The current assembly-language code works well on Intel CPUs, Nehalem-class
and newer. That's because they are amazingly out-of-order. To make it work
faster on other processors, I may need to tweak things more.
