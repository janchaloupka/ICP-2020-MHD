#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include "../parser/parsestreets.hpp"
#include "../parser/parseroutes.hpp"
#include "../parser/parselines.hpp"
#include "../parser/parsetimetables.hpp"
#include "../functions/simulation.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mapScene = nullptr;
    lastOpenedPath = "../examples";

    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeApp()));
    connect(ui->actionOpenSim, SIGNAL(triggered()), this, SLOT(selectSimulationFolder()));
    connect(ui->actionReload, SIGNAL(triggered()), this, SLOT(loadSimulationData()));

    connect(ui->timer, SIGNAL(tick(int)), this, SLOT(simulationStep(int)));
    connect(ui->timer, SIGNAL(reset(QTime)), this, SLOT(simulationReset(QTime)));

    loadSimulationData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selectSimulationFolder()
{
    auto baseDirPath = QFileDialog::getExistingDirectory(this, "Vybrat složku simulace...");
    if(baseDirPath.isEmpty()) return;
    lastOpenedPath = baseDirPath;

    loadSimulationData();
}

void MainWindow::loadSimulationData()
{
    ui->actionReload->setEnabled(true);
    QDir baseDir(lastOpenedPath);

    // Zíksat cesty
    ParseStreets parseStreets;
    if(!parseStreets.openFile(baseDir.filePath("streets.csv"))){
        qDebug() << "Cannot open file streets.csv";
        return;
    }
    auto streets = parseStreets.getStreets();

    // Získat cesty linek
    ParseRoutes parseRoutes;
    if(!parseRoutes.openFile(baseDir.filePath("routes.csv"))){
        qDebug() << "Cannot open file routes.csv";
        return;
    }
    auto routes = parseRoutes.getRoutes(streets);

    // Získat jizdní řády
    ParseTimetables parseTimetables;
    if(!parseTimetables.openFile(baseDir.filePath("timetable.csv"))){
        qDebug() << "Cannot open file timetable.csv";
        return;
    }
    auto timetables = parseTimetables.getTimetables(routes);

    // Získat linky
    ParseLines parseLines;
    if(!parseLines.openFile(baseDir.filePath("lines.csv"))){
        qDebug() << "Cannot open file lines.csv";
        return;
    }
    auto lines = parseLines.getLines(routes, timetables);

    for(const auto l: lines){
        qDebug() << l.getID() << " " << l.getDisplayNumber() << " " << l.getDestination() << " " << l.getRoute();
    }

    ui->timer->reset();
    selectStreet(nullptr);

    if(mapScene != nullptr){
        disconnect(mapScene, &Map::streetSelected, this, &MainWindow::selectStreet);
        disconnect(ui->streetTraffic, SIGNAL(valueChanged(int)), mapScene, SLOT(changeStreetTraffic(int)));
        delete mapScene;
    }

    mapScene = new Map(streets);
    ui->mapView->setScene(mapScene);

    connect(mapScene, &Map::streetSelected, this, &MainWindow::selectStreet);
    connect(ui->streetTraffic, SIGNAL(valueChanged(int)), mapScene, SLOT(changeStreetTraffic(int)));

    simulation.InitializeSimulation(streets,lines);
}

void MainWindow::selectStreet(Street *street)
{
    if(street == nullptr){
        ui->streetParams->setEnabled(false);
        ui->streetName->setText("Nastavení průjezdnosti");
        ui->streetTraffic->setValue(100);
    }else{
        ui->streetParams->setEnabled(true);
        ui->streetName->setText(street->getName());
        ui->streetTraffic->setValue(street->getTraffic());
    }
}

void MainWindow::simulationStep(int seconds)
{
    if(mapScene == nullptr) return;

    qDebug() << "Simulation fired!" << seconds;
    simulation.Simulate(seconds);
    auto currMoveLog = simulation.move_log;
    qDebug() << currMoveLog;

    for(auto busId: currMoveLog.keys()){
        if(!prevMoveLog.contains(busId)){
            auto bus = simulation.GetVehicleById(busId);
            mapScene->addBus(busId, bus.GetLine());
        }

        mapScene->updateBus(busId, currMoveLog[busId]);
    }

    for(auto prevBusId: prevMoveLog.keys()){
        if(!currMoveLog.contains(prevBusId)){
            mapScene->removeBus(prevBusId);
        }
    }

    prevMoveLog = currMoveLog;

}

void MainWindow::simulationReset(QTime time)
{
    qDebug() << "Simulation time set:" << time;
    simulation.SetTime(time.hour(), time.minute());
}

void MainWindow::closeApp()
{
    QCoreApplication::quit();
}
