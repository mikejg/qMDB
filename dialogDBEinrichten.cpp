#include "dialogDBEinrichten.h"

DialogDBEinrichten::DialogDBEinrichten(QProgressBar* pb, DataBase* db, QWidget* parent, Qt::WindowFlags f) :
        QDialog(parent,f)
{
    setGeometry(30,30,300,300);
    setWindowTitle("DBEinrichten");
    QRect dr = parent->geometry();
    QPoint cp(dr.width()/2, dr.height()/2);
    setGeometry(QRect(cp.x()-200, cp.y()-300, 400,600));

    treeView = new QTreeView(this);
    treeView->setHeaderHidden(true);
    database = db;

    jobInsertMetaPaket = new JobInsertMetaPaket(pb, db, this);
    jobDeleteMetaPaket = new JobDeleteMetaPaket(pb, db, this);
    buttonAbbrechen = new QPushButton("Abbrechen", this);
    buttonAnwenden = new QPushButton("Anwenden", this);
    buttonNeuEinlesen = new QPushButton("DB Neu Einlesen", this);
    layoutMaster = new QVBoxLayout(this);

    QHBoxLayout* layoutButton = new QHBoxLayout();
    layoutButton->addWidget(buttonAnwenden);
    layoutButton->addWidget(buttonAbbrechen);

    layoutMaster->addWidget(treeView);
    layoutMaster->addWidget(buttonNeuEinlesen);
    layoutMaster->addLayout(layoutButton);

    rootDir = new QDir("/");
    rootDir->setFilter(QDir::AllDirs);

    QStringList items = rootDir->entryList();
    model = new QStandardItemModel(this);
    QStandardItem *parentItem = model->invisibleRootItem();

    foreach (QString verzeichnis, items)
    {
        if(verzeichnis != "." && verzeichnis != ".." )
        {
        QStandardItem *item = new QStandardItem(verzeichnis);
        item->setWhatsThis(QString("/" + verzeichnis));
        item->setCheckable(true);
        parentItem->appendRow(item);
        }
    }
    treeView->setModel(model);

    readSettings();

    connect(treeView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(treeViewClicked(QModelIndex)));
    connect(model, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(itemChanged(QStandardItem*)));
    connect(jobInsertMetaPaket, SIGNAL(done()),
            this, SLOT(writeSettings()));
    connect(jobInsertMetaPaket, SIGNAL(done()),
            this, SLOT(enableAll()));
    connect(jobDeleteMetaPaket, SIGNAL(done()),
            this, SLOT(writeSettings()));
    connect(jobDeleteMetaPaket, SIGNAL(done()),
            this, SLOT(enableAll()));
    connect(buttonAnwenden, SIGNAL(clicked()),
            this, SLOT(anwenden()));
    connect(buttonAbbrechen, SIGNAL(clicked()),
            this, SLOT(close()));
    connect(buttonNeuEinlesen, SIGNAL(clicked()),
            this, SLOT(neuEinlesen()));
}

void DialogDBEinrichten::anwenden()
{
  //alte Einträge lesen
    QStringList oldDirs = database->readPath();

  //aktuelle Einträge lesen
    QStringList newDirs;
    findCheckedItems(&newDirs, model->invisibleRootItem());

  //Verzeichnisse die neu hinzukommen suchen
    QStringList listInput;
    foreach(QString s, newDirs)
    {
        if(!oldDirs.contains(s))
        {
            listInput << s;
        }
    }

  //Nach Files in den Verzeichnissen suchen
    QStringList listInputFiles;
    QDir dir("/");
    foreach(QString inputDir, listInput)
    {
        dir.cd(inputDir);
        findTracks(dir, &listInputFiles);
    }

  //Verzeichnisse die gelöscht werden sollen suchen
    QStringList listOutput;
    foreach(QString v, oldDirs)
    {
        if(!newDirs.contains(v))
        {
            listOutput << v;
        }
    }

    //Nach Files in den Verzeichnissen suchen
      QStringList listOutputFiles;
      foreach(QString outputDir, listOutput)
      {
          dir.cd(outputDir);
          findTracks(dir, &listOutputFiles);
      }


    jobDeleteMetaPaket->setFileListe(listOutputFiles, listInputFiles);
    //es gibt ja immer arschlöcher die nicht warten können bis der
    //progressBar durchgelaufen ist und in der zwischenzeit wüst rumklicken wollen
    this->setEnabled(false);
    jobDeleteMetaPaket->startJob();

  /*
  //Einträge die in oldDirs sind aus newDirs löschen
    foreach(QString s, oldDirs)
    {
        if(newDirs.contains(s))
        {
            newDirs.removeOne(s);
        }
    }

  //Nach neuen Track suchen
    QStringList* list = new QStringList();
    QDir dir("/");
    foreach(QString dirName, newDirs)
    {
        dir.cd(dirName);
        findTracks(dir, list);
    }

  //Nach Duplikaten checken
    QStringList urls = database->getUrl();
    foreach(QString url, urls)
    {
        if(list->contains(url))
        {
            list->removeOne(url);
        }
    }

  //es gibt ja immer arschlöcher die nicht warten können bis der
  //progressBar durchgelaufen ist und in der zwischenzeit wüst rumklicken wollen
      this->setEnabled(false);

    qDebug() << "Files:" << QString("%1").arg(list->count());
    jobInsertMetaPaket->setFileListe(*list);
    jobInsertMetaPaket->startJob();
    */
}

void DialogDBEinrichten::enableAll()
{
    this->setEnabled(true);
}

void DialogDBEinrichten::findCheckedItems(QStringList* list, QStandardItem* item)
{
    //Suchte alle ausgewählten Verzeichnisse
    if(item->hasChildren())
    {
        int i = 0;
        while(item->child(i))
        {
            if(item->child(i)->checkState() == Qt::Checked)
            {
                *list << item->child(i)->whatsThis();
            }
            if(item->child(i)->checkState() == Qt::PartiallyChecked)
                findCheckedItems(list, item->child(i));
            i++;
        }
    }
}

void DialogDBEinrichten::findTracks(QDir dir, QStringList* list)
{
    //Sucht rekursiv im Verzeichnis dir nach mp3 Files die dann
    //in list abgelegt werden

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    QFileInfoList infoList = dir.entryInfoList();
    foreach(QFileInfo info, infoList)
    {
        if(info.isFile())
        {
            if(info.suffix() == "mp3")
              *list << info.absoluteFilePath();
        }
        if(info.isDir())
        {
            dir.cd(info.absoluteFilePath());
            findTracks(dir, list);
        }
    }
}

void DialogDBEinrichten::readSettings()
{
    /*QSettings settings("qmdb", "d_DBEinrichten");
    QStringList defaultEintrag;
    defaultEintrag << QDir::homePath();

    QStringList eintraege = settings.value("Eintraege", defaultEintrag).toStringList();*/

    QStringList eintraege = database->readPath();

    foreach (QString eintrag, eintraege)
    {
        QStandardItem *parentItem = model->invisibleRootItem();

        //Pfad splitten zb /home/user/musik  home, user, musik
        QStringList pfade = eintrag.split("/", QString::SkipEmptyParts);

        foreach(QString pfad, pfade)
        {
            if(parentItem)
               parentItem = setItem(parentItem,pfad);
        }

        if( parentItem )
        {
            parentItem->setCheckState(Qt::Checked);
            itemChanged(parentItem);
        }
    }
}

QStandardItem* DialogDBEinrichten::setItem(QStandardItem* parentItem, QString itemText)
{
    /* Wenn parentItem children hat wird in den children nach dem child
       mit dem itemText gesucht. Die Funktion parentItem->child(i) liefert
       0 zurück wenn i grösser ist als vorhandene children*/

    if(parentItem->hasChildren())
    {
        int i = 0;
        while(parentItem->child(i) && parentItem->child(i)->text() != itemText)
        {
            i++;
        }

        //Was gefunden? Dann einen click simulieren damit die
        //Unterverzeichnisse eingelesen werden

        if(parentItem->child(i))
            treeViewClicked(parentItem->child(i)->index());

        return parentItem->child(i);
    }
    return 0;
}

//Slots
void DialogDBEinrichten::itemChanged(QStandardItem* item)
{

    QStandardItem* parentItem = item->parent();
    model->blockSignals(true);

    if(item->checkState() == Qt::Checked)
    {

        while(parentItem)
              {
               parentItem->setCheckState(Qt::PartiallyChecked);
               parentItem->setEnabled(false);
               parentItem = parentItem->parent();
              }

        if(item->hasChildren())
          {
            for(int i = 0; i < item->rowCount(); i++)
               {
                item->child(i)->setEnabled(false);
               }
          }
    }

    if(item->checkState() == Qt::Unchecked)
      {

          bool test = false;
          while(parentItem && !test)
               {
                if(parentItem->hasChildren())
                  {
                    for(int i = 0; i < parentItem->rowCount(); i++)
                       {
                        if( parentItem->child(i)->checkState() == Qt::Checked ||
                            parentItem->child(i)->checkState() == Qt::PartiallyChecked)
                            test = true;
                       }
                    if(!test)
                      {
                       parentItem->setCheckState(Qt::Unchecked);
                       parentItem->setEnabled(true);
                      }
                    parentItem = parentItem->parent();
                  }
               }

          if(item->hasChildren())
            {
              for(int i = 0; i < item->rowCount(); i++)
                 {
                  item->child(i)->setEnabled(true);
                 }
            }

      }

    model->blockSignals(false);
}

void DialogDBEinrichten::neuEinlesen()
{
    database->dbNeuErstellen();
    QDir dir("/");
    QStringList* list = new QStringList();
    QStringList* dirs = new QStringList();
    findCheckedItems(dirs, model->invisibleRootItem());

    foreach(QString dirName, *dirs)
    {
        dir.cd(dirName);
        findTracks(dir, list);
    }

    //es gibt ja immer arschlöcher die nicht warten können bis der
    //progressBar durchgelaufen ist und in der zwischenzeit wüst rumklicken wollen
    this->setEnabled(false);

    qDebug() << "Files:" << QString("%1").arg(list->count());
    jobInsertMetaPaket->setFileListe(*list);
    jobInsertMetaPaket->startJob();
}

void DialogDBEinrichten::treeViewClicked(QModelIndex index)
{
    QStandardItem* parentItem = model->itemFromIndex(index);
    if(!parentItem->isEnabled() && parentItem->checkState() != Qt::PartiallyChecked) return;

    QString path = parentItem->whatsThis();
    rootDir->cd(path);

    if(!parentItem->hasChildren())
      {
       QStringList items = rootDir->entryList();
       foreach (QString verzeichnis, items)
               {
                if(verzeichnis != "." && verzeichnis != ".." )
                  {
                   QStandardItem *item = new QStandardItem(verzeichnis);
                   item->setWhatsThis(QString( path + "/" + verzeichnis));
                   item->setCheckable(true);
                   parentItem->appendRow(item);
                   model->blockSignals(true);
                   if (parentItem->checkState() == Qt::Checked)
                       item->setEnabled(false);
                   model->blockSignals(false);
                  }
               }
      }
    (treeView->isExpanded(index)) ? treeView->setExpanded(index, false) :
                                    treeView->setExpanded(index, true);
}

void DialogDBEinrichten::writeSettings()
{
    QSettings settings("qmdb", "d_DBEinrichten");
    QStringList* list = new QStringList();
    findCheckedItems(list, model->invisibleRootItem());
    //settings.setValue("Eintraege", *list);

    database->writePath(*list);
}
