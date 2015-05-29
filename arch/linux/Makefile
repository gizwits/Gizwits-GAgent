SUBDIRS = ./makefile_arch ../../../gagent/cloud ../../gagent/core ../../gagent/lan ../../gagent/local ../../gagent/main 

.PHONY: all
all:
	mkdir -p ./obj
	mkdir -p ./dll
	mkdir -p ./config
	mkdir -p ./target 
	mkdir -p ../../gagent/cloud/src/obj
	mkdir -p ../../gagent/core/src/obj
	mkdir -p ../../gagent/lan/src/obj
	mkdir -p ../../gagent/local/src/obj
	mkdir -p ../../gagent/main/src/obj
    	  
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		echo "Clean in $$subdir";\
		cd $$subdir && $(MAKE);\
	done

.PHONY: clean
clean:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		echo "Clean in $$subdir";\
		cd $$subdir && $(MAKE) clean;\
	done

.PHONY: folders  
folders:
	mkdir -p ./obj
	mkdir -p ./dll
	mkdir -p ./config
	mkdir -p ./target 
	mkdir -p ../../gagent/cloud/src/obj
	mkdir -p ../../gagent/core/src/obj
	mkdir -p ../../gagent/lan/src/obj
	mkdir -p ../../gagent/local/src/obj
	mkdir -p ../../gagent/main/src/obj
  

  
  
     
