// Copyright (C) 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>
#include <QThread>
#include <QRandomGenerator>
#include <QMutex>
#include <QWaitCondition>

#include <qtlogger/qtlogger.h>

Q_LOGGING_CATEGORY(lcApp,              "app")
Q_LOGGING_CATEGORY(lcConfig,           "config")
Q_LOGGING_CATEGORY(lcNetwork,          "network")
Q_LOGGING_CATEGORY(lcDatabase,         "database")
Q_LOGGING_CATEGORY(lcApiRequest,       "api.request")
Q_LOGGING_CATEGORY(lcApiValidation,    "api.validation")
Q_LOGGING_CATEGORY(lcStorageCache,     "storage.cache")
Q_LOGGING_CATEGORY(lcSecurityAuth,     "security.auth.oauth")
Q_LOGGING_CATEGORY(lcSecurityAudit,    "security.audit.access.control")
Q_LOGGING_CATEGORY(lcSecurityEncrypt,  "security.encryption.aes256gcm")

class LoggerThread : public QThread
{
public:
    LoggerThread(int id, QMutex *startMutex, QWaitCondition *startCondition)
        : m_id(id)
        , m_startMutex(startMutex)
        , m_startCondition(startCondition)
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
        qCDebug(lcApiRequest) << QString("GET /api/v2/users/%1").arg(QRandomGenerator::global()->bounded(1000, 9999));
        QThread::msleep(QRandomGenerator::global()->bounded(15, 35));
        qCWarning(lcApiValidation) << "Deprecated header detected";
        QThread::msleep(QRandomGenerator::global()->bounded(20, 40));
        qCDebug(lcApiRequest) << QString("POST /api/v2/orders/%1").arg(QRandomGenerator::global()->bounded(5000, 9999));
        QThread::msleep(QRandomGenerator::global()->bounded(15, 30));
        qCDebug(lcApiValidation) << "Rate limit: 847/1000 requests";
    }

    void runThread2()
    {
        qCDebug(lcStorageCache) << "Cache lookup: key='user:847:profile'";
        QThread::msleep(QRandomGenerator::global()->bounded(20, 40));
        qCInfo(lcStorageCache) << "Cache HIT: 'session:abc123' (TTL: 1842s)";
        QThread::msleep(QRandomGenerator::global()->bounded(15, 35));
        qCWarning(lcStorageCache) << "Cache eviction: memory threshold";
        QThread::msleep(QRandomGenerator::global()->bounded(25, 45));
        qCDebug(lcStorageCache) << "Cache SET: 'product:991:inventory'";
    }

    void runThread3()
    {
        auto userId = QString("user_%1").arg(QRandomGenerator::global()->bounded(1000, 9999));
        qCDebug(lcSecurityAuth) << QString("OAuth2 validation for '%1'").arg(userId);
        QThread::msleep(QRandomGenerator::global()->bounded(20, 40));
        qCDebug(lcSecurityEncrypt) << QString("AES-256-GCM decrypt (iv: 0x%1)")
                                          .arg(QRandomGenerator::global()->generate(), 8, 16, QChar('0'));
        QThread::msleep(QRandomGenerator::global()->bounded(15, 35));
        qCWarning(lcSecurityAudit) << QString("Suspicious activity: '%1' multiple IPs").arg(userId);
        QThread::msleep(QRandomGenerator::global()->bounded(15, 30));
        qCCritical(lcSecurityAudit) << QString("Auth failure: token expired for '%1'").arg(userId);
    }

private:
    int m_id;
    QMutex *m_startMutex;
    QWaitCondition *m_startCondition;
};

void runFirstPart()
{
    qCInfo(lcApp) << "Application started v2.1.0";
    QThread::msleep(20);
    qCDebug(lcConfig) << "Loading config from /etc/myapp/config.yaml";
    QThread::msleep(15);
    qCDebug(lcConfig) << "Environment: production, workers: 8";
    QThread::msleep(20);
    qCDebug(lcApp) << "Initializing plugin subsystem...";
    QThread::msleep(15);
    qCInfo(lcNetwork) << "HTTP server listening on port 8080";
    QThread::msleep(20);
    qCDebug(lcNetwork) << "Binding HTTPS to 0.0.0.0:8443";
    QThread::msleep(15);
    qCWarning(lcNetwork) << "Connection timeout from 203.0.113.42";
    QThread::msleep(20);
    qCInfo(lcDatabase) << "Database pool ready: 5 connections";
    QThread::msleep(15);
    qCDebug(lcDatabase) << "Query executed in 3.2ms: SELECT * FROM users";
    QThread::msleep(20);
    qCWarning(lcDatabase) << "Slow query: 156ms for complex join";
    QThread::msleep(15);
    qCCritical(lcNetwork) << "Failed to bind port 9090: address in use";
    QThread::msleep(20);
    qCDebug(lcConfig) << "Feature flags loaded: 12 active";
    QThread::msleep(20);
    qCDebug(lcApp) << "Memory usage: 256 MB";
}

void runMainThreadLogging(QWaitCondition *startCondition)
{
    startCondition->wakeAll();

    qCDebug(lcApp) << "Starting parallel workers...";
    QThread::msleep(QRandomGenerator::global()->bounded(30, 50));
    qCDebug(lcNetwork) << "WebSocket connection established";
    QThread::msleep(QRandomGenerator::global()->bounded(40, 60));
    qCDebug(lcDatabase) << "Transaction committed: tx_8a4f2c";
    QThread::msleep(QRandomGenerator::global()->bounded(30, 50));
    qCCritical(lcDatabase) << "Connection lost to replica db-2.local";
    QThread::msleep(QRandomGenerator::global()->bounded(20, 40));
    qCInfo(lcApp) << "Shutdown complete";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    gQtLogger
        .moveToOwnThread()
        .formatPretty(true)
        .sendToStdOut();

    gQtLogger.installMessageHandler();

    QMutex startMutex;
    QWaitCondition startCondition;

    LoggerThread thread1(1, &startMutex, &startCondition);
    LoggerThread thread2(2, &startMutex, &startCondition);
    LoggerThread thread3(3, &startMutex, &startCondition);

    thread1.start();
    thread2.start();
    thread3.start();

    runFirstPart();

    QThread::msleep(10);

    runMainThreadLogging(&startCondition);

    thread1.wait();
    thread2.wait();
    thread3.wait();

    QTimer::singleShot(0, &app, [&app]() { app.quit(); });

    return app.exec();
}
