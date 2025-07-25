/*

   This file is part of the KDE libraries
   Copyright (C) 2002 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2002-2003,2007 Oswald Buddenhagen <ossi@kde.org>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

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

#include "kpty_p.h"

#include <QtDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(procLog)

#define HAVE_POSIX_OPENPT

#if defined(__FreeBSD__) || defined(__DragonFly__)
#define HAVE_LOGIN
#define HAVE_LIBUTIL_H
#endif

#if defined(__OpenBSD__)
#define HAVE_LOGIN
#define HAVE_UTIL_H
#endif

#if defined(__NetBSD__)
#define HAVE_LOGIN
#define HAVE_UTIL_H
#define HAVE_OPENPTY
#endif

#if defined(__APPLE__)
#define HAVE_OPENPTY
#define HAVE_UTIL_H
#endif

#ifdef __sgi
#define __svr4__
#endif

#ifdef __osf__
#define _OSF_SOURCE
#include <float.h>
#endif

#ifdef _AIX
#define _ALL_SOURCE
#endif

// __USE_XOPEN isn't defined by default in ICC
// (needed for ptsname(), grantpt() and unlockpt())
#ifdef __INTEL_COMPILER
#  ifndef __USE_XOPEN
#    define __USE_XOPEN
#  endif
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <cerrno>
#include <fcntl.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <grp.h>

#if defined(HAVE_PTY_H)
# include <pty.h>
#endif

#ifdef HAVE_LIBUTIL_H
# include <libutil.h>
#elif defined(HAVE_UTIL_H)
# include <util.h>
#endif

#ifdef HAVE_UTEMPTER
extern "C" {
# include <utempter.h>
}
#else
# include <utmp.h>
# ifdef HAVE_UTMPX
#  include <utmpx.h>
# endif
# if !defined(_PATH_UTMPX) && defined(_UTMPX_FILE)
#  define _PATH_UTMPX _UTMPX_FILE
# endif
# ifdef HAVE_UPDWTMPX
#  if !defined(_PATH_WTMPX) && defined(_WTMPX_FILE)
#   define _PATH_WTMPX _WTMPX_FILE
#  endif
# endif
#endif

/* for HP-UX (some versions) the extern C is needed, and for other
   platforms it doesn't hurt */
extern "C" {
#include <termios.h>
#if defined(HAVE_TERMIO_H)
# include <termio.h> // struct winsize on some systems
#endif
}

#if defined (_HPUX_SOURCE)
# define _TERMIOS_INCLUDED
# include <bsdtty.h>
#endif

#ifdef HAVE_SYS_STROPTS_H
# include <sys/stropts.h> // Defines I_PUSH
# define _NEW_TTY_CTRL
#endif

#if defined (__FreeBSD__) || defined(__FreeBSD_kernel__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__bsdi__) || defined(__APPLE__) || defined (__DragonFly__)
# define _tcgetattr(fd, ttmode) ioctl(fd, TIOCGETA, (char *)ttmode)
#else
# if defined(_HPUX_SOURCE) || defined(__Lynx__) || defined (__CYGWIN__) || defined(__GNU__)
#  define _tcgetattr(fd, ttmode) tcgetattr(fd, ttmode)
# else
#  define _tcgetattr(fd, ttmode) ioctl(fd, TCGETS, (char *)ttmode)
# endif
#endif

#if defined (__FreeBSD__) || defined(__FreeBSD_kernel__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__bsdi__) || defined(__APPLE__) || defined (__DragonFly__)
# define _tcsetattr(fd, ttmode) ioctl(fd, TIOCSETA, (char *)ttmode)
#else
# if defined(_HPUX_SOURCE) || defined(__CYGWIN__) || defined(__GNU__)
#  define _tcsetattr(fd, ttmode) tcsetattr(fd, TCSANOW, ttmode)
# else
#  define _tcsetattr(fd, ttmode) ioctl(fd, TCSETS, (char *)ttmode)
# endif
#endif

//#include <kdebug.h>
//#include <kstandarddirs.h>  // findExe

// not defined on HP-UX for example
#ifndef CTRL
# define CTRL(x) ((x) & 037)
#endif

#define TTY_GROUP "tty"

///////////////////////
// private functions //
///////////////////////

//////////////////
// private data //
//////////////////

KPtyPrivate::KPtyPrivate(KPty* parent) :
        masterFd(-1), slaveFd(-1), ownMaster(true), q_ptr(parent)
{
    qCDebug(procLog) << "KPtyPrivate constructor";
}

KPtyPrivate::~KPtyPrivate()
{
    qCDebug(procLog) << "KPtyPrivate destructor";
}

bool KPtyPrivate::chownpty(bool)
{
//    return !QProcess::execute(KStandardDirs::findExe("kgrantpty"),
//        QStringList() << (grant?"--grant":"--revoke") << QString::number(masterFd));
    return true;
}

/////////////////////////////
// public member functions //
/////////////////////////////

KPty::KPty() :
        d_ptr(new KPtyPrivate(this))
{
    qCDebug(procLog) << "KPty constructor";
}

KPty::KPty(KPtyPrivate *d) :
        d_ptr(d)
{
    qCDebug(procLog) << "KPty constructor with private data";
    d_ptr->q_ptr = this;
}

KPty::~KPty()
{
    qCDebug(procLog) << "KPty destructor";
    close();
    delete d_ptr;
}

bool KPty::open()
{
    Q_D(KPty);

    qCDebug(procLog) << "Opening PTY master/slave pair";
    if (d->masterFd >= 0) {
        qCDebug(procLog) << "PTY master/slave pair already open";
        return true;
    }

    d->ownMaster = true;

    QByteArray ptyName;

    // Find a master pty that we can open ////////////////////////////////

    // Because not all the pty animals are created equal, they want to
    // be opened by several different methods.

    // We try, as we know them, one by one.

#ifdef HAVE_OPENPTY

    char ptsn[PATH_MAX];
    if (::openpty( &d->masterFd, &d->slaveFd, ptsn, 0, 0)) {
        d->masterFd = -1;
        d->slaveFd = -1;
        qCWarning(procLog) << "Can't open a pseudo teletype";
        return false;
    }
    qCDebug(procLog) << "openpty() successful, ttyName:" << ptsn;
    d->ttyName = ptsn;

#else

#ifdef HAVE__GETPTY // irix

    char *ptsn = _getpty(&d->masterFd, O_RDWR|O_NOCTTY, S_IRUSR|S_IWUSR, 0);
    if (ptsn) {
        d->ttyName = ptsn;
        goto grantedpt;
    }

#elif defined(HAVE_PTSNAME) || defined(TIOCGPTN)

#ifdef HAVE_POSIX_OPENPT
    d->masterFd = ::posix_openpt(O_RDWR|O_NOCTTY);
#elif defined(HAVE_GETPT)
    d->masterFd = ::getpt();
#elif defined(PTM_DEVICE)
    d->masterFd = ::open(PTM_DEVICE, O_RDWR|O_NOCTTY);
#else
# error No method to open a PTY master detected.
#endif
    if (d->masterFd >= 0) {
#ifdef HAVE_PTSNAME
        char *ptsn = ptsname(d->masterFd);
        if (ptsn) {
            d->ttyName = ptsn;
#else
    int ptyno;
    if (!ioctl(d->masterFd, TIOCGPTN, &ptyno)) {
        d->ttyName = QByteArray("/dev/pts/") + QByteArray::number(ptyno);
#endif
#ifdef HAVE_GRANTPT
            if (!grantpt(d->masterFd)) {
                goto grantedpt;
            }
#else

    goto gotpty;
#endif
        }
        ::close(d->masterFd);
        d->masterFd = -1;
    }
#endif // HAVE_PTSNAME || TIOCGPTN

    // Linux device names, FIXME: Trouble on other systems?
    for (const char * s3 = "pqrstuvwxyzabcde"; *s3; s3++) {
        for (const char * s4 = "0123456789abcdef"; *s4; s4++) {
            ptyName = QByteArrayLiteral("/dev/pty") + *s3 + *s4;
            d->ttyName = QByteArrayLiteral("/dev/tty") + *s3 + *s4;

            d->masterFd = ::open(ptyName.data(), O_RDWR);
            if (d->masterFd >= 0) {
#ifdef Q_OS_SOLARIS
                /* Need to check the process group of the pty.
                 * If it exists, then the slave pty is in use,
                 * and we need to get another one.
                 */
                int pgrp_rtn;
                if (ioctl(d->masterFd, TIOCGPGRP, &pgrp_rtn) == 0 || errno != EIO) {
                    ::close(d->masterFd);
                    d->masterFd = -1;
                    continue;
                }
#endif /* Q_OS_SOLARIS */
                if (!access(d->ttyName.data(),R_OK|W_OK)) { // checks availability based on permission bits
                    qCDebug(procLog) << "PTY is accessible, proceeding with permission setup for" << d->ttyName;
                    if (!geteuid()) {
                        qCDebug(procLog) << "Running as root, attempting to set tty group";
                        struct group * p = getgrnam(TTY_GROUP);
                        if (!p) {
                            qCDebug(procLog) << "Could not find 'tty' group, trying 'wheel' group";
                            p = getgrnam("wheel");
                        }
                        gid_t gid = p ? p->gr_gid : getgid ();

                        if (!chown(d->ttyName.data(), getuid(), gid)) {
                            qCDebug(procLog) << "Successfully chowned and chmoded" << d->ttyName;
                            chmod(d->ttyName.data(), S_IRUSR|S_IWUSR|S_IWGRP);
                        }
                    }
                    qCDebug(procLog) << "PTY found, jumping to gotpty";
                    goto gotpty;
                }
                ::close(d->masterFd);
                d->masterFd = -1;
            }
        }
    }

    qCWarning(procLog) << "Can't open a pseudo teletype";
    return false;

gotpty:
    struct stat st;
    if (stat(d->ttyName.data(), &st)) {
        qCWarning(procLog) << "stat failed for tty:" << d->ttyName.data() << "errno:" << errno;
        return false; // this just cannot happen ... *cough*  Yeah right, I just
        // had it happen when pty #349 was allocated.  I guess
        // there was some sort of leak?  I only had a few open.
    }
    if (((st.st_uid != getuid()) ||
            (st.st_mode & (S_IRGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH))) &&
            !d->chownpty(true)) {
        qCWarning(procLog) << "chownpty failed or permissions are insecure for tty:" << d->ttyName.data();
        qCWarning(procLog)
        << "chownpty failed for device " << ptyName << "::" << d->ttyName
        << "\nThis means the communication can be eavesdropped.";
    }

#if defined (HAVE__GETPTY) || defined (HAVE_GRANTPT)
grantedpt:
#endif

#ifdef HAVE_REVOKE
    revoke(d->ttyName.data());
#endif

#ifdef HAVE_UNLOCKPT
    unlockpt(d->masterFd);
#elif defined(TIOCSPTLCK)
    int flag = 0;
    ioctl(d->masterFd, TIOCSPTLCK, &flag);
#endif

    d->slaveFd = ::open(d->ttyName.data(), O_RDWR | O_NOCTTY);
    if (d->slaveFd < 0) {
        qCWarning(procLog) << "Can't open slave pseudo teletype";
        ::close(d->masterFd);
        d->masterFd = -1;
        return false;
    }

#if (defined(__svr4__) || defined(__sgi__))
    // Solaris
    ioctl(d->slaveFd, I_PUSH, "ptem");
    ioctl(d->slaveFd, I_PUSH, "ldterm");
#endif

#endif /* HAVE_OPENPTY */

    fcntl(d->masterFd, F_SETFD, FD_CLOEXEC);
    fcntl(d->slaveFd, F_SETFD, FD_CLOEXEC);

    return true;
}

bool KPty::open(int fd)
{
#if !defined(HAVE_PTSNAME) && !defined(TIOCGPTN)
     qCWarning(procLog) << "Unsupported attempt to open pty with fd" << fd;
     return false;
#else
    Q_D(KPty);

    if (d->masterFd >= 0) {
        qCWarning(procLog) << "Attempting to open an already open pty";
         return false;
    }

    d->ownMaster = false;

# ifdef HAVE_PTSNAME
    char *ptsn = ptsname(fd);
    if (ptsn) {
        d->ttyName = ptsn;
# else
    int ptyno;
    if (!ioctl(fd, TIOCGPTN, &ptyno)) {
        const size_t sz = 32;
        char buf[sz];
        const size_t r = snprintf(buf, sz, "/dev/pts/%d", ptyno);
        if (sz <= r) {
            qWarning("KPty::open: Buffer too small\n");
        }
        d->ttyName = buf;
# endif
    } else {
        qCWarning(procLog) << "Failed to determine pty slave device for fd" << fd;
        return false;
    }

    d->masterFd = fd;
    if (!openSlave()) {

        d->masterFd = -1;
        return false;
    }

    return true;
#endif
}

void KPty::closeSlave()
{
    qCDebug(procLog) << "Closing PTY slave";
    Q_D(KPty);

    if (d->slaveFd < 0) {
        qCDebug(procLog) << "PTY slave already closed, fd < 0";
        return;
    }
    ::close(d->slaveFd);
    d->slaveFd = -1;
    qCDebug(procLog) << "Closed PTY slave";
}

bool KPty::openSlave()
{
    Q_D(KPty);

    qCDebug(procLog) << "Opening PTY slave";

    if (d->slaveFd >= 0)
	return true;
    if (d->masterFd < 0) {
	qCInfo(procLog) << "Attempting to open pty slave while master is closed";
	return false;
    }
    //d->slaveFd = KDE_open(d->ttyName.data(), O_RDWR | O_NOCTTY);
    d->slaveFd = ::open(d->ttyName.data(), O_RDWR | O_NOCTTY);
    if (d->slaveFd < 0) {
	qCInfo(procLog) << "Can't open slave pseudo teletype";
	return false;
    }
    fcntl(d->slaveFd, F_SETFD, FD_CLOEXEC);
    qCDebug(procLog) << "Opened PTY slave return true, fd=" << d->slaveFd;
    return true;
}

void KPty::close()
{
    Q_D(KPty);

    qCDebug(procLog) << "Closing PTY master/slave pair";
    if (d->masterFd < 0) {
        qCDebug(procLog) << "PTY fd < 0, return";
        return;
    }
    closeSlave();
    // don't bother resetting unix98 pty, it will go away after closing master anyway.
    if (memcmp(d->ttyName.data(), "/dev/pts/", 9)) {
        qCDebug(procLog) << "Not a standard unix98 pty, attempting to reset permissions for" << d->ttyName.data();
        if (!geteuid()) {
            qCDebug(procLog) << "Running as root, resetting permissions";
            struct stat st;
            if (!stat(d->ttyName.data(), &st)) {
                qCDebug(procLog) << "stat successful, changing owner and mode";
                chown(d->ttyName.data(), 0, st.st_gid == getgid() ? 0 : -1);
                chmod(d->ttyName.data(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
            }
        } else {
            qCDebug(procLog) << "Not running as root, revoking pty";
            fcntl(d->masterFd, F_SETFD, 0);
            d->chownpty(false);
        }
    }
    ::close(d->masterFd);
    d->masterFd = -1;
    qCDebug(procLog) << "Closed PTY master/slave pair";
}

void KPty::setCTty()
{
    qCDebug(procLog) << "Setting controlling TTY";
    Q_D(KPty);

    // Setup job control //////////////////////////////////

    // Become session leader, process group leader,
    // and get rid of the old controlling terminal.
    setsid();

    // make our slave pty the new controlling terminal.
#ifdef TIOCSCTTY
    qCDebug(procLog) << "Setting controlling TTY using TIOCSCTTY";
    ioctl(d->slaveFd, TIOCSCTTY, 0);
#else
    qCDebug(procLog) << "Setting controlling TTY using open/close hack";
    // __svr4__ hack: the first tty opened after setsid() becomes controlling tty
    ::close(::open(d->ttyName, O_WRONLY, 0));
#endif

    // make our new process group the foreground group on the pty
    int pgrp = getpid();
    qCDebug(procLog) << "Setting process group to " << pgrp;
#if defined(_POSIX_VERSION) || defined(__svr4__)
    tcsetpgrp(d->slaveFd, pgrp);
#elif defined(TIOCSPGRP)
    ioctl(d->slaveFd, TIOCSPGRP, (char *)&pgrp);
#endif
}

void KPty::login(const char * user, const char * remotehost)
{
    qCDebug(procLog) << "Logging in to pty slave";
#ifdef HAVE_UTEMPTER
    Q_D(KPty);

    qCDebug(procLog) << "Using utempter to log in";
    addToUtmp(d->ttyName.constData(), remotehost, d->masterFd);
    Q_UNUSED(user);
#else
    qCDebug(procLog) << "Not using utempter, using utmp/utmpx";
# ifdef HAVE_UTMPX
    struct utmpx l_struct;
# else
    struct utmp l_struct;
# endif
    memset(&l_struct, 0, sizeof(l_struct));
    // note: strncpy without terminators _is_ correct here. man 4 utmp

    if (user) {
        qCDebug(procLog) << "Setting user to" << user;
# ifdef HAVE_UTMPX
        strncpy(l_struct.ut_user, user, sizeof(l_struct.ut_user));
# else
        strncpy(l_struct.ut_name, user, sizeof(l_struct.ut_name));
# endif
    }

    if (remotehost) {
        qCDebug(procLog) << "Setting remote host to" << remotehost;
        strncpy(l_struct.ut_host, remotehost, sizeof(l_struct.ut_host));
# ifdef HAVE_STRUCT_UTMP_UT_SYSLEN
        l_struct.ut_syslen = qMin(strlen(remotehost), sizeof(l_struct.ut_host));
# endif
    }

# ifndef __GLIBC__
    Q_D(KPty);
    const char * str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5)) {
        str_ptr += 5;
    }
    strncpy(l_struct.ut_line, str_ptr, sizeof(l_struct.ut_line));
#  ifdef HAVE_STRUCT_UTMP_UT_ID
    strncpy(l_struct.ut_id,
            str_ptr + strlen(str_ptr) - sizeof(l_struct.ut_id),
            sizeof(l_struct.ut_id));
#  endif
# endif

# ifdef HAVE_UTMPX
    gettimeofday(&l_struct.ut_tv, 0);
# else
    l_struct.ut_time = time(nullptr);
# endif

# ifdef HAVE_LOGIN
    qCDebug(procLog) << "Using login/loginx";
#  ifdef HAVE_LOGINX
    ::loginx(&l_struct);
#  else
    ::login(&l_struct);
#  endif
# else
    qCDebug(procLog) << "Not using login/loginx, manually updating utmp/utmpx";
#  ifdef HAVE_STRUCT_UTMP_UT_TYPE
    l_struct.ut_type = USER_PROCESS;
#  endif
#  ifdef HAVE_STRUCT_UTMP_UT_PID
    l_struct.ut_pid = getpid();
#   ifdef HAVE_STRUCT_UTMP_UT_SESSION
    l_struct.ut_session = getsid(0);
#   endif
#  endif
#  ifdef HAVE_UTMPX
    utmpxname(_PATH_UTMPX);
    setutxent();
    pututxline(&l_struct);
    endutxent();
#   ifdef HAVE_UPDWTMPX
    updwtmpx(_PATH_WTMPX, &l_struct);
#   endif
#  else
    utmpname(_PATH_UTMP);
    setutent();
    pututline(&l_struct);
    endutent();
    updwtmp(_PATH_WTMP, &l_struct);
#  endif
# endif
#endif

    qCDebug(procLog) << "Logged in to pty slave";
}

void KPty::logout()
{
    qCDebug(procLog) << "Logging out of pty slave";
#ifdef HAVE_UTEMPTER
    Q_D(KPty);

    qCDebug(procLog) << "Using utempter to log out";
    removeLineFromUtmp(d->ttyName.constData(), d->masterFd);
#else
    qCDebug(procLog) << "Not using utempter, using utmp/utmpx for logout";
    Q_D(KPty);

    const char *str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5)) {
        str_ptr += 5;
    }
# ifdef __GLIBC__
    else {
        const char * sl_ptr = strrchr(str_ptr, '/');
        if (sl_ptr) {
            str_ptr = sl_ptr + 1;
        }
    }
# endif
# ifdef HAVE_LOGIN
    qCDebug(procLog) << "Using logout/logoutx";
#  ifdef HAVE_LOGINX
    ::logoutx(str_ptr, 0, DEAD_PROCESS);
#  else
    ::logout(str_ptr);
#  endif
# else
    qCDebug(procLog) << "Not using logout/logoutx, manually updating utmp/utmpx";
#  ifdef HAVE_UTMPX
    struct utmpx l_struct, *ut;
#  else
    struct utmp l_struct, *ut;
#  endif
    memset(&l_struct, 0, sizeof(l_struct));

    strncpy(l_struct.ut_line, str_ptr, sizeof(l_struct.ut_line));

#  ifdef HAVE_UTMPX
    utmpxname(_PATH_UTMPX);
    setutxent();
    if ((ut = getutxline(&l_struct))) {
#  else
    utmpname(_PATH_UTMP);
    setutent();
    if ((ut = getutline(&l_struct))) {
#  endif
        qCDebug(procLog) << "Found utmp/utmpx entry, clearing it";
#  ifdef HAVE_UTMPX
        memset(ut->ut_user, 0, sizeof(*ut->ut_user));
#  else
        memset(ut->ut_name, 0, sizeof(*ut->ut_name));
#  endif
        memset(ut->ut_host, 0, sizeof(*ut->ut_host));
#  ifdef HAVE_STRUCT_UTMP_UT_SYSLEN
        ut->ut_syslen = 0;
#  endif
#  ifdef HAVE_STRUCT_UTMP_UT_TYPE
        ut->ut_type = DEAD_PROCESS;
#  endif
#  ifdef HAVE_UTMPX
        gettimeofday(&ut->ut_tv, 0);
        pututxline(ut);
    }
    endutxent();
#  else
    ut->ut_time = time(nullptr);
    pututline(ut);
}
endutent();
#  endif
# endif
#endif
    qCDebug(procLog) << "Logged out of pty slave";
}

// XXX Supposedly, tc[gs]etattr do not work with the master on Solaris.
// Please verify.

bool KPty::tcGetAttr(struct ::termios * ttmode) const
{
    qCDebug(procLog) << "Getting termios attributes";
    Q_D(const KPty);

    return _tcgetattr(d->masterFd, ttmode) == 0;
}

bool KPty::tcSetAttr(struct ::termios * ttmode)
{
    qCDebug(procLog) << "Setting termios attributes";
    Q_D(KPty);

    return _tcsetattr(d->masterFd, ttmode) == 0;
}

bool KPty::setWinSize(int lines, int columns)
{
    qCDebug(procLog) << "Setting window size to" << lines << "x" << columns;
    Q_D(KPty);

    struct winsize winSize;
    memset(&winSize, 0, sizeof(winSize));
    winSize.ws_row = (unsigned short)lines;
    winSize.ws_col = (unsigned short)columns;
    return ioctl(d->masterFd, TIOCSWINSZ, (char *)&winSize) == 0;
}

bool KPty::setEcho(bool echo)
{
    qCDebug(procLog) << "Setting echo to" << echo;
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        qCWarning(procLog) << "Failed to get termios attributes";
        return false;
    }
    if (!echo) {
        qCDebug(procLog) << "Disabling echo";
        ttmode.c_lflag &= ~ECHO;
    } else {
        qCDebug(procLog) << "Enabling echo";
        ttmode.c_lflag |= ECHO;
    }
    return tcSetAttr(&ttmode);
}

const char * KPty::ttyName() const
{
    qCDebug(procLog) << "Getting tty name";
    Q_D(const KPty);

    return d->ttyName.data();
}

int KPty::masterFd() const
{
    qCDebug(procLog) << "Getting master fd";
    Q_D(const KPty);

    return d->masterFd;
}

int KPty::slaveFd() const
{
    qCDebug(procLog) << "Getting slave fd";
    Q_D(const KPty);

    return d->slaveFd;
}
