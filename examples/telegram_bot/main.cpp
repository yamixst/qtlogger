// Copyright (C) 2026 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
// SPDX-License-Identifier: MIT

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>

#include <qtlogger/qtlogger.h>

// Telegram Bot API:
//   POST https://api.telegram.org/bot<TOKEN>/sendMessage
//   Body: {"chat_id": "<CHAT_ID>", "text": "<message>", "parse_mode": "HTML"}
//
// Environment variables:
//   TELEGRAM_BOT_TOKEN - Telegram bot token (from @BotFather)
//   TELEGRAM_CHAT_ID   - Target chat/channel/group ID

static QString telegramUrl()
{
    auto token = QString::fromLocal8Bit(qgetenv("TELEGRAM_BOT_TOKEN"));
    return QStringLiteral("https://api.telegram.org/bot%1/sendMessage").arg(token);
}

static QList<QPair<QByteArray, QByteArray>> telegramHeaders()
{
    return {
        { "Content-Type", "application/json; charset=utf-8" }
    };
}

static bool checkTelegramEnv()
{
    return !qgetenv("TELEGRAM_BOT_TOKEN").isEmpty()
           && !qgetenv("TELEGRAM_CHAT_ID").isEmpty();
}

Q_LOGGING_CATEGORY(lcNetwork, "network")
Q_LOGGING_CATEGORY(lcDatabase, "database")

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("TelegramBotExample");
    app.setApplicationVersion("1.0.0");

    if (!checkTelegramEnv()) {
        qWarning() << "Missing required environment variables.";
        qWarning() << "Set TELEGRAM_BOT_TOKEN and TELEGRAM_CHAT_ID";
        qWarning() << "Example:";
        qWarning() << "  export TELEGRAM_BOT_TOKEN=123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11";
        qWarning() << "  export TELEGRAM_CHAT_ID=-1001234567890";
        return 1;
    }

    auto chatId = QString::fromLocal8Bit(qgetenv("TELEGRAM_CHAT_ID"));

    // Configure QtLogger with Telegram integration
    gQtLogger
        .moveToOwnThread()

        // Pipeline 1: Console output for local debugging
        .pipeline()
            .formatPretty(true)
            .sendToStdErr()
        .end()

        // Pipeline 2: Send warnings and above to Telegram
        .pipeline()
            .addAppInfo()
            .addSysInfo()
            .filterLevel(QtWarningMsg)
            .filterDuplicate()
            .format([chatId](const QtLogger::LogMessage &lmsg) {
                auto level = QtLogger::qtMsgTypeToString(lmsg.type()).toUpper();
                auto category = QLatin1String(lmsg.category());
                auto categoryStr = (category == QLatin1String("default"))
                                       ? QString()
                                       : QStringLiteral(" [%1]").arg(category);

                auto text = QStringLiteral("<b>%1</b>%2\n%3\n\n<i>%4</i>")
                                .arg(level, categoryStr,
                                     lmsg.message().toHtmlEscaped(),
                                     lmsg.time().toString(Qt::ISODate));

                return QStringLiteral(R"({"chat_id":"%1","text":"%2","parse_mode":"HTML"})")
                    .arg(chatId,
                         QString(text)
                             .replace(QLatin1Char('\\'), QLatin1String("\\\\"))
                             .replace(QLatin1Char('"'), QLatin1String("\\\""))
                             .replace(QLatin1Char('\n'), QLatin1String("\\n")));
            })
            .sendToHttp(telegramUrl(), telegramHeaders())
        .end();

    gQtLogger.installMessageHandler();

    // Test logging
    qDebug() << "Debug message (console only, not sent to Telegram)";
    qInfo() << "Info message (console only, not sent to Telegram)";
    qWarning() << "Warning: disk usage above 90%";
    qCritical() << "Critical: application state is inconsistent";

    qCWarning(lcNetwork) << "Connection timeout after 30 seconds";
    qCCritical(lcDatabase) << "Failed to execute query: table not found";

    // Give time for async HTTP requests to complete
    QTimer::singleShot(3000, &app, [&app]() {
        qInfo() << "Shutting down...";
        app.quit();
    });

    return app.exec();
}