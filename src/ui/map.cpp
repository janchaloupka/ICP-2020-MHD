#include "map.hpp"
//#include "ui_map.h"
#include "mapstreet.hpp"

#include <QtDebug>

Map::Map(StreetList streets, QObject *parent) : QGraphicsScene(parent)
{
    selectedMapStreet = nullptr;

    this->setBackgroundBrush(QBrush(QColor(210, 210, 210)));
    for(const auto s: streets){
        addItem(new MapStreet(s));
    }
}

Map::~Map()
{

}

void Map::changeStreetTraffic(int flow)
{
    if(selectedMapStreet == nullptr) return;
    selectedMapStreet->getStreet()->setTrafficFlow(flow / 100.0);
    invalidate();
}

void Map::updateBus(QPoint pos)
{

}

void Map::addBus(int id, QString line)
{

}

void Map::removeBus(int id)
{

}

void Map::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    auto item = itemAt(mouseEvent->scenePos(), QTransform());

    for(auto i : items()){
        if(MapBus *b = dynamic_cast<MapBus *>(i)){
            b->setSelected(false);
        }else if(MapStreet *s = dynamic_cast<MapStreet *>(i)){
            s->setSelected(false);
        }
    }

    selectedMapBus = dynamic_cast<MapBus *>(item);
    selectedMapStreet = dynamic_cast<MapStreet *>(item);

    if(selectedMapStreet != nullptr){
        selectedMapStreet->setSelected(true);
        emit streetSelected(selectedMapStreet->getStreet());
    }else{
        emit streetSelected(nullptr);
    }

    if(selectedMapBus != nullptr){
        selectedMapBus->setSelected(true);
        emit busSelected(selectedMapBus->getId());
    }

    invalidate();
    //if(selectedMapStreet != nullptr) selectedMapStreet->setSelected(false);
    QGraphicsScene::mousePressEvent(mouseEvent);
}
