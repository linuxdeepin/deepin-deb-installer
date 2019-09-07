/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bluebutton.h"

BlueButton::BlueButton(DWidget *parent)
    : DSuggestButton(parent) {
    setFixedSize(120, 36);

    // setStyleSheet("BlueButton {"
    //              "color: #2ca7f8;"
    //              "border: 1px solid #2ca7f8;"
    //              "border-radius: 4px;"
    //              "}"
    //              ""
    //              "BlueButton:hover {"
    //              "color: white;"
    //              "background-color: qlineargradient(x1:0 y1:0, x2:0 y2:1, stop:0 #8ccfff, stop:1 #4bb8ff);"
    //              "}"
    //              ""
    //              "BlueButton:pressed {"
    //              "background-color: qlineargradient(x1:0 y1:0, x2:0 y2:1, stop:0 #0b8cff, stop:1 #0aa1ff);"
    //              "}");
}
