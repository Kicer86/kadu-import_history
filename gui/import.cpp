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
#include "debug.h"
#include "misc/misc.h"
// #include "modules.h"
#include "icons/icons-manager.h"

#include "import.h"
#include "importers/ggimporter.h"

/** @ingroup Import
 * @{
 */


Import::Import(QDialog *p) : QWidget(p, Qt::Dialog), thread(false)
{
  //modules_manager->moduleIncUsageCount("import_history");
  setWindowTitle(tr("Import history"));
  setAttribute(Qt::WA_DeleteOnClose);

  ggPathLabel=      new QLabel(tr("Path to gg archive:"));
  ggPath=           new QLineEdit;
  ggBrowseButton=   new QPushButton(IconsManager::instance()->iconByPath("ImportFromFile"),tr("Browse"));
  ggProceedButton=  new QPushButton(IconsManager::instance()->iconByPath("FetchUserList"),tr("Import"));

  accountLabel=     new QLabel(tr("Target account"));
  accountCBox=      new QComboBox();
  cancelButton=     new QPushButton(IconsManager::instance()->iconByPath("CloseWindow"),tr("Close"));
  progressBar=      new QProgressBar();

  connect(ggBrowseButton,   SIGNAL(clicked()), this, SLOT(ggBrowse()));
  connect(ggProceedButton,  SIGNAL(clicked()), this, SLOT(ggProceed()));
  connect(cancelButton,     SIGNAL(clicked()), this, SLOT(close()));

  QVBoxLayout *MainLayout=    new QVBoxLayout(this);  //główny layout

  /* grupa importu z gg */
  QGroupBox *gg_import=       new QGroupBox(tr("Import archives from Gadu-Gadu"));
  QVBoxLayout *gg_lay=        new QVBoxLayout(gg_import);

  QHBoxLayout *gg_path_layout= new QHBoxLayout(); //layout części "wybierz plik"
  gg_path_layout->addWidget(ggPathLabel);
  gg_path_layout->addWidget(ggPath);
  gg_path_layout->addWidget(ggBrowseButton);
  gg_lay->addLayout(gg_path_layout);

  /* przyciski i opcje globalne */
  QVBoxLayout *global_options= new QVBoxLayout();
  global_options->addWidget(progressBar);

  QHBoxLayout *buttons_layout= new QHBoxLayout();  //przycisk "anuluj"
  buttons_layout->addWidget(ggProceedButton);
  buttons_layout->addWidget(cancelButton);
  
  QHBoxLayout *account_layout=new QHBoxLayout();
  account_layout->addWidget(accountLabel);
  account_layout->addWidget(accountCBox);

  global_options->addLayout(account_layout);
  global_options->addLayout(buttons_layout);

  MainLayout->addWidget(gg_import);
  MainLayout->addLayout(global_options);
  
  timer=new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(updateProgress()));
  
  //lista kont
  accountList=AccountManager::instance()->items();
  for(int i=0; i<accountList.size(); i++)
    if (accountList[i].protocolHandler() && accountList[i].protocolHandler()->protocolFactory())
      accountCBox->addItem(accountList[i].protocolHandler()->protocolFactory()->name() + " " + accountList[i].id(), QVariant(i));
}


Import::~Import()
{}


void Import::ggBrowse()
{
  ggPath->setText(QFileDialog::getOpenFileName(this,
                  tr("Choose an archives file"),
                  "",
                  tr("Archives (archives.dat);; All files (*.* *)")
                                              )
                 );
}


void Import::ggProceed()
{
  cancelButton->setText(tr("Cancel"));
  ggProceedButton->setDisabled(true);
  ggBrowseButton->setDisabled(true);
  accountCBox->setDisabled(true);
  ggPath->setDisabled(true);

  imThread=new ImportFromGG(accountList[accountCBox->currentIndex()], ggPath->text(), this);
  connect(imThread,SIGNAL(boundaries(int,int)),progressBar, SLOT(setRange(int,int)));
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

  cancelButton->setText(tr("Close"));
  ggProceedButton->setDisabled(false);
  ggBrowseButton->setDisabled(false);
  accountCBox->setDisabled(false);
  ggPath->setDisabled(false);

  thread=false;                                     //wątek nieaktywny
  progressBar->reset();
  if (!imThread->canceled())
    QMessageBox::information(this,tr("Information"), tr("History imported sucsesfully."));
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
  progressBar->setValue(imThread->getPosition());
}

/** @} */
