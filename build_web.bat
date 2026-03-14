cd emsdk
call .\emsdk_env.bat

cd ..
call emcmake cmake -B build-web
call cmake --build build-web