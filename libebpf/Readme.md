## libebpf
A ebpf library based on existing projects,  
https://github.com/iovisor/ubpf  
https://github.com/generic-ebpf/generic-ebpf  

## Libubpf
包括libubpf的实现，以及指令测试工具。

#### 目录说明
利用visual studio在windows下开发调试完基本功能，再整合进rt-thread  
cmake生成vs 2017项目：
```
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
rd /s /q vs_2017
mkdir vs_2017
cd .\vs_2017
cmake -G "Visual Studio 15 2017" ..
cd .\vs_2017
devenv /build Debug LibUbpf.sln
echo "build project finish"
```

#### 指令测试工具集用法
编译ebpf代码：
``` bash
cd bin/tools
python compile_code.py -s code.c
```