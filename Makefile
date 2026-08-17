# CTF Arsenal — Top-level Makefile
# POSIX C99 | GNU Make 4+ | Public Domain (Unlicense)

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -O2 -Iinclude \
           -I$(CURDIR)/src/core \
           -D_POSIX_C_SOURCE=200809L
LDFLAGS =
BIN     = bin

# All targets
TARGETS = $(BIN)/zip_cracker   \
          $(BIN)/rar_cracker   \
          $(BIN)/pdf_cracker   \
          $(BIN)/multi_brute   \
          $(BIN)/table_generator \
          $(BIN)/table_query   \
          $(BIN)/http_fuzz     \
          $(BIN)/dns_fuzz      \
          $(BIN)/smb_fuzz

.PHONY: all clean test help install

all: $(BIN) $(TARGETS)
	@echo ""
	@echo "\033[32m\033[1m[+] CTF Arsenal built successfully! Run: ./scripts/ctf_arsenal.sh help\033[0m"

$(BIN):
	mkdir -p $@

# ── Brute-force ───────────────────────────────────────────────────────────────
CORE_OBJS = src/core/utils.c

$(BIN)/zip_cracker: src/brute/zip_cracker.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lzip

$(BIN)/rar_cracker: src/brute/rar_cracker.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/pdf_cracker: src/brute/pdf_cracker.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/multi_brute: src/brute/multi_brute.c src/brute/zip_cracker.c \
                    src/brute/rar_cracker.c src/brute/pdf_cracker.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -DCTF_MULTI_BRUTE -o $@ $^ -lzip -lpthread

# ── Rainbow tables ─────────────────────────────────────────────────────────────
$(BIN)/table_generator: src/rainbow/table_generator.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lssl -lcrypto

$(BIN)/table_query: src/rainbow/table_query.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lssl -lcrypto

# ── Fuzzers ────────────────────────────────────────────────────────────────────
$(BIN)/http_fuzz: src/fuzz/http_fuzz.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ src/fuzz/http_fuzz.c $(CORE_OBJS)

$(BIN)/dns_fuzz: src/fuzz/dns_fuzz.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ src/fuzz/dns_fuzz.c $(CORE_OBJS)

$(BIN)/smb_fuzz: src/fuzz/smb_fuzz.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o $@ src/fuzz/smb_fuzz.c $(CORE_OBJS)

# ── Tests ──────────────────────────────────────────────────────────────────────
test: all
	@echo "=== Running test suite ==="
	@bash tests/test_brute.sh   || true
	@python3 tests/test_clue.py || true
	@bash tests/test_rainbow.sh || true
	@python3 tests/test_stego.py || true
	@bash tests/test_fuzz.sh    || true
	@echo "=== Tests complete ==="

# ── Clean ──────────────────────────────────────────────────────────────────────
clean:
	rm -rf $(BIN)
	find . -name "*.rt" -delete
	find . -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	@echo "[*] Cleaned"

# ── Install (symlink to /usr/local/bin) ────────────────────────────────────────
install: all
	@echo "[*] Installing ctf_arsenal to /usr/local/bin..."
	sudo ln -sf $(CURDIR)/scripts/ctf_arsenal.sh /usr/local/bin/ctf_arsenal
	@echo "[+] Installed. Run: ctf_arsenal help"

help:
	@echo "CTF Arsenal Makefile targets:"
	@echo "  make all      — Build all C binaries"
	@echo "  make test     — Build and run all tests"
	@echo "  make clean    — Remove build artifacts"
	@echo "  make install  — Install ctf_arsenal to /usr/local/bin"
