CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -lm -lcurl

# Direktori
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

# Direktori Config
ETC_DIR = /etc/halmos
# Direktori Log
LOG_DIR  = /var/log/halmos_honeypot

HTML_SOURCE_DIR = html
HTML_DEST_DIR = /var/www/html/halmos

AI_SOURCE_DIR = test_ai
AI_DEST_DIR = /home/test_ai

# File sumber dan objek
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BIN_DIR)/halmos_honeypot
CONFIG_FILE = config/halmos_honeypot.conf

# Aturan utama
all: $(TARGET)

# Membuat executable
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Membuat file objek
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

install: all
# Meng-copy-kan halmos_honeypot.conf ke folder /etc/halmos
	@mkdir -p $(ETC_DIR)
	@chmod 0777 $(ETC_DIR)
	@cp $(CONFIG_FILE) $(ETC_DIR)

# Meng-copy-kan contoh HTML ke folder /var/www/html/halmos
	@mkdir -p $(HTML_DEST_DIR)
	@chmod 0777 $(HTML_DEST_DIR)
	@cp -r $(HTML_SOURCE_DIR)/* $(HTML_DEST_DIR)

# Meng-copy-kan contoh scrip AI ke folder /home/test_ai
	@mkdir -p $(AI_DEST_DIR)
	@chmod 0777 $(AI_DEST_DIR)
	@cp -r $(AI_SOURCE_DIR)/* $(AI_DEST_DIR)
	
# Membuat folder log di /var/log/halmos_honeypot
	@mkdir -p $(LOG_DIR)
	@chmod 0777 $(LOG_DIR)
#	@cp $(TARGET) /usr/local/bin/

# Membersihkan file hasil kompilasi
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Membersihkan lebih lengkap (opsional)
dist-clean: clean
	rm -rf $(HTML_DEST_DIR)/*
	rm -rf $(AI_DEST_DIR)/*
	rm -rf $(TARGET)