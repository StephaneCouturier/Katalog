#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QSettings>
#include <QStandardPaths>
#include <QIcon>
#include <QQuickStyle>

#include "appmanager.h"
#include "core/collection.h"
#include "core/language.h"
#include "adapters/search.h"
#include "PageSearch.h"
#include "adapters/devicelistmodel.h"

#include <KAboutData>
#include <KLocalizedString>

#include "version.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Qt Quick Controls style. On Linux (KDE) the system org.kde.desktop/Breeze
    // style is used, which gives dialog buttons a uniform minimum width. On other
    // platforms that style is unavailable and Qt falls back to Basic, whose
    // buttons size to their text (uneven OK/Yes/No/Cancel widths). Force Fusion
    // there for a consistent look across all dialogs — set once, before the QML
    // engine loads, so it covers every current and future button.
#if !defined(Q_OS_LINUX)
    QQuickStyle::setStyle(QStringLiteral("Fusion"));
#endif

    //Set the application icon
    app.setWindowIcon(QIcon(":/images/Katalog_logo_64.ico"));

    // Icon theme setup — mirrors K2 platform-specific behaviour.
    // On Linux, the system KDE theme provides icons for QML icon.name;
    // fallback paths cover any icon the theme is missing.
    // On Windows, there is no system theme so we set "breeze" explicitly.
    {
        bool darkTheme = (app.palette().window().color().lightness() < 128);
#ifdef Q_OS_WINDOWS
        QIcon::setThemeName("breeze");
#endif
        QStringList fallbackPaths = QIcon::fallbackSearchPaths();
        fallbackPaths << (darkTheme ? QStringLiteral(":/fallback-icons-dark")
                                    : QStringLiteral(":/fallback-icons"));
        QIcon::setFallbackSearchPaths(fallbackPaths);
    }

    // Application translator. It is installed further down, once AppManager has
    // resolved the settings file path (the same portable-aware file K2 and the
    // K3 Settings page use), and re-loaded at runtime on language change.
    QTranslator *translator = new QTranslator(&app);

    KAboutData aboutData(
        QStringLiteral("Katalog"),
        "Katalog",
        QStringLiteral(KATALOG_VERSION_STRING),
        "Catalog your devices to search, analyze, and backup your files.",
        KAboutLicense::GPL_V3,
        "(c) 2020-2026");

    aboutData.setDesktopFileName(QStringLiteral("io.github.stephanecouturier.Katalog")); //Temporary, to hide the KDE GetInvolved Donation links
    aboutData.setBugAddress(""); //Temporary, to hide the KDE Bug link
    aboutData.addAuthor(
        "Stéphane Couturier",
        "Founder & Main Developer",
        QStringLiteral("katalog@stephanecouturier.com"),
        QStringLiteral("https://stephanecouturier.github.io/Katalog/"));

    KAboutData::setApplicationData(aboutData);

    //Katalog objects
    AppManager *appManager = new AppManager;
    appManager->initiateApp();
    appManager->collection->appVersion = appManager->currentVersion;
    appManager->startDatabase();

    // Load the user's language and install the translator before the QML engine
    // evaluates any qsTr string. Mirrors K2 (qt_widgets/main.cpp): migrate the
    // legacy Czech code, detect the (validated) system language on first run,
    // and fall back to English US when the stored value is unsupported. Uses the
    // settings file AppManager resolved so the choice persists and stays in sync
    // with K2 and AppManager::getCurrentLanguage().
    {
        QSettings settings(appManager->collection->settingsFilePath, QSettings::IniFormat);
        QString userLanguage = settings.value("Settings/Language").toString();

        //Migrate the legacy Czech code "cz_CZ" to the standard locale "cs_CZ".
        if (userLanguage == "cz_CZ") {
            userLanguage = "cs_CZ";
            settings.setValue("Settings/Language", userLanguage);
        }

        if (userLanguage.isEmpty()) {
            //First run: detect the system language, default to English US.
            userLanguage = Language::getSystemLanguage();
            if (!Language::isLanguageSupported(userLanguage))
                userLanguage = "en_US";
            settings.setValue("Settings/Language", userLanguage);
        }
        else if (!Language::isLanguageSupported(userLanguage)) {
            //Sanitize an already-stored but unsupported value.
            userLanguage = "en_US";
            settings.setValue("Settings/Language", userLanguage);
        }

        if (translator->load("Katalog_" + userLanguage, ":translations"))
            app.installTranslator(translator);
    }

    SearchSync *newSearch = new SearchSync;
    PageSearch pageSearch;

    // Connect the search object to AppManager
    appManager->setSearchObject(newSearch);
    qmlRegisterType<SearchSync>("Katalog", 3, 0, "Search");
    qmlRegisterType<DeviceListModel>("Katalog", 3, 0, "DeviceListModel");

    //App loading
    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Set context properties before loading QML so they are available on first evaluation
    QQmlContext *rootContext = engine.rootContext();
    rootContext->setContextProperty("appManager1", appManager);
    rootContext->setContextProperty("collection1", appManager->collection);
    rootContext->setContextProperty("newSearch1", newSearch);
    rootContext->setContextProperty("pageSearch1", &pageSearch);
    rootContext->setContextProperty("deviceListModel1", appManager->deviceListModel);
    rootContext->setContextProperty("About", QVariant::fromValue(KAboutData::applicationData()));

    engine.loadFromModule("io.github.stephanecouturier.Katalog", "Main");

    // Apply language change at runtime without restart
    QObject::connect(appManager, &AppManager::languageChanged, &engine,
        [&app, &engine, translator](const QString &code) {
            app.removeTranslator(translator);
            if (translator->load("Katalog_" + code, ":translations"))
                app.installTranslator(translator);
            engine.retranslate();
        });

    //appManager->testQuery();

    return app.exec();
}
