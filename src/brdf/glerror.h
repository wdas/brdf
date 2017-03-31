#ifndef GLERROR_H
#define GLERROR_H

bool QueryExtension(const char *extName);
int PrintOpenGLError(const char *file, int line, const char *msg = 0);

#define CKGL() PrintOpenGLError(__FILE__, __LINE__)

#endif // GLERROR_H
