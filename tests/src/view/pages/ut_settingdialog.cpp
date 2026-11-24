// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/pages/settingdialog.h"
#include "../deb-installer/utils/hierarchicalverify.h"

#include <DLabel>

#include <QTextDocument>

#include <stub.h>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

class UT_SettingDialog : public ::testing::Test
{
protected:
    void SetUp()
    {
        dialog = new SettingDialog;
        dialog->show();
    }
    void TearDown() { delete dialog; }

private:
    SettingDialog *dialog;
};

TEST_F(UT_SettingDialog, createProceedDefenderSafetyLabel_Pass)
{
    QWidget *w = SettingDialog::createProceedDefenderSafetyLabel(nullptr);
    DLabel *label = qobject_cast<DLabel *>(w);
    ASSERT_NE(label, nullptr);

    // DLabel 只有显式声明 RichText 才会在重新排版时使用 setHtml，AutoText 会被当成纯文本
    EXPECT_EQ(label->textFormat(), Qt::RichText);
    EXPECT_TRUE(label->text().contains("<a href='#"));
    EXPECT_TRUE(label->text().contains("Security Center"));
    delete w;
}

// 回归测试：主题切换会通过 PaletteChange 触发文案重新设置，重新排版后必须仍是富文本，
// 否则界面会直接显示 <a href=...> 这段 HTML 代码
TEST_F(UT_SettingDialog, paletteChanged_KeepsRichText_Pass)
{
    QWidget *w = SettingDialog::createProceedDefenderSafetyLabel(nullptr);
    DLabel *label = qobject_cast<DLabel *>(w);
    ASSERT_NE(label, nullptr);

    // 换一个高亮色，确保 updateText() 里的 setText() 真正生效
    QPalette palette = label->palette();
    palette.setColor(QPalette::Highlight, QColor("#ff0000"));
    label->setPalette(palette);

    EXPECT_EQ(label->textFormat(), Qt::RichText);
    EXPECT_TRUE(label->text().contains("<a href='#"));
    EXPECT_TRUE(label->text().contains("#ff0000"));

    // 触发一次绘制，DLabel 会在此过程中把文本重新灌入 QTextDocument
    label->resize(600, 80);
    label->grab();

    const auto docs = label->findChildren<QTextDocument *>();
    ASSERT_FALSE(docs.isEmpty());
    EXPECT_FALSE(docs.first()->toPlainText().contains("<a href"));
    EXPECT_TRUE(docs.first()->toPlainText().contains("Security Center"));

    delete w;
}

static bool g_trigger = false;
void stub_proceedDefenderSafetyPage()
{
    g_trigger = true;
}

TEST_F(UT_SettingDialog, linkActivated_Trigger_Pass)
{
    QWidget *w = SettingDialog::createProceedDefenderSafetyLabel(nullptr);
    DLabel *label = qobject_cast<DLabel *>(w);
    ASSERT_NE(label, nullptr);

    Stub stub;
    stub.set(ADDR(HierarchicalVerify, proceedDefenderSafetyPage), stub_proceedDefenderSafetyPage);

    g_trigger = false;
    Q_EMIT label->linkActivated("");
    EXPECT_TRUE(g_trigger);
}

TEST_F(UT_SettingDialog, switchHierarchicalNotify_Enable_Pass)
{
    dialog->switchHierarchicalNotify(true);

    DLabel *label = dialog->findChild<DLabel *>("OptionProceedLabel");
    ASSERT_NE(label, nullptr);
    EXPECT_TRUE(label->isVisible());
    EXPECT_TRUE(label->text().contains("Security Center"));
}

TEST_F(UT_SettingDialog, switchHierarchicalNotify_Disable_Pass)
{
    dialog->switchHierarchicalNotify(false);

    DLabel *label = dialog->findChild<DLabel *>("OptionProceedLabel");
    ASSERT_NE(label, nullptr);
    EXPECT_FALSE(label->isVisible());
    EXPECT_TRUE(label->text().contains("Security Center"));
}
