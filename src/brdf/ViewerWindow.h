#ifndef VIEWERWINDOW_H
#define VIEWERWINDOW_H

#include <QWidget>
#include "ShowingBase.h"

class GLWindow;

class ViewerWindow : public QWidget, public ShowingBase
{
    Q_OBJECT

public:
    ViewerWindow(GLWindow* widget);
    GLWindow* getWidget() ;

protected:
    void setShowing( bool s );
    void resizeEvent(QResizeEvent * event);

protected:
    GLWindow* glWidget;
};

#endif // VIEWERWINDOW_H
