#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "appmanager.h"
#include "core/collection.h"
#include "adapters/search.h"
#include "PageSearch.h"
#include "adapters/devicelistmodel.h"

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    //Set the application icon
    app.setWindowIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoHome));
    //app.setWindowIcon(QIcon::fromTheme("drive-multipartition"));
    //app.setWindowIcon(QIcon(":images/Katalog_logo_64.ico"));
    //app.setWindowIcon(QIcon(":images/logo-64.ico"));

    KLocalizedString::setApplicationDomain("KatalogPlus");
    /*
    KAboutData aboutData(
        QStringLiteral("KatalogPlus"),
        i18nc("@title", "Katalog Plus"),
        QStringLiteral("1.0"),
        i18n("KatalogPlus application"),
        KAboutLicense::GPL,
        i18n("(c) 2021"));
    */
    KAboutData aboutData(
        QStringLiteral("KatalogPlus"),
        "Katalog Plus",
        QStringLiteral("1.0"),
        "KatalogPlus application",
        KAboutLicense::GPL,
        "(c) 2020-2024");

    // aboutData.addAuthor(
    //     i18nc("@info:credit", "Stephane Couturier"),
    //     i18nc("@info:credit", "Founder & Main developper"),
    //     QStringLiteral("your@email.com"),
    //     QStringLiteral("https://yourwebsite.com"));

    aboutData.addAuthor(
        "Stéphane Couturier",
        "Founder & Main developper",
        QStringLiteral("your@email.com"),
        QStringLiteral("https://yourwebsite.com"));

    // Set aboutData as information about the app
    KAboutData::setApplicationData(aboutData);

    //Katalog objects
    AppManager *appManager = new AppManager;
    appManager->initiateApp();
    appManager->collection->appVersion = appManager->currentVersion;
    appManager->startDatabase();
    appManager->selectedDevice->ID=1;
    appManager->selectedDevice->loadDevice(QSqlDatabase::defaultConnection);
    appManager->selectedDevice->type = "All";

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
    engine.loadFromModule("Katalog3", "Main");

    QQmlContext *rootContext = engine.rootContext();
    rootContext->setContextProperty("appManager1", appManager);
    rootContext->setContextProperty("collection1", appManager->collection);
    rootContext->setContextProperty("newSearch1", newSearch);
    rootContext->setContextProperty("pageSearch1", &pageSearch);
    rootContext->setContextProperty("deviceListModel1", appManager->deviceListModel);

    appManager->testQuery();

    return app.exec();
}
