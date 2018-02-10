CC=gcc
LD=gcc
STRIP=$(CROSS_COMPILE)strip

TARGET = cgen

CFLAGS += -g 
LDFLAGS += -g

OBJS += main.o

all:$(TARGET)

$(TARGET):$(OBJS)
	$(LD) $(OBJS) -o $(TARGET)_g $(LDFLAGS) 
	$(STRIP) --strip-unneeded -o $(TARGET) $(TARGET)_g

clean:
	rm -rf *.d* *.o*
	rm -f $(TARGET)_g $(TARGET)

%.o:%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -o $@ -c $< 
%.d:%.c
	@set -e; rm -f $@; $(CC) -MM $< $(CFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(OBJS:.o=.d)

