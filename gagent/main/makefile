CC := gcc  
CFLAGS := -fPIC
AR := ar 
SRC_DIR := ./src
CLOUD_INC := ../cloud/inc 
INSTALL_LIB_PATH := ../../arch/linux/dll
INC_DIR := ../include
TARGET := GAgent_linux

LOCAL_INC=../local/inc
LAN_INC=../lan/inc
HAL=../../arch/linux/hal_serial
NET=../../arch/linux/net
ADAPTER=../../arch/linux/adapter
APP_PATH=../../arch/linux/target
OBJ=../../arch/linux/obj
DLL=../../arch/linux/dll

include ../../arch/linux/vendor.mak
ifdef MY_DEFINE
  CFLAGS += -D$(MY_DEFINE)
endif
ifdef MY_COMPILER  
	CFLAGS += $(MY_COMPILER)
endif 

INC_PATH += $(INC_DIR) ../../iof ../../arch/linux  $(HAL)/ $(LAN_INC)  $(CLOUD_INC) $(LOCAL_INC) $(NET) $(ADAPTER)
CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))

LDFLAGS += ../../arch/linux/dll/libArch.a
LDFLAGS += ../../arch/linux/dll/libCloud.a
LDFLAGS += ../../arch/linux/dll/libCore.a
LDFLAGS += ../../arch/linux/dll/libLan.a
LDFLAGS += ../../arch/linux/dll/libLocal.a

OBJS :=	$(SRC_DIR)/obj/gagent \
	$(SRC_DIR)/obj/main  

OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(SRC_DIR)/obj/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)
	install -m 755 $(TARGET) $(APP_PATH)
	rm -rf $(TARGET)	
	@echo "------------------------ Gizwits GAgent -----------------------"
	@echo "| 1.type "make help" for help infonation." 
	@echo "| 2.run \"make clean;make\" to build target." 
	@echo "| 3.the directory dll store the libs."
	@echo "| 4.use \"root\" or add \"sudo\" to run target !!!!!!!!"
	@echo "| * for more detail refer to the readme."
	@echo "---------------------------------------------------------------"
  
.PHONY:clean  
clean:
	rm -rf $(SRC_DIR)/obj/*
	rm -rf $(SRC_DIR)/obj
	rm -rf $(TARGET)
	rm -rf ../../arch/linux/target
	@echo "clean all"

.PHONY:folders  
folders:
	mkdir -p ./src/obj 
  
print: $(SRC_DIR)/*.c
	lpr -p $?
	touch print

help:
	@echo "\r\n"
	@echo "------------------------ Gizwits GAgent -----------------------"
	@echo "| 1.type "make help" for help infonation." 
	@echo "| 2.run \"make clean;make\" to build target." 
	@echo "| 3.the directory dll store the libs."
	@echo "| 4.use \"root\" or add \"sudo\" to run target !!!!!!!!"
	@echo "| * for more detail refer to the readme."
	@echo "---------------------------------------------------------------"
	@echo "|                      	By FrankLiu from Gizwits GAgent Team"
	@echo "---------------------------------------------------------------"
	@echo "\r\n"
