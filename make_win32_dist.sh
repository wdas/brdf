make clean
qmake-qt4 -spec linux-mingw32-custom
make -j 8

rm -rf brdf-win32
mkdir brdf-win32
pushd brdf-win32
cp -a ../src/{data,probes,images,brdfs,shaderTemplates} ./
cp /jobs2/soft/users/aselle/windows-build/QtSDK/Desktop/Qt/4.6.4/bin/{QtGui4.dll,QtCore4.dll,QtOpenGL4.dll,libgcc_s_dw2-1.dll,mingwm10.dll} ./
cp ../src/brdf/release/brdf.exe ./
cp ../{LICENSE,LICENSE-BINARY} ./
popd
zip -r brdf-win32.zip brdf-win32