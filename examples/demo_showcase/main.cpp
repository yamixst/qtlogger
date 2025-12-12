// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QMutex>
#include <QRandomGenerator>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>

#include <qtlogger/qtlogger.h>

Q_LOGGING_CATEGORY(lcMain, "main")
Q_LOGGING_CATEGORY(lcConfig, "config")
Q_LOGGING_CATEGORY(lcComm, "comm")
Q_LOGGING_CATEGORY(lcComputer, "computer.core")
Q_LOGGING_CATEGORY(lcHelm, "ops.helm")
Q_LOGGING_CATEGORY(lcNavigation, "ops.navigation")
Q_LOGGING_CATEGORY(lcEngineering, "engineering.atf")
Q_LOGGING_CATEGORY(lcSecurity, "sec.access")
Q_LOGGING_CATEGORY(lcTactical, "sec.tactical.sensors")
Q_LOGGING_CATEGORY(lcShields, "sec.shields.deflector")

class LoggerThread : public QThread
{
public:
    LoggerThread(int id, QMutex *startMutex, QWaitCondition *startCondition)
        : m_id(id), m_startMutex(startMutex), m_startCondition(startCondition)
    {
    }

protected:
    void run() override
    {
        m_startMutex->lock();
        m_startCondition->wait(m_startMutex);
        m_startMutex->unlock();

        if (m_id == 1) {
            runThread1();
        } else if (m_id == 2) {
            runThread2();
        } else {
            runThread3();
        }
    }

    void runThread1()
    {
        qCDebug(lcHelm) << QString("Helm control: adjusting course to heading %1 mark %2")
                                   .arg(QRandomGenerator::global()->bounded(0, 360))
                                   .arg(QRandomGenerator::global()->bounded(0, 90));
        QThread::msleep(QRandomGenerator::global()->bounded(15, 350));
        qCWarning(lcNavigation) << "Navigation: minor stellar drift detected, recalculating";
        QThread::msleep(QRandomGenerator::global()->bounded(20, 400));
        qCDebug(lcHelm) << QString("Engaging impulse engines at %1%% power")
                                   .arg(QRandomGenerator::global()->bounded(25, 100));
        QThread::msleep(QRandomGenerator::global()->bounded(15, 300));
        qCDebug(lcNavigation) << "Warp field geometry: nominal, ready for warp";
    }

    void runThread2()
    {
        qCDebug(lcEngineering) << "Antimatter containment: monitoring magnetic bottle integrity";
        QThread::msleep(QRandomGenerator::global()->bounded(20, 400));
        qCInfo(lcEngineering) << "Dilithium crystal matrix: stable at 98.7% efficiency";
        QThread::msleep(QRandomGenerator::global()->bounded(15, 350));
        qCWarning(lcEngineering) << "Warp core temperature elevated: initiating coolant flow";
        QThread::msleep(QRandomGenerator::global()->bounded(25, 450));
        qCDebug(lcEngineering) << "Plasma injector alignment: within tolerance";
    }

    void runThread3()
    {
        auto sector = QString("sector_%1").arg(QRandomGenerator::global()->bounded(1, 999));
        qCDebug(lcSecurity) << QString("Security clearance verified: Bridge access granted");
        QThread::msleep(QRandomGenerator::global()->bounded(20, 400));
        qCDebug(lcShields) << QString("Deflector shield harmonic frequency: %1 MHz")
                                      .arg(QRandomGenerator::global()->bounded(1000, 9999));
        QThread::msleep(QRandomGenerator::global()->bounded(15, 350));
        qCWarning(lcTactical)
                << QString("Long-range sensors: unidentified vessel in %1").arg(sector);
        QThread::msleep(QRandomGenerator::global()->bounded(15, 300));
        qCCritical(lcTactical)
                << QString("RED ALERT: Klingon Bird-of-Prey decloaking in %1!").arg(sector);
    }

private:
    int m_id;
    QMutex *m_startMutex;
    QWaitCondition *m_startCondition;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    gQtLogger.moveToOwnThread()
            .formatPretty(true, 12, true)
            .format([](const QtLogger::LogMessage &lmsg) {
                auto fmsg = lmsg.formattedMessage();
                static QRegularExpression dateRegex(QStringLiteral("\\d{2}\\.\\d{2}\\.\\d{4}"));
                fmsg.replace(dateRegex, QStringLiteral("13.09.2245"));
                return fmsg;
            })
            .sendToStdOut();

    gQtLogger.installMessageHandler();

    QTimer::singleShot(0, &app, [&app]() {
        qCInfo(lcMain) << "USS Enterprise NCC-1701 LCARS System v47.3.1 initializing...";
        QThread::msleep(200);
        qCDebug(lcConfig) << "Loading Starfleet Command protocols from /starfleet/config/ncc1701.cfg";
        QThread::msleep(150);
        qCDebug(lcConfig) << "Operating mode: Deep Space Exploration, crew complement: 430";
        QThread::msleep(200);
        qCDebug(lcMain) << "Initializing ship-wide LCARS interface...";
        QThread::msleep(150);
        qCInfo(lcComm) << "Subspace communication array online, frequency 1701.47 MHz";
        QThread::msleep(200);
        qCDebug(lcComm) << "Establishing link to Starfleet Command, Earth Station McKinley";
        QThread::msleep(150);
        qCWarning(lcComm) << "Subspace interference detected from Neutral Zone coordinates";
        QThread::msleep(200);
        qCInfo(lcComputer) << "Main computer core online: 47 isolinear chip clusters active";
        QThread::msleep(150);
        qCDebug(lcComputer) << "Library computer access: 2.3 petaquads of Federation database loaded";
        QThread::msleep(200);
        qCWarning(lcComputer) << "Memory bank 7-Alpha showing degradation, scheduling maintenance";
        QThread::msleep(150);
        qCCritical(lcComm) << "Unable to reach Deep Space Station K-7: comm relay offline";
        QThread::msleep(200);
        qCDebug(lcConfig) << "Ship systems configuration loaded: 24 decks operational";
        QThread::msleep(200);
        qCDebug(lcMain) << "Duotronic system resources: 94% available";

        QMutex startMutex;
        QWaitCondition startCondition;

        LoggerThread thread1(1, &startMutex, &startCondition);
        LoggerThread thread2(2, &startMutex, &startCondition);
        LoggerThread thread3(3, &startMutex, &startCondition);

        thread1.start();
        thread2.start();
        thread3.start();

        QThread::msleep(100);

        startCondition.wakeAll();

        qCDebug(lcMain) << "Initiating multi-system diagnostic sequence...";
        QThread::msleep(QRandomGenerator::global()->bounded(30, 500));
        qCDebug(lcComm) << "Hailing frequencies open, Captain";
        QThread::msleep(QRandomGenerator::global()->bounded(40, 600));
        qCDebug(lcComputer) << "Ship's log entry recorded: Stardate 47634.44";
        QThread::msleep(QRandomGenerator::global()->bounded(30, 500));
        qCCritical(lcComputer) << "Warning: Auxiliary computer core offline in Engineering Section 31";

        thread1.wait();
        thread2.wait();
        thread3.wait();

        QThread::msleep(QRandomGenerator::global()->bounded(20, 400));
        qCInfo(lcMain) << "All stations report ready. Enterprise standing by, Captain on the bridge.";

        app.quit();
    });

    return app.exec();
}
