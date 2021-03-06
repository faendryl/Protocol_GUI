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

#include "graphwidget.h"
#include "edge.h"
#include "node.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QQueue>
#include <QMenu>
#include <QDir>

#include <math.h>
#include <iostream>

#include <Python.h>

GraphWidget::GraphWidget()
    : timerId(0),creatingEdge(0),mode(MoveNode)
{
    Py_Initialize();
    PyEval_InitThreads();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('modules')");

    QDir moduleDirectory("modules");
    QStringList moduleNameFilter("*.py");
    QStringList moduleNameList=moduleDirectory.entryList(moduleNameFilter);
    for(QStringList::Iterator it=moduleNameList.begin();it!=moduleNameList.end();++it){
        (*it).chop(3);
    }
    signalMapper=new QSignalMapper(this);
    
    foreach(QString moduleName,moduleNameList){
        PyObject *protocolName=PyString_FromString(moduleName.toAscii());
        PyObject *module=PyImport_Import(protocolName);
        hashProtocolNameToModule[moduleName]=module;
        moduleList.append(module);
        Py_DECREF(protocolName);
        QAction *protocolAction=new QAction(moduleName,this);
        moduleActionList.append(protocolAction);
        signalMapper->setMapping(protocolAction,moduleName);
        connect(protocolAction,SIGNAL(triggered()),signalMapper,SLOT(map()));
        qDebug()<<moduleName;
    }

    connect(signalMapper,SIGNAL(mapped(QString)),this,SLOT(createNode(QString)));



    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-200, -200, 400, 400);
    setScene(scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorViewCenter);

    /*Node *node1 = new Node(this);
    Node *node2 = new Node(this);
    Node *node3 = new Node(this);
    Node *node4 = new Node(this);*/
    //PyObject *protocolName=PyString_FromString("test_module");
    centerNode = new Node(this,moduleList.front());
    //Py_DECREF(protocolName);
    /*Node *node6 = new Node(this);
    Node *node7 = new Node(this);
    Node *node8 = new Node(this);
    Node *node9 = new Node(this);
    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(node3);
    scene->addItem(node4);*/
    scene->addItem(centerNode);
    /*scene->addItem(node6);
    scene->addItem(node7);
    scene->addItem(node8);
    scene->addItem(node9);
    scene->addItem(new Edge(node1, node2));
    scene->addItem(new Edge(node2, node3));
    scene->addItem(new Edge(node2, centerNode));
    scene->addItem(new Edge(node3, node6));
    scene->addItem(new Edge(node4, node1));
    scene->addItem(new Edge(node4, centerNode));
    scene->addItem(new Edge(centerNode, node6));
    scene->addItem(new Edge(centerNode, node8));
    scene->addItem(new Edge(node6, node9));
    scene->addItem(new Edge(node7, node4));
    scene->addItem(new Edge(node8, node7));
    scene->addItem(new Edge(node9, node8));

    node1->setPos(-50, -50);
    node2->setPos(0, -50);
    node3->setPos(50, -50);
    node4->setPos(-50, 0);*/
    centerNode->setPos(0, 0);
    /*node6->setPos(50, 0);
    node7->setPos(-50, 50);
    node8->setPos(0, 50);
    node9->setPos(50, 50);*/


    newNodeAct=new QAction(tr("&New Node"),this);
    newNodeAct->setShortcuts(QKeySequence::New);
    newNodeAct->setStatusTip(tr("Create a new node"));
    connect(newNodeAct,SIGNAL(triggered()),this,SLOT(newNode()));

    newEdgeAct=new QAction(tr("N&ew Edge"),this);
    //newEdgeAct->setShortcuts(QKeySequence::New);
    newEdgeAct->setStatusTip(tr("Create a new edge"));
    connect(newEdgeAct,SIGNAL(triggered()),this,SLOT(newEdge()));

    moveNodeAct=new QAction(tr("&Move Nodes"),this);
    moveNodeAct->setStatusTip(tr("Move nodes"));
    connect(moveNodeAct,SIGNAL(triggered()),this,SLOT(moveNode()));


    scale(qreal(0.8), qreal(0.8));
    setMinimumSize(400, 400);
    setWindowTitle(tr("Elastic Nodes"));



}

GraphWidget::~GraphWidget()
{
    foreach(PyObject* module,moduleList){
        Py_DECREF(module);
    }
    Py_Finalize();
    delete signalMapper;
}

void GraphWidget::createNode(QString moduleName)
{
    mode=CreateNode;
    activeModule=moduleName;
}

void GraphWidget::newNode()
{
    std::cout<<"Create new node"<<std::endl;
    mode=CreateNode;
}

void GraphWidget::moveNode()
{
    std::cout<<"Move node"<<std::endl;
    mode=MoveNode;
}

void GraphWidget::itemMoved()
{
    if (!timerId)
        timerId = startTimer(1000 / 25);
}

void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        centerNode->moveBy(0, -20);
        break;
    case Qt::Key_Down:
        centerNode->moveBy(0, 20);
        break;
    case Qt::Key_Left:
        centerNode->moveBy(-20, 0);
        break;
    case Qt::Key_Right:
        centerNode->moveBy(20, 0);
        break;
    case Qt::Key_Plus:
        scaleView(qreal(1.2));
        break;
    case Qt::Key_Minus:
        scaleView(1 / qreal(1.2));
        break;
    case Qt::Key_Space:
    case Qt::Key_Enter:
        execute();
        /*QQueue<NodeWithInputs*> processingNodes;
        processingNodes.append(new NodeWithInputs(centerNode));
        while(!processingNodes.isEmpty()){
            NodeWithInputs* 
            }*/
        /*foreach (QGraphicsItem *item, scene()->items()) {
            if (qgraphicsitem_cast<Node *>(item))
                item->setPos(-150 + qrand() % 300, -150 + qrand() % 300);
                }*/
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void GraphWidget::execute()
{
    QList<Node *> nodes;
    QList<Edge *> edges;
    foreach (QGraphicsItem *item, scene()->items()) {
        if (Node *node = qgraphicsitem_cast<Node *>(item))
            nodes << node;
        if (Edge *edge = qgraphicsitem_cast<Edge *>(item))
            edges << edge;
    }

    foreach (Node *node, nodes)
        node->prepareExecution();

    foreach (Edge *edge, edges)
        edge->prepareExecution();

    QQueue<Node*> activeNodes;
    activeNodes.append(centerNode);
    while(!activeNodes.isEmpty()){
        Node *activeNode=activeNodes.head();
        activeNodes.dequeue();
        if(activeNode->execute()){
            foreach(Edge *edge,activeNode->outgoingEdges()){
                activeNodes.enqueue(edge->destNode());
            }
        }
    }
}

void GraphWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    QList<Node *> nodes;
    QList<Edge *> edges;
    foreach (QGraphicsItem *item, scene()->items()) {
        if (Node *node = qgraphicsitem_cast<Node *>(item))
            nodes << node;
        if (Edge *edge = qgraphicsitem_cast<Edge *>(item))
            edges << edge;
    }

    foreach (Node *node, nodes)
        node->calculateForces();

    foreach (Edge *edge, edges)
        edge->adjust();

    bool itemsMoved = false;
    foreach (Node *node, nodes) {
        if (node->advance())
            itemsMoved = true;
    }

    if (!itemsMoved) {
        killTimer(timerId);
        timerId = 0;
    }
}

void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    //if(scene()->mouseGrabberItem()) return;
    QPointF scenePos(mapToScene(event->pos()));
    if(mode==CreateEdge && event->button()==Qt::LeftButton){
        QList<QGraphicsItem*> startItems=scene()->items(scenePos);
        foreach (QGraphicsItem *item,startItems){
            Node *startNode=qgraphicsitem_cast<Node*>(item);
            if(startNode){
                Edge *edge=new Edge(startNode,0);
                scene()->addItem(edge);
                edge->setPos(mapToScene(event->pos()));
                creatingEdge=edge;
                break;
            }
        }
    }
    else if(mode==CreateNode && event->button()==Qt::LeftButton){
        qDebug()<<"New node "<<activeModule;
        Node *node=new Node(this,hashProtocolNameToModule[activeModule]);
        node->setPos(scenePos.x(),scenePos.y());
        scene()->addItem(node);
    }
    else
        QGraphicsView::mousePressEvent(event);
}

void GraphWidget::newEdge()
{
    std::cout<<"Create new edge"<<std::endl;
    mode=CreateEdge;
}

void GraphWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(mode==CreateEdge && event->button()==Qt::LeftButton && creatingEdge){
        QPointF scenePos(mapToScene(event->pos()));
        QList<QGraphicsItem*> endItems=scene()->items(scenePos);
        foreach (QGraphicsItem* item, endItems){
            Node* endNode=qgraphicsitem_cast<Node*>(item);
            if(!endNode) continue;
            creatingEdge->setDestNode(endNode);
            creatingEdge=0;
            return;
        }
        scene()->removeItem(creatingEdge);
        delete creatingEdge;
        creatingEdge=0;
    }
    else
        QGraphicsView::mouseReleaseEvent(event);
}

void GraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    if(creatingEdge){
        creatingEdge->setPos(mapToScene(event->pos()));
        creatingEdge->adjust();
    }
}

void GraphWidget::wheelEvent(QWheelEvent *event)
{
    scaleView(pow((double)2, -event->delta() / 240.0));
}

void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    // Shadow
    QRectF sceneRect = this->sceneRect();
    QRectF rightShadow(sceneRect.right(), sceneRect.top() + 5, 5, sceneRect.height());
    QRectF bottomShadow(sceneRect.left() + 5, sceneRect.bottom(), sceneRect.width(), 5);
    if (rightShadow.intersects(rect) || rightShadow.contains(rect))
	painter->fillRect(rightShadow, Qt::darkGray);
    if (bottomShadow.intersects(rect) || bottomShadow.contains(rect))
	painter->fillRect(bottomShadow, Qt::darkGray);

    // Fill
    QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::lightGray);
    painter->fillRect(rect.intersect(sceneRect), gradient);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);

    // Text
    QRectF textRect(sceneRect.left() + 4, sceneRect.top() + 4,
                    sceneRect.width() - 4, sceneRect.height() - 4);
    QString message(tr("Click and drag the nodes around, and zoom with the mouse "
                       "wheel or the '+' and '-' keys"));

    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(14);
    painter->setFont(font);
    painter->setPen(Qt::lightGray);
    painter->drawText(textRect.translated(2, 2), message);
    painter->setPen(Qt::black);
    painter->drawText(textRect, message);
}

void GraphWidget::contextMenuEvent(QContextMenuEvent *event)
{
    std::cout<<"ey"<<std::endl;
    QMenu menu(this);
    //menu.addAction(newNodeAct);
    QMenu protocolMenu(&menu);
    foreach(QAction *action,moduleActionList){
        protocolMenu.addAction(action);
    }
    menu.addMenu(&protocolMenu);
    menu.addAction(newEdgeAct);
    menu.addAction(moveNodeAct);
    menu.exec(event->globalPos());
}


void GraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}
