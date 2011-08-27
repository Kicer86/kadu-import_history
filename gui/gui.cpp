/*
  Interface wtyczki
  Copyright (C) 2010  Michał Walenciak

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtGlobal>

#ifdef Q_WS_WIN
#include <windows.h>
#include <shlobj.h>
#endif

#include <QBoxLayout>
#include <QCloseEvent>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QProgressBar>
#include <QTime>
#include <QTimer>

#include "accounts/account-manager.h"
#include "activate.h"
#include "configuration/configuration-manager.h"
#include "debug.h"
#include "misc/misc.h"
#include "icons/icons-manager.h"

#include "gui.h"
#include "importers/gg7/ggimporter.h"
#include "importers/gg8/gg8importer.h"

/** @ingroup Import
 * @{
 */

Import *Import::Instance = 0;

void Import::show()
{
  if (!Instance)
    Instance = new Import();

  _activateWindow(Instance);
}

void Import::destroyInstance()
{
  delete Instance;
  Instance = 0;
}

Import::Import(QDialog *p):
    QWidget(p, Qt::Dialog), ui(new Ui_ImportHistory), thread(false)
{
  ui->setupUi(this);
  setWindowTitle(tr("Import history"));
  setAttribute(Qt::WA_DeleteOnClose);

#ifdef Q_WS_WIN
  QString hintString;
  WCHAR homepath[MAX_PATH + 1];
  if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, homepath)))
    hintString = QString::fromUtf16((const ushort *)homepath);
  else
    hintString = QDir::homePath();

  hintString = "<font size='-1'><i>" + tr("Hint: Search in ") + hintString + "</font></i>";
  ui->hint_new->setText(hintString);
  ui->hint_old->setText(hintString);
#else
  ui->hint_new->hide();
  ui->hint_old->hide();
#endif

  ui->cancelButton->setDisabled(true);

  connect(ui->ggBrowseButton,   SIGNAL(clicked()), this, SLOT(ggBrowse()));
  connect(ui->gg8BrowseButton,  SIGNAL(clicked()), this, SLOT(ggBrowse()));
  connect(ui->proceedButton,    SIGNAL(clicked()), this, SLOT(ggProceed()));
  connect(ui->cancelButton,     SIGNAL(clicked()), this, SLOT(close()));

  timer=new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(updateProgress()));

  //lista kont
  accountList=AccountManager::instance()->items();
  for (int i=0; i<accountList.size(); i++)
    if (accountList[i].protocolHandler() && accountList[i].protocolHandler()->protocolFactory())
      ui->accountCBox->addItem(accountList[i].protocolHandler()->protocolFactory()->name() + " " + accountList[i].id(), QVariant(i));
}


Import::~Import()
{
  delete ui;
  Instance = 0;
}


void Import::ggBrowse()
{
  switch (ui->tabWidget->currentIndex())
  {
    case 0:
      ui->ggPath->setText(QFileDialog::getOpenFileName
                          (this,
                           tr("Choose an archives file"),
                           "",
                           tr("Archives (archives.dat);; All files (*.* *)")
                          )
                         );
      break;
    case 1:
      ui->gg8Path->setText(QFileDialog::getExistingDirectory
                           (this,
                            tr("Choose an archive directory"),
                            ""
                           )
                          );
      break;

    default:
      qFatal("invalid option (%i)",ui->tabWidget->currentIndex());
  }
}


void Import::ggProceed()
{
  ui->cancelButton->setEnabled(true);
  ui->tabWidget->setDisabled(true);


  switch (ui->tabWidget->currentIndex())
  {
    case 0:
      imThread=new ImportFromGG(accountList[ui->accountCBox->currentIndex()], ui->ggPath->text(), this);
      break;
      
    case 1:
      imThread=new ImportFromGG8(accountList[ui->accountCBox->currentIndex()], ui->gg8Path->text(), this);
      break;

    default:
      qFatal("invalid option (%i)",ui->tabWidget->currentIndex());
  }

  connect(imThread,SIGNAL(boundaries(int,int)),ui->progressBar, SLOT(setRange(int,int)));
  connect(imThread,SIGNAL(finished()),this, SLOT(threadFinished()));
  imThread->start();                                //uruchom wątek konwertujący

  thread=true;                                      //wątek aktywny
  timer->start(500);
}



void Import::threadFinished()
{
  //rozłącz wątek
  disconnect(imThread,SIGNAL(finished()),this,0);
  disconnect(imThread,SIGNAL(boundaries(int,int)),this,0);

  ui->cancelButton->setDisabled(true);
  ui->tabWidget->setEnabled(true);

  thread=false;                                     //wątek nieaktywny
  ui->progressBar->reset();
  if (!imThread->canceled())
  {
    QMessageBox::information(this,tr("Information"), tr("History imported sucsesfully."));
    ConfigurationManager::instance()->flush();
  }
  imThread->deleteLater();                          //usuń zaraz obiekt
}


void Import::closeEvent(QCloseEvent *e)
{
  if (thread)
    if (QMessageBox::Yes==QMessageBox::warning(this,
        tr("Warning"), tr("History import process is in progress.\n"
                          "Do you really want to stop it?"),
        QMessageBox::Yes|QMessageBox::No,QMessageBox::No)
       )
    {
      imThread->cancelImport();
      e->ignore();                        //tylko zatrzymujemy wątek
    }
    else
      e->ignore();
  else
    e->accept();
}


void Import::updateProgress()
{
  ui->progressBar->setValue(imThread->getPosition());
}

/** @} */
