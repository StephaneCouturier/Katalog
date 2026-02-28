---
title: A test of QML and Kirigami for Katalog
authors:
  - StephaneCouturier
tags: [pre-development]
---

Looking at the future of Katalog, a few direction have been here since the start of the project:
- Multi-platform: why not even consider Mobile ? (ok, iOS should really be covered first)
- Using KDE KF6 librairies: this could be the best way to grow capabilities such a archives indexing or media files meta data
- User Experience: nicer interface, more responsive.

It happens that there is now a way to address all that at once: QML with Kirigami
- QML is the modern interface design code from Qt (which makes 100% of Katalog today)
- Kirigami is the QML based framework by KDE to provide additional interface and consistency in the KDE world.

Well, so far it works :)
Installation of the Kirigami:
- Linux (using kdebuild)
- Windows (using Craft)

Interface
- globaldrawer
- pages for each former tab

Backend / C++
- Basic Search function, connection to a collection database file and returning results based on the text criteria:
