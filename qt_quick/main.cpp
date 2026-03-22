#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QSettings>
#include <QStandardPaths>

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

    //Set the application icon
    app.setWindowIcon(QIcon(":/images/Katalog_logo_64.ico"));

    // Load translation — same mechanism as K2
    QTranslator *translator = new QTranslator(&app);
    {
        QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
        QString settingsFilePath = homePath + "/.config/katalog_settings.ini";
        QSettings settings(settingsFilePath, QSettings::IniFormat);
        QString userLanguage = settings.value("Settings/Language").toString();

        if (userLanguage.isEmpty()) {
            userLanguage = Language::getSystemLanguage();
            if (!Language::isLanguageSupported(userLanguage))
                userLanguage = "en_US";
            settings.setValue("Settings/Language", userLanguage);
        }

        if (translator->load("Katalog_" + userLanguage, ":translations"))
            app.installTranslator(translator);
    }

    KAboutData aboutData(
        QStringLiteral("Katalog"),
        "Katalog",
        QStringLiteral(KATALOG_VERSION_STRING),
        "Katalog is an application to catalog, search, and manage files from any drive, permanent or removable.",
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
    appManager->selectedDevice->loadDevice(QSqlDatabase::defaultConnection);

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

    appManager->testQuery();

    return app.exec();
}
