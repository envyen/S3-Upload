CC ?= gcc

# build target specs
CFLAGS = #-Wall -DDEBUG

OUT_DIR = objs

LIBS = -lcurl

default: s3-upload

s3-upload: $(OUT_DIR)/base64.c.o $(OUT_DIR)/hmacsha1.c.o $(OUT_DIR)/S3-Upload.c.o
	$(CC) $(CFLAGS) -o s3-upload $(OUT_DIR)/base64.c.o $(OUT_DIR)/hmacsha1.c.o $(OUT_DIR)/S3-Upload.c.o $(LIBS)

$(OUT_DIR)/base64.c.o: base64.c crypto.h
	$(CC) $(CFLAGS) -o $(OUT_DIR)/base64.c.o -c base64.c

$(OUT_DIR)/hmacsha1.c.o: hmacsha1.c crypto.h
	$(CC) $(CFLAGS) -o $(OUT_DIR)/hmacsha1.c.o -c hmacsha1.c

$(OUT_DIR)/S3-Upload.c.o: S3-Upload.c crypto.h config.h
	$(CC) $(CFLAGS) -o $(OUT_DIR)/S3-Upload.c.o -c S3-Upload.c

clean:
	rm -f s3-upload $(OUT_DIR)/*.o

