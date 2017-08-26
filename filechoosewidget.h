/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef FILECHOOSEWIDGET_H
#define FILECHOOSEWIDGET_H

#include <QWidget>

#include <dlinkbutton.h>

class QPushButton;
class FileChooseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileChooseWidget(QWidget *parent = nullptr);

signals:
    void packagesSelected(const QStringList files) const;

private slots:
    void chooseFiles();

private:
    Dtk::Widget::DLinkButton *m_fileChooseBtn;
};

#endif // FILECHOOSEWIDGET_H
