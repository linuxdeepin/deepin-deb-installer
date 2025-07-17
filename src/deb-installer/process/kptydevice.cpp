/*
 * This file is a part of QTerminal - http://gitorious.org/qterminal
 *
 * This file was un-linked from KDE and modified
 * by Maxim Bourmistrov <maxim@unixconn.com>
 *
 */

/*

   This file is part of the KDE libraries
   Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>
   Copyright (C) 2010 KDE e.V. <kde-ev-board@kde.org>
     Author Adriaan de Groot <groot@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kptydevice.h"
#include "kpty_p.h"

#include <QSocketNotifier>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(procLog)

#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#if defined(Q_OS_FREEBSD) || defined(Q_OS_MAC)
  // "the other end's output queue size" - kinda braindead, huh?
# define PTY_BYTES_AVAILABLE TIOCOUTQ
#elif defined(TIOCINQ)
  // "our end's input queue size"
# define PTY_BYTES_AVAILABLE TIOCINQ
#else
  // likewise. more generic ioctl (theoretically)
# define PTY_BYTES_AVAILABLE FIONREAD
#endif




//////////////////
// private data //
//////////////////

// Lifted from Qt. I don't think they would mind. ;)
// Re-lift again from Qt whenever a proper replacement for pthread_once appears
static void qt_ignore_sigpipe()
{
    qCDebug(procLog) << "qt_ignore_sigpipe";
    static QBasicAtomicInt atom = Q_BASIC_ATOMIC_INITIALIZER(0);
    if (atom.testAndSetRelaxed(0, 1)) {
        qCDebug(procLog) << "Setting SIGPIPE to be ignored";
        struct sigaction noaction;
        memset(&noaction, 0, sizeof(noaction));
        noaction.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &noaction, nullptr);
    }
}

#define NO_INTR(ret,func) do { ret = func; } while (ret < 0 && errno == EINTR)

bool KPtyDevicePrivate::_k_canRead()
{
    qCDebug(procLog) << "_k_canRead";
    Q_Q(KPtyDevice);
    qint64 readBytes = 0;

#ifdef Q_OS_IRIX // this should use a config define, but how to check it?
    size_t available;
#else
    int available;
#endif
    if (!::ioctl(q->masterFd(), PTY_BYTES_AVAILABLE, (char *) &available)) {
        qCDebug(procLog) << "ioctl returned available bytes:" << available;
#ifdef Q_OS_SOLARIS
        // A Pty is a STREAMS module, and those can be activated
        // with 0 bytes available. This happens either when ^C is
        // pressed, or when an application does an explicit write(a,b,0)
        // which happens in experiments fairly often. When 0 bytes are
        // available, you must read those 0 bytes to clear the STREAMS
        // module, but we don't want to hit the !readBytes case further down.
        if (!available) {
            qCDebug(procLog) << "0 bytes available on Solaris, clearing STREAMS";
            char c;
            // Read the 0-byte STREAMS message
            NO_INTR(readBytes, read(q->masterFd(), &c, 0));
            // Should return 0 bytes read; -1 is error
            if (readBytes < 0) {
                qCDebug(procLog) << "Error reading 0-byte STREAMS message, disabling notifier";
                readNotifier->setEnabled(false);
                emit q->readEof();
                return false;
            }
            return true;
        }
#endif

        char *ptr = readBuffer.reserve(available);
#ifdef Q_OS_SOLARIS
        // Even if available > 0, it is possible for read()
        // to return 0 on Solaris, due to 0-byte writes in the stream.
        // Ignore them and keep reading until we hit *some* data.
        // In Solaris it is possible to have 15 bytes available
        // and to (say) get 0, 0, 6, 0 and 9 bytes in subsequent reads.
        // Because the stream is set to O_NONBLOCK in finishOpen(),
        // an EOF read will return -1.
        readBytes = 0;
        while (!readBytes)
#endif
        // Useless block braces except in Solaris
        {
          NO_INTR(readBytes, read(q->masterFd(), ptr, available));
        }
        if (readBytes < 0) {
            qCDebug(procLog) << "Error reading from PTY";
            readBuffer.unreserve(available);
            q->setErrorString(QLatin1String("Error reading from PTY"));
            return false;
        }
        readBuffer.unreserve(available - readBytes); // *should* be a no-op
    }

    if (!readBytes) {
        qCDebug(procLog) << "No bytes read, disabling notifier and emitting EOF";
        readNotifier->setEnabled(false);
        emit q->readEof();
        return false;
    } else {
        if (!emittedReadyRead) {
            qCDebug(procLog) << "Emitting readyRead";
            emittedReadyRead = true;
            emit q->readyRead();
            emittedReadyRead = false;
        }
        return true;
    }
}

bool KPtyDevicePrivate::_k_canWrite()
{
    qCDebug(procLog) << "_k_canWrite";
    Q_Q(KPtyDevice);

    writeNotifier->setEnabled(false);
    if (writeBuffer.isEmpty()) {
        qCDebug(procLog) << "Write buffer is empty, nothing to write";
        return false;
    }

    qt_ignore_sigpipe();
    int wroteBytes;
    NO_INTR(wroteBytes,
            write(q->masterFd(),
                  writeBuffer.readPointer(), writeBuffer.readSize()));
    if (wroteBytes < 0) {
        qCDebug(procLog) << "Error writing to PTY";
        q->setErrorString(QLatin1String("Error writing to PTY"));
        return false;
    }
    writeBuffer.free(wroteBytes);

    if (!emittedBytesWritten) {
        qCDebug(procLog) << "Emitting bytesWritten:" << wroteBytes;
        emittedBytesWritten = true;
        emit q->bytesWritten(wroteBytes);
        emittedBytesWritten = false;
    }

    if (!writeBuffer.isEmpty()) {
        qCDebug(procLog) << "More data to write, enabling write notifier";
        writeNotifier->setEnabled(true);
    }
    return true;
}

#ifndef timeradd
// Lifted from GLIBC
# define timeradd(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
        if ((result)->tv_usec >= 1000000) { \
            ++(result)->tv_sec; \
            (result)->tv_usec -= 1000000; \
        } \
    } while (0)
# define timersub(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) { \
            --(result)->tv_sec; \
            (result)->tv_usec += 1000000; \
        } \
    } while (0)
#endif

bool KPtyDevicePrivate::doWait(int msecs, bool reading)
{
    qCDebug(procLog) << "doWait for" << (reading ? "reading" : "writing") << "with timeout" << msecs;
    Q_Q(KPtyDevice);
#ifndef __linux__
    struct timeval etv;
#endif
    struct timeval tv, *tvp;

    if (msecs < 0) {
        qCDebug(procLog) << "Infinite wait";
        tvp = nullptr;
    } else {
        tv.tv_sec = msecs / 1000;
        tv.tv_usec = (msecs % 1000) * 1000;
#ifndef __linux__
        gettimeofday(&etv, 0);
        timeradd(&tv, &etv, &etv);
#endif
        tvp = &tv;
    }

    while (reading ? readNotifier->isEnabled() : !writeBuffer.isEmpty()) {
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        if (readNotifier->isEnabled())
            FD_SET(q->masterFd(), &rfds);
        if (!writeBuffer.isEmpty())
            FD_SET(q->masterFd(), &wfds);

#ifndef __linux__
        if (tvp) {
            gettimeofday(&tv, 0);
            timersub(&etv, &tv, &tv);
            if (tv.tv_sec < 0)
                tv.tv_sec = tv.tv_usec = 0;
        }
#endif

        switch (select(q->masterFd() + 1, &rfds, &wfds, nullptr, tvp)) {
        case -1:
            if (errno == EINTR) {
                qCDebug(procLog) << "select interrupted, continuing";
                break;
            }
            qCDebug(procLog) << "select error";
            return false;
        case 0:
            qCDebug(procLog) << "PTY operation timed out";
            q->setErrorString(QLatin1String("PTY operation timed out"));
            return false;
        default:
            if (FD_ISSET(q->masterFd(), &rfds)) {
                qCDebug(procLog) << "Ready to read";
                bool canRead = _k_canRead();
                if (reading && canRead)
                    return true;
            }
            if (FD_ISSET(q->masterFd(), &wfds)) {
                qCDebug(procLog) << "Ready to write";
                bool canWrite = _k_canWrite();
                if (!reading)
                    return canWrite;
            }
            break;
        }
    }
    qCDebug(procLog) << "doWait finished";
    return false;
}

void KPtyDevicePrivate::finishOpen(QIODevice::OpenMode mode)
{
    qCDebug(procLog) << "finishOpen with mode" << mode;
    Q_Q(KPtyDevice);

    q->QIODevice::open(mode);
    fcntl(q->masterFd(), F_SETFL, O_NONBLOCK);
    readBuffer.clear();
    readNotifier = new QSocketNotifier(q->masterFd(), QSocketNotifier::Read, q);
    writeNotifier = new QSocketNotifier(q->masterFd(), QSocketNotifier::Write, q);
    QObject::connect(readNotifier, SIGNAL(activated(int)), q, SLOT(_k_canRead()));
    QObject::connect(writeNotifier, SIGNAL(activated(int)), q, SLOT(_k_canWrite()));
    readNotifier->setEnabled(true);
}

/////////////////////////////
// public member functions //
/////////////////////////////

KPtyDevice::KPtyDevice(QObject *parent) :
    QIODevice(parent),
    KPty(new KPtyDevicePrivate(this))
{
    qCDebug(procLog) << "KPtyDevice constructed";
}

KPtyDevice::~KPtyDevice()
{
    qCDebug(procLog) << "KPtyDevice destructed";
    close();
}

bool KPtyDevice::open(OpenMode mode)
{
    Q_D(KPtyDevice);
    qCDebug(procLog) << "KPtyDevice::open" << mode;
    if (masterFd() >= 0) {
        qCDebug(procLog) << "PTY already open";
        return true;
    }

    if (!KPty::open()) {
        setErrorString(QLatin1String("Error opening PTY"));
        return false;
    }

    d->finishOpen(mode);
    qCDebug(procLog) << "PTY opened";
    return true;
}

bool KPtyDevice::open(int fd, OpenMode mode)
{
    Q_D(KPtyDevice);
    qCDebug(procLog) << "KPtyDevice::open" << mode;
    if (!KPty::open(fd)) {
        setErrorString(QLatin1String("Error opening PTY"));
        qCDebug(procLog) << "Error opening PTY";
        return false;
    }

    d->finishOpen(mode);
    qCDebug(procLog) << "PTY opened";
    return true;
}

void KPtyDevice::close()
{
    Q_D(KPtyDevice);
    qCDebug(procLog) << "KPtyDevice::close";
    if (masterFd() < 0) {
        qCDebug(procLog) << "PTY not open";
        return;
    }

    delete d->readNotifier;
    delete d->writeNotifier;

    QIODevice::close();

    KPty::close();
    qCDebug(procLog) << "PTY closed";
}

bool KPtyDevice::isSequential() const
{
    qCDebug(procLog) << "isSequential";
    return true;
}

bool KPtyDevice::canReadLine() const
{
    // qCDebug(procLog) << "canReadLine";
    Q_D(const KPtyDevice);
    return QIODevice::canReadLine() || d->readBuffer.canReadLine();
}

bool KPtyDevice::atEnd() const
{
    // qCDebug(procLog) << "atEnd";
    Q_D(const KPtyDevice);
    return QIODevice::atEnd() && d->readBuffer.isEmpty();
}

qint64 KPtyDevice::bytesAvailable() const
{
    // qCDebug(procLog) << "bytesAvailable";
    Q_D(const KPtyDevice);
    return QIODevice::bytesAvailable() + d->readBuffer.size();
}

qint64 KPtyDevice::bytesToWrite() const
{
    // qCDebug(procLog) << "bytesToWrite";
    Q_D(const KPtyDevice);
    return d->writeBuffer.size();
}

bool KPtyDevice::waitForReadyRead(int msecs)
{
    // qCDebug(procLog) << "waitForReadyRead with msecs" << msecs;
    Q_D(KPtyDevice);
    return d->doWait(msecs, true);
}

bool KPtyDevice::waitForBytesWritten(int msecs)
{
    // qCDebug(procLog) << "waitForBytesWritten with msecs" << msecs;
    Q_D(KPtyDevice);
    return d->doWait(msecs, false);
}

void KPtyDevice::setSuspended(bool suspended)
{
    qCDebug(procLog) << "setSuspended" << suspended;
    Q_D(KPtyDevice);
    d->readNotifier->setEnabled(!suspended);
}

bool KPtyDevice::isSuspended() const
{
    qCDebug(procLog) << "isSuspended";
    Q_D(const KPtyDevice);
    return !d->readNotifier->isEnabled();
}

// protected
qint64 KPtyDevice::readData(char *data, qint64 maxlen)
{
    // qCDebug(procLog) << "readData with maxlen" << maxlen;
    Q_D(KPtyDevice);
    return d->readBuffer.read(data, (int)qMin<qint64>(maxlen, KMAXINT));
}

// protected
qint64 KPtyDevice::readLineData(char *data, qint64 maxlen)
{
    // qCDebug(procLog) << "readLineData with maxlen" << maxlen;
    Q_D(KPtyDevice);
    return d->readBuffer.readLine(data, (int)qMin<qint64>(maxlen, KMAXINT));
}

// protected
qint64 KPtyDevice::writeData(const char *data, qint64 len)
{
    // qCDebug(procLog) << "writeData with len" << len;
    Q_D(KPtyDevice);
    Q_ASSERT(len <= KMAXINT);

    d->writeBuffer.write(data, len);
    d->writeNotifier->setEnabled(true);
    return len;
}
