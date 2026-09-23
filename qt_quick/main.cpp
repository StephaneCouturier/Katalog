#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QSettings>
#include <QStandardPaths>
#include <QIcon>
#include <QQuickStyle>
#include <QQuickItem>
#include <QStyleHints>
#include <QWindow>

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
    // On Linux the window icon is handed to the Wayland compositor as an
    // xdg-toplevel-icon buffer. The .ico offers a 16x16 entry, which the
    // compositor upscales into the titlebar slot and so renders with a black
    // rim: qtwayland fills those buffers with straight alpha while declaring
    // them premultiplied. The single 256px master is downscaled instead, which
    // averages the defect away. See SpecApplicationIcon.md (ICO-C1).
#ifdef Q_OS_LINUX
    app.setWindowIcon(QIcon(":/images/Katalog_logo_256.png"));
#else
    app.setWindowIcon(QIcon(":/images/Katalog_logo_64.ico"));
#endif

    // Icon theme setup — mirrors K2 platform-specific behaviour.
    // On Linux, the system KDE theme provides icons for QML icon.name;
    // fallback paths cover any icon the theme is missing.
    // On Windows, there is no system theme so we set "breeze" explicitly.
#ifdef Q_OS_WINDOWS
    QIcon::setThemeName("breeze");
#endif
    // The fallback set (light or dark) follows the host light/dark preference
    // Qt reports, the same one the packaged Kirigami Theme.qml follows. The
    // palette decides only when no preference is reported: packaged builds keep
    // a light platform palette even on a dark desktop. See SpecTheme.md (THM-F11).
    const QStringList baseFallbackPaths = QIcon::fallbackSearchPaths();
    auto applyFallbackIcons = [&app, baseFallbackPaths]() {
        const Qt::ColorScheme scheme = app.styleHints()->colorScheme();
        const bool darkTheme = (scheme == Qt::ColorScheme::Dark)
            || (scheme == Qt::ColorScheme::Unknown
                && app.palette().window().color().lightness() < 128);
        QStringList fallbackPaths = baseFallbackPaths;
        fallbackPaths << (darkTheme ? QStringLiteral(":/fallback-icons-dark")
                                    : QStringLiteral(":/fallback-icons"));
        if (fallbackPaths != QIcon::fallbackSearchPaths())
            QIcon::setFallbackSearchPaths(fallbackPaths);
    };
    applyFallbackIcons();

    // Application translator. It is installed further down, once AppManager has
    // resolved the settings file path (the same portable-aware file K2 and the
    // K3 Settings page use), and re-loaded at runtime on language change.
    QTranslator *translator = new QTranslator(&app);

    //Katalog objects
    AppManager *appManager = new AppManager;
    appManager->initiateApp();
    appManager->collection->appVersion = appManager->currentVersion;

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

    // After the translator: startDatabase() builds the device list, and the
    // Selection card's second line is composed in C++ and stored in the model.
    // Built before the translator was installed, that line kept its English
    // words however complete the translation was.
    appManager->startDatabase();

    // Build KAboutData AFTER the translator is installed so its translatable
    // strings (short description, author role) resolve in the user's language.
    // The strings use QCoreApplication::translate("Main", …) — Qt's tr()
    // mechanism (QTranslator/.qm), the same one the whole K3 UI uses. They are
    // K3-only (K2 has no KAboutData), so they live in the "Main" context.
    KAboutData aboutData(
        QStringLiteral("Katalog"),
        "Katalog",
        QStringLiteral(KATALOG_VERSION_STRING),
        QCoreApplication::translate("Main", "Catalog your devices to search, analyze, and backup your files."),
        KAboutLicense::GPL_V3,
        "(c) 2020-2026");

    aboutData.setDesktopFileName(QStringLiteral("io.github.stephanecouturier.Katalog")); //Temporary, to hide the KDE GetInvolved Donation links
    aboutData.setBugAddress(""); //Temporary, to hide the KDE Bug link
    aboutData.addAuthor(
        "Stéphane Couturier",
        QCoreApplication::translate("Main", "Founder & Main Developer"),
        QStringLiteral("katalog@stephanecouturier.com"),
        QStringLiteral("https://stephanecouturier.github.io/Katalog/"));

    KAboutData::setApplicationData(aboutData);

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
        [&app, &engine, translator, appManager](const QString &code) {
            app.removeTranslator(translator);
            if (translator->load("Katalog_" + code, ":translations"))
                app.installTranslator(translator);
            engine.retranslate();
            // retranslate() reaches qsTr in QML only. The Selection card's
            // second line is built in C++ and held in the model, so it has to
            // be composed again in the new language. reloadDeviceListModel()
            // rather than refreshDeviceList(): the latter probes the filesystem
            // for active states, which a language change must not trigger.
            appManager->reloadDeviceListModel();
        });

    // Follow a light/dark change at runtime without restart (THM-F11). The
    // portal may also report the preference only after startup. Changing the
    // fallback paths invalidates Qt's icon cache, but icons already on screen
    // keep their pixmap: Kirigami.Icon reloads when the Kirigami colours change,
    // Controls icons (QQuickIconImage) only when their name changes, so each one
    // is given its name again to reload from the new set.
    QObject::connect(app.styleHints(), &QStyleHints::colorSchemeChanged, &engine,
        [applyFallbackIcons]() {
            const QStringList before = QIcon::fallbackSearchPaths();
            applyFallbackIcons();
            if (QIcon::fallbackSearchPaths() == before)
                return;
            const QWindowList windows = QGuiApplication::topLevelWindows();
            for (QWindow *window : windows) {
                const QList<QQuickItem *> items = window->findChildren<QQuickItem *>();
                for (QQuickItem *item : items) {
                    if (!item->inherits("QQuickIconImage"))
                        continue;
                    const QVariant name = item->property("name");
                    if (name.toString().isEmpty())
                        continue;
                    item->setProperty("name", QString());
                    item->setProperty("name", name);
                }
            }
        });

    //appManager->testQuery();

    return app.exec();
}
