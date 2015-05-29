#当定义MY_COMPILER为-m64的话采用64位编译，-m32的话采用32位编译 
MY_COMPILER =-m64 -O0 
#-Wall -Werror

#CONFIG_GAGENT_LOCAL为是否裁剪local模块，裁剪:将MY_DEFINE注释掉，否则将其打开,当用户按照自己的需求添加local模块的话，可直接用beyond compare对比gagent/local/src目录下的local.c和local_simu.c,local.c为机智云对mcu的标准实现，local_simu.c为客户自己平台local模块的实现。
MY_DEFINE =CONFIG_GAGENT_LOCAL

#CONFIG_GAGENT_LAN为是否裁剪lan模块，裁剪:将MY_DEFINE注释掉
MY_DEFINE_LAN =CONFIG_GAGENT_LAN