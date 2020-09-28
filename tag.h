#ifndef TAG_H
#define TAG_H


#include <QAbstractTableModel>
//#include <QTextStream>

class Tag : public QAbstractTableModel
{
    Q_OBJECT

public:
    Tag(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void populateTagData(const QList<QString> &folderPath,
                          const QList<QString> &tagName
                          );

private:
    QList<QString> folderPath;
    QList<QString> tagName;

};

#endif // TAG_H
