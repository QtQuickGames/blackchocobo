/****************************************************************************/
//    copyright 2010-2021 Chris Rizzitello <sithlord48@gmail.com>           //
//                                                                          //
//    This file is part of Black Chocobo.                                   //
//                                                                          //
//    Black Chocobo is free software: you can redistribute it and/or modify //
//    it under the terms of the GNU General Public License as published by  //
//    the Free Software Foundation, either version 3 of the License, or     //
//    (at your option) any later version.                                   //
//                                                                          //
//    Black Chocobo is distributed in the hope that it will be useful,      //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of        //
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//    GNU General Public License for more details.                          //
/****************************************************************************/
/*~~~~~~~~~~~Includes~~~~~~~~*/
#include <QAction>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QSpinBox>
#include <QString>
#include <QToolBox>
#include <QTabWidget>
#include <QWidget>

#include "about.h"
#include "achievementdialog.h"
#include "bcdialog.h"
#include "bcsettings.h"
#include "errbox.h"
#include "mainwindow.h"
#include "options.h"
#include "ui_mainwindow.h"

#include <FF7Char.h>
#include <FF7Item.h>
#include <FF7Location.h>
#include <FF7Save.h>
#include <FF7Materia.h>
#include <SaveIcon.h>
#include <OptionsWidget.h>
#include <MateriaEditor.h>
#include <SlotSelect.h>
#include <ChocoboEditor.h>
#include <CharEditor.h>
#include <ItemList.h>
#include <MetadataCreator.h>
#include <PhsListWidget.h>
#include <MenuListWidget.h>
#include <ChocoboManager.h>
#include <LocationViewer.h>

#include <qhexedit/qhexedit.h>
/*~~~~~~~~GUI Set Up~~~~~~~*/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , hexCursorPos(0)
    , _init(true)
    , load(true)
    , ff7(new FF7Save)
    , saveIcon(new SaveIcon)
    , s(0)
    , curchar(0)
    , mslotsel(-1)
    , phsList(new PhsListWidget)
    , menuList(new MenuListWidget)
    , optionsWidget(new OptionsWidget)
    , materia_editor(new MateriaEditor(this))
    , hexEditor(new QHexEdit)
    , chocoboManager(new ChocoboManager)
{
//Initilze Remaining Data
    setWindowIcon(QIcon(":/icon/bchoco"));
    buffer_materia.id = FF7Materia::EmptyId;
    for (int i = 0; i < 3; i++)
        buffer_materia.ap[i] = 0xFF;   //empty buffer incase
    setAcceptDrops(true);
    ui->setupUi(this);
    loadBasicSettings();
    detectTranslations();
    initDisplay();
    setScale(BCSettings::instance()->value(SETTINGS::SCALE).toDouble());
    populateCombos();
    init_style();
    loadChildWidgetSettings();
    init_connections();
    actionNewGame_triggered();
    btnCloud_clicked();
    ff7->setFileModified(false, 0);
}

void MainWindow::detectTranslations()
{
    m_translations.clear();

    QStringList nameFilter ={QStringLiteral("bchoco_*.qm")};
    QMap<QString, QTranslator *> app_translations;
    QDir dir (QStringLiteral("%1").arg(BCSettings::instance()->value(SETTINGS::LANGPATH).toString()));
    QStringList langList = dir.entryList(nameFilter,QDir::Files, QDir::Name);
    for (const QString &translation : langList) {
        QTranslator *translator = new QTranslator;
        translator->load(translation, dir.absolutePath());
        QString lang = translation.mid(7, 2);
        app_translations.insert(lang, translator);
        bool currentLang = (BCSettings::instance()->value(SETTINGS::LANG, QStringLiteral("en")).toString() == lang);
        if (currentLang) {
            BCSettings::instance()->setValue(SETTINGS::LANG, lang);
            QApplication::installTranslator(translator);
        }
    }

    QMap<QString, QTranslator *> ff7tk_translations;
    nameFilter = QStringList{QStringLiteral("ff7tk_*.qm")};
    dir.setPath(QStringLiteral("%1/%2").arg(QCoreApplication::applicationDirPath(), QStringLiteral("lang")));
    langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
    if (langList.isEmpty()) {
        dir.setPath(QStringLiteral("%1/../share/ff7tk/lang").arg(QCoreApplication::applicationDirPath()));
        langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
        if(langList.isEmpty()) {
            dir.setPath(QStringLiteral("%1/%2").arg(QDir::homePath(), QStringLiteral(".local/share/ff7tk/lang")));
            langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
            if(langList.isEmpty()) {
                dir.setPath(QStringLiteral("/usr/local/share/ff7tk/lang"));
                langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
                if(langList.isEmpty()) {
                    dir.setPath(QStringLiteral("/usr/share/ff7tk/lang"));
                    langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
                }
            }
        }
    }
    for (const QString &translation : qAsConst(langList)) {
        QTranslator *translator = new QTranslator;
        translator->load(translation, dir.absolutePath());
        QString lang = translation.mid(6, 2);
        ff7tk_translations.insert(lang, translator);
        bool currentLang = (BCSettings::instance()->value(SETTINGS::LANG, QStringLiteral("en")).toString() == lang);
        if (currentLang)
            QApplication::installTranslator(translator);
    }

    QMap<QString, QTranslator *> qt_translations;
    nameFilter = QStringList{QStringLiteral("qt_*.qm")};
    dir.setPath(QStringLiteral("%1/%2").arg(QCoreApplication::applicationDirPath(), QStringLiteral("translations")));
    langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
    if (langList.isEmpty()) {
        dir.setPath(QStringLiteral("%1/%2").arg(QCoreApplication::applicationDirPath(), QStringLiteral("lang")));
        langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
        if (langList.isEmpty()) {
            dir.setPath(QStringLiteral("%1/../share/qt/translations").arg(QCoreApplication::applicationDirPath()));
            langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
            if(langList.isEmpty()) {
                dir.setPath(QStringLiteral("%1/%2").arg(QDir::homePath(), QStringLiteral(".local/share/qt/translations")));
                langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
                if(langList.isEmpty()) {
                    dir.setPath(QStringLiteral("/usr/local/share/qt/translations"));
                    langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
                    if(langList.isEmpty()) {
                        dir.setPath(QStringLiteral("/usr/share/qt/translations"));
                        langList = dir.entryList(nameFilter, QDir::Files, QDir::Name);
                    }
                }
            }
        }
    }
    for (const QString &translation : qAsConst(langList)) {
        QTranslator *translator = new QTranslator;
        translator->load(translation, dir.absolutePath());
        QString lang = translation.mid(3, 2);
        qt_translations.insert(lang, translator);
        bool currentLang = (BCSettings::instance()->value(SETTINGS::LANG, QStringLiteral("en")).toString() == lang);
        if (currentLang)
            QApplication::installTranslator(translator);
    }
    const QStringList keys = app_translations.keys();
    QList<QTranslator *> tempList;
    for (const QString &lang : keys) {
        tempList.clear();
        tempList.append(app_translations.value(lang));
        tempList.append(ff7tk_translations.value(lang));
        if(lang != QStringLiteral("re"))
            tempList.append(qt_translations.value(lang));
        else
            tempList.append(qt_translations.value(QStringLiteral("en")));
        m_translations.insert(lang, tempList);
    }
}

void MainWindow::initDisplay()
{
    QHBoxLayout *phsLayout = new QHBoxLayout;
    phsLayout->addWidget(phsList);
    ui->Phs_Box->setLayout(phsLayout);

    QHBoxLayout *menuLayout = new QHBoxLayout;
    menuLayout->addWidget(menuList);
    ui->Menu_Box->setLayout(menuLayout);

    chocoboManager->setContentsMargins(0, 20, 0, 0);
    ui->tabWidget->insertTab(3, chocoboManager, tr("Chocobo"));
    ui->tabWidget->insertTab(7, optionsWidget, tr("Game Options"));

    optionsWidget->setControllerMappingVisible(false);
    QVBoxLayout *materia_editor_layout = new QVBoxLayout();
    mat_spacer = new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    materia_editor_layout->addWidget(materia_editor);
    materia_editor_layout->addSpacerItem(mat_spacer);
    ui->group_materia->setLayout(materia_editor_layout);

#ifdef Q_OS_MAC
    hexEditor->setFont(this->font());
#endif
    hexEditor->setAddressAreaColor(palette().alternateBase().color());
    hexEditor->setHighlightingColor(palette().linkVisited().color());
    hexEditor->setSelectionColor(palette().highlight().color());
    hexEditor->setHexCaps(true);
    QVBoxLayout *hexLayout = new QVBoxLayout;
    hexLayout->setContentsMargins(0, 0, 0, 0);
    hexLayout->addWidget(hexEditor);
    ui->group_hexedit->setLayout(hexLayout);

    double scale = BCSettings::instance()->value(SETTINGS::SCALE).toDouble();
    char_editor = new CharEditor(scale);
    QHBoxLayout *char_editor_layout = new QHBoxLayout;
    char_editor_layout->setContentsMargins(0, 0, 0, 0);
    char_editor_layout->setSpacing(0);
    char_editor_layout->addWidget(char_editor);
    ui->group_char_editor_box->setLayout(char_editor_layout);

    itemlist = new ItemList(this);
    ui->group_items->layout()->removeWidget(ui->group_item_options);
    ui->group_items->layout()->addWidget(itemlist);
    ui->group_items->layout()->addWidget(ui->group_item_options);
    ui->group_items->setFixedWidth(itemlist->width() + itemlist->contentsMargins().left() + itemlist->contentsMargins().right() + ui->group_items->contentsMargins().left() + ui->group_items->contentsMargins().right());

    locationViewer = new LocationViewer(scale);
    locationViewer->setRegion("BASCUS-94163FF7-S00");

    QVBoxLayout *locLayout = new QVBoxLayout;
    locLayout->setContentsMargins(0, 0, 0, 0);
    locLayout->addWidget(locationViewer);
    ui->fieldFrame->setLayout(locLayout);

    ui->statusBar->addWidget(ui->frame_status, 1);
    ui->frame_status->setFixedHeight(fontMetrics().height() + 2);
    ui->tblMateria->setIconSize(QSize(fontMetrics().height(), fontMetrics().height()));
    ui->tblUnknown->setColumnWidth(0, fontMetrics().horizontalAdvance(QStringLiteral("WW")));
    ui->tblUnknown->setColumnWidth(1, fontMetrics().horizontalAdvance(QStringLiteral("W")));
    ui->tblUnknown->setColumnWidth(2, fontMetrics().horizontalAdvance(QStringLiteral("W")));
    ui->tblUnknown->setColumnWidth(3, fontMetrics().horizontalAdvance(QStringLiteral("WWWWWW")));
    ui->tblUnknown->setColumnWidth(4, fontMetrics().horizontalAdvance(QStringLiteral("W")));

    ui->tblCompareUnknown->setColumnWidth(0, fontMetrics().horizontalAdvance(QStringLiteral("WW")));
    ui->tblCompareUnknown->setColumnWidth(1, fontMetrics().horizontalAdvance(QStringLiteral("W")));
    ui->tblCompareUnknown->setColumnWidth(2, fontMetrics().horizontalAdvance(QStringLiteral("W")));
    ui->tblCompareUnknown->setColumnWidth(3, fontMetrics().horizontalAdvance(QStringLiteral("WWWWWW")));
    ui->tblCompareUnknown->setColumnWidth(4, fontMetrics().horizontalAdvance(QStringLiteral("W")));

    int width = 0;
    for(int i = 0; i < 5; i++)
        width += ui->tblUnknown->columnWidth(i);

    width +=ui->tblUnknown->verticalScrollBar()->width();
    ui->table_unknown->setFixedWidth(width);
    ui->tblUnknown->setFixedWidth(width);

    width -=ui->tblUnknown->verticalScrollBar()->width();
    ui->compare_table->setFixedWidth(width);
    ui->tblCompareUnknown->setFixedWidth(width);
}

void MainWindow::setScale(double scale)
{
    scale = std::max(scale, 0.5);
    setStyleSheet(QStringLiteral("QListWidget::indicator, QCheckBox::indicator{width: .75em; height: .75em;}\nQListWidget::item{spacing: 1em}"));
    ui->btnCloud->setFixedSize(int(98 * scale), int(110 * scale));
    ui->btnCloud->setIconSize(QSize(int(92 * scale), int(104 * scale)));
    ui->btnBarret->setIconSize(QSize(int(92 * scale), int(104 * scale)));
    ui->btnBarret->setFixedSize(int(98 * scale), int(110 * scale));
    ui->btnTifa->setIconSize(QSize(int(92 * scale), int(104 * scale)));
    ui->btnTifa->setFixedSize(int(98 * scale), int(110 * scale));
    ui->btnAeris->setFixedSize(int(98 * scale), int(110 * scale));
    ui->btnAeris->setIconSize(QSize(int(92 * scale), int(104 * scale)));
    ui->btnRed->setFixedSize(int(98 * scale), int(110 * scale));
    ui->btnRed->setIconSize(QSize(int(92 * scale), int(104 * scale)));
    ui->btnYuffie->setFixedSize(int(98 * scale), int(110 * scale));
    ui->btnYuffie->setIconSize(QSize(int(92 * scale), int(104 * scale)));
    ui->btnCait->setFixedSize(int(98 * scale), int(110 * scale));
    ui->btnCait->setIconSize(QSize(int(92 * scale), int(104 * scale)));
    ui->btnVincent->setFixedSize(int(98 * scale), int(110 * scale));
    ui->btnVincent->setIconSize(QSize(int(92 * scale), int(104 * scale)));
    ui->btnCid->setFixedSize(int(98 * scale), int(110 * scale));
    ui->btnCid->setIconSize(QSize(int(92 * scale), int(104 * scale)));
    ui->comboParty1->setFixedHeight(int(32 * scale));
    ui->comboParty1->setIconSize(QSize(int(32 * scale), int(32 * scale)));
    ui->comboParty2->setFixedHeight(int(32 * scale));
    ui->comboParty2->setIconSize(QSize(int(32 * scale), int(32 * scale)));
    ui->comboParty3->setFixedHeight(int(32 * scale));
    ui->comboParty3->setIconSize(QSize(int(32 * scale), int(32 * scale)));
    ui->groupBox_11->setFixedWidth(int(375 * scale));
    ui->groupBox_18->setFixedWidth(int(273 * scale)); //materia table group.
    ui->scrollArea->setFixedWidth(int(310 * scale));
    ui->scrollAreaWidgetContents->adjustSize();
    ui->world_map_frame->setFixedSize(int(446 * scale), int(381 * scale));
    ui->worldMapView->setPixmap(QPixmap(":/icon/world_map").scaled(ui->world_map_frame->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->worldMapView->setGeometry(int(5 * scale), int(32 * scale), int(432 * scale), int(336 * scale));
    ui->comboMapControls->setFixedHeight(32);
    ui->slideWorldX->setGeometry(-1, int(369 * scale), int(443 * scale), int(10 * scale));
    ui->slideWorldY->setGeometry(int(437 * scale), int(26 * scale), int(10 * scale), int(347 * scale));
    ui->lbl_love_aeris->setFixedSize(int(50 * scale), int(68 * scale));
    ui->lbl_love_aeris->setPixmap(FF7Char::instance()->pixmap(FF7Char::Aerith).scaled(ui->lbl_love_aeris->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_love_barret->setFixedSize(int(50 * scale), int(68 * scale));
    ui->lbl_love_barret->setPixmap(FF7Char::instance()->pixmap(FF7Char::Barret).scaled(ui->lbl_love_barret->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_love_tifa->setFixedSize(int(50 * scale), int(68 * scale));
    ui->lbl_love_tifa->setPixmap(FF7Char::instance()->pixmap(FF7Char::Tifa).scaled(ui->lbl_love_tifa->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_love_yuffie->setFixedSize(int(50 * scale), int(68 * scale));
    ui->lbl_love_yuffie->setPixmap(FF7Char::instance()->pixmap(FF7Char::Yuffie).scaled(ui->lbl_love_yuffie->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_battle_love_aeris->setFixedSize(ui->sbBloveAeris->width(), int(74 * scale));
    ui->lbl_battle_love_aeris->setPixmap(FF7Char::instance()->pixmap(FF7Char::Aerith).scaled(ui->lbl_battle_love_aeris->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_battle_love_barret->setFixedSize(ui->sbBloveBarret->width(), int(74 * scale));
    ui->lbl_battle_love_barret->setPixmap(FF7Char::instance()->pixmap(FF7Char::Barret).scaled(ui->lbl_battle_love_barret->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_battle_love_tifa->setFixedSize(ui->sbBloveTifa->width(), int(74 * scale));
    ui->lbl_battle_love_tifa->setPixmap(FF7Char::instance()->pixmap(FF7Char::Tifa).scaled(ui->lbl_battle_love_tifa->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_battle_love_yuffie->setFixedSize(ui->sbBloveYuffie->width(), int(74 * scale));
    ui->lbl_battle_love_yuffie->setPixmap(FF7Char::instance()->pixmap(FF7Char::Yuffie).scaled(ui->lbl_battle_love_yuffie->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    materia_editor->setStarsSize(int(48 * scale));
    guirefresh(0);
}

void MainWindow::populateCombos()
{
//Party Combos
    if (ui->comboParty1->count() != 0) {
        for (int i = 0; i < 11; i++) {
            ui->comboParty1->setItemText(i, FF7Char::instance()->defaultName(i));
            ui->comboParty2->setItemText(i, FF7Char::instance()->defaultName(i));
            ui->comboParty3->setItemText(i, FF7Char::instance()->defaultName(i));
        }
        ui->comboParty1->setItemText(12, tr("-Empty-"));
        ui->comboParty2->setItemText(12, tr("-Empty-"));
        ui->comboParty3->setItemText(12, tr("-Empty-"));
    } else {
        for (int i = 0; i < 11; i++) {
            ui->comboParty1->addItem(FF7Char::instance()->icon(i), FF7Char::instance()->defaultName(i));
            ui->comboParty2->addItem(FF7Char::instance()->icon(i), FF7Char::instance()->defaultName(i));
            ui->comboParty3->addItem(FF7Char::instance()->icon(i), FF7Char::instance()->defaultName(i));
        }
        ui->comboParty1->addItem(QString("0x0B"));
        ui->comboParty2->addItem(QString("0x0B"));
        ui->comboParty3->addItem(QString("0x0B"));
        ui->comboParty1->addItem(tr("-Empty-"));
        ui->comboParty2->addItem(tr("-Empty-"));
        ui->comboParty3->addItem(tr("-Empty-"));
    }
//World party leader Combo.
    if (ui->comboWorldPartyLeader->count() != 0) {
        ui->comboWorldPartyLeader->setItemText(0, FF7Char::instance()->defaultName(FF7Char::Cloud));
        ui->comboWorldPartyLeader->setItemText(1, FF7Char::instance()->defaultName(FF7Char::Tifa));
        ui->comboWorldPartyLeader->setItemText(2, FF7Char::instance()->defaultName(FF7Char::Cid));
    } else {
        ui->comboWorldPartyLeader->addItem(FF7Char::instance()->icon(FF7Char::Cloud), FF7Char::instance()->defaultName(FF7Char::Cloud));
        ui->comboWorldPartyLeader->addItem(FF7Char::instance()->icon(FF7Char::Tifa), FF7Char::instance()->defaultName(FF7Char::Tifa));
        ui->comboWorldPartyLeader->addItem(FF7Char::instance()->icon(FF7Char::Cid), FF7Char::instance()->defaultName(FF7Char::Cid));
    }
}

void MainWindow::init_style()
{
    QString sliderStyleSheet("QSlider:sub-page{background-color: qlineargradient(spread:pad, x1:0.472, y1:0.011, x2:0.483, y2:1, stop:0 rgba(186, 1, 87,192), stop:0.505682 rgba(209, 128, 173,192), stop:0.931818 rgba(209, 44, 136, 192));}");
    sliderStyleSheet.append(QString("QSlider::add-page{background: qlineargradient(spread:pad, x1:0.5, y1:0.00568182, x2:0.497, y2:1, stop:0 rgba(91, 91, 91, 255), stop:0.494318 rgba(122, 122, 122, 255), stop:1 rgba(106, 106, 106, 255));}"));
    sliderStyleSheet.append(QString("QSlider{border:3px solid;border-left-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(123, 123, 123, 255), stop:1 rgba(172, 172, 172, 255));border-right-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(123, 123, 123, 255), stop:1 rgba(172, 172, 172, 255));border-bottom-color: rgb(172, 172, 172);border-top-color: rgb(172, 172, 172);border-radius: 5px;}"));
    sliderStyleSheet.append(QString("QSlider::groove{height: 12px;background: qlineargradient(spread:pad, x1:0.5, y1:0.00568182, x2:0.497, y2:1, stop:0 rgba(91, 91, 91, 255), stop:0.494318 rgba(122, 122, 122, 255), stop:1 rgba(106, 106, 106, 255));}"));
    sliderStyleSheet.append(QString("QSlider::handle{background: rgba(172, 172, 172,255);border: 1px solid #5c5c5c;width: 3px;border-radius: 2px;}"));
    char_editor->setSliderStyle(sliderStyleSheet);

    QString tabStyle = QString("::tab:hover{background-color:rgba(%1, %2, %3, 128);}").arg(QString::number(this->palette().highlight().color().red()), QString::number(this->palette().highlight().color().green()), QString::number(this->palette().highlight().color().blue()));
    char_editor->setToolBoxStyle(tabStyle);
    ui->locationToolBox->setStyleSheet(tabStyle);

    ui->slideWorldY->setStyleSheet(QString("::handle{image: url(:/icon/prev);}"));
    ui->slideWorldX->setStyleSheet(QString("::handle{image: url(:/icon/slider_up);}"));
}

void MainWindow::init_connections()
{
    //Actions
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionOpenSaveFile, &QAction::triggered, this, &MainWindow::actionOpenSaveFile_triggered);
    connect(ui->actionReload, &QAction::triggered, this, &MainWindow::actionReload_triggered);
    connect(ui->actionImportChar, &QAction::triggered, this , &MainWindow::actionImportChar_triggered);
    connect(ui->actionExportChar, &QAction::triggered, this , &MainWindow::actionExportChar_triggered);
    connect(ui->actionSave, &QAction::triggered, this , &MainWindow::actionSave_triggered);
    connect(ui->actionSaveFileAs, &QAction::triggered, this , &MainWindow::actionSaveFileAs_triggered);
    connect(ui->actionNewGame, &QAction::triggered, this, &MainWindow::actionNewGame_triggered);
    connect(ui->actionNewGamePlus, &QAction::triggered, this, &MainWindow::actionNewGamePlus_triggered);
    connect(ui->actionShowSelectionDialog, &QAction::triggered, this, &MainWindow::actionShowSelectionDialog_triggered);
    connect(ui->actionClearSlot, &QAction::triggered, this, &MainWindow::actionClearSlot_triggered);
    connect(ui->actionPreviousSlot, &QAction::triggered, this, &MainWindow::actionPreviousSlot_triggered);
    connect(ui->actionNextSlot, &QAction::triggered, this, &MainWindow::actionNextSlot_triggered);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::actionAbout_triggered);
    connect(ui->actionCopySlot, &QAction::triggered, this, &MainWindow::actionCopySlot_triggered);
    connect(ui->actionPasteSlot, &QAction::triggered, this, &MainWindow::actionPasteSlot_triggered);
    connect(ui->actionShowOptions, &QAction::triggered, this, &MainWindow::actionShowOptions_triggered);
    connect(ui->actionOpenAchievementFile, &QAction::triggered, this, &MainWindow::actionOpenAchievementFile_triggered);
    connect(ui->actionCreateNewMetadata, &QAction::triggered, this, &MainWindow::actionCreateNewMetadata_triggered);
    connect(ui->actionImportSlotFromFile, &QAction::triggered, this, &MainWindow::actionImportSlotFromFile_triggered);
    connect(ui->actionRegionUSA, &QAction::triggered, this, &MainWindow::actionRegionUSA_triggered);
    connect(ui->actionRegionPALGeneric, &QAction::triggered, this, &MainWindow::actionRegionPALGeneric_triggered);
    connect(ui->actionRegionPALFrench, &QAction::triggered, this, &MainWindow::actionRegionPALFrench_triggered);
    connect(ui->actionRegionPALGerman, &QAction::triggered, this, &MainWindow::actionRegionPALGerman_triggered);
    connect(ui->actionRegionPALSpanish, &QAction::triggered, this, &MainWindow::actionRegionPALSpanish_triggered);
    connect(ui->actionRegionJPN, &QAction::triggered, this, &MainWindow::actionRegionJPN_triggered);
    connect(ui->actionRegionJPNInternational, &QAction::triggered, this, &MainWindow::actionRegionJPNInternational_triggered);
    //Buttons
    connect(ui->btnCloud, &QPushButton::clicked, this, &MainWindow::btnCloud_clicked);
    connect(ui->btnBarret, &QPushButton::clicked, this, &MainWindow::btnBarret_clicked);
    connect(ui->btnTifa, &QPushButton::clicked, this, &MainWindow::btnTifa_clicked);
    connect(ui->btnAeris, &QPushButton::clicked, this, &MainWindow::btnAeris_clicked);
    connect(ui->btnRed, &QPushButton::clicked, this, &MainWindow::btnRed_clicked);
    connect(ui->btnYuffie, &QPushButton::clicked, this, &MainWindow::btnYuffie_clicked);
    connect(ui->btnCait, &QPushButton::clicked, this, &MainWindow::btnCait_clicked);
    connect(ui->btnVincent, &QPushButton::clicked, this, &MainWindow::btnVincent_clicked);
    connect(ui->btnCid, &QPushButton::clicked, this, &MainWindow::btnCid_clicked);
    connect(ui->btnSearchFlyers, &QPushButton::clicked, this, &MainWindow::btnSearchFlyers_clicked);
    connect(ui->btnSearchKeyItems, &QPushButton::clicked, this, &MainWindow::btnSearchKeyItems_clicked);
    connect(ui->btnReplay, &QPushButton::clicked, this, &MainWindow::btnReplay_clicked);
    connect(ui->btnRemoveAllMateria, &QPushButton::clicked, this, &MainWindow::btnRemoveAllMateria_clicked);
    connect(ui->btnRemoveAllStolen, &QPushButton::clicked, this, &MainWindow::btnRemoveAllStolen_clicked);
    connect(ui->btnAddAllItems, &QPushButton::clicked, this, &MainWindow::btnAddAllItems_clicked);
    connect(ui->btnRemoveAllItems, &QPushButton::clicked, this, &MainWindow::btnRemoveAllItems_clicked);
    connect(ui->btnAddAllMateria, &QPushButton::clicked, this, &MainWindow::btnAddAllMateria_clicked);
    connect(ui->btnMaxChar, &QPushButton::clicked, this, &MainWindow::btnMaxChar_clicked);
    //SpinBoxes
    connect(ui->sbGil, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &MainWindow::sbGil_valueChanged);
    connect(ui->sbGp, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbGp_valueChanged);
    connect(ui->sbBattles, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBattles_valueChanged);
    connect(ui->sbRuns, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbRuns_valueChanged);
    connect(ui->sbCurdisc, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbCurdisc_valueChanged);
    connect(ui->sbLoveYuffie, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbLoveYuffie_valueChanged);
    connect(ui->sbLoveTifa, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbLoveTifa_valueChanged);
    connect(ui->sbLoveAeris, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbLoveAeris_valueChanged);
    connect(ui->sbLoveBarret, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbLoveBarret_valueChanged);
    connect(ui->sbTimeSec, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTimeSec_valueChanged);
    connect(ui->sbTimeMin, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTimeMin_valueChanged);
    connect(ui->sbTimeHour, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTimeHour_valueChanged);
    connect(ui->sbTimerTimeSec, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTimerTimeSec_valueChanged);
    connect(ui->sbTimerTimeMin, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTimerTimeMin_valueChanged);
    connect(ui->sbTimerTimeHour, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTimerTimeHour_valueChanged);
    connect(ui->sbBloveYuffie, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBloveYuffie_valueChanged);
    connect(ui->sbBloveTifa, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBloveTifa_valueChanged);
    connect(ui->sbBloveAeris, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBloveAeris_valueChanged);
    connect(ui->sbBloveBarret, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBloveBarret_valueChanged);
    connect(ui->sbUweaponHp, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbUweaponHp_valueChanged);
    connect(ui->sbCoster1, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbCoster1_valueChanged);
    connect(ui->sbCoster2, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbCoster2_valueChanged);
    connect(ui->sbCoster3, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbCoster3_valueChanged);
    connect(ui->sbTurkschurch, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTurkschurch_valueChanged);
    connect(ui->sbMprogress, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbMprogress_valueChanged);
    connect(ui->sbDonprog, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbDonprog_valueChanged);
    connect(ui->sbSteps, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSteps_valueChanged);
    connect(ui->sbSnowBegScore, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowBegScore_valueChanged);
    connect(ui->sbSnowExpScore, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowExpScore_valueChanged);
    connect(ui->sbSnowCrazyScore, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowCrazyScore_valueChanged);
    connect(ui->sbSnowBegMin, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowBegMin_valueChanged);
    connect(ui->sbSnowBegSec, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowBegSec_valueChanged);
    connect(ui->sbSnowBegMsec, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowBegMsec_valueChanged);
    connect(ui->sbSnowExpMin, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowExpMin_valueChanged);
    connect(ui->sbSnowExpSec, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowExpSec_valueChanged);
    connect(ui->sbSnowExpMsec, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowExpMsec_valueChanged);
    connect(ui->sbSnowCrazyMin, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowCrazyMin_valueChanged);
    connect(ui->sbSnowCrazySec, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowCrazySec_valueChanged);
    connect(ui->sbSnowCrazyMsec, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSnowCrazyMsec_valueChanged);
    connect(ui->sbBikeHighScore, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBikeHighScore_valueChanged);
    connect(ui->sbBattlePoints, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBattlePoints_valueChanged);
    connect(ui->sbCondorFunds, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbCondorFunds_valueChanged);
    connect(ui->sbCondorWins, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbCondorWins_valueChanged);
    connect(ui->sbCondorLosses, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbCondorLosses_valueChanged);
    connect(ui->sbSaveMapId, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSaveMapId_valueChanged);
    connect(ui->sbSaveX, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSaveX_valueChanged);
    connect(ui->sbSaveY, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSaveY_valueChanged);
    connect(ui->sbSaveZ, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSaveZ_valueChanged);
    connect(ui->sbLeaderX, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbLeaderX_valueChanged);
    connect(ui->sbLeaderY, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbLeaderY_valueChanged);
    connect(ui->sbLeaderZ, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbLeaderZ_valueChanged);
    connect(ui->sbLeaderId, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbLeaderId_valueChanged);
    connect(ui->sbLeaderAngle, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbLeaderAngle_valueChanged);
    connect(ui->sbTcX, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTcX_valueChanged);
    connect(ui->sbTcY, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTcY_valueChanged);
    connect(ui->sbTcZ, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTcZ_valueChanged);
    connect(ui->sbTcId, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTcId_valueChanged);
    connect(ui->sbTcAngle, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbTcAngle_valueChanged);
    connect(ui->sbBhX, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBhX_valueChanged);
    connect(ui->sbBhY, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBhY_valueChanged);
    connect(ui->sbBhZ, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBhZ_valueChanged);
    connect(ui->sbBhId, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBhId_valueChanged);
    connect(ui->sbBhAngle, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbBhAngle_valueChanged);
    connect(ui->sbSubX, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSubX_valueChanged);
    connect(ui->sbSubY, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSubY_valueChanged);
    connect(ui->sbSubZ, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSubZ_valueChanged);
    connect(ui->sbSubId, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSubId_valueChanged);
    connect(ui->sbSubAngle, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbSubAngle_valueChanged);
    connect(ui->sbWcX, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbWcX_valueChanged);
    connect(ui->sbWcY, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbWcY_valueChanged);
    connect(ui->sbWcZ, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbWcZ_valueChanged);
    connect(ui->sbWcId, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbWcId_valueChanged);
    connect(ui->sbWcAngle, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbWcAngle_valueChanged);
    connect(ui->sbDurwX, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbDurwX_valueChanged);
    connect(ui->sbDurwY, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbDurwY_valueChanged);
    connect(ui->sbDurwZ, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbDurwZ_valueChanged);
    connect(ui->sbDurwId, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbDurwId_valueChanged);
    connect(ui->sbDurwAngle, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::sbDurwAngle_valueChanged);
    //CheckBoxes
    connect(ui->cbTutWorldSave, &QCheckBox::stateChanged, this, &MainWindow::cbTutWorldSave_stateChanged);
    connect(ui->cbBombingInt, &QCheckBox::stateChanged, this, &MainWindow::cbBombingInt_stateChanged);
    connect(ui->cbFlashbackPiano, &QCheckBox::toggled, this, &MainWindow::cbFlashbackPiano_toggled);
    connect(ui->cbSubGameWon, &QCheckBox::toggled, this, &MainWindow::cbSubGameWon_toggled);
    connect(ui->cbMysteryPanties, &QCheckBox::toggled, this, &MainWindow::cbMysteryPanties_toggled);
    connect(ui->cbLetterToDaughter, &QCheckBox::toggled, this, &MainWindow::cbLetterToDaughter_toggled);
    connect(ui->cbLetterToWife, &QCheckBox::toggled, this, &MainWindow::cbLetterToWife_toggled);
    connect(ui->cbPandorasBox, &QCheckBox::toggled, this, &MainWindow::cbPandorasBox_toggled);
    connect(ui->cbVisibleBuggy, &QCheckBox::toggled, this, &MainWindow::cbVisibleBuggy_toggled);
    connect(ui->cbVisibleBronco, &QCheckBox::toggled, this, &MainWindow::cbVisibleBronco_toggled);
    connect(ui->cbVisibleHighwind, &QCheckBox::toggled, this, &MainWindow::cbVisibleHighwind_toggled);
    connect(ui->cbVisibleWildChocobo, &QCheckBox::toggled, this, &MainWindow::cbVisibleWildChocobo_toggled);
    connect(ui->cbVisibleYellowChocobo, &QCheckBox::toggled, this, &MainWindow::cbVisibleYellowChocobo_toggled);
    connect(ui->cbVisibleGreenChocobo, &QCheckBox::toggled, this, &MainWindow::cbVisibleGreenChocobo_toggled);
    connect(ui->cbVisibleBlueChocobo, &QCheckBox::toggled, this, &MainWindow::cbVisibleBlueChocobo_toggled);
    connect(ui->cbVisibleBlackChocobo, &QCheckBox::toggled, this, &MainWindow::cbVisibleBlackChocobo_toggled);
    connect(ui->cbVisibleGoldChocobo, &QCheckBox::toggled, this, &MainWindow::cbVisibleGoldChocobo_toggled);
    connect(ui->cbRubyDead, &QCheckBox::toggled, this, &MainWindow::cbRubyDead_toggled);
    connect(ui->cbEmeraldDead, &QCheckBox::toggled, this, &MainWindow::cbEmeraldDead_toggled);
    connect(ui->cbBm1_1, &QCheckBox::toggled, this, &MainWindow::cbBm1_1_toggled);
    connect(ui->cbBm1_2, &QCheckBox::toggled, this, &MainWindow::cbBm1_2_toggled);
    connect(ui->cbBm1_3, &QCheckBox::toggled, this, &MainWindow::cbBm1_3_toggled);
    connect(ui->cbBm1_4, &QCheckBox::toggled, this, &MainWindow::cbBm1_4_toggled);
    connect(ui->cbBm1_5, &QCheckBox::toggled, this, &MainWindow::cbBm1_5_toggled);
    connect(ui->cbBm1_6, &QCheckBox::toggled, this, &MainWindow::cbBm1_6_toggled);
    connect(ui->cbBm1_7, &QCheckBox::toggled, this, &MainWindow::cbBm1_7_toggled);
    connect(ui->cbBm1_8, &QCheckBox::toggled, this, &MainWindow::cbBm1_8_toggled);
    connect(ui->cbBm2_1, &QCheckBox::toggled, this, &MainWindow::cbBm2_1_toggled);
    connect(ui->cbBm2_2, &QCheckBox::toggled, this, &MainWindow::cbBm2_2_toggled);
    connect(ui->cbBm2_3, &QCheckBox::toggled, this, &MainWindow::cbBm2_3_toggled);
    connect(ui->cbBm2_4, &QCheckBox::toggled, this, &MainWindow::cbBm2_4_toggled);
    connect(ui->cbBm2_5, &QCheckBox::toggled, this, &MainWindow::cbBm2_5_toggled);
    connect(ui->cbBm2_6, &QCheckBox::toggled, this, &MainWindow::cbBm2_6_toggled);
    connect(ui->cbBm2_7, &QCheckBox::toggled, this, &MainWindow::cbBm2_7_toggled);
    connect(ui->cbBm2_8, &QCheckBox::toggled, this, &MainWindow::cbBm2_8_toggled);
    connect(ui->cbBm3_1, &QCheckBox::toggled, this, &MainWindow::cbBm3_1_toggled);
    connect(ui->cbBm3_2, &QCheckBox::toggled, this, &MainWindow::cbBm3_2_toggled);
    connect(ui->cbBm3_3, &QCheckBox::toggled, this, &MainWindow::cbBm3_3_toggled);
    connect(ui->cbBm3_4, &QCheckBox::toggled, this, &MainWindow::cbBm3_4_toggled);
    connect(ui->cbBm3_5, &QCheckBox::toggled, this, &MainWindow::cbBm3_5_toggled);
    connect(ui->cbBm3_6, &QCheckBox::toggled, this, &MainWindow::cbBm3_6_toggled);
    connect(ui->cbBm3_7, &QCheckBox::toggled, this, &MainWindow::cbBm3_7_toggled);
    connect(ui->cbBm3_8, &QCheckBox::toggled, this, &MainWindow::cbBm3_8_toggled);
    connect(ui->cbS7pl_1, &QCheckBox::toggled, this, &MainWindow::cbS7pl_1_toggled);
    connect(ui->cbS7pl_2, &QCheckBox::toggled, this, &MainWindow::cbS7pl_2_toggled);
    connect(ui->cbS7pl_3, &QCheckBox::toggled, this, &MainWindow::cbS7pl_3_toggled);
    connect(ui->cbS7pl_4, &QCheckBox::toggled, this, &MainWindow::cbS7pl_4_toggled);
    connect(ui->cbS7pl_5, &QCheckBox::toggled, this, &MainWindow::cbS7pl_5_toggled);
    connect(ui->cbS7pl_6, &QCheckBox::toggled, this, &MainWindow::cbS7pl_6_toggled);
    connect(ui->cbS7pl_7, &QCheckBox::toggled, this, &MainWindow::cbS7pl_7_toggled);
    connect(ui->cbS7pl_8, &QCheckBox::toggled, this, &MainWindow::cbS7pl_8_toggled);
    connect(ui->cbS7ts_1, &QCheckBox::toggled, this, &MainWindow::cbS7ts_1_toggled);
    connect(ui->cbS7ts_2, &QCheckBox::toggled, this, &MainWindow::cbS7ts_2_toggled);
    connect(ui->cbS7ts_3, &QCheckBox::toggled, this, &MainWindow::cbS7ts_3_toggled);
    connect(ui->cbS7ts_4, &QCheckBox::toggled, this, &MainWindow::cbS7ts_4_toggled);
    connect(ui->cbS7ts_5, &QCheckBox::toggled, this, &MainWindow::cbS7ts_5_toggled);
    connect(ui->cbS7ts_6, &QCheckBox::toggled, this, &MainWindow::cbS7ts_6_toggled);
    connect(ui->cbS7ts_7, &QCheckBox::toggled, this, &MainWindow::cbS7ts_7_toggled);
    connect(ui->cbS7ts_8, &QCheckBox::toggled, this, &MainWindow::cbS7ts_8_toggled);
    connect(ui->cbTutSub, &QCheckBox::toggled, this, &MainWindow::cbTutSub_toggled);
    connect(ui->cbMidgartrain_8, &QCheckBox::toggled, this, &MainWindow::cbMidgartrain_8_toggled);
    connect(ui->cbMidgartrain_7, &QCheckBox::toggled, this, &MainWindow::cbMidgartrain_7_toggled);
    connect(ui->cbMidgartrain_6, &QCheckBox::toggled, this, &MainWindow::cbMidgartrain_6_toggled);
    connect(ui->cbMidgartrain_5, &QCheckBox::toggled, this, &MainWindow::cbMidgartrain_5_toggled);
    connect(ui->cbMidgartrain_4, &QCheckBox::toggled, this, &MainWindow::cbMidgartrain_4_toggled);
    connect(ui->cbMidgartrain_3, &QCheckBox::toggled, this, &MainWindow::cbMidgartrain_3_toggled);
    connect(ui->cbMidgartrain_2, &QCheckBox::toggled, this, &MainWindow::cbMidgartrain_2_toggled);
    connect(ui->cbMidgartrain_1, &QCheckBox::toggled, this, &MainWindow::cbMidgartrain_1_toggled);
    connect(ui->cbYuffieForest, &QCheckBox::toggled, this, &MainWindow::cbYuffieForest_toggled);
    connect(ui->cbRegYuffie, &QCheckBox::toggled, this, &MainWindow::cbRegYuffie_toggled);
    connect(ui->cbRegVinny, &QCheckBox::toggled, this, &MainWindow::cbRegVinny_toggled);
    //QComboBoxes
    connect(ui->comboHexEditor, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboHexEditor_currentIndexChanged);
    connect(ui->comboParty1, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboParty1_currentIndexChanged);
    connect(ui->comboParty2, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboParty2_currentIndexChanged);
    connect(ui->comboParty3, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboParty3_currentIndexChanged);
    connect(ui->comboRegionSlot, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboRegionSlot_currentIndexChanged);
    connect(ui->comboHighwindBuggy, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboHighwindBuggy_currentIndexChanged);
    connect(ui->comboMapControls, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboMapControls_currentIndexChanged);
    connect(ui->comboZVar, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboZVar_currentIndexChanged);
    connect(ui->comboCompareSlot, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboCompareSlot_currentIndexChanged);
    connect(ui->comboS7Slums, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboS7Slums_currentIndexChanged);
    connect(ui->comboReplay, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboReplay_currentIndexChanged);
    //Misc
    connect(ui->tblUnknown, &QTableWidget::itemChanged, this, &MainWindow::tblUnknown_itemChanged);
    connect(ui->slideWorldX, &QSlider::valueChanged, this, &MainWindow::slideWorldX_valueChanged);
    connect(ui->slideWorldY, &QSlider::valueChanged, this, &MainWindow::slideWorldY_valueChanged);
    connect(ui->worldMapView, &QWidget::customContextMenuRequested, this, &MainWindow::worldMapView_customContextMenuRequested);
    
    connect(ui->linePsxDesc, &QLineEdit::textChanged, this, &MainWindow::linePsxDesc_textChanged);
    connect(ui->tblMateria, &QTableWidget::currentCellChanged, this, &MainWindow::tblMateria_currentCellChanged);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabWidget_currentChanged);
    connect(ui->locationToolBox, &QToolBox::currentChanged, this, &MainWindow::locationToolBox_currentChanged);
    connect(ui->testDataTabWidget, &QTabWidget::currentChanged, this, &MainWindow::testDataTabWidget_currentChanged);
    //UI -> UI
    connect(ui->tblUnknown->verticalScrollBar(), &QScrollBar::valueChanged, ui->tblCompareUnknown->verticalScrollBar(), &QScrollBar::setValue);
    connect(ui->tblCompareUnknown->verticalScrollBar(), &QScrollBar::valueChanged, ui->tblUnknown->verticalScrollBar(), &QScrollBar::setValue);
    connect(ui->comboWorldPartyLeader, qOverload<int>(&QComboBox::currentIndexChanged), ui->sbLeaderId, &QSpinBox::setValue);
    connect(ui->sbLeaderId, qOverload<int>(&QSpinBox::valueChanged), ui->comboWorldPartyLeader, &QComboBox::setCurrentIndex);

    connect(saveIcon, &SaveIcon::nextIcon, ui->lblPsxIcon, &QLabel::setPixmap);

    connect(ff7, &FF7Save::fileChanged, this, &MainWindow::fileModified);

    connect(itemlist, &ItemList::itemsChanged, this, &MainWindow::Items_Changed);

    connect(materia_editor, &MateriaEditor::apChanged, this, &MainWindow::materia_ap_changed);
    connect(materia_editor, &MateriaEditor::idChanged, this, &MainWindow::materia_id_changed);

    connect(phsList, &PhsListWidget::allowedToggled, this, &MainWindow::phsList_box_allowed_toggled);
    connect(phsList, &PhsListWidget::visibleToggled, this, &MainWindow::phsList_box_visible_toggled);

    connect(menuList, &MenuListWidget::lockedToggled, this, &MainWindow::menuList_box_locked_toggled);
    connect(menuList, &MenuListWidget::visibleToggled, this, &MainWindow::menuList_box_visible_toggled);

    connect(char_editor, &CharEditor::id_changed, this, &MainWindow::char_id_changed);
    connect(char_editor, &CharEditor::level_changed, this, &MainWindow::char_level_changed);
    connect(char_editor, &CharEditor::str_changed, this, &MainWindow::char_str_changed);
    connect(char_editor, &CharEditor::vit_changed, this, &MainWindow::char_vit_changed);
    connect(char_editor, &CharEditor::mag_changed, this, &MainWindow::char_mag_changed);
    connect(char_editor, &CharEditor::spi_changed, this, &MainWindow::char_spi_changed);
    connect(char_editor, &CharEditor::dex_changed, this, &MainWindow::char_dex_changed);
    connect(char_editor, &CharEditor::lck_changed, this, &MainWindow::char_lck_changed);
    connect(char_editor, &CharEditor::strBonus_changed, this, &MainWindow::char_strBonus_changed);
    connect(char_editor, &CharEditor::vitBonus_changed, this, &MainWindow::char_vitBonus_changed);
    connect(char_editor, &CharEditor::magBonus_changed, this, &MainWindow::char_magBonus_changed);
    connect(char_editor, &CharEditor::spiBonus_changed, this, &MainWindow::char_spiBonus_changed);
    connect(char_editor, &CharEditor::dexBonus_changed, this, &MainWindow::char_dexBonus_changed);
    connect(char_editor, &CharEditor::lckBonus_changed, this, &MainWindow::char_lckBonus_changed);
    connect(char_editor, &CharEditor::limitLevel_changed, this, &MainWindow::char_limitLevel_changed);
    connect(char_editor, &CharEditor::limitBar_changed, this, &MainWindow::char_limitBar_changed);
    connect(char_editor, &CharEditor::name_changed, this, &MainWindow::char_name_changed);
    connect(char_editor, &CharEditor::weapon_changed, this, &MainWindow::char_weapon_changed);
    connect(char_editor, &CharEditor::armor_changed, this, &MainWindow::char_armor_changed);
    connect(char_editor, &CharEditor::accessory_changed, this, &MainWindow::char_accessory_changed);
    connect(char_editor, &CharEditor::curHp_changed, this, &MainWindow::char_curHp_changed);
    connect(char_editor, &CharEditor::maxHp_changed, this, &MainWindow::char_maxHp_changed);
    connect(char_editor, &CharEditor::curMp_changed, this, &MainWindow::char_curMp_changed);
    connect(char_editor, &CharEditor::maxMp_changed, this, &MainWindow::char_maxMp_changed);
    connect(char_editor, &CharEditor::kills_changed, this, &MainWindow::char_kills_changed);
    connect(char_editor, &CharEditor::row_changed, this, &MainWindow::char_row_changed);
    connect(char_editor, &CharEditor::levelProgress_changed, this, &MainWindow::char_levelProgress_changed);
    connect(char_editor, &CharEditor::sadnessfury_changed, this, &MainWindow::char_sadnessfury_changed);
    connect(char_editor, &CharEditor::limits_changed, this, &MainWindow::char_limits_changed);
    connect(char_editor, &CharEditor::timesused1_changed, this, &MainWindow::char_timesused1_changed);
    connect(char_editor, &CharEditor::timesused2_changed, this, &MainWindow::char_timeused2_changed);
    connect(char_editor, &CharEditor::timesused3_changed, this, &MainWindow::char_timeused3_changed);
    connect(char_editor, &CharEditor::baseHp_changed, this, &MainWindow::char_baseHp_changed);
    connect(char_editor, &CharEditor::baseMp_changed, this, &MainWindow::char_baseMp_changed);
    connect(char_editor, &CharEditor::exp_changed, this, &MainWindow::char_exp_changed);
    connect(char_editor, &CharEditor::mslotChanged, this, &MainWindow::char_mslot_changed);
    connect(char_editor, &CharEditor::Materias_changed, this, &MainWindow::char_materia_changed);
    connect(char_editor, &CharEditor::expNext_changed, this, &MainWindow::char_expNext_changed);

    connect(chocoboManager, &ChocoboManager::ownedChanged, this, &MainWindow::cm_stablesOwnedChanged);
    connect(chocoboManager, &ChocoboManager::stableMaskChanged, this, &MainWindow::cm_stableMaskChanged);
    connect(chocoboManager, &ChocoboManager::occupiedChanged, this, &MainWindow::cm_stablesOccupiedChanged);
    connect(chocoboManager, &ChocoboManager::nameChanged, this, &MainWindow::cm_nameChanged);
    connect(chocoboManager, &ChocoboManager::cantMateChanged, this, &MainWindow::cm_mated_toggled);
    connect(chocoboManager, &ChocoboManager::speedChanged, this, &MainWindow::cm_speedChanged);
    connect(chocoboManager, &ChocoboManager::mSpeedChanged, this, &MainWindow::cm_maxspeedChanged);
    connect(chocoboManager, &ChocoboManager::sprintChanged, this, &MainWindow::cm_sprintChanged);
    connect(chocoboManager, &ChocoboManager::mSprintChanged, this, &MainWindow::cm_maxsprintChanged);
    connect(chocoboManager, &ChocoboManager::staminaChanged, this, &MainWindow::cm_staminaChanged);
    connect(chocoboManager, &ChocoboManager::sexChanged, this, &MainWindow::cm_sexChanged);
    connect(chocoboManager, &ChocoboManager::typeChanged, this, &MainWindow::cm_typeChanged);
    connect(chocoboManager, &ChocoboManager::accelChanged, this, &MainWindow::cm_accelChanged);
    connect(chocoboManager, &ChocoboManager::coopChanged, this, &MainWindow::cm_coopChanged);
    connect(chocoboManager, &ChocoboManager::intelligenceChanged, this, &MainWindow::cm_intelChanged);
    connect(chocoboManager, &ChocoboManager::personalityChanged, this, &MainWindow::cm_personalityChanged);
    connect(chocoboManager, &ChocoboManager::pCountChanged, this, &MainWindow::cm_pcountChanged);
    connect(chocoboManager, &ChocoboManager::winsChanged, this, &MainWindow::cm_raceswonChanged);
    connect(chocoboManager, &ChocoboManager::penChanged, this, &MainWindow::cm_pensChanged);
    connect(chocoboManager, &ChocoboManager::ratingChanged, this, &MainWindow::cm_ratingChanged);

    connect(locationViewer, &LocationViewer::locationStringChanged, this, &MainWindow::location_textChanged);
    connect(locationViewer, &LocationViewer::locIdChanged, this, &MainWindow::loc_id_valueChanged);
    connect(locationViewer, &LocationViewer::mapIdChanged, this, &MainWindow::map_id_valueChanged);
    connect(locationViewer, &LocationViewer::xChanged, this, &MainWindow::coord_x_valueChanged);
    connect(locationViewer, &LocationViewer::yChanged, this, &MainWindow::coord_y_valueChanged);
    connect(locationViewer, &LocationViewer::tChanged, this, &MainWindow::coord_t_valueChanged);
    connect(locationViewer, &LocationViewer::dChanged, this, &MainWindow::coord_d_valueChanged);
    connect(locationViewer, &LocationViewer::locationChanged, this, &MainWindow::locationSelectionChanged);
    connect(locationViewer, &LocationViewer::fieldItemConnectRequest, this, &MainWindow::connectFieldItem);
    connect(locationViewer, &LocationViewer::fieldItemCheck, this, &MainWindow::checkFieldItem);
    connect(locationViewer, &LocationViewer::fieldItemChanged, this, &MainWindow::fieldItemStateChanged);

    connect(optionsWidget, &OptionsWidget::dialogColorLLChanged, this, &MainWindow::setDialogColorLL);
    connect(optionsWidget, &OptionsWidget::dialogColorLRChanged, this, &MainWindow::setDialogColorLR);
    connect(optionsWidget, &OptionsWidget::dialogColorULChanged, this, &MainWindow::setDialogColorUL);
    connect(optionsWidget, &OptionsWidget::dialogColorURChanged, this, &MainWindow::setDialogColorUR);
    connect(optionsWidget, &OptionsWidget::magicOrderChanged, this, &MainWindow::setMagicOrder);
    connect(optionsWidget, &OptionsWidget::cameraChanged, this, &MainWindow::setCameraMode);
    connect(optionsWidget, &OptionsWidget::atbChanged, this, &MainWindow::setAtbMode);
    connect(optionsWidget, &OptionsWidget::cursorChanged, this, &MainWindow::setCursorMode);
    connect(optionsWidget, &OptionsWidget::controllerModeChanged, this, &MainWindow::setControlMode);
    connect(optionsWidget, &OptionsWidget::soundChanged, this, &MainWindow::setSoundMode);
    connect(optionsWidget, &OptionsWidget::fieldMessageSpeedChanged, this, &MainWindow::setFieldMessageSpeed);
    connect(optionsWidget, &OptionsWidget::battleMessageSpeedChanged, this, &MainWindow::setBattleMessageSpeed);
    connect(optionsWidget, &OptionsWidget::battleSpeedChanged, this, &MainWindow::setBattleSpeed);
    connect(optionsWidget, &OptionsWidget::fieldHelpChanged, this, &MainWindow::setFieldHelp);
    connect(optionsWidget, &OptionsWidget::battleTargetsChanged, this, &MainWindow::setBattleTargets);
    connect(optionsWidget, &OptionsWidget::battleHelpChanged, this, &MainWindow::setBattleHelp);
    connect(optionsWidget, &OptionsWidget::inputChanged, this, &MainWindow::setButtonMapping);
}

void MainWindow::loadBasicSettings()
{
    if (BCSettings::instance()->value(SETTINGS::SCALE).isNull()) {
        double stdDPI = 96.0;
#ifdef Q_OS_MAC
        stdDPI = 72.0;
#endif
        double scale = QString::number(qApp->desktop()->logicalDpiX() / stdDPI, 'f', 2).toDouble();
        double sy = int(scale * 100) % 25;
        scale -= (sy / 100);
        scale = ( sy < 12.49) ? scale : scale + 0.25;
        BCSettings::instance()->setValue(SETTINGS::SCALE, std::max(scale, 0.5));
    }

    if (BCSettings::instance()->value(SETTINGS::MAINGEOMETRY).isNull())
        saveGeometry();
    else
        restoreGeometry(BCSettings::instance()->value(SETTINGS::MAINGEOMETRY).toByteArray());
}

void MainWindow::loadChildWidgetSettings()
{
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, !BCSettings::instance()->value(SETTINGS::USENATIVEDIALOGS, false).toBool());
    char_editor->setEditableComboBoxes(BCSettings::instance()->value(SETTINGS::EDITABLECOMBOS, true).toBool());
    materia_editor->setEditableMateriaCombo(BCSettings::instance()->value(SETTINGS::EDITABLECOMBOS, true).toBool());
    itemlist->setEditableItemCombo(BCSettings::instance()->value(SETTINGS::EDITABLECOMBOS, true).toBool());
    char_editor->setAdvancedMode(BCSettings::instance()->value(SETTINGS::CHARADVANCED, false).toBool());
    char_editor->setAutoLevel(BCSettings::instance()->value(SETTINGS::AUTOGROWTH, true).toBool());
    char_editor->setAutoStatCalc(BCSettings::instance()->value(SETTINGS::AUTOGROWTH, true).toBool());
    chocoboManager->setAdvancedMode(BCSettings::instance()->value(SETTINGS::CHOCOADVANCED, false).toBool());
    locationViewer->setAdvancedMode(BCSettings::instance()->value(SETTINGS::LOCVIEWADVANCED, false).toBool());
    ui->tabWidget->setTabEnabled(9, BCSettings::instance()->value(SETTINGS::ENABLETEST, false).toBool());
    if (FF7SaveInfo::instance()->isTypePC(ff7->format()) || ff7->format() == FF7SaveInfo::FORMAT::UNKNOWN)
        setControllerMappingVisible(BCSettings::instance()->value(SETTINGS::ALWAYSSHOWCONTROLLERMAP, false).toBool());
    ui->bm_unknown->setVisible(BCSettings::instance()->value(SETTINGS::PROGRESSADVANCED, false).toBool());
    ui->sbBhId->setVisible(BCSettings::instance()->value(SETTINGS::WORLDMAPADVANCED, false).toBool());
    ui->sbLeaderId->setVisible(BCSettings::instance()->value(SETTINGS::WORLDMAPADVANCED, false).toBool());
}
/*~~~~~~ END GUI SETUP ~~~~~~~*/
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        QPalette palette = BCSettings::instance()->paletteForSetting();
        QList<QWidget*> widgets = findChildren<QWidget *>(QString(), Qt::FindChildrenRecursively);
        for (QWidget * widget : widgets)
             widget->setPalette(palette);
        hexEditor->setAddressAreaColor(palette.alternateBase().color());
    } else if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        ui->tabWidget->setTabText(3, tr("Chocobo"));
        ui->tabWidget->setTabText(7, tr("Game Options"));
        populateCombos();
        materiaupdate();
        updateStolenMateria();
        if (ui->psxExtras->isVisible())
            update_hexEditor_PSXInfo();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    if (ff7->isFileModified()) {
        if (!saveChanges())
            return;
    }

    auto mimeData = e->mimeData();
    if (mimeData->hasUrls())
        loadFileFull(mimeData->urls().at(0).toLocalFile(), 0);
}

bool MainWindow::saveChanges(void)
{
    //return 0 to ingore the event/ return 1 to process event.
    int result;
    result = QMessageBox::question(this, tr("Unsaved Changes"), tr("Save Changes to the File:\n%1").arg(ff7->fileName()), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
    switch (result) {
    case QMessageBox::Yes:
        actionSave_triggered();
        return true;
    case QMessageBox::No:
        return true;
    default: return false;
    }
}
void MainWindow::closeEvent(QCloseEvent *e)
{
    if (ff7->isFileModified()) {
        if(!saveChanges())
            e->ignore();
        else
            e->accept();
    }
    BCSettings::instance()->setValue(SETTINGS::MAINGEOMETRY, saveGeometry());
}
void MainWindow::resizeEvent(QResizeEvent *)
{
    BCSettings::instance()->setValue(SETTINGS::MAINGEOMETRY, saveGeometry());
    fileModified(ff7->isFileModified());
}
void MainWindow::moveEvent(QMoveEvent *)
{
    BCSettings::instance()->setValue(SETTINGS::MAINGEOMETRY, saveGeometry());
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~LOAD/SAVE FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::actionOpenSaveFile_triggered()
{
    if (ff7->isFileModified()) {
        if (!saveChanges())
            return;//cancel load.
    }

    QString fileName = BCDialog::getOpenFileName(this, tr("Open Final Fantasy 7 Save"), BCSettings::instance()->value(SETTINGS::LOADPATH).toString(), FF7SaveInfo::instance()->knownTypesFilter());
    if (!fileName.isEmpty())
        loadFileFull(fileName, 0);
}

void MainWindow::actionReload_triggered()
{
    if (!ff7->fileName().isEmpty())
        loadFileFull(ff7->fileName(), 1);
}

/*~~~~~~~~~~~~~~~~~Load Full ~~~~~~~~~~~~~~~~~~*/
void MainWindow::loadFileFull(const QString &fileName, int reload)
{
    //if called from reload then int reload ==1 (don't call slot select)
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Black Chocobo"), tr("Cannot read file %1:\n%2.") .arg(fileName, file.errorString()));
        return;
    }

    prevFile = ff7->fileName();
    hexEditor->setCursorPosition(0);
    hexCursorPos = 0;

    if (!ff7->loadFile(fileName)) {
        QMessageBox::information(this, tr("Load Failed"), tr("Failed to Load File"));
        return;
    }

    _init = false;
    fileModified(false);

    if (ff7->numberOfSlots() == 1 ) {
        s = 0;
        guirefresh(0);
        return;
    }

    if (reload) {
        guirefresh(0);
        return;
    }

    SlotSelect slotselect(BCSettings::instance()->value(SETTINGS::SCALE).toDouble(), ff7, true, this);
    int i = slotselect.exec();
    if(i == -1) {
        actionOpenSaveFile_triggered();
        return;
    }

    s = i;
    guirefresh(0);
}

/*~~~~~~~~~~~~~~~~~IMPORT PSX~~~~~~~~~~~~~~~~~~*/
void MainWindow::actionImportSlotFromFile_triggered()
{
    QString fileName = BCDialog::getOpenFileName(this,
                       tr("Open Final Fantasy 7 Save"), BCSettings::instance()->value(SETTINGS::LOADPATH).toString(),
                       FF7SaveInfo::instance()->knownTypesFilter());
    if (!fileName.isEmpty()) {
        FF7Save *tempSave = new FF7Save();
        if (tempSave->loadFile(fileName)) {
            int fileSlot = 0;
            if (FF7SaveInfo::instance()->slotCount(tempSave->format()) > 1) {
                SlotSelect *SSelect = new SlotSelect(BCSettings::instance()->value(SETTINGS::SCALE).toDouble(), tempSave, false, this);
                fileSlot = SSelect->exec();
                if (fileSlot == -1) {
                    actionImportSlotFromFile_triggered();
                    return;
                }
                ui->statusBar->showMessage(QString(tr("Imported Slot:%2 from %1 -> Slot:%3")).arg(fileName, QString::number(fileSlot + 1), QString::number(s + 1)), 2000);
            } else {
                ui->statusBar->showMessage(QString(tr("Imported %1 -> Slot:%2")).arg(fileName, QString::number(s + 1)), 2000);
            }
            ff7->importSlot(s, fileName, fileSlot);
            guirefresh(0);
        } else {
            ui->statusBar->showMessage(QString(tr("Error Loading File %1")).arg(fileName), 2000);
        }
    }
    ff7->setFileModified(true, 0);
}
/*~~~~~~~~~~~~~~~~~IMPORT Char~~~~~~~~~~~~~~~~~*/
void MainWindow::actionImportChar_triggered()
{
    QString fileName = BCDialog::getOpenFileName(this, tr("Select FF7 Character Stat File"), BCSettings::instance()->value(SETTINGS::STATFOLDER).toString(), tr("FF7 Character Stat File(*.char)"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Black Chocobo"), tr("Cannot read file %1:\n%2.").arg(fileName, file.errorString()));
        return;
    }
    if (file.size() != 0x84) {
        QMessageBox::warning(this, tr("Black Chocobo"), tr("%1:\n%2 is Not a FF7 Character Stat File.").arg(fileName, file.errorString()));
        return;
    }
    QByteArray new_char;
    new_char = file.readAll();
    ff7->importCharacter(s, curchar, new_char);
    char_editor->setChar(ff7->character(s, curchar), ff7->charName(s, curchar));
    set_char_buttons();
}

void MainWindow::actionExportChar_triggered()
{
    QString fileName = BCDialog::getSaveFileName(this, ff7->region(s),
                       tr("Save FF7 Character File"), BCSettings::instance()->value(SETTINGS::STATFOLDER).toString(),
                       tr("FF7 Character Stat File(*.char)"));
    if (!fileName.isEmpty()) {
        if (ff7->exportCharacter(s, curchar, fileName))
            ui->statusBar->showMessage(tr("Character Export Successful"), 1000);
        else
            ui->statusBar->showMessage(tr("Character Export Failed"), 2000);
    }
}
bool MainWindow::actionSave_triggered()
{
    if (_init || ff7->fileName().isEmpty())
        return actionSaveFileAs_triggered();
    return saveFileFull(ff7->fileName());
}

bool MainWindow::actionSaveFileAs_triggered()
{
    QMap<QString, FF7SaveInfo::FORMAT> typeMap;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PC)] = FF7SaveInfo::FORMAT::PC;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::SWITCH)] = FF7SaveInfo::FORMAT::SWITCH;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::VMC)] = FF7SaveInfo::FORMAT::VMC;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::VGS)] = FF7SaveInfo::FORMAT::VGS;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::DEX)] = FF7SaveInfo::FORMAT::DEX;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PSP)] = FF7SaveInfo::FORMAT::PSP;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PSX)] = FF7SaveInfo::FORMAT::PSX;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PS3)] = FF7SaveInfo::FORMAT::PS3;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PGE)] = FF7SaveInfo::FORMAT::PGE;
    typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PDA)] = FF7SaveInfo::FORMAT::PDA;
    QString selectedType = typeMap.key(ff7->format(), QString());
    const QStringList typeKeys = typeMap.keys();

    QString path;
    if (ff7->format() == FF7SaveInfo::FORMAT::PC)
            path = BCSettings::instance()->value(SETTINGS::PCSAVEPATH).toString();
    else if ((ff7->format() == FF7SaveInfo::FORMAT::VMC)
             || (ff7->format() == FF7SaveInfo::FORMAT::VGS)
             || (ff7->format() == FF7SaveInfo::FORMAT::DEX))
        path = BCSettings::instance()->value(SETTINGS::EMUSAVEPATH).toString();

    QString fileName = BCDialog::getSaveFileName(this,
                                                 ff7->region(s),
                                                 tr("Select A File to Save As"),
                                                 path,
                                                 typeKeys.join(QStringLiteral(";;")),
                                                 &selectedType,
                                                 QFile(ff7->fileName()).fileName());

    if (fileName.isEmpty())
        return false;
    FF7SaveInfo::FORMAT newType = typeMap[selectedType];

    if (ff7->format() == newType)
        return saveFileFull(fileName);

    if (ff7->exportFile(fileName, newType, s)) {
        ui->statusBar->showMessage(tr("Export Successful"), 1000);
        fileModified(false);
        return true;
    }

    ui->statusBar->showMessage(tr("Export Failed"), 2000);
    return false;
}
/*~~~~~~~~~~~SHORT SAVE~~~~~~~~~~~~*/
bool MainWindow::saveFileFull(const QString &fileName)
{
    if (ff7->saveFile(fileName, s)) {
        //if no save was loaded and new game was clicked be sure to act like a game was loaded.
        if (_init)
            _init = false;

        fileModified(false);
        guirefresh(0);
        return true;
    }

    QMessageBox::information(this, tr("Save Error"), tr("Failed to save file\n%1").arg(fileName));
    return false;
}
/*~~~~~~~~~~~~~~~New_Game~~~~~~~~~~~*/

void MainWindow::actionNewGame_triggered()
{
    QString save_name = BCSettings::instance()->value(SETTINGS::CUSTOMDEFAULTSAVE).toBool() ?
                BCSettings::instance()->value(SETTINGS::DEFAULTSAVE).toString() : QString();

    QString region = ff7->region(s).isEmpty() ?
                BCSettings::instance()->value(SETTINGS::REGION).toString() : ff7->region(s);

    ff7->newGame(s, region, save_name);//call the new game function
    ui->statusBar->showMessage(tr("New Game Created - Using: %1")
                               .arg(save_name.isEmpty() ? tr("Builtin Data") : save_name), 2000);
    _init = false;
    guirefresh(1);
}
/*~~~~~~~~~~End New_Game~~~~~~~~~~~*/
/*~~~~~~~~~~New Game + ~~~~~~~~~~~~*/
void MainWindow::actionNewGamePlus_triggered()
{
    QString save_name;
    if (BCSettings::instance()->value(SETTINGS::CUSTOMDEFAULTSAVE).toBool())
        save_name = BCSettings::instance()->value(SETTINGS::DEFAULTSAVE).toString();

    ff7->newGamePlus(s, ff7->fileName(), save_name);
    ui->statusBar->showMessage(tr("New Game Plus Created - Using: %1")
                               .arg(save_name.isEmpty() ? tr("Builtin Data") : save_name), 2000);
    guirefresh(0);
}
/*~~~~~~~~~~End New_Game +~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~END LOAD/SAVE FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MENU ACTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~Simple Menu Stuff~~~~~~~~~~~~~~~~*/
void MainWindow::actionClearSlot_triggered()
{
    ff7->clearSlot(s);
    guirefresh(0);
}
void MainWindow::actionPreviousSlot_triggered()
{
    if (ff7->format() == FF7SaveInfo::FORMAT::UNKNOWN)
        return;

    if (s > 0) {
        s--;
        guirefresh(0);
    }
}

void MainWindow::actionNextSlot_triggered()
{
    if (ff7->format() == FF7SaveInfo::FORMAT::UNKNOWN)
        return;

    if (s < 14) {
        s++;
        guirefresh(0);
    }
}

void MainWindow::actionAbout_triggered()
{
    About adialog(this);
    adialog.exec();
}

void MainWindow::actionCopySlot_triggered()
{
    ff7->copySlot(s);
    ui->actionPasteSlot->setEnabled(true);
}

void MainWindow::actionPasteSlot_triggered()
{
    ff7->pasteSlot(s);
    guirefresh(0);
}
void MainWindow::actionShowOptions_triggered()
{
    Options odialog(this);
    connect(&odialog, &Options::requestLanguageChange, this, &MainWindow::changeLanguage);
    connect(&odialog, &Options::requestChangeNativeDialog, this, [] (bool useNative){
        QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, !useNative);
    });
    if (odialog.exec()) {
        setScale(BCSettings::instance()->value(SETTINGS::SCALE).toDouble());
        loadChildWidgetSettings();
    }
    disconnect(&odialog, nullptr, nullptr, nullptr);
}

void MainWindow::actionCreateNewMetadata_triggered()
{
    MetadataCreator mdata(this, ff7);
    mdata.exec();
}

void MainWindow::actionShowSelectionDialog_triggered()
{
    SlotSelect slotselect(BCSettings::instance()->value(SETTINGS::SCALE).toDouble(), ff7, false, this);
    s = slotselect.exec();
    guirefresh(0);
}

void MainWindow::actionOpenAchievementFile_triggered()
{
    QString temp = ff7->fileName();
    temp.chop(temp.length() - (temp.lastIndexOf("/")));
    temp.append(QStringLiteral("/achievement.dat"));
    QFile tmp(temp);
    if (!tmp.exists())
        temp = BCDialog::getOpenFileName(this, tr("Select Achievement File"), QDir::homePath(), tr("Dat File (*.dat)"));

    if (temp.isEmpty())
        return;

    achievementDialog achDialog(temp, this);
    achDialog.exec();
}

/*~~~~~~~~~~~~LANGUAGE & REGION ACTIONS~~~~~~~~~~~~~~*/

void MainWindow::changeLanguage(const QVariant &data)
{
    if(!m_translations.contains(data.toString()))
        detectTranslations();

    for(auto translation : m_translations.value(BCSettings::instance()->value(SETTINGS::LANG).toString()))
        QApplication::removeTranslator(translation);

    BCSettings::instance()->setValue(SETTINGS::LANG, data);
    for(auto translation : m_translations.value(data.toString()))
        QApplication::installTranslator(translation);
}

void MainWindow::setOpenFileText(const QString &openFile)
{
    int maxWidth = width() * .85;
    ui->lbl_fileName->setMaximumWidth(maxWidth);
    ui->lbl_fileName->setText(fontMetrics().elidedText(openFile, Qt::ElideMiddle, maxWidth));
}
/*~~~~~~~~~~~~~SET USA MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::actionRegionUSA_triggered(bool checked)
{
    if (!load) {
        if (!checked) {
            ff7->setRegion(s, QString());
            ui->lbl_sg_region->clear();
            itemlist->setMaximumItemQty(127);
        } else {
            if (ff7->isPAL(s))
                set_ntsc_time();   //Convert Time?

            ff7->setRegion(s, "NTSC-U");
            itemlist->setMaximumItemQty(127);
            ui->actionRegionPALGeneric->setChecked(false);
            ui->actionRegionPALFrench->setChecked(false);
            ui->actionRegionPALGerman->setChecked(false);
            ui->actionRegionPALSpanish->setChecked(false);
            ui->actionRegionJPN->setChecked(false);
            ui->actionRegionJPNInternational->setChecked(false);
            load = true;
            ui->lbl_sg_region->setText(ff7->region(s).mid(0, ff7->region(s).lastIndexOf("-") + 1));
            ui->comboRegionSlot->setCurrentIndex(ff7->region(s).midRef(ff7->region(s).lastIndexOf("S") + 1, 2).toInt() - 1);
            load = false;
        }
    }
    locationViewer->setRegion(ff7->region(s));
}
/*~~~~~~~~~~~~~SET PAL MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::actionRegionPALGeneric_triggered(bool checked)
{
    if (!load) {
        if (!checked) {
            ff7->setRegion(s, QString());
            ui->lbl_sg_region->clear();
            itemlist->setMaximumItemQty(127);
        } else {
            if (ff7->isNTSC(s))
                set_pal_time();   //Call RegionTime Converter

            ff7->setRegion(s, "PAL-E");
            itemlist->setMaximumItemQty(127);
            ui->actionRegionUSA->setChecked(false);
            ui->actionRegionPALGerman->setChecked(false);
            ui->actionRegionPALFrench->setChecked(false);
            ui->actionRegionPALSpanish->setChecked(false);
            ui->actionRegionJPN->setChecked(false);
            ui->actionRegionJPNInternational->setChecked(false);
            load = true;
            ui->lbl_sg_region->setText(ff7->region(s).mid(0, ff7->region(s).lastIndexOf("-") + 1));
            ui->comboRegionSlot->setCurrentIndex(ff7->region(s).midRef(ff7->region(s).lastIndexOf("S") + 1, 2).toInt() - 1);
            load = false;
        }
    } locationViewer->setRegion(ff7->region(s));
}
/*~~~~~~~~~~~~~SET PAL_German MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::actionRegionPALGerman_triggered(bool checked)
{
    if (!load) {
        if (!checked) {
            ff7->setRegion(s, QString());
            ui->lbl_sg_region->clear();
            itemlist->setMaximumItemQty(127);
        } else {
            if (ff7->isNTSC(s))
                set_pal_time();   //Call RegionTime Converter

            ff7->setRegion(s, "PAL-DE");
            itemlist->setMaximumItemQty(127);
            ui->actionRegionUSA->setChecked(false);
            ui->actionRegionPALGeneric->setChecked(false);
            ui->actionRegionPALFrench->setChecked(false);
            ui->actionRegionPALSpanish->setChecked(false);
            ui->actionRegionJPN->setChecked(false);
            ui->actionRegionJPNInternational->setChecked(false);
            load = true;
            ui->lbl_sg_region->setText(ff7->region(s).mid(0, ff7->region(s).lastIndexOf("-") + 1));
            ui->comboRegionSlot->setCurrentIndex(ff7->region(s).midRef(ff7->region(s).lastIndexOf("S") + 1, 2).toInt() - 1);
            load = false;
        }
    } locationViewer->setRegion(ff7->region(s));
}
/*~~~~~~~~~~~~~SET PAL_Spanish MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::actionRegionPALSpanish_triggered(bool checked)
{
    if (!load) {
        if (!checked) {
            ff7->setRegion(s, QString());
            ui->lbl_sg_region->clear();
            itemlist->setMaximumItemQty(127);
        } else {
            if (ff7->isNTSC(s))
                set_pal_time();   //Call RegionTime Converter

            ff7->setRegion(s, "PAL-ES");
            itemlist->setMaximumItemQty(127);
            ui->actionRegionUSA->setChecked(false);
            ui->actionRegionPALGeneric->setChecked(false);
            ui->actionRegionPALFrench->setChecked(false);
            ui->actionRegionPALGerman->setChecked(false);
            ui->actionRegionJPN->setChecked(false);
            ui->actionRegionJPNInternational->setChecked(false);
            load = true;
            ui->lbl_sg_region->setText(ff7->region(s).mid(0, ff7->region(s).lastIndexOf("-") + 1));
            ui->comboRegionSlot->setCurrentIndex(ff7->region(s).midRef(ff7->region(s).lastIndexOf("S") + 1, 2).toInt() - 1);
            load = false;
        }
    } locationViewer->setRegion(ff7->region(s));
}
/*~~~~~~~~~~~~~SET PAL_French MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::actionRegionPALFrench_triggered(bool checked)
{
    if (!load) {
        if (!checked) {
            ff7->setRegion(s, QString());
            ui->lbl_sg_region->clear();
            itemlist->setMaximumItemQty(127);
        } else {
            if (ff7->isNTSC(s))
                set_pal_time();   //Call RegionTime Converter

            ff7->setRegion(s, "PAL-FR");
            itemlist->setMaximumItemQty(127);
            ui->actionRegionUSA->setChecked(false);
            ui->actionRegionPALGeneric->setChecked(false);
            ui->actionRegionPALSpanish->setChecked(false);
            ui->actionRegionPALGerman->setChecked(false);
            ui->actionRegionJPN->setChecked(false);
            ui->actionRegionJPNInternational->setChecked(false);
            load = true;
            ui->lbl_sg_region->setText(ff7->region(s).mid(0, ff7->region(s).lastIndexOf("-") + 1));
            ui->comboRegionSlot->setCurrentIndex(ff7->region(s).midRef(ff7->region(s).lastIndexOf("S") + 1, 2).toInt() - 1);
            load = false;
        }
    } locationViewer->setRegion(ff7->region(s));
}
/*~~~~~~~~~~~~~SET JPN MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::actionRegionJPN_triggered(bool checked)
{
    if (!load) {
        if (!checked) {
            ff7->setRegion(s, QString());
            ui->lbl_sg_region->clear();
            itemlist->setMaximumItemQty(127);
        } else {
            //First Check If Coming From PAL
            if (ff7->isPAL(s))
                set_ntsc_time();   //Convert Time?
            ff7->setRegion(s, "NTSC-J");
            itemlist->setMaximumItemQty(99);
            ui->actionRegionUSA->setChecked(false);
            ui->actionRegionPALGeneric->setChecked(false);
            ui->actionRegionPALFrench->setChecked(false);
            ui->actionRegionPALGerman->setChecked(false);
            ui->actionRegionPALSpanish->setChecked(false);
            ui->actionRegionJPNInternational->setChecked(false);
            load = true;
            ui->lbl_sg_region->setText(ff7->region(s).mid(0, ff7->region(s).lastIndexOf("-") + 1));
            ui->comboRegionSlot->setCurrentIndex(ff7->region(s).midRef(ff7->region(s).lastIndexOf("S") + 1, 2).toInt() - 1);
            load = false;
        }
    } locationViewer->setRegion(ff7->region(s));
}
/*~~~~~~~~~~~~~SET JPN_International MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::actionRegionJPNInternational_triggered(bool checked)
{
    if (!load) {
        if (!checked) {
            ff7->setRegion(s, QString());
            ui->lbl_sg_region->clear();
            itemlist->setMaximumItemQty(127);
        } else {
            if (ff7->isPAL(s))
                set_ntsc_time();   //Convert Time?
            ff7->setRegion(s, "NTSC-JI");
            itemlist->setMaximumItemQty(127);
            ui->actionRegionUSA->setChecked(false);
            ui->actionRegionPALGeneric->setChecked(false);
            ui->actionRegionPALFrench->setChecked(false);
            ui->actionRegionPALGerman->setChecked(false);
            ui->actionRegionPALSpanish->setChecked(false);
            ui->actionRegionJPN->setChecked(false);
            load = true;
            ui->lbl_sg_region->setText(ff7->region(s).mid(0, ff7->region(s).lastIndexOf("-") + 1));
            ui->comboRegionSlot->setCurrentIndex(ff7->region(s).midRef(ff7->region(s).lastIndexOf("S") + 1, 2).toInt() - 1);
            load = false;
        }
    } locationViewer->setRegion(ff7->region(s));
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~END MENU ACTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~GUI FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~Set Menu Items~~~~~~~~~~*/
void MainWindow::setmenu(bool newgame)
{
    load = true;
    /*~~Disable All Items that are dependent on File Type~~*/
    ui->actionClearSlot->setEnabled(0);
    ui->actionRegionUSA->setChecked(false);
    ui->actionRegionPALGeneric->setChecked(false);
    ui->actionRegionPALGerman->setChecked(false);
    ui->actionRegionPALFrench->setChecked(false);
    ui->actionRegionPALSpanish->setChecked(false);
    ui->actionRegionJPN->setChecked(false);
    ui->actionRegionJPNInternational->setChecked(false);
    ui->actionNextSlot->setEnabled(0);
    ui->actionPreviousSlot->setEnabled(0);
    ui->actionShowSelectionDialog->setEnabled(0);
    ui->actionNewGame->setEnabled(0);
    ui->compare_table->setEnabled(0);
    ui->lbl_current_slot_txt->clear();
    ui->actionImportChar->setEnabled(1);
    ui->actionExportChar->setEnabled(1);
    /*~~End Clear Menu Items~~*/
    /*~~~~~~~Set Actions By Type~~~~~~~*/
    //For first file load.Don't Bother to disable these again.
    //new game should always be exported. no header...

    //if not FF7 user is stuck in the hex editor tab.
    if (!ff7->isFF7(s) && !ff7->region(s).isEmpty()) {
        if (ui->comboHexEditor->currentIndex() != 0)
            ui->comboHexEditor->setCurrentIndex(0);
        ui->tabWidget->setCurrentIndex(8);
        for (int i = 0; i < 8; i++)
            ui->tabWidget->setTabEnabled(i, false);
        ui->tabWidget->setTabEnabled(9, false);
    } else {
        for (int i = 0; i < 9; i++)
            ui->tabWidget->setTabEnabled(i, true);
        ui->tabWidget->setTabEnabled(9, BCSettings::instance()->value(SETTINGS::ENABLETEST).toBool());
    }

    if (!newgame && !ff7->fileName().isEmpty()) {
        ui->actionSaveFileAs->setEnabled(1);
        ui->actionReload->setEnabled(1);
    }

    ui->actionImportChar->setEnabled(1);
    ui->actionSave->setEnabled(1);

    //we haven't loaded a file yet.
    if (!_init) {
        ui->actionNewGamePlus->setEnabled(1);
        ui->actionImportSlotFromFile->setEnabled(1);
        ui->actionCopySlot->setEnabled(1);
        ui->actionPasteSlot->setEnabled(ff7->isBufferSlotPopulated());
        ui->actionNewGame->setEnabled(1);
    }

    if ( FF7SaveInfo::instance()->slotCount(ff7->format()) > 1) { //more then one slot, or unknown Type
        ui->actionNextSlot->setEnabled(s != 14);
        ui->actionPreviousSlot->setEnabled(s != 0);
        ui->actionShowSelectionDialog->setEnabled(1);
        ui->actionClearSlot->setEnabled(1);
        ui->actionNewGame->setEnabled(1);
        ui->compare_table->setEnabled(1);
        ui->lbl_current_slot_txt->setText(tr("Current Slot:%1").arg(QString::number(s + 1), 2, QChar('0')));
    }
    /*~~~End Set Actions By Type~~~*/
    /*~~Set Detected Region ~~*/
    if (ff7->region(s).contains("94163"))
        ui->actionRegionUSA->setChecked(true);
    else if (ff7->region(s).contains("00867"))
        ui->actionRegionPALGeneric->setChecked(true);
    else if (ff7->region(s).contains("00868"))
        ui->actionRegionPALFrench->setChecked(true);
    else if (ff7->region(s).contains("00869"))
        ui->actionRegionPALGerman->setChecked(true);
    else if (ff7->region(s).contains("00900"))
        ui->actionRegionPALSpanish->setChecked(true);
    else if (ff7->region(s).contains("00700"))
        ui->actionRegionJPN->setChecked(true);
    else if (ff7->region(s).contains("01057"))
        ui->actionRegionJPNInternational->setChecked(true);
    else if (!ff7->region(s).isEmpty()) {
        //not FF7 unset some menu options.
        ui->actionNewGamePlus->setEnabled(0);
        ui->actionCopySlot->setEnabled(0);
        ui->actionExportChar->setEnabled(0);
        ui->actionImportChar->setEnabled(0);
    }
    load = false;
}
void MainWindow::fileModified(bool changed)
{
    if (changed)
        setOpenFileText(ff7->fileName().append("*"));
    else
        setOpenFileText(ff7->fileName());
}
/*~~~~~~~~~End Set Menu~~~~~~~~~~~*/
void MainWindow::set_ntsc_time(void)
{
    if (BCDialog::fixTimeDialog(this, ff7->isPAL(s)) == QMessageBox::Yes) {
        ff7->setTime(s, quint32(ff7->time(s) * 1.2));
        load = true;
        ui->sbTimeHour->setValue(ff7->time(s) / 3600);
        ui->sbTimeMin->setValue(ff7->time(s) / 60 % 60);
        ui->sbTimeSec->setValue(int(ff7->time(s)) - ((ui->sbTimeHour->value() * 3600) + ui->sbTimeMin->value() * 60));
        load = false;
    }
}
void MainWindow::set_pal_time(void)
{
    if (BCDialog::fixTimeDialog(this, ff7->isPAL(s)) == QMessageBox::Yes) {
        ff7->setTime(s, quint32(ff7->time(s) / 1.2));
        load = true;
        ui->sbTimeHour->setValue(ff7->time(s) / 3600);
        ui->sbTimeMin->setValue(ff7->time(s) / 60 % 60);
        ui->sbTimeSec->setValue(int(ff7->time(s)) - ((ui->sbTimeHour->value() * 3600) + ui->sbTimeMin->value() * 60));
        load = false;
    }
}
void MainWindow::materiaupdate(void)
{
    load = true;
    int j = std::max(0, ui->tblMateria->currentRow());
    ui->tblMateria->reset();
    ui->tblMateria->clearContents();
    ui->tblMateria->setColumnWidth(0, int(ui->tblMateria->width()*.65));
    ui->tblMateria->setColumnWidth(1, int(ui->tblMateria->width()*.25));
    ui->tblMateria->setRowCount(200);

    for (int mat = 0; mat < 200; mat++) { // partys materias
        QTableWidgetItem *newItem;
        ui->tblMateria->setRowHeight(mat, fontMetrics().height() + 9);
        qint32 current_ap = ff7->partyMateriaAp(s, mat);
        quint8 current_id = ff7->partyMateriaId(s, mat);
        QString ap;

        if (current_id <= 0x5A) {
            newItem = new QTableWidgetItem(QIcon(QPixmap::fromImage(Materias.image(current_id).scaledToHeight(fontMetrics().height(), Qt::SmoothTransformation))), Materias.name(current_id), 0);
            ui->tblMateria->setItem(mat, 0, newItem);
        }

        if (current_id == FF7Materia::EnemySkill) {
            if (current_ap == FF7Materia::MaxMateriaAp) {
                newItem = new QTableWidgetItem(tr("Master"));
                ui->tblMateria->setItem(mat, 1, newItem);
            } else {
                newItem = new QTableWidgetItem(QString(), 0);
                ui->tblMateria->setItem(mat, 1, newItem);
            }
        }

        else if (current_id == FF7Materia::MasterCommand || current_id == FF7Materia::MasterMagic || current_id == FF7Materia::MasterSummon || current_id == FF7Materia::Underwater) {
            newItem = new QTableWidgetItem(QString(), 0);
            ui->tblMateria->setItem(mat, 1, newItem);
        } else if (current_id != FF7Materia::EmptyId) {
            if (current_ap == FF7Materia::MaxMateriaAp) {
                newItem = new QTableWidgetItem(tr("Master"));
                ui->tblMateria->setItem(mat, 1, newItem);
            } else {
                newItem = new QTableWidgetItem(ap.setNum(current_ap));
                ui->tblMateria->setItem(mat, 1, newItem);
            }
        } else {
            //We need to clear invalid to prevent data issues. to keep file changes correct we back up our change vars and replace later.
            bool fileTemp = ff7->isFileModified();
            ff7->setPartyMateria(s, mat, FF7Materia::EmptyId, FF7Materia::MaxMateriaAp); //invalid insure its clear.
            newItem = new QTableWidgetItem(tr("===Empty Slot==="), 0);
            ui->tblMateria->setItem(mat, 0, newItem);
            newItem = new QTableWidgetItem(QString(), 0);
            ui->tblMateria->setItem(mat, 1, newItem);
            ff7->setFileModified(fileTemp, s);
        }
    }
    if (ff7->partyMateriaId(s, j) == FF7Materia::EnemySkill)
        mat_spacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    else
        mat_spacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    materia_editor->setMateria(ff7->partyMateriaId(s, j), ff7->partyMateriaAp(s, j));
    ui->tblMateria->setCurrentCell(j, 1); //so that right side is set correctly.
    load = false;
}
void MainWindow::materia_ap_changed(qint32 ap)
{
    if (!load) {
        ff7->setPartyMateria(s, ui->tblMateria->currentRow(), ff7->partyMateriaId(s, ui->tblMateria->currentRow()), ap);
        materiaupdate();
    }
}
void MainWindow::materia_id_changed(qint8 id)
{
    if (!load) {
        ff7->setPartyMateria(s, ui->tblMateria->currentRow(), quint8(id), ff7->partyMateriaAp(s, ui->tblMateria->currentRow()));
        materiaupdate();
    }
}
void MainWindow::CheckGame()
{
    if ((!ff7->isFF7(s) && !ff7->region(s).isEmpty())
            || ((!ff7->isFF7(s)) && !FF7SaveInfo::instance()->isTypePC(ff7->format()) && (ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_EMPTY)))) {
        // NOT FF7
        errbox error(this, ff7, s);
        if (FF7SaveInfo::instance()->slotCount(ff7->format()) == 1)
            error.setSingleSlot(true);
        switch (error.exec()) {
        case 0://View Anyway..
            setmenu(0);
            hexTabUpdate(0);
            break;

        case 1://Previous Clicked
            s--;
            CheckGame();
            break;

        case 2://Next Clicked
            s++;
            CheckGame();
            break;

        case 3://exported Clicked
            QMap<QString, FF7SaveInfo::FORMAT> typeMap;
            typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PSX)] = FF7SaveInfo::FORMAT::PSX;
            typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PS3)] = FF7SaveInfo::FORMAT::PS3;
            typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PGE)] = FF7SaveInfo::FORMAT::PGE;
            typeMap[FF7SaveInfo::instance()->typeFilter(FF7SaveInfo::FORMAT::PDA)] = FF7SaveInfo::FORMAT::PDA;
            QString selectedType = typeMap.key(FF7SaveInfo::FORMAT::PSX);
            const QStringList typeKeys = typeMap.keys();

            QString fileName = BCDialog::getSaveFileName(this,
                                                         ff7->region(s),
                                                         tr("Select A File to Save As"),
                                                         QStringLiteral("%1/%2").arg(QDir::homePath(),ff7->region(s)),
                                                         typeKeys.join(QStringLiteral(";;")),
                                                         &selectedType);

            if (fileName.isEmpty())
                return;
            FF7SaveInfo::FORMAT newType = typeMap[selectedType];

            if (ff7->exportFile(fileName, newType, s))
                ui->statusBar->showMessage(QString(tr("Exported Slot:%2 from %1 -> %3")).arg(ff7->fileName(), QString::number(s + 1), fileName), 2000);
            else 
                ui->statusBar->showMessage(tr("Export Failed"));
            break;
        }
    } else {
        guirefresh(0);
    }
}
void MainWindow::othersUpdate()
{
    load = true;

    ui->cbRubyDead->setChecked(ff7->killedRubyWeapon(s));
    ui->cbEmeraldDead->setChecked(ff7->killedEmeraldWeapon(s));

    ui->cbPandorasBox->setChecked(ff7->seenPandorasBox(s));
    ui->cbSubGameWon->setChecked(ff7->subMiniGameVictory(s));
    ui->sbCondorWins->setValue(ff7->condorWins(s));
    ui->sbCondorLosses->setValue(ff7->condorLosses(s));
    ui->sbCondorFunds->setValue(ff7->condorFunds(s));

    ui->sbCoster1->setValue(ff7->speedScore(s, 1));
    ui->sbCoster2->setValue(ff7->speedScore(s, 2));
    ui->sbCoster3->setValue(ff7->speedScore(s, 3));

    for (int i = 0; i < 9; i++) //Allowed in Phs
            phsList->setChecked(i, PhsListWidget::PHSALLOWED, !ff7->phsAllowed(s, i));

    for (int i = 0; i < 9; i++) //Visible
            phsList->setChecked(i, PhsListWidget::PHSVISIBLE, ff7->phsVisible(s, i));

    for (int i = 0; i < 10; i++) //visible_menu
            menuList->setChecked(i, MenuListWidget::MENUVISIBLE, ff7->menuVisible(s, i));

    for (int i = 0; i < 10; i++) //menu_locked
            menuList->setChecked(i, MenuListWidget::MENULOCKED, ff7->menuLocked(s, i));

    ui->sbSteps->setValue(ff7->steps(s));

    ui->sbLoveBarret->setValue(ff7->love(s, false, FF7Save::LOVE_BARRET));
    ui->sbLoveTifa->setValue(ff7->love(s, false, FF7Save::LOVE_TIFA));
    ui->sbLoveAeris->setValue(ff7->love(s, false, FF7Save::LOVE_AERIS));
    ui->sbLoveYuffie->setValue(ff7->love(s, false, FF7Save::LOVE_YUFFIE));

    ui->sbTimeHour->setValue(ff7->time(s) / 3600);
    ui->sbTimeMin->setValue(ff7->time(s) / 60 % 60);
    ui->sbTimeSec->setValue(int(ff7->time(s)) - ((ui->sbTimeHour->value() * 3600) + ui->sbTimeMin->value() * 60));

    ui->sbTimerTimeHour->setValue(ff7->countdownTimer(s) / 3600);
    ui->sbTimerTimeMin->setValue(ff7->countdownTimer(s) / 60 % 60);
    ui->sbTimerTimeSec->setValue(int(ff7->countdownTimer(s)) - ((ui->sbTimerTimeHour->value() * 3600) + ui->sbTimerTimeMin->value() * 60));
    ui->cbYuffieForest->setChecked(ff7->canFightNinjaInForest(s));
    ui->cbRegYuffie->setChecked(ff7->yuffieUnlocked(s));
    ui->cbRegVinny->setChecked(ff7->vincentUnlocked(s));

    /*~~~~~Stolen Materia~~~~~~~*/
    ui->tblMateriaStolen->reset();
    ui->tblMateriaStolen->clearContents();
    ui->tblMateriaStolen->setColumnWidth(0, int(ui->tblMateriaStolen->width()*.65));
    ui->tblMateriaStolen->setColumnWidth(1, int(ui->tblMateriaStolen->width()*.25));
    ui->tblMateriaStolen->setRowCount(48);
    updateStolenMateria();

    //SnowBoard Mini Game Data.
    ui->sbSnowBegMin->setValue(ff7->snowboardTime(s, 0).midRef(0, 2).toInt());
    ui->sbSnowBegSec->setValue(ff7->snowboardTime(s, 0).midRef(2, 2).toInt());
    ui->sbSnowBegMsec->setValue(ff7->snowboardTime(s, 0).midRef(4, 3).toInt());
    ui->sbSnowBegScore->setValue(ff7->snowboardScore(s, 0));

    ui->sbSnowExpMin->setValue(ff7->snowboardTime(s, 1).midRef(0, 2).toInt());
    ui->sbSnowExpSec->setValue(ff7->snowboardTime(s, 1).midRef(2, 2).toInt());
    ui->sbSnowExpMsec->setValue(ff7->snowboardTime(s, 1).midRef(4, 3).toInt());
    ui->sbSnowExpScore->setValue(ff7->snowboardScore(s, 1));

    ui->sbSnowCrazyMin->setValue(ff7->snowboardTime(s, 2).midRef(0, 2).toInt());
    ui->sbSnowCrazySec->setValue(ff7->snowboardTime(s, 2).midRef(2, 2).toInt());
    ui->sbSnowCrazyMsec->setValue(ff7->snowboardTime(s, 2).midRef(4, 3).toInt());
    ui->sbSnowCrazyScore->setValue(ff7->snowboardScore(s, 2));

    ui->sbBikeHighScore->setValue(ff7->bikeHighScore(s));
    ui->sbBattlePoints->setValue(ff7->battlePoints(s));
    ui->cbFlashbackPiano->setChecked(ff7->playedPianoOnFlashback(s));
    load = false;
}
void MainWindow::updateStolenMateria()
{
    for (int mat = 0; mat < 48; mat++) { //materias stolen by yuffie
        QString ap;
        quint8 current_id = ff7->stolenMateriaId(s, mat);
        QTableWidgetItem *newItem;
        if (current_id != FF7Materia::EmptyId) {
            newItem = new QTableWidgetItem(QPixmap::fromImage(Materias.image(current_id)), Materias.name(current_id), 0);
            ui->tblMateriaStolen->setItem(mat, 0, newItem);
            qint32 current_ap = ff7->stolenMateriaAp(s, mat);
            if (current_ap == FF7Materia::MaxMateriaAp) {
                newItem = new QTableWidgetItem(tr("Master"));
                ui->tblMateriaStolen->setItem(mat, 1, newItem);
            } else {
                newItem = new QTableWidgetItem(ap.setNum(current_ap));
                ui->tblMateriaStolen->setItem(mat, 1, newItem);
            }
        } else {
            newItem = new QTableWidgetItem(tr("===Empty Slot==="), 0);
            ui->tblMateriaStolen->setItem(mat, 0, newItem);
        }
        ui->tblMateriaStolen->setRowHeight(mat, fontMetrics().height() + 9);
    }
}
void MainWindow::update_hexEditor_PSXInfo(void)
{
    load = true;

    saveIcon->setAll(ff7->slotIcon(s));
    ui->lblRegionString->setText(ff7->region(s));
    QString SlotSizeText;

    if (ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_MIDLINK)
      && ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_ENDLINK)
      && ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_DELETED_MIDLINK)
      && ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_DELETED_ENDLINK))
        ui->linePsxDesc->setText(ff7->psxDesc(s));

    if (ff7->psx_block_type(s) == char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_MIDLINK)
      || ff7->psx_block_type(s) == char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_DELETED_MIDLINK))
        SlotSizeText.append(tr("\n Mid-Linked Block "));

    else if (ff7->psx_block_type(s) == char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_ENDLINK)
      || ff7->psx_block_type(s) == char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_DELETED_ENDLINK))
        SlotSizeText.append(tr("\n End Of Linked Data"));

    if (ff7->psx_block_type(s) == char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_DELETED)
      || ff7->psx_block_type(s) == char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_DELETED_MIDLINK)
      || ff7->psx_block_type(s) == char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_DELETED_ENDLINK))
        SlotSizeText.append(tr("(Deleted)"));

    if (ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_MIDLINK)
      && ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_ENDLINK)
      && ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_DELETED_MIDLINK)
      && ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_DELETED_ENDLINK))
        SlotSizeText.append(tr("Game Uses %n Save Block(s)", nullptr, ff7->psx_block_size(s)));

    if (ff7->psx_block_size(s) != 1) {
        if (ff7->format() != FF7SaveInfo::FORMAT::PSX
                && ff7->format() != FF7SaveInfo::FORMAT::PS3
                && ff7->psx_block_next(s) != 0xFF) {
            SlotSizeText.append(tr("\n  Next Data Chunk @ Slot:%1").arg(QString::number(ff7->psx_block_next(s) + 1)));
        }
    }
    ui->lblSlotSize->setText(SlotSizeText);
    load = false;
}
void MainWindow::tabWidget_currentChanged(int index)
{
    //Update the shown tab.
    load = true;
    hexCursorPos = hexEditor->cursorPosition();
    switch (index) {
    case 0://Party Tab
        if (ff7->party(s, 0) >= 0x0C)
            ui->comboParty1->setCurrentIndex(12);
        else
            ui->comboParty1->setCurrentIndex(ff7->party(s, 0));

        if (ff7->party(s, 1) >= 0x0C)
            ui->comboParty2->setCurrentIndex(12);
        else
            ui->comboParty2->setCurrentIndex(ff7->party(s, 1));

        if (ff7->party(s, 2) >= 0x0C)
            ui->comboParty3->setCurrentIndex(12);
        else
            ui->comboParty3->setCurrentIndex(ff7->party(s, 2));

        set_char_buttons();
        switch (curchar) {
        case 0: btnCloud_clicked(); break;
        case 1: btnBarret_clicked(); break;
        case 2: btnTifa_clicked(); break;
        case 3: btnAeris_clicked(); break;
        case 4: btnRed_clicked(); break;
        case 5: btnYuffie_clicked(); break;
        case 6: btnCait_clicked(); break;
        case 7: btnVincent_clicked(); break;
        case 8: btnCid_clicked(); break;
        }
        break;

    case 1://Item Tab
        itemlist->setItems(ff7->items(s));
        ui->sbGil->setValue(ff7->gil(s));
        ui->sbGp->setValue(ff7->gp(s));
        ui->sbRuns->setValue(ff7->runs(s));
        ui->sbBattles->setValue(ff7->battles(s));
        ui->cbMysteryPanties->setChecked(ff7->keyItem(s, FF7Save::MYSTERYPANTIES));
        ui->cbLetterToDaughter->setChecked(ff7->keyItem(s, FF7Save::LETTERTOADAUGHTER));
        ui->cbLetterToWife->setChecked(ff7->keyItem(s, FF7Save::LETTERTOAWIFE));
        break;

    case 2://Materia
        materiaupdate();
        break;

    case 3://Chocobo Tab
        chocoboManager->setData(ff7->chocobos(s), ff7->chocobosNames(s), ff7->chocobosStaminas(s), ff7->chocoboCantMates(s), ff7->stablesOwned(s), ff7->stablesOccupied(s), ff7->stableMask(s), ff7->chocoboPens(s), ff7->chocoboRatings(s));
        break;

    case 4://Location Tab
        locationToolBox_currentChanged(ui->locationToolBox->currentIndex());
        break;

    case 5://Game Progress Tab
        progress_update();
        break;

    case 6:// Other Tab
        othersUpdate();
        break;

    case 7:// Game Options Tab
        optionsWidget->setDialogColors(ff7->dialogColorUL(s), ff7->dialogColorUR(s), ff7->dialogColorLL(s), ff7->dialogColorLR(s));
        optionsWidget->setFieldHelp(ff7->fieldHelp(s));
        optionsWidget->setBattleHelp(ff7->battleHelp(s));
        optionsWidget->setBattleTargets(ff7->battleTargets(s));
        optionsWidget->setSoundMode(ff7->soundMode(s));
        optionsWidget->setControllerMode(ff7->controlMode(s));
        optionsWidget->setCursor(ff7->cursorMode(s));
        optionsWidget->setCamera(ff7->cameraMode(s));
        optionsWidget->setAtbMode(ff7->atbMode(s));
        optionsWidget->setMagicOrder(ff7->magicOrder(s));
        optionsWidget->setBattleSpeed(ff7->battleSpeed(s));
        optionsWidget->setBattleMessageSpeed(ff7->battleMessageSpeed(s));
        optionsWidget->setFieldMessageSpeed(ff7->messageSpeed(s));
        for (int i = 0; i < 16; i++) {
            optionsWidget->setInput(i, ff7->controllerMapping(s, i));
        }
        if ((!FF7SaveInfo::instance()->isTypePC(ff7->format()) && ff7->format() != FF7SaveInfo::FORMAT::UNKNOWN)
                || BCSettings::instance()->value(SETTINGS::ALWAYSSHOWCONTROLLERMAP).toBool()) {
            setControllerMappingVisible(true);
            if (optionsWidget->verticalScrollBar()->isVisible()) {
                optionsWidget->setFixedWidth(optionsWidget->width() - 1);
            }
        } else {
            setControllerMappingVisible(false);
        }
        break;

    case 8://HexEditor Tab
        hexTabUpdate(ui->comboHexEditor->currentIndex());
        break;

    case 9: //Test Data Tab
        testDataTabWidget_currentChanged(ui->testDataTabWidget->currentIndex());
        break;
    }
    load = false;
}
void MainWindow::hexTabUpdate(int viewMode)
{
    ui->psxExtras->setVisible(false);
    ui->boxHexData->setVisible(false);
    disconnect(hexEditor, &QHexEdit::dataChanged, this, &MainWindow::hexEditorChanged);
    if (FF7SaveInfo::instance()->isTypePC(ff7->format()) || ff7->format() == FF7SaveInfo::FORMAT::UNKNOWN) {
        hexEditor->setData(ff7->slotFF7Data(s));
    } else {
        ui->psxExtras->setVisible(true);
        update_hexEditor_PSXInfo();
        if (ff7->isFF7(s)) {
            ui->boxHexData->setVisible(true);
            switch (viewMode) {
            case 0:
                hexEditor->setData(ff7->slotPsxRawData(s));
                break;
            case 1:
                hexEditor->setData(ff7->slotFF7Data(s));
                break;
            }
        } else {
            hexEditor->setData(ff7->slotPsxRawData(s));
        }
    }
    hexEditor->setCursorPosition(hexCursorPos);
    connect(hexEditor, &QHexEdit::dataChanged, this, &MainWindow::hexEditorChanged);
}

void MainWindow::setControllerMappingVisible(bool Visible)
{
    optionsWidget->setControllerMappingVisible(Visible);
}
/*~~~~~~~~~~~~~~~~~~~~~GUIREFRESH~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::guirefresh(bool newgame)
{
    load = true;
    /*~~~~Check for SG type and ff7~~~~*/
    if ((!ff7->isFF7(s) && !ff7->region(s).isEmpty()) ||
            ((!ff7->isFF7(s)) && !FF7SaveInfo::instance()->isTypePC(ff7->format())
             && (ff7->psx_block_type(s) != char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_EMPTY)) && (ff7->psx_block_type(s) != '\x00'))) {
        CheckGame();//Not FF7! Handled By CheckGame()
    } else {
        //IS FF7 Slot
        if (FF7SaveInfo::instance()->isTypePC(ff7->format()) || ff7->format() == FF7SaveInfo::FORMAT::UNKNOWN) {
            if (ui->comboHexEditor->currentIndex() != 1)
                ui->comboHexEditor->setCurrentIndex(1);
        }
        //QByteArray text;
        if (ff7->region(s).isEmpty()
                && (ff7->format() == FF7SaveInfo::FORMAT::VMC || ff7->format() == FF7SaveInfo::FORMAT::VGS || ff7->format() == FF7SaveInfo::FORMAT::DEX || ff7->format() == FF7SaveInfo::FORMAT::PSP)
                && ff7->psx_block_type(s) == char(FF7SaveInfo::PSXBLOCKTYPE::BLOCK_EMPTY)) {
            //if empty region string and a virtual memcard format and dir frame says empty.
            ff7->clearSlot(s); //fileModified(false);//checking only
        }
        locationViewer->setRegion(ff7->region(s));
        setmenu(newgame);
        tabWidget_currentChanged(ui->tabWidget->currentIndex());
        load = false;
    }
}/*~~~~~~~~~~~~~~~~~~~~End GUIREFRESH ~~~~~~~~~~~~~~~~~*/
void MainWindow::set_char_buttons()
{
    ui->btnCloud->setIcon(QIcon(FF7Char::instance()->pixmap(ff7->charID(s, 0)).scaled(ui->btnCloud->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    ui->btnBarret->setIcon(QIcon(FF7Char::instance()->pixmap(ff7->charID(s, 1)).scaled(ui->btnCloud->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    ui->btnTifa->setIcon(QIcon(FF7Char::instance()->pixmap(ff7->charID(s, 2)).scaled(ui->btnCloud->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    ui->btnAeris->setIcon(QIcon(FF7Char::instance()->pixmap(ff7->charID(s, 3)).scaled(ui->btnCloud->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    ui->btnRed->setIcon(QIcon(FF7Char::instance()->pixmap(ff7->charID(s, 4)).scaled(ui->btnCloud->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    ui->btnYuffie->setIcon(QIcon(FF7Char::instance()->pixmap(ff7->charID(s, 5)).scaled(ui->btnCloud->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    ui->btnCait->setIcon(QIcon(FF7Char::instance()->pixmap(ff7->charID(s, 6)).scaled(ui->btnCloud->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    ui->btnVincent->setIcon(QIcon(FF7Char::instance()->pixmap(ff7->charID(s, 7)).scaled(ui->btnCloud->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    ui->btnCid->setIcon(QIcon(FF7Char::instance()->pixmap(ff7->charID(s, 8)).scaled(ui->btnCloud->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
}
void MainWindow::progress_update()
{
    load = true;
    ui->sbCurdisc->setValue(ff7->disc(s));
    ui->sbMprogress->setValue(ff7->mainProgress(s));

    ui->sbTurkschurch->setValue(ff7->churchProgress(s));
    ui->sbDonprog->setValue(ff7->donProgress(s));
    ui->comboS7Slums->setCurrentIndex(0);

    ui->cbBm1_1->setChecked(ff7->bmProgress1(s, 0));
    ui->cbBm1_2->setChecked(ff7->bmProgress1(s, 1));
    ui->cbBm1_3->setChecked(ff7->bmProgress1(s, 2));
    ui->cbBm1_4->setChecked(ff7->bmProgress1(s, 3));
    ui->cbBm1_5->setChecked(ff7->bmProgress1(s, 4));
    ui->cbBm1_6->setChecked(ff7->bmProgress1(s, 5));
    ui->cbBm1_7->setChecked(ff7->bmProgress1(s, 6));
    ui->cbBm1_8->setChecked(ff7->bmProgress1(s, 7));
    ui->cbBm2_1->setChecked(ff7->bmProgress2(s, 0));
    ui->cbBm2_2->setChecked(ff7->bmProgress2(s, 1));
    ui->cbBm2_3->setChecked(ff7->bmProgress2(s, 2));
    ui->cbBm2_4->setChecked(ff7->bmProgress2(s, 3));
    ui->cbBm2_5->setChecked(ff7->bmProgress2(s, 4));
    ui->cbBm2_6->setChecked(ff7->bmProgress2(s, 5));
    ui->cbBm2_7->setChecked(ff7->bmProgress2(s, 6));
    ui->cbBm2_8->setChecked(ff7->bmProgress2(s, 7));
    ui->cbBm3_1->setChecked(ff7->bmProgress3(s, 0));
    ui->cbBm3_2->setChecked(ff7->bmProgress3(s, 1));
    ui->cbBm3_3->setChecked(ff7->bmProgress3(s, 2));
    ui->cbBm3_4->setChecked(ff7->bmProgress3(s, 3));
    ui->cbBm3_5->setChecked(ff7->bmProgress3(s, 4));
    ui->cbBm3_6->setChecked(ff7->bmProgress3(s, 5));
    ui->cbBm3_7->setChecked(ff7->bmProgress3(s, 6));
    ui->cbBm3_8->setChecked(ff7->bmProgress3(s, 7));
    ui->cbMidgartrain_1->setChecked(ff7->midgarTrainFlags(s, 0));
    ui->cbMidgartrain_2->setChecked(ff7->midgarTrainFlags(s, 1));
    ui->cbMidgartrain_3->setChecked(ff7->midgarTrainFlags(s, 2));
    ui->cbMidgartrain_4->setChecked(ff7->midgarTrainFlags(s, 3));
    ui->cbMidgartrain_5->setChecked(ff7->midgarTrainFlags(s, 4));
    ui->cbMidgartrain_6->setChecked(ff7->midgarTrainFlags(s, 5));
    ui->cbMidgartrain_7->setChecked(ff7->midgarTrainFlags(s, 6));
    ui->cbMidgartrain_8->setChecked(ff7->midgarTrainFlags(s, 7));
    ui->cbBombingInt->setChecked(ff7->startBombingMission(s));

    ui->cbS7pl_1->setChecked((ff7->unknown(s, 26).at(0) & (1 << 0)));
    ui->cbS7pl_2->setChecked((ff7->unknown(s, 26).at(0) & (1 << 1)));
    ui->cbS7pl_3->setChecked((ff7->unknown(s, 26).at(0) & (1 << 2)));
    ui->cbS7pl_4->setChecked((ff7->unknown(s, 26).at(0) & (1 << 3)));
    ui->cbS7pl_5->setChecked((ff7->unknown(s, 26).at(0) & (1 << 4)));
    ui->cbS7pl_6->setChecked((ff7->unknown(s, 26).at(0) & (1 << 5)));
    ui->cbS7pl_7->setChecked((ff7->unknown(s, 26).at(0) & (1 << 6)));
    ui->cbS7pl_8->setChecked((ff7->unknown(s, 26).at(0) & (1 << 7)));

    ui->cbS7ts_1->setChecked((ff7->unknown(s, 26).at(8) & (1 << 0)));
    ui->cbS7ts_2->setChecked((ff7->unknown(s, 26).at(8) & (1 << 1)));
    ui->cbS7ts_3->setChecked((ff7->unknown(s, 26).at(8) & (1 << 2)));
    ui->cbS7ts_4->setChecked((ff7->unknown(s, 26).at(8) & (1 << 3)));
    ui->cbS7ts_5->setChecked((ff7->unknown(s, 26).at(8) & (1 << 4)));
    ui->cbS7ts_6->setChecked((ff7->unknown(s, 26).at(8) & (1 << 5)));
    ui->cbS7ts_7->setChecked((ff7->unknown(s, 26).at(8) & (1 << 6)));
    ui->cbS7ts_8->setChecked((ff7->unknown(s, 26).at(8) & (1 << 7)));
    
    if (ff7->unknown(s, 26).mid(0, 6) == "\x00\x00\x00\x00\x00\x00")
        ui->comboS7Slums->setCurrentIndex(1);
    else if (ff7->unknown(s, 26).mid(0, 6) == "\xFF\x03\x04\x0F\x1F\x6F" || ff7->unknown(s, 26).mid(0, 6) == "\xBF\x51\x05\x17\x5D\xEF")
        ui->comboS7Slums->setCurrentIndex(2);
    else if (static_cast<unsigned char>(ff7->unknown(s, 26).at(2)) == 0x13)
        ui->comboS7Slums->setCurrentIndex(3);
    else
        ui->comboS7Slums->setCurrentIndex(0);
    
    load = false;
}
/*~~~~~~~~~Char Buttons.~~~~~~~~~~~*/
void MainWindow::btnCloud_clicked()
{
    curchar = 0;
    char_editor->setChar(ff7->character(s, 0), ff7->charName(s, 0));
}
void MainWindow::btnBarret_clicked()
{
    curchar = 1;
    char_editor->setChar(ff7->character(s, 1), ff7->charName(s, 1));
}
void MainWindow::btnTifa_clicked()
{
    curchar = 2;
    char_editor->setChar(ff7->character(s, 2), ff7->charName(s, 2));
}
void MainWindow::btnAeris_clicked()
{
    curchar = 3;
    char_editor->setChar(ff7->character(s, 3), ff7->charName(s, 3));
}
void MainWindow::btnRed_clicked()
{
    curchar = 4;
    char_editor->setChar(ff7->character(s, 4), ff7->charName(s, 4));
}
void MainWindow::btnYuffie_clicked()
{
    curchar = 5;
    char_editor->setChar(ff7->character(s, 5), ff7->charName(s, 5));
}
void MainWindow::btnCait_clicked()
{
    curchar = 6;
    char_editor->setChar(ff7->character(s, 6), ff7->charName(s, 6));
}
void MainWindow::btnVincent_clicked()
{
    curchar = 7;
    char_editor->setChar(ff7->character(s, 7), ff7->charName(s, 7));
}
void MainWindow::btnCid_clicked()
{
    curchar = 8;
    char_editor->setChar(ff7->character(s, 8), ff7->charName(s, 8));
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Party TAB~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::comboParty1_currentIndexChanged(int index)
{
    if (!load) {
        if (index == 0x0C) { //empty char slot (index 12)
            ff7->setParty(s, 0, 0xFF);
            //wipe all desc data if noone is there
            ff7->setDescParty(s, 0, ff7->party(s, 0));
            ff7->setDescCurHP(s, 0);
            ff7->setDescMaxHP(s, 0);
            ff7->setDescCurMP(s, 0);
            ff7->setDescMaxMP(s, 0);
            ff7->setDescLevel(s, 0);
            ff7->setDescName(s, QString(QByteArray(16, char(0xFF))));
        } else {
            ff7->setParty(s, 0, index);
            ff7->setDescParty(s, 0, ff7->party(s, 0));
            // IF ID >8 no char slot so for 9, 10, 11 Use slot 6,7,8 char data.
            if (ff7->party(s, 0) == FF7Char::YoungCloud) {
                ff7->setDescCurHP(s, ff7->charCurrentHp(s, 6));
                ff7->setDescMaxHP(s, ff7->charMaxHp(s, 6));
                ff7->setDescCurMP(s, ff7->charCurrentMp(s, 6));
                ff7->setDescMaxMP(s, ff7->charMaxMp(s, 6));
                ff7->setDescLevel(s, ff7->charLevel(s, 6));
                ff7->setDescName(s, ff7->charName(s, 6));
            } else if (ff7->party(s, 0) == FF7Char::Sephiroth) {
                ff7->setDescCurHP(s, ff7->charCurrentHp(s, 7));
                ff7->setDescMaxHP(s, ff7->charMaxHp(s, 7));
                ff7->setDescCurMP(s, ff7->charCurrentMp(s, 7));
                ff7->setDescMaxMP(s, ff7->charMaxMp(s, 7));
                ff7->setDescLevel(s, ff7->charLevel(s, 7));
                ff7->setDescName(s, ff7->charName(s, 7));
            } else if (ff7->party(s, 0) == 11) {
                //chocobo? that never really works.
                ff7->setDescCurHP(s, ff7->charCurrentHp(s, 8));
                ff7->setDescMaxHP(s, ff7->charMaxHp(s, 8));
                ff7->setDescCurMP(s, ff7->charCurrentMp(s, 8));
                ff7->setDescMaxMP(s, ff7->charMaxMp(s, 8));
                ff7->setDescLevel(s, ff7->charLevel(s, 8));
                ff7->setDescName(s, ff7->charName(s, 8));
            } else {
                ff7->setDescCurHP(s, ff7->charCurrentHp(s, ff7->party(s, 0)));
                ff7->setDescMaxHP(s, ff7->charMaxHp(s, ff7->party(s, 0)));
                ff7->setDescCurMP(s, ff7->charCurrentMp(s, ff7->party(s, 0)));
                ff7->setDescMaxMP(s, ff7->charMaxMp(s, ff7->party(s, 0)));
                ff7->setDescLevel(s, ff7->charLevel(s, ff7->party(s, 0)));
                ff7->setDescName(s, ff7->charName(s, ff7->party(s, 0)));
            }
        }
    }
}

void MainWindow::comboParty2_currentIndexChanged(int index)
{
    if (!load) {
        if (index == 12)
            ff7->setParty(s, 1, FF7Char::Empty);
        else
            ff7->setParty(s, 1, index);
        //either way set the desc
        ff7->setDescParty(s, 1, ff7->party(s, 1));
    }
}

void MainWindow::comboParty3_currentIndexChanged(int index)
{
    if (!load) {
        if (index == 12)
            ff7->setParty(s, 2, FF7Char::Empty);
        else
            ff7->setParty(s, 2, index);
        //either way set the desc
        ff7->setDescParty(s, 2, ff7->party(s, 2));
    }
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~Chocobo Tab~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::cm_stablesOwnedChanged(qint8 owned)
{
    if (!load)
        ff7->setStablesOwned(s, owned);
}
void MainWindow::cm_stableMaskChanged(qint8 mask)
{
    if (!load)
        ff7->setStableMask(s, mask);
}
void MainWindow::cm_stablesOccupiedChanged(qint8 occupied)
{
    if (!load)
        ff7->setStablesOccupied(s, occupied);
}
//set data for stables inside
void MainWindow::cm_nameChanged(int stable, QString text)
{
    if (!load)
        ff7->setChocoName(s, stable, text);
}
void MainWindow::cm_staminaChanged(int stable, quint16 value)
{
    if (!load)
        ff7->setChocoStamina(s, stable, value);
}
void MainWindow::cm_speedChanged(int stable, quint16 value)
{
    if (!load)
        ff7->setChocoSpeed(s, stable, value);
}
void MainWindow::cm_maxspeedChanged(int stable, quint16 value)
{
    if (!load)
        ff7->setChocoMaxSpeed(s, stable, value);
}
void MainWindow::cm_sprintChanged(int stable, quint16 value)
{
    if (!load)
        ff7->setChocoSprintSpeed(s, stable, value);
}
void MainWindow::cm_maxsprintChanged(int stable, quint16 value)
{
    if (!load)
        ff7->setChocoMaxSprintSpeed(s, stable, value);
}
void MainWindow::cm_sexChanged(int stable, quint8 index)
{
    if (!load)
        ff7->setChocoSex(s, stable, index);
}
void MainWindow::cm_typeChanged(int stable, quint8 index)
{
    if (!load)
        ff7->setChocoType(s, stable, index);
}
void MainWindow::cm_coopChanged(int stable, quint8 value)
{
    if (!load)
        ff7->setChocoCoop(s, stable, value);
}
void MainWindow::cm_accelChanged(int stable, quint8 value)
{
    if (!load)
        ff7->setChocoAccel(s, stable, value);
}
void MainWindow::cm_intelChanged(int stable, quint8 value)
{
    if (!load)
        ff7->setChocoIntelligence(s, stable, value);
}
void MainWindow::cm_raceswonChanged(int stable, quint8 value)
{
    if (!load)
        ff7->setChocoRaceswon(s, stable, value);
}
void MainWindow::cm_pcountChanged(int stable, quint8 value)
{
    if (!load)
        ff7->setChocoPCount(s, stable, value);
}
void MainWindow::cm_personalityChanged(int stable, quint8 value)
{
    if (!load)
        ff7->setChocoPersonality(s, stable, value);
}
void MainWindow::cm_mated_toggled(int stable, bool checked)
{
    if (!load)
        ff7->setChocoCantMate(s, stable, checked);
}
void MainWindow::cm_ratingChanged(int stable, quint8 rating)
{
    if (!load)
        ff7->setChocoboRating(s, stable, rating);
}
//set data for pens outside
void MainWindow::cm_pensChanged(int pen, int index)
{
    if (!load)
        ff7->setChocoboPen(s, pen, index);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OTHERS TAB~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void MainWindow::sbLoveBarret_valueChanged(int value)
{
    if (!load)
        ff7->setLove(s, false, FF7Save::LOVE_BARRET, quint8(value));
}
void MainWindow::sbLoveAeris_valueChanged(int value)
{
    if (!load)
        ff7->setLove(s, false, FF7Save::LOVE_AERIS, quint8(value));
}
void MainWindow::sbLoveTifa_valueChanged(int value)
{
    if (!load)
        ff7->setLove(s, false, FF7Save::LOVE_TIFA, quint8(value));
}
void MainWindow::sbLoveYuffie_valueChanged(int value)
{
    if (!load)
        ff7->setLove(s, false, FF7Save::LOVE_YUFFIE, quint8(value));
}

void MainWindow::sbTimeHour_valueChanged(int value)
{
    if (!load)
        ff7->setTime(s, quint32((value * 3600) + (ui->sbTimeMin->value() * 60) + (ui->sbTimeSec->value())));
}
void MainWindow::sbTimeMin_valueChanged(int value)
{
    if (!load)
        ff7->setTime(s, quint32((ui->sbTimeHour->value() * 3600) + ((value * 60)) + (ui->sbTimeSec->value())));
}
void MainWindow::sbTimeSec_valueChanged(int value)
{
    if (!load)
        ff7->setTime(s, quint32((ui->sbTimeHour->value() * 3600) + (ui->sbTimeMin->value() * 60) + (value)));
}

void MainWindow::sbSteps_valueChanged(int value)
{
    if (!load)
        ff7->setSteps(s, value);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Item Tab~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void MainWindow::sbGil_valueChanged(double value)
{
    if (!load)
        ff7->setGil(s, quint32(value));
}
void MainWindow::sbGp_valueChanged(int value)
{
    if (!load)
        ff7->setGp(s, value);
}
void MainWindow::sbBattles_valueChanged(int value)
{
    if (!load)
        ff7->setBattles(s, value);
}
void MainWindow::sbRuns_valueChanged(int value)
{
    if (!load)
        ff7->setRuns(s, value);
}

void MainWindow::cbMysteryPanties_toggled(bool checked)
{
    if (!load)
        ff7->setKeyItem(s, FF7Save::MYSTERYPANTIES, checked);
}
void MainWindow::cbLetterToDaughter_toggled(bool checked)
{
    if (!load)
        ff7->setKeyItem(s, FF7Save::LETTERTOADAUGHTER, checked);
}
void MainWindow::cbLetterToWife_toggled(bool checked)
{
    if (!load)
        ff7->setKeyItem(s, FF7Save::LETTERTOAWIFE, checked);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MATERIA TAB~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void MainWindow::tblMateria_currentCellChanged(int row)
{
    if (!load) {
        load = true;
        materia_editor->setMateria(ff7->partyMateriaId(s, row), ff7->partyMateriaAp(s, row));
        load = false;
    }
}

void MainWindow::btnAddAllMateria_clicked()
{
    //place one of each at lowest possible point
    for (int i = 117; i < 142; i++) {
        //Starting With Magic Materia
        if (i < 132)
            ff7->setPartyMateria(s, i, quint8(i - 68), FF7Materia::MaxMateriaAp);
        else if (i < 136)
            ff7->setPartyMateria(s, (i - 1), quint8(i - 68), FF7Materia::MaxMateriaAp);
        else if (i < 142)
            ff7->setPartyMateria(s, (i - 3), quint8(i - 68), FF7Materia::MaxMateriaAp);
    }
    // Then Support
    for (int i = 139; i < 152; i++)
        ff7->setPartyMateria(s, i, quint8(i - 116), FF7Materia::MaxMateriaAp);

    for (int i = 152; i < 166; i++) {
        //Then Command
        if (i < 154)
            ff7->setPartyMateria(s, i, quint8(i - 138), FF7Materia::MaxMateriaAp);
        else if (i < 157)
            ff7->setPartyMateria(s, i, quint8(i - 135), FF7Materia::MaxMateriaAp);
        else if (i < 159)
            ff7->setPartyMateria(s, i, quint8(i - 121), FF7Materia::MaxMateriaAp);
        else if (i < 165)
            ff7->setPartyMateria(s, i, quint8(i - 120), FF7Materia::MaxMateriaAp);
        else
            ff7->setPartyMateria(s, i, 0x30, FF7Materia::MaxMateriaAp);
    }
    for (int i = 166; i < 183; i++) {
        //And Independent
        if (i < 180)
            ff7->setPartyMateria(s, i,  quint8(i - 166), FF7Materia::MaxMateriaAp);
        else
            ff7->setPartyMateria(s, i,  quint8(i - 164), FF7Materia::MaxMateriaAp);
    }
    //Finish With Summons
    for (int i = 183; i < 200; i++)
        ff7->setPartyMateria(s, i,  quint8(i - 109), FF7Materia::MaxMateriaAp);
    materiaupdate();
    statusBar()->showMessage(tr("All Materia Added!"), 750);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SAVE LOCATION TAB~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::locationSelectionChanged(QString fieldName)
{
    if (!load) {
        ff7->setMapId(s, FF7Location::instance()->mapID(fieldName).toUShort());
        ff7->setLocationId(s, FF7Location::instance()->locationID(fieldName).toUShort());
        ff7->setLocationX(s, FF7Location::instance()->x(fieldName).toShort());
        ff7->setLocationY(s, FF7Location::instance()->y(fieldName).toShort());
        ff7->setLocationT(s, FF7Location::instance()->t(fieldName).toUShort());
        ff7->setLocationD(s, quint8(FF7Location::instance()->d(fieldName).toInt()));
        ff7->setLocation(s, FF7Location::instance()->locationString(fieldName));
        statusBar()->showMessage(tr("Set Save Location: %1").arg(fieldName), 750);
    }
}
void MainWindow::map_id_valueChanged(int value)
{
    if (!load)
        ff7->setMapId(s, quint16(value));
}
void MainWindow::loc_id_valueChanged(int value)
{
    if (!load)
        ff7->setLocationId(s, quint16(value));
}
void MainWindow::coord_x_valueChanged(int value)
{
    if (!load)
        ff7->setLocationX(s, qint16(value));
}
void MainWindow::coord_y_valueChanged(int value)
{
    if (!load)
        ff7->setLocationY(s, qint16(value));
}
void MainWindow::coord_t_valueChanged(int value)
{
    if (!load)
        ff7->setLocationT(s, quint16(value));
}
void MainWindow::coord_d_valueChanged(int value)
{
    if (!load)
        ff7->setLocationD(s, quint8(value));
}
void MainWindow::location_textChanged(QString text)
{
    if (!load)
        ff7->setLocation(s, text);
}

/*~~~~~~~~~~~~~~~~~~~ Game Options~~~~~~~~~~~~~~~~~~*/
void MainWindow::setDialogColorUL(QColor color)
{
    if (!load)
        ff7->setDialogColorUL(s, color);
}
void MainWindow::setDialogColorUR(QColor color)
{
    if (!load)
        ff7->setDialogColorUR(s, color);
}
void MainWindow::setDialogColorLL(QColor color)
{
    if (!load)
        ff7->setDialogColorLL(s, color);
}
void MainWindow::setDialogColorLR(QColor color)
{
    if (!load)
        ff7->setDialogColorLR(s, color);
}

void MainWindow::setBattleSpeed(int value)
{
    if (!load)
        ff7->setBattleSpeed(s, value);
}
void MainWindow::setBattleMessageSpeed(int value)
{
    if (!load)
        ff7->setBattleMessageSpeed(s, value);
}
void MainWindow::setFieldMessageSpeed(int value)
{
    if (!load)
        ff7->setMessageSpeed(s, value);
}
void MainWindow::setBattleHelp(bool checked)
{
    if (!load)
        ff7->setBattleHelp(s, checked);
}
void MainWindow::setFieldHelp(bool checked)
{
    if (!load)
        ff7->setFieldHelp(s, checked);
}
void MainWindow::setBattleTargets(bool checked)
{
    if (!load)
        ff7->setBattleTargets(s, checked);
}

void MainWindow::setControlMode(int mode)
{
    if (!load)
        ff7->setControlMode(s, mode);
}
void MainWindow::setSoundMode(int mode)
{
    if (!load)
        ff7->setSoundMode(s, mode);
}
void MainWindow::setCursorMode(int mode)
{
    if (!load)
        ff7->setCursorMode(s, mode);
}
void MainWindow::setAtbMode(int mode)
{
    if (!load)
        ff7->setAtbMode(s, mode);
}
void MainWindow::setCameraMode(int mode)
{
    if (!load)
        ff7->setCameraMode(s, mode);
}
void MainWindow::setMagicOrder(int order)
{
    if (!load)
        ff7->setMagicOrder(s, order);
}

/*--------GAME PROGRESS-------*/
void MainWindow::sbCurdisc_valueChanged(int value)
{
    if (!load)
        ff7->setDisc(s, value);
}
void MainWindow::sbMprogress_valueChanged(int value)
{
    if (!load)
        ff7->setMainProgress(s, value);
}
void MainWindow::sbTurkschurch_valueChanged(int value)
{
    if (!load)
        ff7->setChurchProgress(s, value);
}
void MainWindow::sbDonprog_valueChanged(int value)
{
    if (!load)
        ff7->setDonProgress(s, value);
}
void MainWindow::cbBm1_1_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress1(s, 0, checked);
}
void MainWindow::cbBm1_2_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress1(s, 1, checked);
}
void MainWindow::cbBm1_3_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress1(s, 2, checked);
}
void MainWindow::cbBm1_4_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress1(s, 3, checked);
}
void MainWindow::cbBm1_5_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress1(s, 4, checked);
}
void MainWindow::cbBm1_6_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress1(s, 5, checked);
}
void MainWindow::cbBm1_7_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress1(s, 6, checked);
}
void MainWindow::cbBm1_8_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress1(s, 7, checked);
}
void MainWindow::cbBm2_1_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress2(s, 0, checked);
}
void MainWindow::cbBm2_2_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress2(s, 1, checked);
}
void MainWindow::cbBm2_3_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress2(s, 2, checked);
}
void MainWindow::cbBm2_4_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress2(s, 3, checked);
}
void MainWindow::cbBm2_5_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress2(s, 4, checked);
}
void MainWindow::cbBm2_6_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress2(s, 5, checked);
}
void MainWindow::cbBm2_7_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress2(s, 6, checked);
}
void MainWindow::cbBm2_8_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress2(s, 7, checked);
}
void MainWindow::cbBm3_1_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress3(s, 0, checked);
}
void MainWindow::cbBm3_2_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress3(s, 1, checked);
}
void MainWindow::cbBm3_3_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress3(s, 2, checked);
}
void MainWindow::cbBm3_4_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress3(s, 3, checked);
}
void MainWindow::cbBm3_5_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress3(s, 4, checked);
}
void MainWindow::cbBm3_6_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress3(s, 5, checked);
}
void MainWindow::cbBm3_7_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress3(s, 6, checked);
}
void MainWindow::cbBm3_8_toggled(bool checked)
{
    if (!load)
        ff7->setBmProgress3(s, 7, checked);
}

void MainWindow::cbS7pl_1_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(0);
        if (checked)
            t |= (1 << 0);
        else
            t &= ~(1 << 0);
        temp[0] = t;
        ff7->setUnknown(s, 26, temp);
    }
}
void MainWindow::cbS7pl_2_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(0);
        if (checked)
            t |= (1 << 1);
        else
            t &= ~(1 << 1);
        temp[0] = t;
        ff7->setUnknown(s, 26, temp);
    }
}
void MainWindow::cbS7pl_3_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(0);
        if (checked)
            t |= (1 << 2);
        else
            t &= ~(1 << 2);
        temp[0] = t;
        ff7->setUnknown(s, 26, temp);
    }
}
void MainWindow::cbS7pl_4_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(0);
        if (checked)
            t |= (1 << 3);
        else
            t &= ~(1 << 3);
        temp[0] = t;
        ff7->setUnknown(s, 26, temp);
    }
}
void MainWindow::cbS7pl_5_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(0);
        if (checked)
            t |= (1 << 4);
        else
            t &= ~(1 << 4);
        temp[0] = t;
        ff7->setUnknown(s, 26, temp);
    }
}
void MainWindow::cbS7pl_6_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(0);
        if (checked)
            t |= (1 << 5);
        else
            t &= ~(1 << 5);
        temp[0] = t;
        ff7->setUnknown(s, 26, temp);
    }
}
void MainWindow::cbS7pl_7_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(0);
        if (checked)
            t |= (1 << 6);
        else
            t &= ~(1 << 6);
        temp[0] = t;
        ff7->setUnknown(s, 26, temp);
    }
}
void MainWindow::cbS7pl_8_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(0);
        if (checked)
            t |= (1 << 7);
        else
            t &= ~(1 << 7);
        temp[0] = t;
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::cbS7ts_1_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(8);
        if (checked)
            t |= (1 << 0);
        else
            t &= ~(1 << 0);
        temp[8] = t;
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::cbS7ts_2_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(8);
        if (checked)
            t |= (1 << 1);
        else
            t &= ~(1 << 1);
        temp[8] = t;
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::cbS7ts_3_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(8);
        if (checked)
            t |= (1 << 2);
        else
            t &= ~(1 << 2);
        temp[8] = t;
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::cbS7ts_4_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(8);
        if (checked)
            t |= (1 << 3);
        else
            t &= ~(1 << 3);
        temp[8] = t;
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::cbS7ts_5_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(8);
        if (checked)
            t |= (1 << 4);
        else
            t &= ~(1 << 4);
        temp[8] = t;
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::cbS7ts_6_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(8);
        if (checked)
            t |= (1 << 5);
        else
            t &= ~(1 << 5);
        temp[8] = t;
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::cbS7ts_7_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(8);
        if (checked)
            t |= (1 << 6);
        else
            t &= ~(1 << 6);
        temp[8] = t;
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::cbS7ts_8_toggled(bool checked)
{
    if (!load) {
        QByteArray temp = ff7->unknown(s, 26); char t = temp.at(8);
        if (checked)
            t |= (1 << 7);
        else
            t &= ~(1 << 7);
        temp[8] = t;
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::cbBombingInt_stateChanged(int checked)
{
    if (!load)
        ff7->setStartBombingMission(s, checked);
}

void MainWindow::comboReplay_currentIndexChanged(int index)
{
    if (index > 0)
        ui->btnReplay->setEnabled(true);
    else
        ui->btnReplay->setEnabled(false);

    //Display Info on the Selected Reset
    //see on_btnReplay_clicked() for the reset data
    if (index == 1) //Bombing Mission Reset
        ui->label_replaynote->setText(tr("Replay the bombing mission from right after you get off the train."));
    else if (index == 2) //Church in the Slums Reset
        ui->label_replaynote->setText(tr("Meeting Aeris"));
    else if (index == 3) //Cloud's Flashback Reset
        ui->label_replaynote->setText(tr("This Will Copy Cloud as is to young cloud (caitsith's slot). Sephiroth's stats will come directly from the Default Save. Be Sure to back up your CaitSith and Vincent if you want to use them again"));
    else if (index == 4) //Date Scene
        ui->label_replaynote->setText(tr("Replay the Date Scene, Your Location will be set To The Ropeway Station Talk to man by the Tram to start event. If Your Looking for a special Date be sure to set your love points too."));
    else if (index == 5) //Aerith's Death
        ui->label_replaynote->setText(tr("Replay the death of Aeris.This option Will remove Aeris from your PHS"));
    else
        ui->label_replaynote->setText(tr("         INFO ON CURRENTLY SELECTED REPLAY MISSION"));
}
void MainWindow::btnReplay_clicked()
{
    if (ui->comboReplay->currentIndex() == 1) { // bombing mission
        ui->sbCurdisc->setValue(1);
        ui->sbMprogress->setValue(1);
        ff7->setBmProgress1(s, 0);
        ff7->setBmProgress2(s, 0);
        ff7->setBmProgress3(s, 0);
        ff7->setMidgarTrainFlags(s, 0);
        ui->cbBombingInt->setChecked(true);
        ui->comboS7Slums->setCurrentIndex(1);
        ui->sbTurkschurch->setValue(0); // reset turks.
        locationViewer->setMapId(1);
        locationViewer->setLocationId(116);
        statusBar()->showMessage(tr("Progression Reset Complete"), 750);
    } else if (ui->comboReplay->currentIndex() == 2) { // The Church In The Slums
        ui->sbCurdisc->setValue(1);
        ui->sbMprogress->setValue(130);
        ui->sbTurkschurch->setValue(0);
        ff7->setBmProgress1(s, 120);
        ff7->setBmProgress2(s, 198);
        ff7->setBmProgress3(s, 3);
        ui->cbBombingInt->setChecked(false);
        locationViewer->setMapId(1);
        locationViewer->setLocationId(183);
        ui->comboParty1->setCurrentIndex(0);
        ui->comboParty2->setCurrentIndex(12);
        ui->comboParty3->setCurrentIndex(12);
        statusBar()->showMessage(tr("Progression Reset Complete"), 750);
    } else if (ui->comboReplay->currentIndex() == 3) { // Flash back
        ui->sbCurdisc->setValue(1);
        ui->sbMprogress->setValue(341);
        ff7->setBmProgress1(s, 120);
        ff7->setBmProgress2(s, 198);
        ff7->setBmProgress3(s, 3);
        ui->cbBombingInt->setChecked(false);
        locationViewer->setMapId(1);
        locationViewer->setLocationId(332);
        // set up young cloud, Copy Cloud Change ID to young Cloud
        ff7->setCharacter(s, FF7Char::CaitSith, ff7->character(s, FF7Char::Cloud));
        ff7->setCharID(s, FF7Char::CaitSith, FF7Char::YoungCloud);
        //set up Sephiroth
        FF7Save *temp = new FF7Save();
        temp->newGame(s);
        ff7->setCharacter(s, FF7Char::Vincent, temp->character(s, FF7Char::Vincent));
        if (ff7->isJPN(s))
            ff7->setCharName(s, 7, QString::fromUtf8("セフィロス"));
        else
            ff7->setCharName(s, 7, QString::fromUtf8("Sephiroth"));

        set_char_buttons();
        if (curchar == FF7Char::CaitSith)
            char_editor->setChar(ff7->character(s, 6), ff7->charName(s, 6));
        else if (curchar == FF7Char::Vincent)
            char_editor->setChar(ff7->character(s, 7), ff7->charName(s, 7));
        statusBar()->showMessage(tr("Progression Reset Complete"), 750);
    }

    else if (ui->comboReplay->currentIndex() == 4) { // The Date Scene
        ui->sbCurdisc->setValue(1);
        ui->sbMprogress->setValue(583);
        ff7->setBmProgress1(s, 120);
        ff7->setBmProgress2(s, 198);
        ff7->setBmProgress3(s, 3);
        ui->cbBombingInt->setChecked(false);
        locationViewer->setMapId(1);
        locationViewer->setLocationId(496);
        statusBar()->showMessage(tr("Progression Reset Complete"), 750);
    }

    else if (ui->comboReplay->currentIndex() == 5) { //Aeris Death
        ui->sbCurdisc->setValue(1);
        ui->sbMprogress->setValue(664);
        ff7->setBmProgress1(s, 120);
        ff7->setBmProgress2(s, 198);
        ff7->setBmProgress3(s, 3);
        ui->cbBombingInt->setChecked(false);
        locationViewer->setMapId(1);
        locationViewer->setLocationId(646);
        phsList->setChecked(FF7Char::Aerith, PhsListWidget::PHSALLOWED, false);
        phsList->setChecked(FF7Char::Aerith, PhsListWidget::PHSVISIBLE, false);
        statusBar()->showMessage(tr("Progression Reset Complete"), 750);
    }
    ui->comboReplay->setCurrentIndex(0);
    if (!load)
        progress_update();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FUNCTIONS FOR TESTING~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::btnRemoveAllItems_clicked()
{
    for (int i = 0; i < 320; i++)
        ff7->setItem(s, i, FF7Item::EmptyItemData);
    itemlist->setItems(ff7->items(s));
}

void MainWindow::btnRemoveAllMateria_clicked()
{
    for (int i = 0; i < 200; i++)
        ff7->setPartyMateria(s, i, FF7Materia::EmptyId, FF7Materia::MaxMateriaAp);
    materiaupdate();
}

void MainWindow::btnRemoveAllStolen_clicked()
{
    for (int i = 0; i < 48; i++)
        ff7->setStolenMateria(s, i, FF7Materia::EmptyId, FF7Materia::MaxMateriaAp);
    guirefresh(0);
}

void MainWindow::sbBloveAeris_valueChanged(int value)
{
    if (!load)
        ff7->setLove(s, true, FF7Save::LOVE_AERIS, quint8(value));
}
void MainWindow::sbBloveTifa_valueChanged(int value)
{
    if (!load)
        ff7->setLove(s, true, FF7Save::LOVE_TIFA, quint8(value));
}
void MainWindow::sbBloveYuffie_valueChanged(int value)
{
    if (!load)
        ff7->setLove(s, true, FF7Save::LOVE_YUFFIE, quint8(value));
}
void MainWindow::sbBloveBarret_valueChanged(int value)
{
    if (!load)
        ff7->setLove(s, true, FF7Save::LOVE_BARRET, quint8(value));
}
void MainWindow::sbCoster1_valueChanged(int value)
{
    if (!load)
        ff7->setSpeedScore(s, 1, quint16(value));
}
void MainWindow::sbCoster2_valueChanged(int value)
{
    if (!load)
        ff7->setSpeedScore(s, 2, quint16(value));
}
void MainWindow::sbCoster3_valueChanged(int value)
{
    if (!load)
        ff7->setSpeedScore(s, 3, quint16(value));
}
void MainWindow::sbTimerTimeHour_valueChanged(int value)
{
    if (!load)
        ff7->setCountdownTimer(s, quint32((value * 3600) + (ui->sbTimerTimeMin->value() * 60) + (ui->sbTimerTimeSec->value())));
}
void MainWindow::sbTimerTimeMin_valueChanged(int value)
{
    if (!load)
        ff7->setCountdownTimer(s, quint32((ui->sbTimerTimeHour->value() * 3600) + ((value * 60)) + (ui->sbTimerTimeSec->value())));
}
void MainWindow::sbTimerTimeSec_valueChanged(int value)
{
    if (!load)
        ff7->setCountdownTimer(s, quint32((ui->sbTimerTimeHour->value() * 3600) + (ui->sbTimerTimeMin->value() * 60) + (value)));
}
void MainWindow::sbUweaponHp_valueChanged(int value)
{
    if (!load)
        ff7->setUWeaponHp(s, value);
}
void MainWindow::cbRegVinny_toggled(bool checked)
{
    if (!load)
        ff7->setVincentUnlocked(s, checked);
}
void MainWindow::cbRegYuffie_toggled(bool checked)
{
    if (!load)
        ff7->setYuffieUnlocked(s, checked);
}
void MainWindow::cbYuffieForest_toggled(bool checked)
{
    if (!load)
        ff7->setCanFightNinjaInForest(s, checked);
}

void MainWindow::cbMidgartrain_1_toggled(bool checked)
{
    if (!load)
        ff7->setMidgarTrainFlags(s, 0, checked);
}
void MainWindow::cbMidgartrain_2_toggled(bool checked)
{
    if (!load)
        ff7->setMidgarTrainFlags(s, 1, checked);
}
void MainWindow::cbMidgartrain_3_toggled(bool checked)
{
    if (!load)
        ff7->setMidgarTrainFlags(s, 2, checked);
}
void MainWindow::cbMidgartrain_4_toggled(bool checked)
{
    if (!load)
        ff7->setMidgarTrainFlags(s, 3, checked);
}
void MainWindow::cbMidgartrain_5_toggled(bool checked)
{
    if (!load)
        ff7->setMidgarTrainFlags(s, 4, checked);
}
void MainWindow::cbMidgartrain_6_toggled(bool checked)
{
    if (!load)
        ff7->setMidgarTrainFlags(s, 5, checked);
}
void MainWindow::cbMidgartrain_7_toggled(bool checked)
{
    if (!load)
        ff7->setMidgarTrainFlags(s, 6, checked);

}
void MainWindow::cbMidgartrain_8_toggled(bool checked)
{
    if (!load)
        ff7->setMidgarTrainFlags(s, 7, checked);
}

void MainWindow::cbTutWorldSave_stateChanged(int value)
{
    if (!load) {
        if (value == 0)
            ff7->setTutSave(s, 0x00);
        else if (value == 1)
            ff7->setTutSave(s, 0x32);
        else if (value == 2)
            ff7->setTutSave(s, 0x3A);
        ui->lcdNumber_7->display(ff7->tutSave(s));
    }
}

void MainWindow::comboRegionSlot_currentIndexChanged(int index)
{
    if (!load) {
        if (ff7->isFF7(s)) {
            ff7->setSaveNumber(s, index);
            guirefresh(0);
        }
    }
}

void MainWindow::cbTutSub_toggled(bool checked)
{
    if (!load) {
        ff7->setTutSub(s, 2, checked);
        ui->lcdTutSub->display(ff7->tutSub(s));
    }
}

void MainWindow::cbRubyDead_toggled(bool checked)
{
    if (!load)
        ff7->setKilledRubyWeapon(s, checked);
}
void MainWindow::cbEmeraldDead_toggled(bool checked)
{
    if (!load)
        ff7->setKilledEmeraldWeapon(s, checked);
}

void MainWindow::comboHighwindBuggy_currentIndexChanged(int index)
{
    if (!load) {
        switch (index) {
        case 0: ui->sbBhId->setValue(0x00); ui->cbVisibleBuggy->setChecked(false); ui->cbVisibleHighwind->setChecked(false); break;
        case 1: ui->sbBhId->setValue(0x06); ui->cbVisibleBuggy->setChecked(true); break; //buggy
        case 2: ui->sbBhId->setValue(0x03); ui->cbVisibleHighwind->setChecked(true); break; //highwind
        default: break;
        }
    }
}
void MainWindow::cbVisibleBuggy_toggled(bool checked)
{
    if (!load) {
        ff7->setWorldVehicle(s, FF7Save::WVEHCILE_BUGGY, checked);
        if (checked) {
            if (ui->cbVisibleHighwind->isChecked())
                ui->cbVisibleHighwind->setChecked(false);
            load = true;
            ui->comboHighwindBuggy->setCurrentIndex(1);
            ui->sbBhId->setValue(0x06);
            load = false;
        } else {
            if (!ui->cbVisibleBuggy->isChecked()) {
                load = true;
                ui->comboHighwindBuggy->setCurrentIndex(0);
                ui->sbBhId->setValue(0x00);
                load = false;
            }
        }

    }
}
void MainWindow::cbVisibleBronco_toggled(bool checked)
{
    if (!load)
        ff7->setWorldVehicle(s, FF7Save::WVEHCILE_TBRONCO, checked);
}
void MainWindow::cbVisibleHighwind_toggled(bool checked)
{
    if (!load) {
        ff7->setWorldVehicle(s, FF7Save::WVEHCILE_HIGHWIND, checked);
        if (checked) {
            if (ui->cbVisibleBuggy->isChecked()) {
                ui->cbVisibleBuggy->setChecked(false);
            }
            load = true;
            ui->comboHighwindBuggy->setCurrentIndex(2);
            ui->sbBhId->setValue(0x03);
            load = false;
        } else {
            if (!ui->cbVisibleHighwind->isChecked()) {
                load = true;
                ui->comboHighwindBuggy->setCurrentIndex(0);
                ui->sbBhId->setValue(0x00);
                load = false;
            }
        }
    }
}
void MainWindow::cbVisibleWildChocobo_toggled(bool checked)
{
    if (!load)
        ff7->setWorldChocobo(s, FF7Save::WCHOCO_WILD, checked);

    if (!checked) {
        ui->cbVisibleYellowChocobo->setChecked(false);
        ui->cbVisibleGreenChocobo->setChecked(false);
        ui->cbVisibleBlueChocobo->setChecked(false);
        ui->cbVisibleBlackChocobo->setChecked(false);
        ui->cbVisibleGoldChocobo->setChecked(false);
    }
    ui->cbVisibleYellowChocobo->setEnabled(checked);
    ui->cbVisibleGreenChocobo->setEnabled(checked);
    ui->cbVisibleBlueChocobo->setEnabled(checked);
    ui->cbVisibleBlackChocobo->setEnabled(checked);
    ui->cbVisibleGoldChocobo->setEnabled(checked);

}
void MainWindow::cbVisibleYellowChocobo_toggled(bool checked)
{
    if (!load) {
        ff7->setWorldChocobo(s, FF7Save::WCHOCO_YELLOW, checked);
        if (checked) {
            ui->cbVisibleGreenChocobo->setChecked(false);
            ui->cbVisibleBlueChocobo->setChecked(false);
            ui->cbVisibleBlackChocobo->setChecked(false);
            ui->cbVisibleGoldChocobo->setChecked(false);
        }
    }
}
void MainWindow::cbVisibleGreenChocobo_toggled(bool checked)
{
    if (!load) {
        ff7->setWorldChocobo(s, FF7Save::WCHOCO_GREEN, checked);
        if (checked) {
            ui->cbVisibleYellowChocobo->setChecked(false);
            ui->cbVisibleBlueChocobo->setChecked(false);
            ui->cbVisibleBlackChocobo->setChecked(false);
            ui->cbVisibleGoldChocobo->setChecked(false);
        }
    }
}
void MainWindow::cbVisibleBlueChocobo_toggled(bool checked)
{
    if (!load) {
        ff7->setWorldChocobo(s, FF7Save::WCHOCO_BLUE, checked);
        if (checked) {
            ui->cbVisibleYellowChocobo->setChecked(false);
            ui->cbVisibleGreenChocobo->setChecked(false);
            ui->cbVisibleBlackChocobo->setChecked(false);
            ui->cbVisibleGoldChocobo->setChecked(false);
        }
    }
}

void MainWindow::cbVisibleBlackChocobo_toggled(bool checked)
{
    if (!load) {
        ff7->setWorldChocobo(s, FF7Save::WCHOCO_BLACK, checked);
        if (checked) {
            ui->cbVisibleYellowChocobo->setChecked(false);
            ui->cbVisibleGreenChocobo->setChecked(false);
            ui->cbVisibleBlueChocobo->setChecked(false);
            ui->cbVisibleGoldChocobo->setChecked(false);
        }
    }
}

void MainWindow::cbVisibleGoldChocobo_toggled(bool checked)
{
    if (!load) {
        ff7->setWorldChocobo(s, FF7Save::WCHOCO_GOLD, checked);
        if (checked) {
            ui->cbVisibleYellowChocobo->setChecked(false);
            ui->cbVisibleGreenChocobo->setChecked(false);
            ui->cbVisibleBlueChocobo->setChecked(false);
            ui->cbVisibleBlackChocobo->setChecked(false);
        }
    }
}
// Leader's world map stuff. 0
void MainWindow::sbLeaderId_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsLeaderID(s, value);
}
void MainWindow::sbLeaderAngle_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsLeaderAngle(s, value);
}
void MainWindow::sbLeaderZ_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsLeaderZ(s, value);
}
void MainWindow::sbLeaderX_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsLeaderX(s, value);
        if (ui->comboMapControls->currentIndex() == 0) {
            load = true;
            ui->slideWorldX->setValue(value);
            load = false;
        }
    }
}

void MainWindow::sbLeaderY_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsLeaderY(s, value);
        if (ui->comboMapControls->currentIndex() == 0) {
            load = true;
            ui->slideWorldY->setValue(value);
            load = false;
        }
    }
}

//Tiny bronco / chocobo world 1
void MainWindow::sbTcId_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsTcID(s, value);
}
void MainWindow::sbTcAngle_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsTcAngle(s, value);
}
void MainWindow::sbTcZ_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsTcZ(s, value);
}
void MainWindow::sbTcX_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsTcX(s, value);
        if (ui->comboMapControls->currentIndex() == 1) {
            load = true;
            ui->slideWorldX->setValue(value);
            load = false;
        }
    }
}
void MainWindow::sbTcY_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsTcY(s, value);
        if (ui->comboMapControls->currentIndex() == 1) {
            load = true;
            ui->slideWorldY->setValue(value);
            load = false;
        }
    }
}

//buggy / highwind world 2
void MainWindow::sbBhId_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsBhID(s, value);
}
void MainWindow::sbBhAngle_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsBhAngle(s, value);
}
void MainWindow::sbBhZ_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsBhZ(s, value);
}
void MainWindow::sbBhX_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsBhX(s, value);
        if (ui->comboMapControls->currentIndex() == 2) {
            load = true;
            ui->slideWorldX->setValue(value);
            load = false;
        }
    }
}
void MainWindow::sbBhY_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsBhY(s, value);
        if (ui->comboMapControls->currentIndex() == 2) {
            load = true;
            ui->slideWorldY->setValue(value);
            load = false;
        }
    }
}
// sub world 3
void MainWindow::sbSubId_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsSubID(s, value);
}
void MainWindow::sbSubAngle_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsSubAngle(s, value);
}
void MainWindow::sbSubZ_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsSubZ(s, value);
}
void MainWindow::sbSubX_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsSubX(s, value);
        if (ui->comboMapControls->currentIndex() == 3) {
            load = true;
            ui->slideWorldX->setValue(value);
            load = false;
        }
    }
}
void MainWindow::sbSubY_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsSubY(s, value);
        if (ui->comboMapControls->currentIndex() == 3) {
            load = true;
            ui->slideWorldY->setValue(value);
            load = false;
        }
    }
}

//Wild Chocobo 4
void MainWindow::sbWcId_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsWchocoID(s, value);
}
void MainWindow::sbWcAngle_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsWchocoAngle(s, value);
}
void MainWindow::sbWcZ_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsWchocoZ(s, value);
}
void MainWindow::sbWcX_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsWchocoX(s, value);
        if (ui->comboMapControls->currentIndex() == 4) {
            load = true;
            ui->slideWorldX->setValue(value);
            load = false;
        }
    }
}
void MainWindow::sbWcY_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsWchocoY(s, value);
        if (ui->comboMapControls->currentIndex() == 4) {
            load = true;
            ui->slideWorldY->setValue(value);
            load = false;
        }
    }
}

//Ruby world stuff 5
void MainWindow::sbDurwId_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsDurwID(s, value);
}
void MainWindow::sbDurwAngle_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsDurwAngle(s, value);
}
void MainWindow::sbDurwZ_valueChanged(int value)
{
    if (!load)
        ff7->setWorldCoordsDurwZ(s, value);
}
void MainWindow::sbDurwX_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsDurwX(s, value);
        if (ui->comboMapControls->currentIndex() == 5) {
            load = true;
            ui->slideWorldX->setValue(value);
            load = false;
        }
    }
}
void MainWindow::sbDurwY_valueChanged(int value)
{
    if (!load) {
        ff7->setWorldCoordsDurwY(s, value);
        if (ui->comboMapControls->currentIndex() == 5) {
            load = true;
            ui->slideWorldY->setValue(value);
            load = false;
        }
    }
}
void MainWindow::comboMapControls_currentIndexChanged(int index)
{
    load = true;
    switch (index) {
    case 0: ui->slideWorldX->setValue(ff7->worldCoordsLeaderX(s)); ui->slideWorldY->setValue(ff7->worldCoordsLeaderY(s));   break;
    case 1: ui->slideWorldX->setValue(ff7->worldCoordsTcX(s));     ui->slideWorldY->setValue(ff7->worldCoordsTcY(s));       break;
    case 2: ui->slideWorldX->setValue(ff7->worldCoordsBhX(s));     ui->slideWorldY->setValue(ff7->worldCoordsBhY(s));       break;
    case 3: ui->slideWorldX->setValue(ff7->worldCoordsSubX(s));    ui->slideWorldY->setValue(ff7->worldCoordsSubY(s));      break;
    case 4: ui->slideWorldX->setValue(ff7->worldCoordsWchocoX(s));  ui->slideWorldY->setValue(ff7->worldCoordsWchocoY(s));   break;
    case 5: ui->slideWorldX->setValue(ff7->worldCoordsDurwX(s));   ui->slideWorldY->setValue(ff7->worldCoordsDurwY(s));     break;
    }
    load = false;
}

void MainWindow::slideWorldX_valueChanged(int value)
{
    if (!load) {
        fileModified(true);
        switch (ui->comboMapControls->currentIndex()) {
        case 0: ui->sbLeaderX->setValue(value);  break;
        case 1: ui->sbTcX->setValue(value);      break;
        case 2: ui->sbBhX->setValue(value);      break;
        case 3: ui->sbSubX->setValue(value);     break;
        case 4: ui->sbWcX->setValue(value);      break;
        case 5: ui->sbDurwX->setValue(value);    break;
        }
    }
}

void MainWindow::slideWorldY_valueChanged(int value)
{
    if (!load) {
        fileModified(true);
        switch (ui->comboMapControls->currentIndex()) {
        case 0: ui->sbLeaderY->setValue(value);  break;
        case 1: ui->sbTcY->setValue(value);      break;
        case 2: ui->sbBhY->setValue(value);      break;
        case 3: ui->sbSubY->setValue(value);     break;
        case 4: ui->sbWcY->setValue(value);      break;
        case 5: ui->sbDurwY->setValue(value);    break;
        }
    }
}

void MainWindow::worldMapView_customContextMenuRequested(QPoint pos)
{
    //Need to create a Paint System Here To put Dots where Chars Are Placed.
    QMenu menu(this);
    QAction *sel;
    menu.addAction(tr("&Place Leader"));
    menu.addAction(tr("Place &Tiny Bronco/Chocobo"));
    menu.addAction(tr("Place &Buggy/Highwind"));
    menu.addAction(tr("Place &Sub"));
    menu.addAction(tr("Place &Wild Chocobo"));
    menu.addAction(tr("Place &Diamond/Ultimate/Ruby Weapon"));
    /* Do Nothing. Don't know emerald weapon Coords
    menu.addAction(tr("Place Emerald Weapon?"));
    */
    sel = menu.exec(ui->worldMapView->mapToGlobal(pos));
    if (!sel)
        return;

    fileModified(true);
    if (sel->text() == tr("&Place Leader")) {
        ui->sbLeaderX->setValue(pos.x() * (295000 / ui->worldMapView->width()));
        ui->sbLeaderY->setValue(pos.y() * (230000 / ui->worldMapView->height()));
    } else if (sel->text() == tr("Place &Tiny Bronco/Chocobo")) {
        ui->sbTcX->setValue(pos.x() * (295000 / ui->worldMapView->width()));
        ui->sbTcY->setValue(pos.y() * (230000 / ui->worldMapView->height()));
    } else if (sel->text() == tr("Place &Buggy/Highwind")) {
        ui->sbBhX->setValue(pos.x() * (295000 / ui->worldMapView->width()));
        ui->sbBhY->setValue(pos.y() * (230000 / ui->worldMapView->height()));
    } else if (sel->text() == tr("Place &Sub")) {
        ui->sbSubX->setValue(pos.x() * (295000 / ui->worldMapView->width()));
        ui->sbSubY->setValue(pos.y() * (230000 / ui->worldMapView->height()));
    } else if (sel->text() == tr("Place &Wild Chocobo")) {
        ui->sbWcX->setValue(pos.x() * (295000 / ui->worldMapView->width()));
        ui->sbWcY->setValue(pos.y() * (230000 / ui->worldMapView->height()));
    } else if (sel->text() == tr("Place &Diamond/Ultimate/Ruby Weapon")) {
        ui->sbDurwX->setValue(pos.x() * (295000 / ui->worldMapView->width()));
        ui->sbDurwY->setValue(pos.y() * (230000 / ui->worldMapView->height()));
    } else {
        return;
    }
}//End Of Map Context Menu

void MainWindow::btnAddAllItems_clicked()
{
    ui->btnRemoveAllItems->click();
    for (int i = 0; i < 320; i++) {
        //Replaced by new item engine. (Vegeta_Ss4)
        if (FF7Item::instance()->name(i) != tr("DON'T USE")) {
            if (i < 106)
                ff7->setItem(s, i, quint16(i), 127);
            else // after the block of empty items shift up 23 spots.
                ff7->setItem(s, (i - 23), quint16(i), 127);
        } else {
            ff7->setItem(s, i, 0x1FF, 0x7F);   //exclude the test items
        }
        if (i > 296)
            ff7->setItem(s, i, 0x1FF, 0x7F);   //replace the shifted ones w/ empty slots
    }
    itemlist->setItems(ff7->items(s));
    statusBar()->showMessage(tr("All Items Added"), 750);
}

void MainWindow::unknown_refresh(int z)//remember to add/remove case statments in all 3 switches when number of z vars changes.
{
    load = true;

    int rows = 0;
    QTableWidgetItem *newItem;
    QByteArray temp, temp2;
    int s2;

    ui->tblUnknown->reset();
    ui->tblCompareUnknown->reset();

    if (z <= ff7->unknown_zmax())
        temp = ff7->unknown(s, z);
    else if (z == ff7->unknown_zmax() + 1)
        temp = ff7->slotFF7Data(s);

    rows = temp.size();

    ui->tblUnknown->setRowCount(rows);
    if (ui->comboCompareSlot->currentIndex() != 0)
        ui->tblCompareUnknown->setRowCount(rows);

    for (int i = 0; i < rows; i++) {
        if (ui->comboZVar->currentText() == "SLOT") {
            QString hex_str = QString("%1").arg(i, 4, 16, QChar('0')).toUpper(); //format ex: 000C
            newItem = new QTableWidgetItem(hex_str, 0);
            ui->tblUnknown->setItem(i, 0, newItem);
        } else {
            newItem = new QTableWidgetItem(QString::number(i), 0);
            ui->tblUnknown->setItem(i, 0, newItem);
        }

        quint8 value = quint8(temp.at(i));

        //Write Hex
        newItem = new QTableWidgetItem(QString("%1").arg(value, 2, 16, QChar('0')).toUpper(), 0);
        newItem->setTextAlignment(Qt::AlignHCenter);
        ui->tblUnknown->setItem(i, 1, newItem);
        //Write Dec
        newItem = new QTableWidgetItem(QString("%1").arg(value, 3, 10, QChar('0')), 0);
        newItem->setTextAlignment(Qt::AlignHCenter);
        ui->tblUnknown->setItem(i, 2, newItem);
        //Write Bin
        newItem = new QTableWidgetItem(QString("%1").arg(value, 8, 2, QChar('0')), 0);
        newItem->setTextAlignment(Qt::AlignHCenter);
        ui->tblUnknown->setItem(i, 3, newItem);
        //Write Char
        newItem = new QTableWidgetItem(QString("%1").arg(QChar(value)), 0);
        newItem->setTextAlignment(Qt::AlignHCenter);
        ui->tblUnknown->setItem(i, 4, newItem);
        //Set Height
        ui->tblUnknown->setRowHeight(i, fontMetrics().height() + 6);

        if (ui->comboCompareSlot->currentIndex() != 0) {
            //do the same for the compare slot if one has been selected.
            if (ui->comboZVar->currentText() == "SLOT") {
                newItem = new QTableWidgetItem(QString("%1").arg(i, 4, 16, QChar('0')).toUpper(), 0);
                ui->tblCompareUnknown->setItem(i, 0, newItem);
            } else {
                newItem = new QTableWidgetItem(QString::number(i), 0);
                newItem->setTextAlignment(Qt::AlignHCenter);
                ui->tblCompareUnknown->setItem(i, 0, newItem);
            }

            s2 = ui->comboCompareSlot->currentIndex() - 1;
            if (z <= ff7->unknown_zmax())
                temp2 = ff7->unknown(s2, z);
            else if (z == ff7->unknown_zmax() + 1)
                temp2 = ff7->slotFF7Data(s2);
            value = quint8(temp2.at(i));

            //Write Hex
            newItem = new QTableWidgetItem(QString("%1").arg(value, 2, 16, QChar('0')).toUpper(), 0);
            newItem->setTextAlignment(Qt::AlignHCenter);
            ui->tblCompareUnknown->setItem(i, 1, newItem);
            //Write Dec
            newItem = new QTableWidgetItem(QString("%1").arg(value, 3, 10, QChar('0')), 0);
            newItem->setTextAlignment(Qt::AlignHCenter);
            ui->tblCompareUnknown->setItem(i, 2, newItem);
            //Write Bin
            newItem = new QTableWidgetItem(QString("%1").arg(value, 8, 2, QChar('0')), 0);
            newItem->setTextAlignment(Qt::AlignHCenter);
            ui->tblCompareUnknown->setItem(i, 3, newItem);
            //Write Char
            newItem = new QTableWidgetItem(QChar(value), 0);
            newItem->setTextAlignment(Qt::AlignHCenter);
            ui->tblCompareUnknown->setItem(i, 4, newItem);

            ui->tblCompareUnknown->setRowHeight(i, fontMetrics().height() +6);
            if (ui->tblCompareUnknown->item(i, 1)->text() != ui->tblUnknown->item(i, 1)->text()) {
                for (int c = 0; c < 5; c++) {
                    //color the diffs ;)
                    ui->tblCompareUnknown->item(i, c)->setBackground(Qt::yellow);
                    ui->tblCompareUnknown->item(i, c)->setForeground(Qt::red);
                    ui->tblUnknown->item(i, c)->setBackground(Qt::yellow);
                    ui->tblUnknown->item(i, c)->setForeground(Qt::red);
                }
            }
        }
    }
    for (int i = 0; i < rows; i++) { //set up the item flags
        ui->tblUnknown->item(i, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->tblUnknown->item(i, 1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        ui->tblUnknown->item(i, 2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        ui->tblUnknown->item(i, 3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        ui->tblUnknown->item(i, 4)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        if (ui->comboCompareSlot->currentIndex() != 0) {
            ui->tblCompareUnknown->item(i, 0)->setFlags(Qt::ItemIsEnabled);
            ui->tblCompareUnknown->item(i, 1)->setFlags(Qt::ItemIsEnabled);
            ui->tblCompareUnknown->item(i, 2)->setFlags(Qt::ItemIsEnabled);
            ui->tblCompareUnknown->item(i, 3)->setFlags(Qt::ItemIsEnabled);
            ui->tblCompareUnknown->item(i, 4)->setFlags(Qt::ItemIsEnabled);
        }
    }
    load = false;
}

void MainWindow::comboZVar_currentIndexChanged(int z)
{
    unknown_refresh(z);
}

void MainWindow::comboCompareSlot_currentIndexChanged(int index)
{
    if (index == 0) {
        ui->tblCompareUnknown->clearContents();
        ui->tblCompareUnknown->setRowCount(0);
    } else
        unknown_refresh(ui->comboZVar->currentIndex());
}

void MainWindow::tblUnknown_itemChanged(QTableWidgetItem *item)
{
    if (!load) {
        QByteArray temp;

        int z = ui->comboZVar->currentIndex();
        if (z <= ff7->unknown_zmax())
            temp = ff7->unknown(s, z);
        else if (z == ff7->unknown_zmax() + 1)
            temp = ff7->slotFF7Data(s);

        switch (item->column()) {
        case 1: temp[item->row()] = char(item->text().toInt(nullptr, 16));  break;
        case 2: temp[item->row()] = char(item->text().toInt());      break;
        case 3: temp[item->row()] = char(item->text().toInt(nullptr, 2));   break;
        }

        if (z <= ff7->unknown_zmax())
            ff7->setUnknown(s, z, temp);
        else if (z == ff7->unknown_zmax() + 1)
            ff7->setSlotFF7Data(s, temp);

        unknown_refresh(z);
    }
}

void MainWindow::comboS7Slums_currentIndexChanged(int index)
{
    if (!load) {
        QByteArray temp(ff7->unknown(s, 26));
        switch (index) {
        default: break; //do nothing
        case 1: //initial slums setting
            temp.replace(0, 6, "\x00\x00\x00\x00\x00\x00");
            break;

        case 2://after first scene. needs game global progress set to 105
            temp.replace(0, 6, "\xBF\x03\x05\x17\x5D\xEF");
            break;

        case 3://plate falling
            temp.replace(0, 6, "\xBF\x13\x05\x17\x5D\xEF");
            break;
        }
        ff7->setUnknown(s, 26, temp);
    }
}

void MainWindow::char_materia_changed(materia mat)
{
    if (!load) {} ff7->setCharMateria(s, curchar, mslotsel, mat);
}
void MainWindow::char_accessory_changed(quint8 accessory)
{
    ff7->setCharAccessory(s, curchar, accessory);
}
void MainWindow::char_armor_changed(quint8 armor)
{
    ff7->setCharArmor(s, curchar, armor);
}
void MainWindow::char_baseHp_changed(quint16 hp)
{
    ff7->setCharBaseHp(s, curchar, hp);
}
void MainWindow::char_baseMp_changed(quint16 mp)
{
    ff7->setCharBaseMp(s, curchar, mp);
}
void MainWindow::char_curHp_changed(quint16 hp)
{
    ff7->setCharCurrentHp(s, curchar, hp);
    if (curchar == ff7->party(s, 0))
        ff7->setDescCurHP(s, hp);
}
void MainWindow::char_curMp_changed(quint16 mp)
{
    ff7->setCharCurrentMp(s, curchar, mp);
    if (curchar == ff7->party(s, 0))
        ff7->setDescCurMP(s, mp);
}
void MainWindow::char_id_changed(qint8 id)
{
    ff7->setCharID(s, curchar, id);
    set_char_buttons();
}
void MainWindow::char_level_changed(qint8 level)
{
    ff7->setCharLevel(s, curchar, level);
    if (curchar == ff7->party(s, 0))
        ff7->setDescLevel(s, level);
}
void MainWindow::char_str_changed(quint8 str)
{
    ff7->setCharStr(s, curchar, str);
}
void MainWindow::char_vit_changed(quint8 vit)
{
    ff7->setCharVit(s, curchar, vit);
}
void MainWindow::char_mag_changed(quint8 mag)
{
    ff7->setCharMag(s, curchar, mag);
}
void MainWindow::char_spi_changed(quint8 spi)
{
    ff7->setCharSpi(s, curchar, spi);
}
void MainWindow::char_dex_changed(quint8 dex)
{
    ff7->setCharDex(s, curchar, dex);
}
void MainWindow::char_lck_changed(quint8 lck)
{
    ff7->setCharLck(s, curchar, lck);
}
void MainWindow::char_strBonus_changed(quint8 value)
{
    ff7->setCharStrBonus(s, curchar, value);
}
void MainWindow::char_vitBonus_changed(quint8 value)
{
    ff7->setCharVitBonus(s, curchar, value);
}
void MainWindow::char_magBonus_changed(quint8 value)
{
    ff7->setCharMagBonus(s, curchar, value);
}
void MainWindow::char_spiBonus_changed(quint8 value)
{
    ff7->setCharSpiBonus(s, curchar, value);
}
void MainWindow::char_dexBonus_changed(quint8 value)
{
    ff7->setCharDexBonus(s, curchar, value);
}
void MainWindow::char_lckBonus_changed(quint8 value)
{
    ff7->setCharLckBonus(s, curchar, value);
}
void MainWindow::char_limitLevel_changed(qint8 value)
{
    ff7->setCharLimitLevel(s, curchar, value);
}
void MainWindow::char_limitBar_changed(quint8 value)
{
    ff7->setCharLimitBar(s, curchar, value);
}
void MainWindow::char_weapon_changed(quint8 value)
{
    ff7->setCharWeapon(s, curchar, value);
}
void MainWindow::char_kills_changed(quint16 value)
{
    ff7->setCharKills(s, curchar, value);
}
void MainWindow::char_row_changed(quint8 value)
{
    ff7->setCharFlag(s, curchar, 1, value);
}
void MainWindow::char_levelProgress_changed(quint8 value)
{
    ff7->setCharFlag(s, curchar, 2, value);
}
void MainWindow::char_sadnessfury_changed(quint8 value)
{
    ff7->setCharFlag(s, curchar, 0, value);
}
void MainWindow::char_limits_changed(quint16 value)
{
    ff7->setCharLimits(s, curchar, value);
}
void MainWindow::char_timesused1_changed(quint16 value)
{
    ff7->setCharTimeLimitUsed(s, curchar, 1, value);
}
void MainWindow::char_timeused2_changed(quint16 value)
{
    ff7->setCharTimeLimitUsed(s, curchar, 2, value);
}
void MainWindow::char_timeused3_changed(quint16 value)
{
    ff7->setCharTimeLimitUsed(s, curchar, 3, value);
}
void MainWindow::char_exp_changed(quint32 value)
{
    ff7->setCharCurrentExp(s, curchar, value);
}
void MainWindow::char_expNext_changed(quint32 value)
{
    ff7->setCharNextExp(s, curchar, value);
}
void MainWindow::char_mslot_changed(int slot)
{
    mslotsel = slot;
}

void MainWindow::char_name_changed(QString name)
{
    ff7->setCharName(s, curchar, name);
    if (curchar == ff7->party(s, 0))
        ff7->setDescName(s, name);
}

void MainWindow::char_maxHp_changed(quint16 value)
{
    ff7->setCharMaxHp(s, curchar, value);
    if (curchar == ff7->party(s, 0))
        ff7->setDescMaxHP(s, value);
}
void MainWindow::char_maxMp_changed(quint16 value)
{
    ff7->setCharMaxMp(s, curchar, value);
    if (curchar == ff7->party(s, 0))
        ff7->setDescMaxMP(s, value);
}

void MainWindow::btnMaxChar_clicked()
{
    if (ff7->charID(s, curchar) == FF7Char::YoungCloud || ff7->charID(s, curchar) == FF7Char::Sephiroth  ||  _init)
        return;   //no char selected, sephiroth and young cloud.

    int result = QMessageBox::question(this, tr("Black Chocobo"), tr("Do You Want To Also Replace %1's Equipment and Materia?").arg(ff7->charName(s, curchar)), QMessageBox::Yes, QMessageBox::No);
    switch (result) {
    case QMessageBox::Yes: char_editor->MaxEquip(); char_editor->MaxStats(); break;
    case QMessageBox::No: char_editor->MaxStats(); break;
    }
    switch (curchar) {
    case 0: btnCloud_clicked(); break;
    case 1: btnBarret_clicked(); break;
    case 2: btnTifa_clicked(); break;
    case 3: btnAeris_clicked(); break;
    case 4: btnRed_clicked(); break;
    case 5: btnYuffie_clicked(); break;
    case 6: btnCait_clicked(); break;
    case 7: btnVincent_clicked(); break;
    case 8: btnCid_clicked(); break;
    }

}
void MainWindow::Items_Changed(QList<quint16> items)
{
    ff7->setItems(s, items);
}
void MainWindow::sbSnowBegScore_valueChanged(int value)
{
    if (!load)
        ff7->setSnowboardScore(s, 0, quint8(value));
}
void MainWindow::sbSnowExpScore_valueChanged(int value)
{
    if (!load)
        ff7->setSnowboardScore(s, 1, quint8(value));
}
void MainWindow::sbSnowCrazyScore_valueChanged(int value)
{
    if (!load)
        ff7->setSnowboardScore(s, 2, quint8(value));
}

void MainWindow::sbSnowBegMin_valueChanged(int value)
{
    if (!load) {
        QString time = ff7->snowboardTime(s, 0);
        time.replace(0, 2, QString("%1").arg(value, 2, 10, QChar('0')));
        ff7->setSnowboardTime(s, 0, time);
    }
}

void MainWindow::sbSnowBegSec_valueChanged(int value)
{
    if (!load) {
        QString time = ff7->snowboardTime(s, 0);
        time.replace(2, 2, QString("%1").arg(value, 2, 10, QChar('0')));
        ff7->setSnowboardTime(s, 0, time);
    }
}

void MainWindow::sbSnowBegMsec_valueChanged(int value)
{
    if (!load) {
        QString time = ff7->snowboardTime(s, 0);
        time.replace(4, 3, QString("%1").arg(value, 3, 10, QChar('0')));
        ff7->setSnowboardTime(s, 0, time);
    }
}

void MainWindow::sbSnowExpMin_valueChanged(int value)
{
    if (!load) {
        QString time = ff7->snowboardTime(s, 1);
        time.replace(0, 2, QString("%1").arg(value, 2, 10, QChar('0')));
        ff7->setSnowboardTime(s, 1, time);
    }
}

void MainWindow::sbSnowExpSec_valueChanged(int value)
{
    if (!load) {
        QString time = ff7->snowboardTime(s, 1);
        time.replace(2, 2, QString("%1").arg(value, 2, 10, QChar('0')));
        ff7->setSnowboardTime(s, 1, time);
    }
}

void MainWindow::sbSnowExpMsec_valueChanged(int value)
{
    if (!load) {
        QString time = ff7->snowboardTime(s, 1);
        time.replace(4, 3, QString("%1").arg(value, 3, 10, QChar('0')));
        ff7->setSnowboardTime(s, 1, time);
    }
}

void MainWindow::sbSnowCrazyMin_valueChanged(int value)
{
    if (!load) {
        QString time = ff7->snowboardTime(s, 2);
        time.replace(0, 2, QString("%1").arg(value, 2, 10, QChar('0')));
        ff7->setSnowboardTime(s, 2, time);
    }
}

void MainWindow::sbSnowCrazySec_valueChanged(int value)
{
    if (!load) {
        QString time = ff7->snowboardTime(s, 2);
        time.replace(2, 2, QString("%1").arg(value, 2, 10, QChar('0')));
        ff7->setSnowboardTime(s, 2, time);
    }
}

void MainWindow::sbSnowCrazyMsec_valueChanged(int value)
{
    if (!load) {
        QString time = ff7->snowboardTime(s, 2);
        time.replace(4, 3, QString("%1").arg(value, 3, 10, QChar('0')));
        ff7->setSnowboardTime(s, 2, time);
    }
}
void MainWindow::sbBikeHighScore_valueChanged(int arg1)
{
    if (!load)
        ff7->setBikeHighScore(s, quint16(arg1));
}
void MainWindow::sbBattlePoints_valueChanged(int arg1)
{
    if (!load)
        ff7->setBattlePoints(s, quint16(arg1));
}

void MainWindow::comboHexEditor_currentIndexChanged(int index)
{
    hexTabUpdate(index);
}

void MainWindow::hexEditorChanged(void)
{
    if (FF7SaveInfo::instance()->isTypePC(ff7->format())) {
        ff7->setSlotFF7Data(s, hexEditor->data());
    } else {
        switch (ui->comboHexEditor->currentIndex()) {
        case 0:
            ff7->setSlotPsxRawData(s, hexEditor->data());
            update_hexEditor_PSXInfo();
            break;
        case 1:
            ff7->setSlotFF7Data(s, hexEditor->data());
            break;
        }
    }
    fileModified(true);
}

void MainWindow::phsList_box_allowed_toggled(int row, bool checked)
{
    if (!load)
        ff7->setPhsAllowed(s, row, !checked);
}
void MainWindow::phsList_box_visible_toggled(int row, bool checked)
{
    if (!load)
        ff7->setPhsVisible(s, row, checked);
}
void MainWindow::menuList_box_locked_toggled(int row, bool checked)
{
    if (!load)
        ff7->setMenuLocked(s, row, checked);
}
void MainWindow::menuList_box_visible_toggled(int row, bool checked)
{
    if (!load)
        ff7->setMenuVisible(s, row, checked);
}

void MainWindow::locationToolBox_currentChanged(int index)
{
    //LocationTabs
    load = true;
    switch (index) {
    case 0:
        locationViewer->setX(ff7->locationX(s));
        locationViewer->setY(ff7->locationY(s));
        locationViewer->setT(ff7->locationT(s));
        locationViewer->setD(ff7->locationD(s));
        locationViewer->setMapId(ff7->mapId(s));
        locationViewer->setLocationId(ff7->locationId(s));
        locationViewer->setLocationString(ff7->location(s));
        locationViewer->init_fieldItems();
        break;

    case 1:
        ui->cbVisibleBuggy->setChecked(ff7->worldVehicle(s, FF7Save::WVEHCILE_BUGGY));
        ui->cbVisibleBronco->setChecked(ff7->worldVehicle(s, FF7Save::WVEHCILE_TBRONCO));
        ui->cbVisibleHighwind->setChecked(ff7->worldVehicle(s, FF7Save::WVEHCILE_HIGHWIND));
        ui->cbVisibleWildChocobo->setChecked(ff7->worldChocobo(s, FF7Save::WCHOCO_WILD));
        ui->cbVisibleYellowChocobo->setChecked(ff7->worldChocobo(s, FF7Save::WCHOCO_YELLOW));
        ui->cbVisibleGreenChocobo->setChecked(ff7->worldChocobo(s, FF7Save::WCHOCO_GREEN));
        ui->cbVisibleBlueChocobo->setChecked(ff7->worldChocobo(s, FF7Save::WCHOCO_BLUE));
        ui->cbVisibleBlackChocobo->setChecked(ff7->worldChocobo(s, FF7Save::WCHOCO_BLACK));
        ui->cbVisibleGoldChocobo->setChecked(ff7->worldChocobo(s, FF7Save::WCHOCO_GOLD));

        switch (ui->comboMapControls->currentIndex()) {
        case 0: ui->slideWorldX->setValue(ff7->worldCoordsLeaderX(s));
            ui->slideWorldY->setValue(ff7->worldCoordsLeaderY(s));
            break;
        case 1: ui->slideWorldX->setValue(ff7->worldCoordsTcX(s));
            ui->slideWorldY->setValue(ff7->worldCoordsTcY(s));
            break;
        case 2:  ui->slideWorldX->setValue(ff7->worldCoordsBhX(s));
            ui->slideWorldY->setValue(ff7->worldCoordsBhY(s));
            break;
        case 3: ui->slideWorldX->setValue(ff7->worldCoordsSubX(s));
            ui->slideWorldY->setValue(ff7->worldCoordsSubY(s));
            break;
        case 4: ui->slideWorldX->setValue(ff7->worldCoordsWchocoX(s));
            ui->slideWorldY->setValue(ff7->worldCoordsWchocoY(s));
            break;
        case 5: ui->slideWorldX->setValue(ff7->worldCoordsDurwX(s));
            ui->slideWorldY->setValue(ff7->worldCoordsDurwY(s));
            break;
        }
        //WORLD TAB
        ui->sbLeaderId->setValue(ff7->worldCoordsLeaderID(s));
        ui->sbLeaderX->setValue(ff7->worldCoordsLeaderX(s));
        ui->sbLeaderAngle->setValue(ff7->worldCoordsLeaderAngle(s));
        ui->sbLeaderY->setValue(ff7->worldCoordsLeaderY(s));
        ui->sbLeaderZ->setValue(ff7->worldCoordsLeaderZ(s));

        ui->sbDurwX->setValue(ff7->worldCoordsDurwX(s));
        ui->sbDurwId->setValue(ff7->worldCoordsDurwID(s));
        ui->sbDurwAngle->setValue(ff7->worldCoordsDurwAngle(s));
        ui->sbDurwY->setValue(ff7->worldCoordsDurwY(s));
        ui->sbDurwZ->setValue(ff7->worldCoordsDurwZ(s));

        ui->sbWcX->setValue(ff7->worldCoordsWchocoX(s));
        ui->sbWcId->setValue(ff7->worldCoordsWchocoID(s));
        ui->sbWcAngle->setValue(ff7->worldCoordsWchocoAngle(s));
        ui->sbWcY->setValue(ff7->worldCoordsWchocoY(s));
        ui->sbWcZ->setValue(ff7->worldCoordsWchocoZ(s));

        ui->sbTcX->setValue(ff7->worldCoordsTcX(s));
        ui->sbTcId->setValue(ff7->worldCoordsTcID(s));
        ui->sbTcAngle->setValue(ff7->worldCoordsTcAngle(s));
        ui->sbTcY->setValue(ff7->worldCoordsTcY(s));
        ui->sbTcZ->setValue(ff7->worldCoordsTcZ(s));

        ui->sbBhX->setValue(ff7->worldCoordsBhX(s));
        ui->sbBhId->setValue(ff7->worldCoordsBhID(s));

        switch (ui->sbBhId->value()) {
        case 0: ui->comboHighwindBuggy->setCurrentIndex(0); break; //empty
        case 6: ui->comboHighwindBuggy->setCurrentIndex(1); break; //buggy
        case 3: ui->comboHighwindBuggy->setCurrentIndex(2); break; //highwind
        default: QMessageBox::information(this, tr("Black Chocobo"), tr("Unknown Id in Buggy/Highwind Location")); break;
        }

        ui->sbBhAngle->setValue(ff7->worldCoordsBhAngle(s));
        ui->sbBhY->setValue(ff7->worldCoordsBhY(s));
        ui->sbBhZ->setValue(ff7->worldCoordsBhZ(s));

        ui->sbSubX->setValue(ff7->worldCoordsSubX(s));
        ui->sbSubId->setValue(ff7->worldCoordsSubID(s));
        ui->sbSubAngle->setValue(ff7->worldCoordsSubAngle(s));
        ui->sbSubY->setValue(ff7->worldCoordsSubY(s));
        ui->sbSubZ->setValue(ff7->worldCoordsSubZ(s));
        break;
    }
    load = false;
}

void MainWindow::testDataTabWidget_currentChanged(int index)
{

    switch (index) {
    case 0:
        load = true;
        ui->sbBloveAeris->setValue(ff7->love(s, true, FF7Save::LOVE_AERIS));
        ui->sbBloveTifa->setValue(ff7->love(s, true, FF7Save::LOVE_TIFA));
        ui->sbBloveYuffie->setValue(ff7->love(s, true, FF7Save::LOVE_YUFFIE));
        ui->sbBloveBarret->setValue(ff7->love(s, true, FF7Save::LOVE_BARRET));
        ui->sbUweaponHp->setValue(int(ff7->uWeaponHp(s)));
        ui->cbTutSub->setChecked(ff7->tutSub(s, 2));

        ui->lcdTutSub->display(ff7->tutSub(s));

        if (ff7->tutSave(s) == 0x3A)
            ui->cbTutWorldSave->setCheckState(Qt::Checked);
        else if (ff7->tutSave(s) == 0x32)
            ui->cbTutWorldSave->setCheckState(Qt::PartiallyChecked);
        else
            ui->cbTutWorldSave->setCheckState(Qt::Unchecked);

        ui->lcdNumber_7->display(ff7->tutSave(s));

        ui->cbRegYuffie->setChecked(ff7->yuffieUnlocked(s));
        ui->cbRegVinny->setChecked(ff7->vincentUnlocked(s));

        ui->sbSaveMapId->setValue(ff7->craterSavePointMapID(s));
        ui->sbSaveX->setValue(ff7->craterSavePointX(s));
        ui->sbSaveY->setValue(ff7->craterSavePointY(s));
        ui->sbSaveZ->setValue(ff7->craterSavePointZ(s));

        ui->lbl_sg_region->setText(ff7->region(s).mid(0, ff7->region(s).lastIndexOf("-") + 1));
        ui->comboRegionSlot->setCurrentIndex(ff7->region(s).midRef(ff7->region(s).lastIndexOf("S") + 1, 2).toInt() - 1);
        if (!FF7SaveInfo::instance()->isTypePC(ff7->format()) && ff7->format() != FF7SaveInfo::FORMAT::UNKNOWN) //we Display an icon. or all formats except for pc and switch.
            ui->lbl_slot_icon->setPixmap(SaveIcon(ff7->slotIcon(s)).icon().scaled(ui->lbl_slot_icon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        load = false;
        break;

    case 1: unknown_refresh(ui->comboZVar->currentIndex()); break;
    }
}

void MainWindow::sbCondorFunds_valueChanged(int arg1)
{
    if (!load)
        ff7->setCondorFunds(s, quint16(arg1));
}
void MainWindow::sbCondorWins_valueChanged(int arg1)
{
    if (!load)
        ff7->setCondorWins(s, quint8(arg1));
}
void MainWindow::sbCondorLosses_valueChanged(int arg1)
{
    if (!load)
        ff7->setCondorLosses(s, quint8(arg1));
}
void MainWindow::cbPandorasBox_toggled(bool checked)
{
    if (!load)
        ff7->setSeenPandorasBox(s, checked);
}
void MainWindow::cbSubGameWon_toggled(bool checked)
{
    if (!load)
        ff7->setSubMiniGameVictory(s, checked);
}

void MainWindow::connectFieldItem(quint8 boxID, QList<quint16>Offset, QList<quint8> Bit)
{
    if (boxID == 0) {
        //if box is 0 then new list.
        fieldItemOffset = new QList<fieldItemOffsetList>;
        fieldItemBit = new QList<fieldItemBitList>;
    }
    fieldItemOffset->append(Offset);
    fieldItemBit->append(Bit);
}
void MainWindow::checkFieldItem(int boxID)
{
    //Will always be called in numerical Order
    fieldItemOffsetList offsetList = fieldItemOffset->at(boxID);
    fieldItemBitList bitList = fieldItemBit->at(boxID);

    if (offsetList.count() == bitList.count()) {
        bool checked = false;
        bool check1 = false;
        for (int i = 0; i < offsetList.count(); i++) {
            //attempt to cope with multi bits.
            int offset = offsetList.at(i);
            int bit = bitList.at(i);
            if ((ff7->slotFF7Data(s).at(offset) & (1 << bit)))
                check1 = true;
            else
                check1 = false;

            if (i == 0)
                checked = check1;
            else
                checked = (check1 & checked);
        }
        locationViewer->setFieldItemChecked(boxID, checked);
    }
}
void MainWindow::fieldItemStateChanged(int boxID, bool checked)
{
    fieldItemOffsetList offsetList = fieldItemOffset->at(boxID);
    fieldItemBitList bitList = fieldItemBit->at(boxID);
    if (offsetList.count() == bitList.count()) {
        for (int i = 0; i < offsetList.count(); i++) {
            int offset = offsetList.at(i);
            int bit = bitList.at(i);
            QByteArray temp = ff7->slotFF7Data(s); char t = temp.at(offset);
            if (checked)
                t |= (1 << bit);
            else
                t &= ~(1 << bit);

            temp[offset] = t;
            ff7->setSlotFF7Data(s, temp);
        }
    }
}

void MainWindow::sbSaveMapId_valueChanged(int arg1)
{
    if (!load)
        ff7->setCraterSavePointMapID(s, arg1);
}
void MainWindow::sbSaveX_valueChanged(int arg1)
{
    if (!load)
        ff7->setCraterSavePointX(s, arg1);
}
void MainWindow::sbSaveY_valueChanged(int arg1)
{
    if (!load)
        ff7->setCraterSavePointY(s, arg1);
}
void MainWindow::sbSaveZ_valueChanged(int arg1)
{
    if (!load)
        ff7->setCraterSavePointZ(s, arg1);
}

void MainWindow::btnSearchFlyers_clicked()
{
    ui->tabWidget->setCurrentIndex(4);
    ui->locationToolBox->setCurrentIndex(0);
    locationViewer->setFilterString(tr("Turtle Paradise"), LocationViewer::ITEM);
}

void MainWindow::btnSearchKeyItems_clicked()
{
    ui->tabWidget->setCurrentIndex(4);
    ui->locationToolBox->setCurrentIndex(0);
    locationViewer->setFilterString(tr("KeyItem"), LocationViewer::ITEM);
}

void MainWindow::linePsxDesc_textChanged(const QString &arg1)
{
    if (!load) {
        ff7->setPsxDesc(arg1, s);
        update_hexEditor_PSXInfo();
    }
}

void MainWindow::cbFlashbackPiano_toggled(bool checked)
{
    if (!load)
        ff7->setPlayedPianoOnFlashback(s, checked);
}

void MainWindow::setButtonMapping(int controlAction, int newButton)
{
    if (!load)
        ff7->setControllerMapping(s, controlAction, newButton);
}
