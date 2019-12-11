TEMPLATE = app
CONFIG += qt5  #debug

isEmpty(prefix) {
	prefix = $$system(pf-makevar --absolute root 2>/dev/null)
}
isEmpty(prefix) {
	error("$prefix is undefined. Please pass prefix=<path> to qmake")
}

DEST = $$prefix
isEmpty(LIBDIR) {
	LIBDIR = $$system(pf-makevar lib 2>/dev/null)
}
isEmpty(LIBDIR) {
	LIBDIR = lib
}

TARGET = brdf
target.path = $$DEST/bin

HEADERS = *.h
SOURCES = \
    Quad.cpp \
    Sphere.cpp \
    BRDFAnalytic.cpp \
    BRDFBase.cpp \
    BRDFImageSlice.cpp \
    BRDFMeasuredAniso.cpp \
    BRDFMeasuredMERL.cpp \
    ColorVarWidget.cpp \
    FloatVarWidget.cpp \
    DGLFrameBuffer.cpp \
    DGLShader.cpp \
    IBLWidget.cpp \
    IBLWindow.cpp \
    ImageSliceWidget.cpp \
    ImageSliceWindow.cpp \
    LitSphereWindow.cpp \
    main.cpp \
    glerror.cpp \
    MainWindow.cpp \
    ViewerWindow.cpp \
    ParameterGroupWidget.cpp \
    ParameterWindow.cpp \
    SharedContextGLWidget.cpp \
    ShowingDockWidget.cpp \
    PlotCartesianWindow.cpp \
    PlotCartesianWidget.cpp \
    PlotPolarWidget.cpp \
    Plot3DWidget.cpp \
    LitSphereWidget.cpp \
    SimpleModel.cpp \
    Paths.cpp \
    ptex/PtexReader.cpp \
    ptex/PtexUtils.cpp \
    ptex/PtexCache.cpp \
    ptex/PtexHalf.cpp


QT   += widgets opengl
DEFINES += PTEX_STATIC NOMINMAX

macx {
    CONFIG -= app_bundle
}

brdfs.path = $$DEST/share/brdf/brdfs
brdfs.files = ../brdfs/*

data.path = $$DEST/share/brdf/data
data.files = ../data/*

images.path = $$DEST/share/brdf/images
images.files = ../images/*

probes.path = $$DEST/share/brdf/probes
probes.files = ../probes/*

shaderTemplates.path = $$DEST/share/brdf/shaderTemplates
shaderTemplates.files = ../shaderTemplates/*

pkgconfig.path = $$DEST/$$LIBDIR/pkgconfig
pkgconfig.files = brdf.pc

INSTALLS = target brdfs data images probes shaderTemplates pkgconfig

win32-msvc*{
    INCLUDEPATH += ZLIB_DIR
    DEFINES += ZLIB_WINAPI
    LIBS += ZLIB_LIB
}

win32-g++*{
    LIBS += -lz
}

unix*{
    LIBS += -lz
}

# Windows cross compile at disney
linux-mingw32-custom{
    WINDOWS_BUILD=/jobs2/soft/users/aselle/windows-build
    LIBS += -static-libgcc
}
