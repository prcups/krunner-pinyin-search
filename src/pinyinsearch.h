/*
    SPDX-FileCopyrightText: 2025 SignKirigami <prcups@krgm.moe>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <KRunner/AbstractRunner>
#include <QList>
#include <QProcess>
#include <KService>
#include <KApplicationTrader>
#include <KSycoca>
#include <KLocalization>
#include <KIO/ApplicationLauncherJob>
#include <KIO/DesktopExecParser>
#include <KShell>
#include <QUrlQuery>
#include <KNotificationJobUiDelegate>
#include "pinyinmatch.h"

class PinyinSearch : public KRunner::AbstractRunner
{
    Q_OBJECT
    QList<KService::Ptr> m_services;
    bool m_matching = false;

    void setupMatch(const KService::Ptr &service, KRunner::QueryMatch &match);
    QString resolvedArgs(const QString &exec);
    PinyinMatch matcher;
public:
    PinyinSearch(QObject *parent, const KPluginMetaData &data);

    void init() override;

    // KRunner::AbstractRunner API
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;
};
