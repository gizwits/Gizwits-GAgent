CC := gcc  
CFLAGS := -fPIC
AR := ar 
SRC_DIR := ../src
CLOUD_INC := ../../../gagent/cloud/inc
INSTALL_LIB_PATH := ../../../arch/linux/dll
INC_DIR= ../../../gagent/include
ADAPTER := ../adapter
HAL_SERIAL := ../hal_serial
NET := ../net

LOCAL_INC=../../../gagent/local/inc
LAN_INC=../../../lan/local/inc
OBJ=../obj
DLL=../dll
LIB_TARGET := libArch.a

include ../vendor.mak
ifdef MY_DEFINE
  CFLAGS += -D$(MY_DEFINE)
endif
ifdef MY_COMPILER  
	CFLAGS += $(MY_COMPILER)
endif 

INC_PATH += $(INC_DIR) ../../../iof ../../../arch/linux  $(HAL)/ $(LAN_INC)  $(CLOUD_INC) $(LOCAL_INC) $(NET) $(ADAPTER) $(HAL_SERIAL)
CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir)) 

$(LIB_TARGET):\
	$(OBJ)/adapter_linux.o\
	$(OBJ)/hal_uart.o\
	$(OBJ)/netevent.o

	$(AR) crus $(LIB_TARGET) $^
	install -m 0755 $(LIB_TARGET) $(DLL)
	rm -rf $@
	@echo "---------build libArch.a ok.----------"

$(OBJ)/adapter_linux.o:\
	$(ADAPTER)/adapter_linux.c
	$(CC) $(CFLAGS) -c -o $@ $^
	@echo "build adapter_linux.o ok."
  
$(OBJ)/hal_uart.o:\
	$(HAL_SERIAL)/hal_uart.c
	$(CC) $(CFLAGS) -c -o $@ $^
	@echo "build hal_uart.o ok."
  
$(OBJ)/netevent.o:\
	$(NET)/netevent.c
	$(CC) $(CFLAGS) -c -o $@ $^
	@echo "build netevent.o ok."	

folders:
	mkdir -p ../obj
	mkdir -p ../dll
	mkdir -p ../config
	mkdir -p ../target
  
.PHONY:clean help 
clean:
	rm -rf $(DLL)/*
	rm -rf $(DLL)
	rm -rf $(OBJ)/*
	rm -rf $(OBJ)
  
print: $(SRC_DIR)/*.c
	lpr -p $?
	touch print

help:
	@echo "\r\n"
	@echo "------------------------ Gizwits GAgent -----------------------"
	@echo "| 1.type "make help" for help infonation." 
	@echo "| 2.run \"make clean;make\" to build lib." 
	@echo "| 3.the directory dll store the libs."
	@echo "| 4.use \"root\" or add \"sudo\" to run target !!!!!!!!"
	@echo "| * for more detail refer to the readme."
	@echo "---------------------------------------------------------------"
	@echo "|                      	By FrankLiu from Gizwits GAgent Team"
	@echo "---------------------------------------------------------------"
	@echo "\r\n"
