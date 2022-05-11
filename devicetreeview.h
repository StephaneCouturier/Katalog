#ifndef DEVICETREEVIEW_H
#define DEVICETREEVIEW_H

#include <QSortFilterProxyModel>

class DeviceTreeView  : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    DeviceTreeView(QObject *parent = nullptr);

    QString m_selectedDeviceName;
    QString m_selectedDeviceType;
    void setSelectedDeviceInfo(QString selectedName,QString selectedType);

private:
    QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

};

#endif // DEVICETREEVIEW_H
