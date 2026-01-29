// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>
#include <QThread>
#include <QRandomGenerator>

#include <qtlogger/qtlogger.h>

Q_LOGGING_CATEGORY(lcApp,      "app")
Q_LOGGING_CATEGORY(lcConfig,   "config")
Q_LOGGING_CATEGORY(lcNetwork,  "network")
Q_LOGGING_CATEGORY(lcDatabase, "database")
Q_LOGGING_CATEGORY(lcAuth,     "auth")
Q_LOGGING_CATEGORY(lcCache,    "cache")
Q_LOGGING_CATEGORY(lcWorker,   "worker")
Q_LOGGING_CATEGORY(lcPlugin,   "plugin")

void simulateStartup()
{
    qCInfo(lcApp) << "═══════════════════════════════════════════════════════════════";
    qCInfo(lcApp) << "     QtLogger Demo Application v2.1.0";
    qCInfo(lcApp) << "     Demonstrating beautiful colored log output";
    qCInfo(lcApp) << "═══════════════════════════════════════════════════════════════";
    qCInfo(lcApp);

    qCInfo(lcApp) << "Starting application initialization...";
    QThread::msleep(50);

    qCDebug(lcConfig) << "Loading configuration from /etc/myapp/config.yaml";
    QThread::msleep(30);
    qCDebug(lcConfig) << "Parsing environment variables...";
    qCDebug(lcConfig) << "  → APP_ENV=production";
    qCDebug(lcConfig) << "  → LOG_LEVEL=debug";
    qCDebug(lcConfig) << "  → MAX_WORKERS=8";
    QThread::msleep(20);
    qCInfo(lcConfig) << "Configuration loaded successfully";
}

void simulatePluginLoading()
{
    qCInfo(lcPlugin) << "Discovering plugins in /usr/lib/myapp/plugins/";
    QThread::msleep(40);

    qCDebug(lcPlugin) << "Loading plugin: analytics-collector v1.2.3";
    QThread::msleep(25);
    qCInfo(lcPlugin) << "  ✓ Plugin 'analytics-collector' initialized";

    qCDebug(lcPlugin) << "Loading plugin: metrics-exporter v2.0.1";
    QThread::msleep(25);
    qCInfo(lcPlugin) << "  ✓ Plugin 'metrics-exporter' initialized";

    qCDebug(lcPlugin) << "Loading plugin: legacy-adapter v0.9.8";
    QThread::msleep(25);
    qCWarning(lcPlugin) << "  ⚠ Plugin 'legacy-adapter' is deprecated, consider upgrading";

    qCInfo(lcPlugin) << "3 plugins loaded successfully";
}

void simulateDatabaseConnection()
{
    qCInfo(lcDatabase) << "Initializing database connection pool...";
    QThread::msleep(60);

    qCDebug(lcDatabase) << "Connecting to PostgreSQL at db.example.com:5432";
    qCDebug(lcDatabase) << "  → Database: myapp_production";
    qCDebug(lcDatabase) << "  → User: app_service";
    qCDebug(lcDatabase) << "  → SSL: enabled (TLSv1.3)";
    QThread::msleep(100);

    qCInfo(lcDatabase) << "Primary connection established (latency: 12ms)";
    QThread::msleep(50);

    qCDebug(lcDatabase) << "Setting up connection pool: min=5, max=20";
    qCDebug(lcDatabase) << "Running health check query...";
    QThread::msleep(30);
    qCInfo(lcDatabase) << "Database pool ready: 5 connections active";
}

void simulateCacheInit()
{
    qCInfo(lcCache) << "Connecting to Redis cluster...";
    QThread::msleep(40);

    qCDebug(lcCache) << "Node discovery: redis-1.local:6379, redis-2.local:6379, redis-3.local:6379";
    QThread::msleep(30);
    qCInfo(lcCache) << "Redis cluster connected (3 nodes, 16384 slots)";

    qCDebug(lcCache) << "Warming up cache from persistent storage...";
    QThread::msleep(80);
    qCInfo(lcCache) << "Cache warmed: 15,432 keys loaded (memory: 128 MB)";
}

void simulateNetworkServices()
{
    qCInfo(lcNetwork) << "Starting network services...";
    QThread::msleep(30);

    qCDebug(lcNetwork) << "Binding HTTP server to 0.0.0.0:8080";
    QThread::msleep(20);
    qCInfo(lcNetwork) << "HTTP server listening on port 8080";

    qCDebug(lcNetwork) << "Binding HTTPS server to 0.0.0.0:8443";
    qCDebug(lcNetwork) << "  → Certificate: /etc/ssl/certs/server.crt";
    qCDebug(lcNetwork) << "  → Key: /etc/ssl/private/server.key";
    QThread::msleep(30);
    qCInfo(lcNetwork) << "HTTPS server listening on port 8443";

    qCDebug(lcNetwork) << "Starting WebSocket endpoint on /ws";
    QThread::msleep(20);
    qCInfo(lcNetwork) << "WebSocket server ready";
}

void simulateWorkers()
{
    qCInfo(lcWorker) << "Spawning worker threads...";
    QThread::msleep(20);

    for (int i = 1; i <= 4; ++i) {
        qCDebug(lcWorker) << QString("Worker-%1 started (thread: 0x%2)")
                                .arg(i)
                                .arg(QRandomGenerator::global()->generate(), 8, 16, QChar('0'));
        QThread::msleep(15);
    }

    qCInfo(lcWorker) << "4 worker threads active and ready";
}

void simulateRuntime()
{
    qCInfo(lcApp);
    qCInfo(lcApp) << "═══════════════════════════════════════════════════════════════";
    qCInfo(lcApp) << "     Application started successfully!";
    qCInfo(lcApp) << "═══════════════════════════════════════════════════════════════";
    qCInfo(lcApp);

    QThread::msleep(100);

    // Simulate some runtime activity
    qCDebug(lcNetwork) << "Incoming connection from 192.168.1.100:52431";
    qCInfo(lcAuth) << "User 'john.doe@example.com' authenticated via OAuth2";
    qCDebug(lcDatabase) << "Query executed in 3.2ms: SELECT * FROM users WHERE id = 42";
    qCDebug(lcCache) << "Cache HIT for key 'user:42:profile' (TTL: 3542s)";

    QThread::msleep(50);

    qCDebug(lcNetwork) << "Incoming connection from 10.0.0.55:48921";
    qCInfo(lcAuth) << "API key validated for service 'mobile-app'";
    qCDebug(lcWorker) << "Task #1847 queued: process_order (priority: high)";
    qCDebug(lcWorker) << "Task #1847 completed in 45ms";

    QThread::msleep(50);

    qCWarning(lcNetwork) << "Connection timeout from 203.0.113.42:33012 after 30s";
    qCDebug(lcCache) << "Cache MISS for key 'product:9981:inventory'";
    qCDebug(lcDatabase) << "Query executed in 8.7ms: SELECT stock FROM inventory WHERE product_id = 9981";
    qCDebug(lcCache) << "Cache SET 'product:9981:inventory' (TTL: 300s)";

    QThread::msleep(50);

    qCWarning(lcDatabase) << "Slow query detected (156ms): SELECT * FROM orders WHERE created_at > '2024-01-01'";
    qCDebug(lcWorker) << "Task #1848 queued: generate_report (priority: low)";

    QThread::msleep(50);

    qCCritical(lcAuth) << "Authentication failure: invalid credentials for 'admin@example.com' (attempt 3/5)";
    qCWarning(lcAuth) << "IP 198.51.100.23 added to rate-limit watchlist";

    QThread::msleep(50);

    qCInfo(lcApp);
    qCInfo(lcApp) << "───────────────────────────────────────────────────────────────";
    qCInfo(lcApp) << "     Runtime Statistics";
    qCInfo(lcApp) << "───────────────────────────────────────────────────────────────";
    qCDebug(lcApp) << "  Requests processed:     1,247";
    qCDebug(lcApp) << "  Average response time:  23ms";
    qCDebug(lcApp) << "  Cache hit ratio:        94.2%";
    qCDebug(lcApp) << "  Active connections:     42";
    qCDebug(lcApp) << "  Memory usage:           512 MB";
    qCInfo(lcApp) << "───────────────────────────────────────────────────────────────";
}

void simulateShutdown()
{
    qCInfo(lcApp);
    qCInfo(lcApp) << "Received SIGTERM, initiating graceful shutdown...";
    QThread::msleep(50);

    qCInfo(lcWorker) << "Stopping worker threads...";
    qCDebug(lcWorker) << "Waiting for 2 pending tasks to complete...";
    QThread::msleep(80);
    qCInfo(lcWorker) << "All workers stopped gracefully";

    qCInfo(lcNetwork) << "Closing network connections...";
    qCDebug(lcNetwork) << "Draining 42 active connections (timeout: 5s)";
    QThread::msleep(60);
    qCInfo(lcNetwork) << "All connections closed";

    qCInfo(lcCache) << "Flushing cache to persistent storage...";
    QThread::msleep(40);
    qCInfo(lcCache) << "Cache persisted successfully";

    qCInfo(lcDatabase) << "Closing database connections...";
    qCDebug(lcDatabase) << "Releasing 5 pooled connections";
    QThread::msleep(30);
    qCInfo(lcDatabase) << "Database connections closed";

    qCInfo(lcPlugin) << "Unloading plugins...";
    QThread::msleep(40);
    qCInfo(lcPlugin) << "All plugins unloaded";

    qCInfo(lcApp);
    qCInfo(lcApp) << "═══════════════════════════════════════════════════════════════";
    qCInfo(lcApp) << "     Application shutdown complete. Goodbye!";
    qCInfo(lcApp) << "═══════════════════════════════════════════════════════════════";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Configure beautiful colored output with timestamps and categories
    gQtLogger
        .format("%{time HH:mm:ss.zzz} "
                "%{if-debug}\033[38;5;245m DBG \033[0m%{endif}"
                "%{if-info}\033[38;5;75m INF \033[0m%{endif}"
                "%{if-warning}\033[38;5;214m WRN \033[0m%{endif}"
                "%{if-critical}\033[38;5;196m CRT \033[0m%{endif}"
                "\033[38;5;243m│\033[0m "
                "\033[38;5;141m%{category:>10}\033[0m "
                "\033[38;5;243m│\033[0m "
                "%{if-debug}\033[38;5;250m%{endif}"
                "%{if-info}\033[38;5;255m%{endif}"
                "%{if-warning}\033[38;5;214m%{endif}"
                "%{if-critical}\033[38;5;196;1m%{endif}"
                "%{message}"
                "\033[0m")
        .sendToStdOut(true);

    gQtLogger.installMessageHandler();

    // Run the demo
    simulateStartup();
    simulatePluginLoading();
    simulateDatabaseConnection();
    simulateCacheInit();
    simulateNetworkServices();
    simulateWorkers();
    simulateRuntime();
    simulateShutdown();

    QTimer::singleShot(0, &app, [&app]() { app.quit(); });

    return app.exec();
}
