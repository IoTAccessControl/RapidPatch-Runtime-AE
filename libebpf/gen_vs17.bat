call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
rd /s /q vs_2017
mkdir vs_2017
cd .\vs_2017
cmake -G "Visual Studio 15 2017" ..
cd .\vs_2017
devenv /build Debug LibEbpf.sln
echo "build project finish"