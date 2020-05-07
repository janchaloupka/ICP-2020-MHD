#include "mapstreet.hpp"
#include <QPainterPath>
#include <QMatrix>
#include <QtMath>
#include <QtDebug>

const QBrush MapStreet::highlightBrush = QBrush(QColor(200, 200, 255));
const QBrush MapStreet::blackBrush = QBrush(QColor(0, 0, 0));
const QBrush MapStreet::whiteBrush = QBrush(QColor(255, 255, 255));

MapStreet::MapStreet(Street *street, QGraphicsItem *parent): QGraphicsObject(parent)
{
    this->street = street;
    selected = false;

    QFont font;
    font.setPixelSize(10);

    QFontMetrics fontMetrics(font);
    auto fontWidth = fontMetrics.width(street->getName());

    QLineF line(street->getBegin(), street->getEnd());
    QPoint diff = street->getEnd() - street->getBegin();
    float angle = qRadiansToDegrees(qAtan2(diff.y(), diff.x()));

    paintPath.setFillRule(Qt::FillRule::WindingFill);
    paintPath.addRoundedRect(-8, -8, line.length() + 8, 16, 8, 8);
    if(street->getStopPos() >= 0){
        paintPath.addEllipse(QPoint(line.length()*street->getStopPos(), 0), 11, 11);
    }

    namePath.addText(line.length()/2 - fontWidth/2, 4, font, street->getName());

    QMatrix mat;
    mat.rotate(angle);
    paintPath = mat.map(paintPath);
    namePath = mat.map(namePath);

    mat.reset();
    mat.translate(street->getBegin().x(), street->getBegin().y());
    paintPath = mat.map(paintPath);
    namePath = mat.map(namePath);
}

void MapStreet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(QPen(selected ? whiteBrush.color() : highlightBrush.color(), 3));
    painter->setRenderHints(QPainter::Antialiasing);

    painter->fillPath(paintPath, QBrush(selected ? QColor(100, 100, 255) : QColor(255, 255*street->getTrafficFlow(), 255*street->getTrafficFlow())));

    if(street->getStopPos() >= 0){
        auto busStop = street->getBegin() + (street->getEnd() - street->getBegin()) * street->getStopPos();
        painter->drawEllipse(busStop, 6, 6);
    }

    painter->fillPath(namePath, selected ? whiteBrush : blackBrush);
}

QRectF MapStreet::boundingRect() const
{
    return paintPath.boundingRect();
}

QPainterPath MapStreet::shape() const
{
    return paintPath;
}

void MapStreet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    setSelected(shape().contains(event->pos()));

    event->accept();
    QGraphicsObject::mousePressEvent(event);
}

bool MapStreet::getSelected() const
{
    return selected;
}

void MapStreet::setSelected(bool value)
{
    auto origSelected = selected;
    selected = value;
    if(origSelected != value){
        emit selectionChanged(this);
    }
}

Street *MapStreet::getStreet()
{
    return street;
}
