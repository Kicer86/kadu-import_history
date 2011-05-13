/*
 * Interface wtyczki
 * Copyright (C) 2010  Michał Walenciak
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef IMPORT_H
#define IMPORT_H

#include <QWidget>

#include "accounts/account.h"

class QTimer;
class QComboBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QProgressBar;
class QCloseEvent;

class ImportFromGG;
class Importer;

/**
 * @defgroup Import "Import"
 * @{
 */

class Import: public QWidget
{
    Q_OBJECT

    void closeEvent(QCloseEvent *event);

    /* gg */
    QLabel         *ggPathLabel;
    QLineEdit      *ggPath;
    QPushButton    *ggBrowseButton, *ggProceedButton;

    /* wspólne */
    QPushButton    *cancelButton;
    QProgressBar   *progressBar;
    QComboBox      *accountCBox;
    QLabel         *accountLabel;

    Importer       *imThread;
    bool           thread;

    QTimer *timer;
    QList<Account> accountList;

  private slots:
    void ggProceed();
    void ggBrowse();
    void threadFinished();
    void updateProgress();

  public:
    Import(QDialog *parent=0);
    ~Import();
};

/** @} */

#endif
