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

#include "version.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    //Set the application icon
    app.setWindowIcon(QIcon(":/images/Katalog_logo_64.ico"));

    KLocalizedString::setApplicationDomain("Katalog");

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

    engine.loadFromModule("Katalog3", "Main");

    appManager->testQuery();

    return app.exec();
}
