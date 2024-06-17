#ifndef MAIN_H
#define MAIN_H

#include <QMouseEvent>
#include <mainwindow.h>

// Declarations of the functions to handle mouse events
void handleMousePressed(QMouseEvent *event);
void handleMouseReleased(QMouseEvent *event);
void handleMouseMoved(QMouseEvent *event);

#endif // MAIN_H
