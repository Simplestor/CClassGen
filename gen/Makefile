#CROSS_COMPILE=arm-linux-
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

TARGET = simple_bin

CFLAGS += -g -Wall -I./common

LDFLAGS += -g -lpthread

OBJS += main.o simple.o\
		common/lock.o common/log.o common/wait.o

all:$(TARGET)

$(TARGET):$(OBJS)
	$(LD) $(OBJS) -o $(TARGET)_g $(LDFLAGS) 
	$(STRIP) --strip-unneeded -o $(TARGET) $(TARGET)_g

clean:
	rm -rf *.d* *.o*
	rm -rf common/*.d* common/*.o*
	rm -f $(TARGET)_g $(TARGET)

%.o:%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -o $@ -c $< 
%.d:%.c
	@set -e; rm -f $@; $(CC) -MM $< $(CFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(OBJS:.o=.d)

