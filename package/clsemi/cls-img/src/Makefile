all: mkclsimg checkclsimg

mkclsimg: mkclsimg.o
	$(CC) $(LDFLAGS) -o $@ $< -lz

checkclsimg: checkclsimg.o
	$(CC) $(LDFLAGS) -o $@ $< -lz

%.o: %.c clsimg.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm mkclsimg checkclsimg split_img_dir -rf
	@rm *.o *.img -f

test: all
	./mkclsimg -p ../images -u u-boot.bin -d qemu.dtb -f firmware -s write-image.sh -i image-info -o img
	./checkclsimg img

.PHONY: clean
