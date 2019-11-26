#include "quitconfirmdialog.h"
#include "utils.h"

#include <DPushButton>

QuitConfirmDialog::QuitConfirmDialog(QWidget *parent)
    :DDialog(parent)
//    , m_icon(new DLabel)
    , m_tipLabel(new DLabel)
    , m_infoWrapperWidget(new QWidget(this))
    , m_cancelBtn(new DPushButton)
    , m_confirmBtn(new DPushButton)
{
    setFixedSize(400, 200);
    initUI();
}

QuitConfirmDialog::QuitConfirmDialog(const QString &title, const QString& message, QWidget *parent)
    :DDialog(title, message, parent)
    , m_tipLabel(new DLabel)
    , m_infoWrapperWidget(new QWidget(this))
    , m_cancelBtn(new DPushButton)
    , m_confirmBtn(new DPushButton)
{
    setFixedSize(400, 200);
    initUI();
}

void QuitConfirmDialog::initUI()
{
//    const QIcon icon = QIcon::fromTheme("application-x-deb");

//    m_icon->setFixedSize(64, 64);
//    m_icon->setPixmap(icon.pixmap(64, 64));

    QStringList btnTextList;
    btnTextList << tr("Stop Installation") << tr("Continue Installation");
    addButtons(btnTextList);


    QPushButton* btnYes = qobject_cast<QPushButton*>(getButton(0));
    QPushButton* btnNo = qobject_cast<QPushButton*>(getButton(1));

    btnYes->setFocusPolicy(Qt::NoFocus);
    btnNo->setFocusPolicy(Qt::NoFocus);

    connect(btnYes, &DPushButton::clicked, this, &QuitConfirmDialog::rejected);
    connect(btnNo, &DPushButton::clicked, this, &QuitConfirmDialog::accepted);
}

void QuitConfirmDialog::setTipMessage(const QString &msg)
{
    m_tipLabel->setText(msg);
}
