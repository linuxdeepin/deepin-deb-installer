/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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

#include <stdint.h>
#include <stddef.h>
#include <iostream>
using namespace std;

#include <QApt/Backend>
#include <QApt/DebFile>

using namespace QApt;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    Q_UNUSED(size);
    Q_UNUSED(data);

//    DebFile *debfile = new DebFile(reinterpret_cast<char *>(const_cast<uint8_t *>(data)));
//    debfile->isValid();
//    delete debfile;
    return 0;
}



