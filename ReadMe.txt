目录说明：
base:
        src基础目录，包含了基于epoll的tcp服务端以及所需要的基础库
common：
        src:tcp的回调类
        test：测试模块
执行make后，会生成对应的底层库与可执行的测试文件到out目录

基于epoll
编译命令：
release: make VER=release
debug: make
clean:make clean
cleanall: make cleanall
