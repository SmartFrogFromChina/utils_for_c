CROSS_COMPILE = /home/frog/montage/dueros/panther_sdk_20180124/panther_sdk/panther/toolchains/toolchain-mipsel_interaptiv_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-

CC = $(CROSS_COMPILE)gcc -Wall -std=c99 
AR = $(CROSS_COMPILE)ar
A   = @

CFLAGS  = -I .
LDFLAGS = -L .
ENCODER_LIBS  = -lm
DECODER_LIBS  = -lm

ENCODE_TARGET = encode_main
DECODE_TARGET = decode_main

ENCODE_LIB_TARGET = libvoice_encode.a
DECODE_LIB_TARGET = libvoice_decode.a

MIPS_DECODE_LIB   = libvoice.a

DECODER_MAIN =  decode_main.o
DECODER_OBJS += kiss_fft/kiss_fft.o
DECODER_OBJS += kiss_fft/kiss_fftr.o
DECODER_OBJS += rscode/rscode.o
DECODER_OBJS += voice_decode.o
DECODER_OBJS += fifobuffer.o 

ENCODER_MAIN  = encode_main.o
ENCODER_OBJS += rscode/rscode.o
ENCODER_OBJS += voice_encode.o

all:$(DECODE_TARGET) $(ENCODE_TARGET) $(DECODE_LIB_TARGET) $(ENCODE_LIB_TARGET) $(MIPS_DECODE_LIB)
	

$(DECODE_TARGET):$(DECODER_OBJS) $(DECODER_MAIN)
	@echo "Building $@ start........."
	$(A) $(CC) -o $@ $^ $(LDFLAGS) $(DECODER_LIBS)
	@echo "Building $@ finish........"

$(ENCODE_TARGET):$(ENCODER_OBJS) $(ENCODER_MAIN)
	@echo "Building $@ start........."
	$(A) $(CC) -o $@ $^ $(LDFLAGS) $(ENCODER_LIBS)
	@echo "Building $@ finish........"

$(DECODE_LIB_TARGET):$(DECODER_OBJS)
	$(AR) -rcs $@ $(DECODER_OBJS)

$(ENCODE_LIB_TARGET):$(ENCODER_OBJS)
	$(AR) -rcs $@  $(ENCODER_OBJS)


$(MIPS_DECODE_LIB):$(DECODER_OBJS)
	$(AR) -rcs $@ $(DECODER_OBJS)
	cp $@ /home/frog/montage/dueros/panther_sdk_20180124/panther_sdk/openwrt/package/legacy/sinvoice/files/lib
	

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(A) rm -rf *.o *.a $(DECODER_OBJS) $(ENCODER_OBJS) $(DECODE_TARGET) $(ENCODER_TARGET)
