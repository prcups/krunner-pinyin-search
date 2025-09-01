/*
    SPDX-FileCopyrightText: 2025 SignKirigami <prcups@krgm.moe>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "pinyinsearch.h"

PinyinSearch::PinyinSearch(QObject *parent, const KPluginMetaData &data)
    : KRunner::AbstractRunner(parent, data)
{
    addSyntax(QStringLiteral(":q:"), i18n("Finds applications whose name is :q: in pinyin"));
    // Disallow short queries
    setMinLetterCount(2);

    connect(this, &KRunner::AbstractRunner::prepare, this, [this]() {
        m_matching = true;
        if (m_services.isEmpty()) {
            m_services = KApplicationTrader::query([](const KService::Ptr &service) {
                return !service->noDisplay();
            });
        } else {
            KSycoca::self()->ensureCacheValid();
        }
    });
    connect(this, &KRunner::AbstractRunner::teardown, this, [this]() {
        m_matching = false;
    });
}

void PinyinSearch::init()
{
    //  connect to the thread-local singleton here
    connect(KSycoca::self(), &KSycoca::databaseChanged, this, [this]() {
        if (m_matching) {
            m_services = KApplicationTrader::query([](const KService::Ptr &service) {
                return !service->noDisplay();
            });
        } else {
            // Invalidate for the next match session
            m_services.clear();
        }
    });
}

void PinyinSearch::match(KRunner::RunnerContext &context)
{
    QString term = context.query();

    for (const auto &service : m_services) {
        auto name = service->name();
        for (unsigned int i = 0; i < name.size(); ++i)
            if (matcher.MatchStr(name.sliced(i), term)) {
                KRunner::QueryMatch match(this);
                match.setRelevance(0.8);
                setupMatch(service, match);
                context.addMatch(match);
                break;
            }
    }
}

void PinyinSearch::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
{
    Q_UNUSED(context)
    Q_UNUSED(match)

    const QUrl dataUrl = match.data().toUrl();

    KService::Ptr service = KService::serviceByStorageId(dataUrl.path());
    if (!service) {
        return;
    }

    KIO::ApplicationLauncherJob *job = nullptr;

    const QString actionName = QUrlQuery(dataUrl).queryItemValue(QStringLiteral("action"));
    if (actionName.isEmpty()) {
        job = new KIO::ApplicationLauncherJob(service);
    } else {
        const auto actions = service->actions();
        auto it = std::find_if(actions.begin(), actions.end(), [&actionName](const KServiceAction &action) {
            return action.name() == actionName;
        });
        Q_ASSERT(it != actions.end());

        job = new KIO::ApplicationLauncherJob(*it);
    }

    auto *delegate = new KNotificationJobUiDelegate;
    delegate->setAutoErrorHandlingEnabled(true);
    job->setUiDelegate(delegate);
    job->start();
}

void PinyinSearch::setupMatch(const KService::Ptr &service, KRunner::QueryMatch &match)
{
    const QString name = service->name();
    const QString exec = service->exec();

    match.setText(name);

    QUrl url(service->storageId());
    url.setScheme(QStringLiteral("applications"));
    match.setData(url);
    match.setUrls({QUrl::fromLocalFile(service->entryPath())});

    QString urlPath = resolvedArgs(exec);
    if (urlPath.isEmpty()) {
        // Otherwise we might filter out broken services. Rather than hiding them, it is better to show an error message on launch (as done by KIO's jobs)
        urlPath = exec;
    }
    match.setId(QString(u"exec://" + urlPath));
    if (!service->genericName().isEmpty() && service->genericName() != name) {
        match.setSubtext(service->genericName());
    } else if (!service->comment().isEmpty()) {
        match.setSubtext(service->comment());
    }

    if (!service->icon().isEmpty()) {
        match.setIconName(service->icon());
    }
}

QString PinyinSearch::resolvedArgs(const QString &exec)
{
    const KService syntheticService(QString(), exec, QString());
    KIO::DesktopExecParser parser(syntheticService, {});
    QStringList resultingArgs = parser.resultingArguments();
    if (const auto error = parser.errorMessage(); resultingArgs.isEmpty() && !error.isEmpty()) {
        return QString();
    }

    // Remove any environment variables.
    if (KIO::DesktopExecParser::executableName(exec) == QLatin1String("env")) {
        resultingArgs.removeFirst(); // remove "env".

        while (!resultingArgs.isEmpty() && resultingArgs.first().contains(QLatin1Char('='))) {
            resultingArgs.removeFirst();
        }

        // Now parse it again to resolve the path.
        resultingArgs = KIO::DesktopExecParser(KService(QString(), KShell::joinArgs(resultingArgs), QString()), {}).resultingArguments();
        return resultingArgs.join(QLatin1Char(' '));
    }

    // Remove special arguments that have no real impact on the application.
    static const auto specialArgs = {QStringLiteral("-qwindowtitle"), QStringLiteral("-qwindowicon"), QStringLiteral("--started-from-file")};

    for (const auto &specialArg : specialArgs) {
        int index = resultingArgs.indexOf(specialArg);
        if (index > -1) {
            if (resultingArgs.count() > index) {
                resultingArgs.removeAt(index);
            }
            if (resultingArgs.count() > index) {
                resultingArgs.removeAt(index); // remove value, too, if any.
            }
        }
    }
    return resultingArgs.join(QLatin1Char(' '));
}

K_PLUGIN_CLASS_WITH_JSON(PinyinSearch, "pinyinsearch.json")

// needed for the QObject subclass declared as part of K_PLUGIN_CLASS_WITH_JSON
#include "pinyinsearch.moc"

#include "moc_pinyinsearch.cpp"
