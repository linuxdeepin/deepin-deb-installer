
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

    DebFile *debfile = new DebFile(reinterpret_cast<char *>(const_cast<uint8_t *>(data)));
    debfile->isValid();
    return 0;
}



