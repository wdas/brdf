#include "ViewerWindow.h"
#include <QVBoxLayout>
#include "SharedContextGLWidget.h"

ViewerWindow::ViewerWindow(GLWindow *widget)
{
    glWidget = widget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(QWidget::createWindowContainer(glWidget));
    setLayout(mainLayout);
}

void ViewerWindow::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event)
    glWidget->updateGL();
}

void ViewerWindow::setShowing( bool s )
{
    if( glWidget ){
        glWidget->setShowing( s );
        if(s)
            glWidget->updateGL();
    }
}

GLWindow* ViewerWindow::getWidget()
{
    return glWidget;
}
