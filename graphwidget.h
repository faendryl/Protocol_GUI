/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QtGui/QGraphicsView>
#include <QList>
#include <QHash>
#include <QSignalMapper>
#include <Python.h>


class Node;
class Edge;

class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    enum Mode {CreateNode, CreateEdge, MoveNode};

    GraphWidget();
    ~GraphWidget();

    void itemMoved();

private slots:
    void newNode();
    void newEdge();
    void moveNode();
    void createNode(QString moduleName);

signals:
    void requestProtocolNode(char *protocolName);

protected:
    
    void keyPressEvent(QKeyEvent *event);
    void execute();
    void timerEvent(QTimerEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void contextMenuEvent(QContextMenuEvent *event);

    void scaleView(qreal scaleFactor);

private:
    int timerId;
    Node *centerNode;
    Edge *creatingEdge;
    QAction *newEdgeAct;
    QAction *newNodeAct;
    QAction *moveNodeAct;
    Mode mode;
    QString activeModule;
    QList<PyObject*> moduleList;
    QList<QAction*> moduleActionList;
    QSignalMapper *signalMapper;
    QHash<QString,PyObject*> hashProtocolNameToModule;
};

#endif
