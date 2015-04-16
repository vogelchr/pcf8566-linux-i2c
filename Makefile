
CFLAGS  = -Wall -Wextra -ggdb -Os
CFLAGS += -fdiagnostics-color=auto -ffunction-sections -fdata-sections

OBJS = pcf8566.o pcf8566_test.o

all : pcf8566_test

pcf8566_test : $(OBJS)

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d)
endif

.PHONY : clean
clean :
	rm -f *~ *.o *.d pcf8566_test

%.d : %.c
	$(CC) $(CPPFLAGS) -o $@ -MM $^
