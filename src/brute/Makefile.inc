# Brute-force sub-make rules
BRUTE_SRCS = src/brute/zip_cracker.c src/brute/rar_cracker.c src/brute/pdf_cracker.c src/brute/multi_brute.c
BRUTE_BINS = bin/zip_cracker bin/rar_cracker bin/pdf_cracker bin/multi_brute
BRUTE_LIBS = -lzip -lpthread

bin/zip_cracker: src/brute/zip_cracker.c src/core/utils.c
	$(CC) $(CFLAGS) -o $@ $^ -lzip

bin/rar_cracker: src/brute/rar_cracker.c src/core/utils.c
	$(CC) $(CFLAGS) -o $@ $^

bin/pdf_cracker: src/brute/pdf_cracker.c src/core/utils.c
	$(CC) $(CFLAGS) -o $@ $^

bin/multi_brute: src/brute/multi_brute.c src/brute/zip_cracker.c src/brute/rar_cracker.c src/brute/pdf_cracker.c src/core/utils.c
	$(CC) $(CFLAGS) -o $@ $^ -lzip -lpthread
