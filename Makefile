CC ?= gcc

INCLUDES = inc
SRC = src

# build target specs
CFLAGS = -I$(INCLUDES) #-Wall -DDEBUG

OUT_DIR = objs

LIBS = -lcurl
default: s3-upload

s3-upload: $(OUT_DIR)/base64.c.o $(OUT_DIR)/hmacsha1.c.o $(OUT_DIR)/S3-Upload.c.o
	$(CC) $(CFLAGS) -o s3-upload $(OUT_DIR)/base64.c.o $(OUT_DIR)/hmacsha1.c.o $(OUT_DIR)/S3-Upload.c.o $(LIBS)

$(OUT_DIR)/base64.c.o: $(SRC)/base64.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)/base64.c.o -c $(SRC)/base64.c

$(OUT_DIR)/hmacsha1.c.o: $(SRC)/hmacsha1.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)/hmacsha1.c.o -c $(SRC)/hmacsha1.c

$(OUT_DIR)/S3-Upload.c.o: $(SRC)/S3-Upload.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)/S3-Upload.c.o -c $(SRC)/S3-Upload.c

clean:
	rm -f s3-upload $(OUT_DIR)/*.o

