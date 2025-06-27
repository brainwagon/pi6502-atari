
pi6502.bin:	pi6502-atari.c
	cl65 -O -t atari -C atari-xex.cfg -o pi6502.xex pi6502-atari.c
