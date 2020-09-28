#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "storage.h"

//---------------------------------------------------------------------------------------------------
//--- UI methods-------------------------------------------------------------------------------------
//--------- Full list --------------------------------------------------
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_CreateList_clicked()
{

    // Define storage file
    storageFilePath = collectionFolder + "/" + "storage.csv";

    // Create if is does not exist
    QFile newStorageFile(storageFilePath);
    if(!newStorageFile.open(QIODevice::ReadOnly)) {

        if (newStorageFile.open(QFile::WriteOnly | QFile::Text)) {

              QTextStream stream(&newStorageFile);

              stream << "Name"      << "\t"
                     << "ID"        << "\t"
                     << "Type"      << "\t"
                     << "Location"  << "\t"
                     << "Path"      << "\t"
                     << "Label"     << "\t"
                     << "FileSystem"<< "\t"
                     << "Total"     << "\t"
                     << "Free"      << "\t"
                     << '\n';

              newStorageFile.close();

              ui->Storage_PB_Reload->setEnabled(true);
              ui->Storage_PB_EditAll->setEnabled(true);
              //ui->Storage_PB_SaveAll->setEnabled(true);

              KMessageBox::information(this,"A storage file was created:\n" + newStorageFile.fileName()
                                       + "\nYou can edit it now.");

              //Disable button so it cannot be overwritten
              ui->Storage_PB_CreateList->setEnabled(false);

              //Even if empty, load it to the model
              loadStorageModel();

        return;
        }
    }
}
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_Reload_clicked()
{
    loadStorageModel();
}
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_EditAll_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(storageFilePath));
}
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_SaveAll_clicked()
{
    //Prepare export file name

    QString filename = "storage2.csv";
    filename = collectionFolder + "/" + filename;
    QFile exportFile(filename);

    //QFile exportFile(collectionFolder+"/file.txt");

      if (exportFile.open(QFile::WriteOnly | QFile::Text)) {

          QTextStream stream(&exportFile);
          for (int i = 0; i < filesFoundList.size(); ++i)
            stream << filesFoundList.at(i) << '\n';
        }

      KMessageBox::information(this,"Results exported to the collection folder:\n"+exportFile.fileName());
      exportFile.close();
}
//----------------------------------------------------------------------
void MainWindow::on_Storage_TrV_activated(const QModelIndex &index)
{
    selectedStorageName      = ui->Storage_TrV->model()->index(index.row(), 2, QModelIndex()).data().toString();
    selectedStorageLocation = ui->Storage_TrV->model()->index(index.row(), 0, QModelIndex()).data().toString();
    selectedStoragePath      = ui->Storage_TrV->model()->index(index.row(), 5, QModelIndex()).data().toString();

    //display buttons
    ui->Storage_PB_SearchStorage->setEnabled(true);
    //ui->Storage_PB_SearchLocation->setEnabled(true);
    ui->Storage_PB_CreateCatalog->setEnabled(true);
    //ui->PB_S_Update->setEnabled(true);

    selectedStorageIndexRow = index.row();
}

//--------- With seleted storage ---------------------------------------
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_New_clicked()
{
    //KMessageBox::information(this,"on_Storage_PB_New_clicked\n");
}
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_SearchStorage_clicked()
{//KMessageBox::information(this,"on_PB_S_Search_clicked\n"+ selectedStorageName);
    //Change tab to show the Search screen
    ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab

    ui->CB_SelectCatalog->setCurrentText("Selected Storage");
}
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_SearchLocation_clicked()
{
    //Change tab to show the Search screen
    ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab

    ui->CB_SelectCatalog->setCurrentText("Selected Location");
}
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_CreateCatalog_clicked()
{
    //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
    ui->LE_NewCatalogPath->setText(selectedStoragePath);

    //Select this directory in the treeview.
    LoadFileSystem(selectedStoragePath);

    //Change tab to show the result of the catalog creation
    ui->tabWidget->setCurrentIndex(3); // tab 3 is the Create Catalog tab

}
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_OpenFilelight_clicked()
{
    //KMessageBox::information(this,"on_Storage_PB_OpenFilelight_clicked\n");
}
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_Update_clicked()
{
    KMessageBox::information(this,"test:\n" + QString::number(selectedStorageIndexRow));

    //getStorageInfo(selectedStoragePath);
    //QModelIndex index;
    //index selectedStorageIndexRow;
    //QString test;
    //QModelIndex index = index.sibling(selectedStorageIndexRow, 1);
    //QVariant data = ui->TrV_Storage->model()->data(index);
    //QString text = data.toString();

    //ui->TrV_Storage->model()->removeRow(selectedStorageIndexRow);
    //ui->TrV_Storage->model()->removeRow(selectedStorageIndexRow);
    //listModel->removeRows(ui->listView_3->currentIndex().row(),1);

    //KMessageBox::information(this,"test:\n" + text);

}
//----------------------------------------------------------------------
void MainWindow::on_Storage_PB_Delete_clicked()
{
    //ui->TrV_Storage->model()->removeRow(selectedStorageIndexRow);
    //storageListModel->removeRows(selectedStorageIndexRow,1);
    //proxyStorageModel->removeRows(ui->listView_3->currentIndex().row(),1);
}

//---------------------------------------------------------------------------------------------------
//--DATA methods ------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------
void MainWindow::loadStorageModel()
{
    //Set up temporary lists
    QList<QString> sNames;
    QList<QString> sIDs;
    QList<QString> sTypes;
    QList<QString> sLocations;
    QList<QString> sPaths;
    QList<QString> sLabels;
    QList<QString> sFileSystemTypes;
    QList<qint64>  sBytesTotals;
    QList<qint64>  sBytesFrees;
    QList<QIcon>   sIcons;

    //Get data

    //Define storage file
    storageFilePath = collectionFolder + "/" + "storage.csv";
    QFile storageFile(storageFilePath);

    //Open file or return information
    if(!storageFile.open(QIODevice::ReadOnly)) {

//        KMessageBox::information(this,"No storage file was found in the current collection folder."
//                                      "\nPlease create one with the button 'Create list'\n");
        return;
    }

    QTextStream textStream(&storageFile);

    qint64 storageGrandTotal = 0;
    qint64 storageGrandFree  = 0;

    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
            if (line.left(4)!="Name"){//skip the first line with headers

                //Split the string with tabulation into a list
                QStringList fieldList = line.split('\t');

                QIcon icon;
                icon.fromTheme("drive-harddisk");

                //Append data to the lists
                sNames.append(fieldList[0]);
                sIDs.append(fieldList[1]);
                sIcons.append(icon);
                sTypes.append(fieldList[2]);
                sLocations.append(fieldList[3]);
                sPaths.append(fieldList[4]);
                sLabels.append(fieldList[5]);
                sFileSystemTypes.append(fieldList[6]);
                sBytesTotals.append(fieldList[7].toLongLong());
                sBytesFrees.append(fieldList[8].toLongLong());

                storageGrandTotal = storageGrandTotal + fieldList[7].toLongLong();
                storageGrandFree  = storageGrandFree  + fieldList[8].toLongLong();
            }
        }

        //Calculate totals
        int storageCount = sNames.count();
        qint64 storageGrandUsed = storageGrandTotal - storageGrandFree;

    // Create model
    Storage *storageModel = new Storage(this);

    // Populate model with data
    storageModel->populateStorageData(sNames,
                                      sIDs,
                                      sTypes,
                                      sLocations,
                                      sPaths,
                                      sLabels,
                                      sFileSystemTypes,
                                      sBytesTotals,
                                      sBytesFrees,
                                      sIcons
                                      );

    QSortFilterProxyModel *proxyStorageModel = new QSortFilterProxyModel(this);
    proxyStorageModel->setSourceModel(storageModel);

    // Connect model to tree/table view
    ui->Storage_TrV->setModel(proxyStorageModel);
    ui->Storage_TrV->QTreeView::sortByColumn(1,Qt::AscendingOrder);
    ui->Storage_TrV->QTreeView::sortByColumn(0,Qt::AscendingOrder);
    ui->Storage_TrV->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->Storage_TrV->header()->resizeSection(0, 200); //Location
    ui->Storage_TrV->header()->resizeSection(1,  50); //Icon
    ui->Storage_TrV->header()->resizeSection(2, 175); //Name
    ui->Storage_TrV->header()->resizeSection(3,  50); //ID
    ui->Storage_TrV->header()->resizeSection(4,  75); //Type
    ui->Storage_TrV->header()->resizeSection(5, 250); //Path
    ui->Storage_TrV->header()->resizeSection(6,  75); //FS
    ui->Storage_TrV->header()->resizeSection(7,  75); //Total
    ui->Storage_TrV->header()->resizeSection(8,  75); //Free
    ui->Storage_TrV->header()->hideSection(1); //Path

    ui->L_StorageCountValue->setText(QString::number(storageCount));
    ui->L_StorageSpaceTotalValue->setText(QString::number(storageGrandTotal));
    ui->L_StorageSpaceUsedValue->setText(QString::number(storageGrandUsed));
    ui->L_StorageSpaceFreeValue->setText(QString::number(storageGrandFree));

    //Get list of storage names for Create screen
    storageNameList = sNames;

    ui->Storage_PB_Reload->setEnabled(true);
    ui->Storage_PB_EditAll->setEnabled(true);
    //ui->Storage_PB_SaveAll->setEnabled(true);

//    KMessageBox::information(this,"A storage file was created:\n"
//                             + "\nYou can edit it now.");

    //Disable button so it cannot be overwritten
    ui->Storage_PB_CreateList->setEnabled(false);


}
//----------------------------------------------------------------------
void MainWindow::getStorageInfo(const QString &storagePath)
{
    //QStorageInfo storage = QStorageInfo::root();
    KMessageBox::information(this,"path:\n" + storagePath);

    QStorageInfo storage;
    storage.setPath(storagePath);

    KMessageBox::information(this,"test:\n" + storage.rootPath());
    if (storage.isReadOnly())
        qDebug() << "isReadOnly:" << storage.isReadOnly();

    KMessageBox::information(this,"test:\n" + storage.name());
    KMessageBox::information(this,"test:\n" + storage.fileSystemType());
    qint64 sizeTotal = storage.bytesTotal()/1024/1024;
    qint64 sizeAvailable = storage.bytesFree()/1024/1024;
    KMessageBox::information(this,"test:\n" + QString::number(sizeTotal));
    KMessageBox::information(this,"test:\n" + QString::number(sizeAvailable));

    //return storage.name();
}
