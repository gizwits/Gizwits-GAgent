CC=gcc -fPIC 
FLAGS=-m32
CLOUD=../../gagent/cloud/src
CLOUD_INC=../../gagent/cloud/inc
UTILS=../../gagent/core
LOCAL=../../gagent/local/src
LOCAL_INC=../../gagent/local/inc
LAN=../../gagent/lan/src
LAN_INC=../../gagent/lan/inc
HAL=hal_serial
NET=net
OBJ=obj
DLL=dll

MAIN=../../gagent/main.c
INC=-I../../iof -I../../arch/linux -I../../gagent/include -I$(HAL)/ -I$(LAN_INC) -I$(CLOUD_INC) -I$(LOCAL_INC) -I$(NET)
TARGE=target
      
# $@ 目标文件
# $^ 全部依赖文件
# $< 第一个依赖文件
# -I 指明头文件位置，后面跟路径.
# -L 指明库所在路径，后面跟库路径.
# -l 指明所需要的库，后面跟库的名字，库的后缀不用写.

$(TARGE)/GAgent_linux:\
	$(OBJ)/user_misc.o\
	$(OBJ)/utilcoding.o\
	$(OBJ)/http.o\
	$(OBJ)/mqttlib.o\
	$(OBJ)/mqttbase.o\
	$(OBJ)/mqttxpg.o\
	$(OBJ)/cloud.o\
	$(OBJ)/mcu_common.o\
	$(OBJ)/hal_uart.o\
	$(OBJ)/local.o\
	$(OBJ)/hal_receive.o\
	$(OBJ)/adapter_linux.o\
	$(OBJ)/netevent.o\
	$(OBJ)/gagent_netevent.o\
	$(OBJ)/lanudp.o \
	$(OBJ)/lantcp.o \
	$(OBJ)/gagent.o \
	$(OBJ)/lan.o\
	$(OBJ)/webconfig.o
	

	$(CC) $(MAIN) $(INC) $(FLAGS) -L$(DLL) -lrt -lm -o $@ $^
	@echo "build GAgent_linux ok."
	@echo "\r\n"
	@echo "------------------------ Gizwits GAgent -----------------------"
	@echo "| 1.type "make help" for help infonation." 
	@echo "| 2.run \"make folders\" to make folders in first time."
	@echo "| 3.run \"make\" to build target." 
	@echo "| 4.run libGAgent.so to make lib file."
	@echo "| 5.use \"root\" or add \"sudo\" to run target !!!!!!!!"
	@echo "| * for more detail refer to the readme."
	@echo "---------------------------------------------------------------"
	@echo "|                      	By Alex.lin from Gizwits GAgent Team"
	@echo "---------------------------------------------------------------"
	@echo "\r\n"

folders:
	mkdir -p ./obj
	mkdir -p ./dll
	mkdir -p ./config
	mkdir -p $(TARGE)
    
	@echo "mkdir folders ok."
libGAgent.so:\
	$(OBJ)/user_misc.o\
	$(OBJ)/utilcoding.o\
	$(OBJ)/http.o\
	$(OBJ)/mqttlib.o\
	$(OBJ)/mqttbase.o\
	$(OBJ)/mqttxpg.o\
	$(OBJ)/cloud.o\
	$(OBJ)/mcu_common.o\
	$(OBJ)/hal_uart.o\
	$(OBJ)/local.o\
	$(OBJ)/hal_receive.o\
	$(OBJ)/adapter_linux.o\
	$(OBJ)/netevent.o\
	$(OBJ)/gagent_netevent.o\
	$(OBJ)/lanudp.o \
	$(OBJ)/lantcp.o \
	$(OBJ)/gagent.o \
	$(OBJ)/lan.o\
	$(OBJ)/webconfig.o

	$(CC) $(INC) $(FLAGS) -shared -fPIC  -lrt -lm -o $@ $^
	mv $@ $(DLL)
	@echo "GAgent.so Build ok."

$(OBJ)/http.o:\
	$(CLOUD)/http.c
	$(CC) $(INC) -c -o $@ $^
	@echo "build http.o ok."
$(OBJ)/cloud.o:\
	$(CLOUD)/cloud.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build cloud.o ok."	
$(OBJ)/mqttxpg.o:\
	$(OBJ)/mqttbase.o\
	$(CLOUD)/mqttxpg.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build mqttxpg.o ok."

$(OBJ)/mqttbase.o:\
	$(OBJ)/mqttlib.o\
	$(CLOUD)/mqttbase.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build mqttbase.o ok."
$(OBJ)/netevent.o:\
	$(NET)/netevent.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build netevent.o ok."

$(OBJ)/gagent_netevent.o:\
	../../gagent/gagent_netevent.c	
	$(CC) -c $(INC) -o $@ $^
	@echo "build gagent_netevent.o ok."
$(OBJ)/mqttlib.o:\
	$(CLOUD)/mqttlib.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build mqttlib.o ok."
$(OBJ)/adapter_linux.o:\
	adapter_linux.c
	$(CC) -lrt -lm -c $(INC) -o $@ $^
	@echo "build adapter_linux.o ok."
$(OBJ)/gagent.o:\
	../../gagent/gagent.c	
	$(CC) -c $(INC) -o $@ $^
	@echo "build gagent.o ok."
$(OBJ)/hal_uart.o:\
	$(HAL)/hal_uart.c
	$(CC) -c $(INC) -o $@ $^
	@echo $@ "ok."

$(OBJ)/user_misc.o:\
	$(UTILS)/user_misc.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build user_misc.o ok."
$(OBJ)/utilcoding.o:\
	$(UTILS)/utilcoding.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build utilcoding.o ok."

$(OBJ)/hal_receive.o:\
	$(LOCAL)/hal_receive.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build hal_receive.o ok."
$(OBJ)/local.o:\
	$(LOCAL)/local.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build local.o ok."
$(OBJ)/mcu_common.o:\
	$(LOCAL)/mcu_common.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build mcu_common.o ok."
$(OBJ)/lan.o:\
	$(LAN)/lan.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build lan.o ok."

$(OBJ)/lanudp.o:\
	$(LAN)/lanudp.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build lanudp.o ok."

$(OBJ)/lantcp.o:\
	$(LAN)/lantcp.c
	$(CC) -c $(INC) -o $@ $^
	@echo "build lantcp.o ok."
	
$(OBJ)/webconfig.o:\
	$(LAN)/webconfig.c	
	$(CC) -c $(INC) -o $@ $^
	@echo "build webconfig.o ok."

.PHONY:clean help 
clean:
	rm -rf $(OBJ)/*
	rm -rf $(DLL)/*
	rm -rf $(TARGE)/*
	@echo "clean all"
help:
	@echo "\r\n"
	@echo "------------------------ Gizwits GAgent -----------------------"
	@echo "| 1.type "make help" for help infonation." 
	@echo "| 2.run \"make folders\" to make folders in first time."
	@echo "| 3.run \"make\" to build target." 
	@echo "| 4.run libGAgent.so to make lib file."
	@echo "| 5.use \"root\" or add \"sudo\" to run target !!!!!!!!"
	@echo "| * for more detail refer to the readme."
	@echo "---------------------------------------------------------------"
	@echo "|                      	By Alex.lin from Gizwits GAgent Team"
	@echo "---------------------------------------------------------------"
	@echo "\r\n"