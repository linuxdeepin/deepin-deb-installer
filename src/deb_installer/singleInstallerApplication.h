#ifndef SINGLEFONTAPPLICATION_H
#define SINGLEFONTAPPLICATION_H

#include <DApplication>
#include <DMainWindow>
#include <QCommandLineParser>

#include <QDebug>

DWIDGET_USE_NAMESPACE

class SingleInstallerApplication : public DApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.DebInstaller")
public:
    explicit SingleInstallerApplication(int &argc, char **argv);

    virtual ~SingleInstallerApplication();

    void activateWindow();
    bool parseCmdLine();

public slots:
    Q_SCRIPTABLE void InstallerDeb(const QStringList &debPathList);

private:
    QStringList m_selectedFiles;

    QScopedPointer<DMainWindow> m_qspMainWnd;  // MainWindow ptr
};

#endif // SINGLEFONTAPPLICATION_H
