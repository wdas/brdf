
VERSION=1.0.0

make clean
qmake-qt4 -spec linux-mingw32-custom
make -j 8

DIR=brdf-$VERSION-win32
rm -rf $DIR
mkdir $DIR
pushd $DIR
cp -a ../src/{data,probes,images,brdfs,shaderTemplates} ./
cp /jobs2/soft/users/aselle/windows-build/QtSDK/Desktop/Qt/4.6.4/bin/{QtGui4.dll,QtCore4.dll,QtOpenGL4.dll,libgcc_s_dw2-1.dll,mingwm10.dll} ./
cp ../src/brdf/release/brdf.exe ./
cp ../{LICENSE,LICENSE-BINARY,README} ./
popd
zip -r brdf-$VERSION-win32.zip brdf-$VERSION-win32