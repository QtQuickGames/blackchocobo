/****************************************************************************/
//    copyright 2010-2012 Chris Rizzitello <sithlord48@gmail.com>           //
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

/*~~~~~~~~GUI Set Up~~~~~~~*/
MainWindow::MainWindow(QWidget *parent,FF7Save *ff7data,QSettings *configdata)
    :QMainWindow(parent),ui(new Ui::MainWindow)
{
    this->setAcceptDrops(true);
    //Get Font Info Before Setting up the GUI!
    settings =configdata;
    if(!settings->value("font-size").toString().isEmpty()){QApplication::setFont(QFont(QApplication::font().family(),settings->value("font-size").toInt(),-1,false));}
    if(!settings->value("font-family").toString().isEmpty()){QApplication::setFont(QFont(settings->value("font-family").toString(),QApplication::font().pointSize(),-1,false));}
    ui->setupUi(this);
    _init=true;
    ff7 =ff7data;
    load=true;
    curchar =0;
    mslotsel=-1;
    s=0;
    buffer_materia.id=FF7Materia::EmptyId;
    for(int i=0;i<4;i++){buffer_materia.ap[i]=0xFF;} //empty buffer incase
    if(QResource::registerResource(QApplication::applicationDirPath().append(QString("/locations.rcc")))){showLocPreview=true;}
    else{showLocPreview=false;}
    init_display();
    init_style();
    init_connections();
    init_settings();
    //setup locations previews if found
    on_actionNew_Game_triggered();
    file_modified(false);
}
void MainWindow::init_display()
{
    //set up tables..
    ui->tbl_location_field->setColumnWidth(0,160);
    ui->tbl_location_field->setColumnWidth(1,50);
    ui->tbl_location_field->setColumnWidth(2,50);
    ui->tbl_location_field->setColumnWidth(3,50);
    ui->tbl_location_field->setColumnWidth(4,50);
    ui->tbl_location_field->setColumnWidth(5,50);

    QTableWidgetItem *newItem;
    FF7Location Locations;
    ui->tbl_location_field->setRowCount(Locations.len());
    for (int i=0;i<ui->tbl_location_field->rowCount();i++)
    {
        newItem = new QTableWidgetItem(Locations.locationString(i),0);
        if(showLocPreview)
        {//set the tooltip to the needed file if the locations resource is found.
            QString tooltip(QString("<html><head/><body><p><br>%1<br><img src=\":/locations/%2_%3\"/></p></body></html>").arg(Locations.fileName(i),Locations.mapID(i),Locations.locationID(i)));
            newItem->setToolTip(tooltip);
        }
        ui->tbl_location_field->setItem(i,0,newItem);
        newItem = new QTableWidgetItem(Locations.mapID(i),0);
        newItem->setTextAlignment(Qt::AlignHCenter);
        ui->tbl_location_field->setItem(i,1,newItem);
        newItem = new QTableWidgetItem(Locations.locationID(i),0);
        newItem->setTextAlignment(Qt::AlignHCenter);
        ui->tbl_location_field->setItem(i,2,newItem);
        newItem = new QTableWidgetItem(Locations.x(i),0);
        newItem->setTextAlignment(Qt::AlignHCenter);
        ui->tbl_location_field->setItem(i,3,newItem);
        newItem = new QTableWidgetItem(Locations.y(i),0);
        newItem->setTextAlignment(Qt::AlignHCenter);
        ui->tbl_location_field->setItem(i,4,newItem);
        newItem = new QTableWidgetItem(Locations.z(i),0);
        newItem->setTextAlignment(Qt::AlignHCenter);
        ui->tbl_location_field->setItem(i,5,newItem);
    }

    //Hide the stuff that needs to be hidden.
    ui->compare_table->setEnabled(false);
    ui->tbl_diff->setVisible(0);
    ui->bm_unknown->setVisible(0);
    ui->bh_id->setVisible(0);
    ui->leader_id->setVisible(false);

    //chocobo boxes
    ui->box_stable1->setEnabled(false);
    ui->box_stable2->setEnabled(false);
    ui->box_stable3->setEnabled(false);
    ui->box_stable4->setEnabled(false);
    ui->box_stable5->setEnabled(false);
    ui->box_stable6->setEnabled(false);
    //testing stuff.

    ui->tabWidget->setTabEnabled(9,0);
    ui->cb_Region_Slot->setEnabled(false);
    ui->actionNew_Window->setVisible(0);

    // Temp hidden (show only via debug)
    ui->cb_farm_items_1->setVisible(false);
    ui->cb_farm_items_2->setVisible(false);
    ui->cb_farm_items_3->setVisible(false);
    ui->cb_farm_items_4->setVisible(false);
    ui->cb_farm_items_5->setVisible(false);
    ui->cb_farm_items_6->setVisible(false);
    ui->cb_farm_items_7->setVisible(false);
    ui->cb_farm_items_8->setVisible(false);
    load=false;

    ui->lbl_love_barret->setPixmap(Chars.pixmap(FF7Char::Barret));
    ui->lbl_love_tifa->setPixmap(Chars.pixmap(FF7Char::Tifa));
    ui->lbl_love_aeris->setPixmap(Chars.pixmap(FF7Char::Aerith));
    ui->lbl_love_yuffie->setPixmap(Chars.pixmap(FF7Char::Yuffie));

    ui->lbl_battle_love_barret->setPixmap(Chars.pixmap(FF7Char::Barret));
    ui->lbl_battle_love_tifa->setPixmap(Chars.pixmap(FF7Char::Tifa));
    ui->lbl_battle_love_aeris->setPixmap(Chars.pixmap(FF7Char::Aerith));
    ui->lbl_battle_love_yuffie->setPixmap(Chars.pixmap(FF7Char::Yuffie));

    for(int i=0;i<11;i++){ui->combo_party1->addItem(Chars.icon(i),Chars.defaultName(i));}
    for(int i=0;i<11;i++){ui->combo_party2->addItem(Chars.icon(i),Chars.defaultName(i));}
    for(int i=0;i<11;i++){ui->combo_party3->addItem(Chars.icon(i),Chars.defaultName(i));}
    ui->combo_party1->addItem(QString("0x0B"));
    ui->combo_party1->addItem(tr("-Empty-"));
    ui->combo_party2->addItem(QString("0x0B"));
    ui->combo_party2->addItem(tr("-Empty-"));
    ui->combo_party3->addItem(QString("0x0B"));
    ui->combo_party3->addItem(tr("-Empty-"));

    ui->cb_world_party_leader->addItem(Chars.icon(FF7Char::Cloud),Chars.defaultName(FF7Char::Cloud));
    ui->cb_world_party_leader->addItem(Chars.icon(FF7Char::Tifa),Chars.defaultName(FF7Char::Tifa));
    ui->cb_world_party_leader->addItem(Chars.icon(FF7Char::Cid),Chars.defaultName(FF7Char::Cid));

    phsList = new PhsListWidget;
    QHBoxLayout *phsLayout = new QHBoxLayout;
    phsLayout->addWidget(phsList);
    ui->Phs_Box->setLayout(phsLayout);

    menuList = new MenuListWidget;
    QHBoxLayout *menuLayout = new QHBoxLayout;
    menuLayout->addWidget(menuList);
    ui->Menu_Box->setLayout(menuLayout);

    optionsWidget = new OptionsWidget;
    optionsWidget->setControllerMappingVisible(false);
    ui->tabWidget->insertTab(7,optionsWidget,tr("Game Options"));
    optionsWidget->adjustSize();

    materia_editor = new MateriaEditor(this);
    materia_editor->setStarsSize(48);
    QVBoxLayout *materia_editor_layout = new QVBoxLayout();
    mat_spacer = new QSpacerItem(0,0,QSizePolicy::Preferred,QSizePolicy::MinimumExpanding);
    materia_editor_layout->setContentsMargins(0,6,0,0);
    materia_editor_layout->setSpacing(0);
    materia_editor_layout->addWidget(materia_editor);
    materia_editor_layout->addSpacerItem(mat_spacer);
    ui->group_materia->setLayout(materia_editor_layout);
    ui->group_materia->setContentsMargins(0,6,0,0);

    char_editor = new CharEditor;
    QHBoxLayout *char_editor_layout = new QHBoxLayout;
    char_editor_layout->setSpacing(0);
    char_editor_layout->setContentsMargins(0,0,0,0);
    char_editor_layout->addWidget(char_editor);
    ui->group_char_editor_box->setLayout(char_editor_layout);

    itemlist= new ItemList;
    QHBoxLayout *itemlist_layout = new QHBoxLayout;
    itemlist_layout->setSpacing(0);
    itemlist_layout->setContentsMargins(0,0,0,0);
    itemlist_layout->addWidget(itemlist);
    ui->frm_itemlist->setLayout(itemlist_layout);
    ui->frm_itemlist->adjustSize();

    chocobo_stable_1 = new ChocoboEditor;
    QVBoxLayout *stable_1_layout = new QVBoxLayout;
    stable_1_layout->setContentsMargins(0,0,0,0);
    stable_1_layout->addWidget(chocobo_stable_1);
    ui->box_stable1->setLayout(stable_1_layout);

    chocobo_stable_2 = new ChocoboEditor;
    QVBoxLayout *stable_2_layout = new QVBoxLayout;
    stable_2_layout->setContentsMargins(0,0,0,0);
    stable_2_layout->addWidget(chocobo_stable_2);
    ui->box_stable2->setLayout(stable_2_layout);

    chocobo_stable_3 = new ChocoboEditor;
    QVBoxLayout *stable_3_layout = new QVBoxLayout;
    stable_3_layout->setContentsMargins(0,0,0,0);
    stable_3_layout->addWidget(chocobo_stable_3);
    ui->box_stable3->setLayout(stable_3_layout);

    chocobo_stable_4 = new ChocoboEditor;
    QVBoxLayout *stable_4_layout = new QVBoxLayout;
    stable_4_layout->setContentsMargins(0,0,0,0);
    stable_4_layout->addWidget(chocobo_stable_4);
    ui->box_stable4->setLayout(stable_4_layout);

    chocobo_stable_5 = new ChocoboEditor;
    QVBoxLayout *stable_5_layout = new QVBoxLayout;
    stable_5_layout->setContentsMargins(0,0,0,0);
    stable_5_layout->addWidget(chocobo_stable_5);
    ui->box_stable5->setLayout(stable_5_layout);

    chocobo_stable_6 = new ChocoboEditor;
    QVBoxLayout *stable_6_layout = new QVBoxLayout;
    stable_6_layout->setContentsMargins(0,0,0,0);
    stable_6_layout->addWidget(chocobo_stable_6);
    ui->box_stable6->setLayout(stable_6_layout);

    hexEditor = new QHexEdit;
    hexEditor->setHighlightingColor(QColor(98,192,247));
    hexEditor->setAddressAreaColor(QColor(64,65,64));
    QVBoxLayout *hexLayout = new QVBoxLayout;
    hexLayout->setContentsMargins(0,0,0,0);

    hexLayout->addWidget(hexEditor);
    ui->group_hexedit->setLayout(hexLayout);

    //Set up Status Bar..
    ui->statusBar->addWidget(ui->frame_status,1);
}
void MainWindow::init_style()
{
    QString tablestyle = "::section{background-color:qlineargradient(spread:pad, x1:0.5, y1:0.00568182, x2:0.497, y2:1, stop:0 rgba(67, 67, 67, 128), stop:0.5 rgba(98, 192, 247, 128), stop:1 rgba(67, 67, 67, 128));;color: white;padding-left:4px;border:1px solid #6c6c6c;}";
    tablestyle.append("QHeaderView:down-arrow{image: url(:/icon/arrow_down);min-width:9px;}");
    tablestyle.append("QHeaderView:up-arrow{image: url(:/icon/arrow_up);min-width:9px;}");
    ui->tbl_location_field->horizontalHeader()->setStyleSheet(tablestyle);
    ui->tbl_unknown->horizontalHeader()->setStyleSheet(tablestyle);
    ui->tbl_compare_unknown->horizontalHeader()->setStyleSheet(tablestyle);
    ui->tbl_diff->horizontalHeader()->setStyleSheet(tablestyle);

   QString sliderStyleSheet("QSlider:sub-page{background-color: qlineargradient(spread:pad, x1:0.472, y1:0.011, x2:0.483, y2:1, stop:0 rgba(186, 1, 87,192), stop:0.505682 rgba(209, 128, 173,192), stop:0.931818 rgba(209, 44, 136, 192));}");
    sliderStyleSheet.append(QString("QSlider::add-page{background: qlineargradient(spread:pad, x1:0.5, y1:0.00568182, x2:0.497, y2:1, stop:0 rgba(91, 91, 91, 255), stop:0.494318 rgba(122, 122, 122, 255), stop:1 rgba(106, 106, 106, 255));}"));
    sliderStyleSheet.append(QString("QSlider{border:3px solid;border-left-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(123, 123, 123, 255), stop:1 rgba(172, 172, 172, 255));border-right-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(123, 123, 123, 255), stop:1 rgba(172, 172, 172, 255));border-bottom-color: rgb(172, 172, 172);border-top-color: rgb(172, 172, 172);border-radius: 5px;}"));
    sliderStyleSheet.append(QString("QSlider::groove{height: 12px;background: qlineargradient(spread:pad, x1:0.5, y1:0.00568182, x2:0.497, y2:1, stop:0 rgba(91, 91, 91, 255), stop:0.494318 rgba(122, 122, 122, 255), stop:1 rgba(106, 106, 106, 255));}"));
    sliderStyleSheet.append(QString("QSlider::handle{background: rgba(172, 172, 172,255);border: 1px solid #5c5c5c;width: 3px;border-radius: 2px;}"));

  optionsWidget->setSliderStyle(sliderStyleSheet);
  char_editor->setSliderStyle(sliderStyleSheet);

  optionsWidget->setScrollAreaStyleSheet(QString("background-color: rgba(10,10,10,16);font:;color:rgb(255,255,255);"));
  char_editor->setToolBoxStyle(QString("::tab:hover{background-color:qlineargradient(spread:pad, x1:0.5, y1:0.00568182, x2:0.497, y2:1, stop:0 rgba(67, 67, 67, 128), stop:0.5 rgba(98,192,247,128), stop:1 rgba(67, 67, 67, 128));}"));
  hexEditor->setStyleSheet(QString("background-color: rgb(64,65,64);font:;color:rgb(255,255,255);"));

  ui->slide_world_y->setStyleSheet(QString("::handle{image: url(:/icon/prev);}"));
  ui->slide_world_x->setStyleSheet(QString("::handle{image: url(:/icon/slider_up);}"));
  ui->world_map_view->setStyleSheet(QString("background-image: url(:/icon/world_map);"));

}
void MainWindow::init_connections()
{//check Qt Version and Connect With Apporate Method.
        connect( ui->tbl_unknown->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->tbl_compare_unknown->verticalScrollBar(), SLOT(setValue(int)) );
        connect( ui->tbl_compare_unknown->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->tbl_unknown->verticalScrollBar(), SLOT(setValue(int)) );

        connect(phsList,SIGNAL(box1_toggled(int,bool)),this,SLOT(phsList_box_allowed_toggled(int,bool)));
        connect(phsList,SIGNAL(box2_toggled(int,bool)),this,SLOT(phsList_box_visible_toggled(int,bool)));

        connect(menuList,SIGNAL(box1_toggled(int,bool)),this,SLOT(menuList_box_locked_toggled(int,bool)));
        connect(menuList,SIGNAL(box2_toggled(int,bool)),this,SLOT(menuList_box_visible_toggled(int,bool)));

        //ItemList
        connect(itemlist,SIGNAL(itemsChanged(QList<quint16>)),this,SLOT(Items_Changed(QList<quint16>)));
        //Materia_Editor
        connect(materia_editor,SIGNAL(ap_changed(qint32)),this,SLOT(materia_ap_changed(qint32)));
        connect(materia_editor,SIGNAL(id_changed(qint8)),this,SLOT(materia_id_changed(qint8)));
        //Char_Editor
        connect(ui->action_show_debug,SIGNAL(toggled(bool)),char_editor,(SLOT(setDebug(bool))));
        connect(char_editor,SIGNAL(id_changed(qint8)),this,SLOT(char_id_changed(qint8)));
        connect(char_editor,SIGNAL(level_changed(qint8)),this,SLOT(char_level_changed(qint8)));
        connect(char_editor,SIGNAL(str_changed(quint8)),this,SLOT(char_str_changed(quint8)));
        connect(char_editor,SIGNAL(vit_changed(quint8)),this,SLOT(char_vit_changed(quint8)));
        connect(char_editor,SIGNAL(mag_changed(quint8)),this,SLOT(char_mag_changed(quint8)));
        connect(char_editor,SIGNAL(spi_changed(quint8)),this,SLOT(char_spi_changed(quint8)));
        connect(char_editor,SIGNAL(dex_changed(quint8)),this,SLOT(char_dex_changed(quint8)));
        connect(char_editor,SIGNAL(lck_changed(quint8)),this,SLOT(char_lck_changed(quint8)));
        connect(char_editor,SIGNAL(strBonus_changed(quint8)),this,SLOT(char_strBonus_changed(quint8)));
        connect(char_editor,SIGNAL(vitBonus_changed(quint8)),this,SLOT(char_vitBonus_changed(quint8)));
        connect(char_editor,SIGNAL(magBonus_changed(quint8)),this,SLOT(char_magBonus_changed(quint8)));
        connect(char_editor,SIGNAL(spiBonus_changed(quint8)),this,SLOT(char_spiBonus_changed(quint8)));
        connect(char_editor,SIGNAL(dexBonus_changed(quint8)),this,SLOT(char_dexBonus_changed(quint8)));
        connect(char_editor,SIGNAL(lckBonus_changed(quint8)),this,SLOT(char_lckBonus_changed(quint8)));
        connect(char_editor,SIGNAL(limitLevel_changed(qint8)),this,SLOT(char_limitLevel_changed(qint8)));
        connect(char_editor,SIGNAL(limitBar_changed(quint8)),this,SLOT(char_limitBar_changed(quint8)));
        connect(char_editor,SIGNAL(name_changed(QString)),this,SLOT(char_name_changed(QString)));
        connect(char_editor,SIGNAL(weapon_changed(quint8)),this,SLOT(char_weapon_changed(quint8)));
        connect(char_editor,SIGNAL(armor_changed(quint8)),this,SLOT(char_armor_changed(quint8)));
        connect(char_editor,SIGNAL(accessory_changed(quint8)),this,SLOT(char_accessory_changed(quint8)));
        connect(char_editor,SIGNAL(curHp_changed(quint16)),this,SLOT(char_curHp_changed(quint16)));
        connect(char_editor,SIGNAL(maxHp_changed(quint16)),this,SLOT(char_maxHp_changed(quint16)));
        connect(char_editor,SIGNAL(curMp_changed(quint16)),this,SLOT(char_curMp_changed(quint16)));
        connect(char_editor,SIGNAL(maxMp_changed(quint16)),this,SLOT(char_maxMp_changed(quint16)));
        connect(char_editor,SIGNAL(kills_changed(quint16)),this,SLOT(char_kills_changed(quint16)));
        connect(char_editor,SIGNAL(row_changed(quint8)),this,SLOT(char_row_changed(quint8)));
        connect(char_editor,SIGNAL(levelProgress_changed(quint8)),this,SLOT(char_levelProgress_changed(quint8)));
        connect(char_editor,SIGNAL(sadnessfury_changed(quint8)),this,SLOT(char_sadnessfury_changed(quint8)));
        connect(char_editor,SIGNAL(limits_changed(quint16)),this,SLOT(char_limits_changed(quint16)));
        connect(char_editor,SIGNAL(timesused1_changed(quint16)),this,SLOT(char_timesused1_changed(quint16)));
        connect(char_editor,SIGNAL(timesused2_changed(quint16)),this,SLOT(char_timeused2_changed(quint16)));
        connect(char_editor,SIGNAL(timesused3_changed(quint16)),this,SLOT(char_timeused3_changed(quint16)));
        connect(char_editor,SIGNAL(baseHp_changed(quint16)),this,SLOT(char_baseHp_changed(quint16)));
        connect(char_editor,SIGNAL(baseMp_changed(quint16)),this,SLOT(char_baseMp_changed(quint16)));
        connect(char_editor,SIGNAL(exp_changed(quint32)),this,SLOT(char_exp_changed(quint32)));
        connect(char_editor,SIGNAL(mslotChanged(int)),this,SLOT(char_mslot_changed(int)));
        connect(char_editor,SIGNAL(Materias_changed(materia)),this,SLOT(char_materia_changed(materia)));
        connect(char_editor,SIGNAL(expNext_changed(quint32)),this,SLOT(char_expNext_changed(quint32)));
        //Chocobo Editor 1
        connect(chocobo_stable_1,SIGNAL(name_changed(QString)),this,SLOT(c1_nameChanged(QString)));
        connect(chocobo_stable_1,SIGNAL(cantMate_changed(bool)),this,SLOT(c1_mated_toggled(bool)));
        connect(chocobo_stable_1,SIGNAL(speed_changed(quint16)),this,SLOT(c1_speedChanged(quint16)));
        connect(chocobo_stable_1,SIGNAL(mSpeed_changed(quint16)),this,SLOT(c1_maxspeedChanged(quint16)));
        connect(chocobo_stable_1,SIGNAL(sprint_changed(quint16)),this,SLOT(c1_sprintChanged(quint16)));
        connect(chocobo_stable_1,SIGNAL(mSprint_changed(quint16)),this,SLOT(c1_maxsprintChanged(quint16)));
        connect(chocobo_stable_1,SIGNAL(stamina_changed(quint16)),this,SLOT(c1_staminaChanged(quint16)));
        connect(chocobo_stable_1,SIGNAL(sex_changed(quint8)),this,SLOT(c1_sexChanged(quint8)));
        connect(chocobo_stable_1,SIGNAL(type_changed(quint8)),this,SLOT(c1_typeChanged(quint8)));
        connect(chocobo_stable_1,SIGNAL(accel_changed(quint8)),this,SLOT(c1_accelChanged(quint8)));
        connect(chocobo_stable_1,SIGNAL(coop_changed(quint8)),this,SLOT(c1_coopChanged(quint8)));
        connect(chocobo_stable_1,SIGNAL(intelligence_changed(quint8)),this,SLOT(c1_intelChanged(quint8)));
        connect(chocobo_stable_1,SIGNAL(personality_changed(quint8)),this,SLOT(c1_personalityChanged(quint8)));
        connect(chocobo_stable_1,SIGNAL(pCount_changed(quint8)),this,SLOT(c1_pcountChanged(quint8)));
        connect(chocobo_stable_1,SIGNAL(wins_changed(quint8)),this,SLOT(c1_raceswonChanged(quint8)));
        //Chocobo Editor 2
        connect(chocobo_stable_2,SIGNAL(name_changed(QString)),this,SLOT(c2_nameChanged(QString)));
        connect(chocobo_stable_2,SIGNAL(cantMate_changed(bool)),this,SLOT(c2_mated_toggled(bool)));
        connect(chocobo_stable_2,SIGNAL(speed_changed(quint16)),this,SLOT(c2_speedChanged(quint16)));
        connect(chocobo_stable_2,SIGNAL(mSpeed_changed(quint16)),this,SLOT(c2_maxspeedChanged(quint16)));
        connect(chocobo_stable_2,SIGNAL(sprint_changed(quint16)),this,SLOT(c2_sprintChanged(quint16)));
        connect(chocobo_stable_2,SIGNAL(mSprint_changed(quint16)),this,SLOT(c2_maxsprintChanged(quint16)));
        connect(chocobo_stable_2,SIGNAL(stamina_changed(quint16)),this,SLOT(c2_staminaChanged(quint16)));
        connect(chocobo_stable_2,SIGNAL(sex_changed(quint8)),this,SLOT(c2_sexChanged(quint8)));
        connect(chocobo_stable_2,SIGNAL(type_changed(quint8)),this,SLOT(c2_typeChanged(quint8)));
        connect(chocobo_stable_2,SIGNAL(accel_changed(quint8)),this,SLOT(c2_accelChanged(quint8)));
        connect(chocobo_stable_2,SIGNAL(coop_changed(quint8)),this,SLOT(c2_coopChanged(quint8)));
        connect(chocobo_stable_2,SIGNAL(intelligence_changed(quint8)),this,SLOT(c2_intelChanged(quint8)));
        connect(chocobo_stable_2,SIGNAL(personality_changed(quint8)),this,SLOT(c2_personalityChanged(quint8)));
        connect(chocobo_stable_2,SIGNAL(pCount_changed(quint8)),this,SLOT(c2_pcountChanged(quint8)));
        connect(chocobo_stable_2,SIGNAL(wins_changed(quint8)),this,SLOT(c2_raceswonChanged(quint8)));
        //Chocobo Editor 3
        connect(chocobo_stable_3,SIGNAL(name_changed(QString)),this,SLOT(c3_nameChanged(QString)));
        connect(chocobo_stable_3,SIGNAL(cantMate_changed(bool)),this,SLOT(c3_mated_toggled(bool)));
        connect(chocobo_stable_3,SIGNAL(speed_changed(quint16)),this,SLOT(c3_speedChanged(quint16)));
        connect(chocobo_stable_3,SIGNAL(mSpeed_changed(quint16)),this,SLOT(c3_maxspeedChanged(quint16)));
        connect(chocobo_stable_3,SIGNAL(sprint_changed(quint16)),this,SLOT(c3_sprintChanged(quint16)));
        connect(chocobo_stable_3,SIGNAL(mSprint_changed(quint16)),this,SLOT(c3_maxsprintChanged(quint16)));
        connect(chocobo_stable_3,SIGNAL(stamina_changed(quint16)),this,SLOT(c3_staminaChanged(quint16)));
        connect(chocobo_stable_3,SIGNAL(sex_changed(quint8)),this,SLOT(c3_sexChanged(quint8)));
        connect(chocobo_stable_3,SIGNAL(type_changed(quint8)),this,SLOT(c3_typeChanged(quint8)));
        connect(chocobo_stable_3,SIGNAL(accel_changed(quint8)),this,SLOT(c3_accelChanged(quint8)));
        connect(chocobo_stable_3,SIGNAL(coop_changed(quint8)),this,SLOT(c3_coopChanged(quint8)));
        connect(chocobo_stable_3,SIGNAL(intelligence_changed(quint8)),this,SLOT(c3_intelChanged(quint8)));
        connect(chocobo_stable_3,SIGNAL(personality_changed(quint8)),this,SLOT(c3_personalityChanged(quint8)));
        connect(chocobo_stable_3,SIGNAL(pCount_changed(quint8)),this,SLOT(c3_pcountChanged(quint8)));
        connect(chocobo_stable_3,SIGNAL(wins_changed(quint8)),this,SLOT(c3_raceswonChanged(quint8)));
        //Chocobo Editor 4
        connect(chocobo_stable_4,SIGNAL(name_changed(QString)),this,SLOT(c4_nameChanged(QString)));
        connect(chocobo_stable_4,SIGNAL(cantMate_changed(bool)),this,SLOT(c4_mated_toggled(bool)));
        connect(chocobo_stable_4,SIGNAL(speed_changed(quint16)),this,SLOT(c4_speedChanged(quint16)));
        connect(chocobo_stable_4,SIGNAL(mSpeed_changed(quint16)),this,SLOT(c4_maxspeedChanged(quint16)));
        connect(chocobo_stable_4,SIGNAL(sprint_changed(quint16)),this,SLOT(c4_sprintChanged(quint16)));
        connect(chocobo_stable_4,SIGNAL(mSprint_changed(quint16)),this,SLOT(c4_maxsprintChanged(quint16)));
        connect(chocobo_stable_4,SIGNAL(stamina_changed(quint16)),this,SLOT(c4_staminaChanged(quint16)));
        connect(chocobo_stable_4,SIGNAL(sex_changed(quint8)),this,SLOT(c4_sexChanged(quint8)));
        connect(chocobo_stable_4,SIGNAL(type_changed(quint8)),this,SLOT(c4_typeChanged(quint8)));
        connect(chocobo_stable_4,SIGNAL(accel_changed(quint8)),this,SLOT(c4_accelChanged(quint8)));
        connect(chocobo_stable_4,SIGNAL(coop_changed(quint8)),this,SLOT(c4_coopChanged(quint8)));
        connect(chocobo_stable_4,SIGNAL(intelligence_changed(quint8)),this,SLOT(c4_intelChanged(quint8)));
        connect(chocobo_stable_4,SIGNAL(personality_changed(quint8)),this,SLOT(c4_personalityChanged(quint8)));
        connect(chocobo_stable_4,SIGNAL(pCount_changed(quint8)),this,SLOT(c4_pcountChanged(quint8)));
        connect(chocobo_stable_4,SIGNAL(wins_changed(quint8)),this,SLOT(c4_raceswonChanged(quint8)));
        //Chocobo Editor 5
        connect(chocobo_stable_5,SIGNAL(name_changed(QString)),this,SLOT(c5_nameChanged(QString)));
        connect(chocobo_stable_5,SIGNAL(cantMate_changed(bool)),this,SLOT(c5_mated_toggled(bool)));
        connect(chocobo_stable_5,SIGNAL(speed_changed(quint16)),this,SLOT(c5_speedChanged(quint16)));
        connect(chocobo_stable_5,SIGNAL(mSpeed_changed(quint16)),this,SLOT(c5_maxspeedChanged(quint16)));
        connect(chocobo_stable_5,SIGNAL(sprint_changed(quint16)),this,SLOT(c5_sprintChanged(quint16)));
        connect(chocobo_stable_5,SIGNAL(mSprint_changed(quint16)),this,SLOT(c5_maxsprintChanged(quint16)));
        connect(chocobo_stable_5,SIGNAL(stamina_changed(quint16)),this,SLOT(c5_staminaChanged(quint16)));
        connect(chocobo_stable_5,SIGNAL(sex_changed(quint8)),this,SLOT(c5_sexChanged(quint8)));
        connect(chocobo_stable_5,SIGNAL(type_changed(quint8)),this,SLOT(c5_typeChanged(quint8)));
        connect(chocobo_stable_5,SIGNAL(accel_changed(quint8)),this,SLOT(c5_accelChanged(quint8)));
        connect(chocobo_stable_5,SIGNAL(coop_changed(quint8)),this,SLOT(c5_coopChanged(quint8)));
        connect(chocobo_stable_5,SIGNAL(intelligence_changed(quint8)),this,SLOT(c5_intelChanged(quint8)));
        connect(chocobo_stable_5,SIGNAL(personality_changed(quint8)),this,SLOT(c5_personalityChanged(quint8)));
        connect(chocobo_stable_5,SIGNAL(pCount_changed(quint8)),this,SLOT(c5_pcountChanged(quint8)));
        connect(chocobo_stable_5,SIGNAL(wins_changed(quint8)),this,SLOT(c5_raceswonChanged(quint8)));
        //Chocobo Editor 6
        connect(chocobo_stable_6,SIGNAL(name_changed(QString)),this,SLOT(c6_nameChanged(QString)));
        connect(chocobo_stable_6,SIGNAL(cantMate_changed(bool)),this,SLOT(c6_mated_toggled(bool)));
        connect(chocobo_stable_6,SIGNAL(speed_changed(quint16)),this,SLOT(c6_speedChanged(quint16)));
        connect(chocobo_stable_6,SIGNAL(mSpeed_changed(quint16)),this,SLOT(c6_maxspeedChanged(quint16)));
        connect(chocobo_stable_6,SIGNAL(sprint_changed(quint16)),this,SLOT(c6_sprintChanged(quint16)));
        connect(chocobo_stable_6,SIGNAL(mSprint_changed(quint16)),this,SLOT(c6_maxsprintChanged(quint16)));
        connect(chocobo_stable_6,SIGNAL(stamina_changed(quint16)),this,SLOT(c6_staminaChanged(quint16)));
        connect(chocobo_stable_6,SIGNAL(sex_changed(quint8)),this,SLOT(c6_sexChanged(quint8)));
        connect(chocobo_stable_6,SIGNAL(type_changed(quint8)),this,SLOT(c6_typeChanged(quint8)));
        connect(chocobo_stable_6,SIGNAL(accel_changed(quint8)),this,SLOT(c6_accelChanged(quint8)));
        connect(chocobo_stable_6,SIGNAL(coop_changed(quint8)),this,SLOT(c6_coopChanged(quint8)));
        connect(chocobo_stable_6,SIGNAL(intelligence_changed(quint8)),this,SLOT(c6_intelChanged(quint8)));
        connect(chocobo_stable_6,SIGNAL(personality_changed(quint8)),this,SLOT(c6_personalityChanged(quint8)));
        connect(chocobo_stable_6,SIGNAL(pCount_changed(quint8)),this,SLOT(c6_pcountChanged(quint8)));
        connect(chocobo_stable_6,SIGNAL(wins_changed(quint8)),this,SLOT(c6_raceswonChanged(quint8)));

        //options
        connect(optionsWidget,SIGNAL(dialogColorLLChanged(QColor)),this,SLOT(setDialogColorLL(QColor)));
        connect(optionsWidget,SIGNAL(dialogColorLRChanged(QColor)),this,SLOT(setDialogColorLR(QColor)));
        connect(optionsWidget,SIGNAL(dialogColorULChanged(QColor)),this,SLOT(setDialogColorUL(QColor)));
        connect(optionsWidget,SIGNAL(dialogColorURChanged(QColor)),this,SLOT(setDialogColorUR(QColor)));
        connect(optionsWidget,SIGNAL(MagicOrderChanged(int)),this,SLOT(setMagicOrder(int)));
        connect(optionsWidget,SIGNAL(CameraChanged(int)),this,SLOT(setCameraMode(int)));
        connect(optionsWidget,SIGNAL(AtbChanged(int)),this,SLOT(setAtbMode(int)));
        connect(optionsWidget,SIGNAL(CursorChanged(int)),this,SLOT(setCursorMode(int)));
        connect(optionsWidget,SIGNAL(ControllerModeChanged(int)),this,SLOT(setControlMode(int)));
        connect(optionsWidget,SIGNAL(SoundChanged(int)),this,SLOT(setSoundMode(int)));
        connect(optionsWidget,SIGNAL(FieldMessageSpeedChanged(int)),this,SLOT(setFieldMessageSpeed(int)));
        connect(optionsWidget,SIGNAL(BattleMessageSpeedChanged(int)),this,SLOT(setBattleMessageSpeed(int)));
        connect(optionsWidget,SIGNAL(BattleSpeedChanged(int)),this,SLOT(setBattleSpeed(int)));
        connect(optionsWidget,SIGNAL(FieldHelpChanged(bool)),this,SLOT(setFieldHelp(bool)));
        connect(optionsWidget,SIGNAL(BattleTargetsChanged(bool)),this,SLOT(setBattleTargets(bool)));
        connect(optionsWidget,SIGNAL(BattleHelpChanged(bool)),this,SLOT(setBattleHelp(bool)));
        connect(optionsWidget,SIGNAL(BtnCameraChanged(int)),this,SLOT(setButtonCamera(int)));
        connect(optionsWidget,SIGNAL(BtnTargetChanged(int)),this,SLOT(setButtonTarget(int)));
        connect(optionsWidget,SIGNAL(BtnPgUpChanged(int)),this,SLOT(setButtonPageUp(int)));
        connect(optionsWidget,SIGNAL(BtnPgDnChanged(int)),this,SLOT(setButtonPageDown(int)));
        connect(optionsWidget,SIGNAL(BtnMenuChanged(int)),this,SLOT(setButtonMenu(int)));
        connect(optionsWidget,SIGNAL(BtnOkChanged(int)),this,SLOT(setButtonOk(int)));
        connect(optionsWidget,SIGNAL(BtnCancelChanged(int)),this,SLOT(setButtonCancel(int)));
        connect(optionsWidget,SIGNAL(BtnSwitchChanged(int)),this,SLOT(setButtonSwitch(int)));
        connect(optionsWidget,SIGNAL(BtnHelpChanged(int)),this,SLOT(setButtonHelp(int)));
        connect(optionsWidget,SIGNAL(Btn9Changed(int)),this,SLOT(setButtonUnknown1(int)));
        connect(optionsWidget,SIGNAL(Btn10Changed(int)),this,SLOT(setButtonUnknown2(int)));
        connect(optionsWidget,SIGNAL(BtnPauseChanged(int)),this,SLOT(setButtonPause(int)));
        connect(optionsWidget,SIGNAL(BtnUpChanged(int)),this,SLOT(setButtonUp(int)));
        connect(optionsWidget,SIGNAL(BtnDownChanged(int)),this,SLOT(setButtonDown(int)));
        connect(optionsWidget,SIGNAL(BtnLeftChanged(int)),this,SLOT(setButtonLeft(int)));
        connect(optionsWidget,SIGNAL(BtnRightChanged(int)),this,SLOT(setButtonRight(int)));
        //HexEditor.
        connect(hexEditor,SIGNAL(dataChanged()),this,SLOT(hexEditorChanged()));
}
void MainWindow::init_settings()
{
    //are any empty? if so set them accordingly.
    if(settings->value("autochargrowth").isNull()){settings->setValue("autochargrowth",1);}
    if(settings->value("load_path").isNull()){settings->setValue("load_path",QDir::homePath());}
    if(settings->value("char_stat_folder").isNull()){settings->setValue("char_stat_folder",QDir::homePath());}
    if(settings->value("color1_r").isNull()){settings->setValue("color1_r","7");}
    if(settings->value("color1_g").isNull()){settings->setValue("color1_g","6");}
    if(settings->value("color1_b").isNull()){settings->setValue("color1_b","6");}
    if(settings->value("color2_r").isNull()){settings->setValue("color2_r","35");}
    if(settings->value("color2_g").isNull()){settings->setValue("color2_g","33");}
    if(settings->value("color2_b").isNull()){settings->setValue("color2_b","33");}
    if(settings->value("color3_r").isNull()){settings->setValue("color3_r","65");}
    if(settings->value("color3_g").isNull()){settings->setValue("color3_g","65");}
    if(settings->value("color3_b").isNull()){settings->setValue("color3_b","65");}
    skip_slot_mask = settings->value("skip_slot_mask").toBool(); //skips setting the mask of last saved slot on writes. testing function

    if(settings->value("show_test").toBool()){ui->action_show_debug->setChecked(Qt::Checked);}
    else{ui->action_show_debug->setChecked(Qt::Unchecked);}

    QString style="QWidget#centralWidget{background-color: qlineargradient(spread:repeat, x1:1, y1:1, x2:0, y2:0, stop:0.0625 rgba(";
    style.append(settings->value("color1_r").toString());   style.append(",");
    style.append(settings->value("color1_g").toString());   style.append(",");
    style.append(settings->value("color1_b").toString());   style.append(", 255), stop:0.215909 rgba(");
    style.append(settings->value("color2_r").toString());   style.append(",");
    style.append(settings->value("color2_g").toString());   style.append(",");
    style.append(settings->value("color2_b").toString());   style.append(", 255), stop:0.818182 rgba(");
    style.append(settings->value("color3_r").toString());   style.append(",");
    style.append(settings->value("color3_g").toString());   style.append(",");
    style.append(settings->value("color3_b").toString());   style.append(", 255));}");
    ui->centralWidget->setStyleSheet(style);

    if(settings->value("autochargrowth").toBool()){ui->action_auto_char_growth->setChecked(Qt::Checked);}
    else{ui->action_auto_char_growth->setChecked(Qt::Unchecked);}

    /* LANGUAGE SELECT */
    if(settings->value("lang").toString() == "en"){ui->action_Lang_en->setChecked(Qt::Checked);}
    else if(settings->value("lang").toString() == "es"){ui->action_Lang_es->setChecked(Qt::Checked);}
    else if(settings->value("lang").toString() == "fr"){ui->action_Lang_fr->setChecked(Qt::Checked);}
    else if(settings->value("lang").toString() == "ja"){ui->action_Lang_jp->setChecked(Qt::Checked);}
    else if(settings->value("lang").toString() == "de"){ui->action_Lang_de->setChecked(Qt::Checked);}
}
/*~~~~~~ END GUI SETUP ~~~~~~~*/
MainWindow::~MainWindow(){delete ui;}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    };
}
void MainWindow::dragEnterEvent(QDragEnterEvent *e) { e->accept(); }
void MainWindow::dropEvent(QDropEvent *e)
{
    if(ff7->isFileModified())
    {
        switch(save_changes())
        {
            case 0: return;//cancel load.
            case 1: break;//continue load
        }
    }
    const QMimeData *mimeData = e->mimeData();
    if(mimeData->hasUrls())
    {
        QStringList fileList;
        QList<QUrl> urlList = mimeData->urls();

        fileList.append(urlList.at(0).toLocalFile());
        loadFileFull(fileList.at(0),0);
     }
}
int MainWindow::save_changes(void)
{//return 0 to ingore the event/ return 1 to process event.
    int result; int rtn=0;
    result = QMessageBox::question(this,tr("Unsaved Changes"),tr("Save Changes to the File:\n%1").arg(ff7->fileName()),QMessageBox::Yes,QMessageBox::No,QMessageBox::Cancel);
    switch(result)
    {
        case QMessageBox::Yes:
                if(ui->action_Save->isEnabled()){on_action_Save_triggered();}
                else
                {//user trying to save a file with no header.
                    QStringList types;
                    types << tr("PC")<<tr("Raw Psx Save")<<tr("Generic Emulator Memorycard")<<tr("PSP")<<tr("PS3")<<tr("Dex Drive Memorycard")<<tr("VGS Memorycard");
                    QString result = QInputDialog::getItem(this,tr("Save Error"),tr("Please Select A File Type To Save"),types,-1,0,0,0);
                    //check the string. and assign a type
                    if(result ==types.at(0)){on_actionExport_PC_Save_triggered();}
                        else if(result ==types.at(1)){on_actionExport_PSX_triggered();}
                        else if(result ==types.at(2)){on_actionExport_MC_triggered();}
                        else if(result ==types.at(3)){QMessageBox::information(this,tr("Black Chocobo"),tr("Can Not Export This Format"));}
                        else if(result ==types.at(4)){QMessageBox::information(this,tr("Black Chocobo"),tr("Can Not Export This Format"));}
                        else if(result ==types.at(5)){on_actionExport_DEX_triggered();}
                        else if(result ==types.at(6)){on_actionExport_VGS_triggered();}
                        else{return rtn;}
                    }
                    rtn=1;
                    break;
         case QMessageBox::Cancel: rtn=0; break;
        case QMessageBox::No:rtn=1;break;
     }
    return rtn;
}
void MainWindow::closeEvent(QCloseEvent *e)
{if(ff7->isFileModified()){
    switch(save_changes())
    {
        case 0: e->ignore(); break;
        case 1: e->accept(); break;
    }
}}
/*~~~~~ New Window ~~~~~*/
void MainWindow::on_actionNew_Window_triggered(){QProcess::startDetached(QCoreApplication::applicationFilePath());}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~LOAD/SAVE FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::on_actionOpen_Save_File_triggered()
{
    if(ff7->isFileModified())
    {
        switch(save_changes())
        {
            case 0: return;//cancel load.
            case 1: break;//yes or no pressed..
        }
    }
    QString fileName = QFileDialog::getOpenFileName(this,
    tr("Open Final Fantasy 7 Save"),settings->value("load_path").toString(),
    tr("Known FF7 Save Types (*.ff7 *-S* *.psv *.vmp *.vgs *.mem *.gme *.mcr *.mcd *.mci *.mc *.ddf *.ps *.psm *.VM1 *.bin);;PC FF7 SaveGame (*.ff7);;Raw PSX FF7 SaveGame (*-S*);;MC SaveGame (*.mcr *.mcd *.mci *.mc *.ddf *.ps *.psm *.VM1 *.bin);;PSV SaveGame (*.psv);;PSP SaveGame (*.vmp);;VGS SaveGame(*.vgs *.mem);;Dex-Drive SaveGame(*.gme);;All Files(*)"));
    if (!fileName.isEmpty()){loadFileFull(fileName,0);}
}
void MainWindow::on_actionReload_triggered(){if(!ff7->fileName().isEmpty()){loadFileFull(ff7->fileName(),1);}}
/*~~~~~~~~~~~~~~~~~Load Full ~~~~~~~~~~~~~~~~~~*/
void MainWindow::loadFileFull(const QString &fileName,int reload)
{//if called from reload then int reload ==1 (don't call slot select)
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly )){QMessageBox::warning(this, tr("Black Chocobo"), tr("Cannot read file %1:\n%2.") .arg(fileName).arg(file.errorString()));  return; }

    if(ff7->loadFile(fileName))
    {
        _init=false;//we have now loaded a file
        file_modified(false);
    }
    else{QMessageBox::information(this,tr("Load Failed"),tr("Failed to Load File"));return;}

    if (ff7->type() == "PC"){if(reload){guirefresh(0);}else{on_actionShow_Selection_Dialog_triggered();}}

    else if (ff7->type() == "PSX" || ff7->type() =="PSV"){s=0;guirefresh(0);}

    else if (ff7->type() == "MC" || ff7->type() =="PSP" || ff7->type() == "VGS" ||ff7->type()=="DEX")
    {
        if(reload){guirefresh(0);}   else{on_actionShow_Selection_Dialog_triggered();}
    }
    else{/*UNKNOWN FILETYPE*/}
}
/*~~~~~~~~~~~~~~~~~IMPORT PSX~~~~~~~~~~~~~~~~~~*/
void MainWindow::on_actionFrom_PSX_Slot_triggered()
{//should check better to be sure its a raw PSX SAVE. then make file filter *
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Final Fantasy 7 PSX Save"),QDir::homePath(),tr("Raw PSX FF7 SaveGame (*)"));
    if(fileName.isEmpty()){return;}
    else
    {
        ff7->importPSX(s,fileName);
        guirefresh(0);
    }
}
/*~~~~~~~~~~~~~~~~~IMPORT PSV~~~~~~~~~~~~~~~~~~*/
void MainWindow::on_actionFrom_PSV_Slot_triggered()
{//check beter to be sure its the correct PSV type file.
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select Final Fantasy 7 PSV Save"),QDir::homePath(),tr("PSV FF7 SaveGame (*.psv)"));
    if (fileName.isEmpty()){return;}
    else
    {
        ff7->importPSV(s,fileName);
        guirefresh(0);
    }
}
/*~~~~~~~~~~~~~~~~~IMPORT Char~~~~~~~~~~~~~~~~~*/
void MainWindow::on_actionImport_char_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select FF7 Character Stat File"),settings->value("char_stat_folder").toString(),tr("FF7 Character Stat File(*.char)"));
    if (fileName.isEmpty()){return;}
    else
    {
        QFile file(fileName);
        if(!file.open(QFile::ReadOnly)){QMessageBox::warning(this, tr("Black Chocobo"),tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString())); return; }
        if(file.size() !=0x84){QMessageBox::warning(this, tr("Black Chocobo"),tr("%1:\n%2 is Not a FF7 Character Stat File.").arg(fileName).arg(file.errorString()));return;}
        QByteArray new_char;
        new_char = file.readAll();
        ff7->importCharacter(s,curchar,new_char);
    }
    char_editor->setChar(ff7->character(s,curchar),ff7->charName(s,curchar));
    set_char_buttons();
}

void MainWindow::on_actionExport_char_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
    tr("Save FF7 Character File"), settings->value("char_stat_folder").toString(),
    tr("FF7 Character Stat File(*.char)"));
    if (!fileName.isEmpty())
    {
        if(ff7->exportCharacter(s,curchar,fileName)){ui->statusBar->showMessage(tr("Character Export Successful"),1000);}
        else{ui->statusBar->showMessage(tr("Character Export Failed"),2000);}
    }
}
void MainWindow::on_action_Save_triggered()
{
    if(_init)//no file loaded user saving a New Game
    {
        QStringList types;
        types << tr("PC")<<tr("Raw Psx Save")<<tr("Generic Emulator Memorycard")<<tr("PSP")<<tr("PS3")<<tr("Dex Drive Memorycard")<<tr("VGS Memorycard");
        QString result = QInputDialog::getItem(this,tr("Save Error"),tr("Please Select A File Type To Save"),types,-1,0,0,0);
        //check the string. and assign a type
        if(result ==types.at(0)){on_actionExport_PC_Save_triggered();}
            else if(result ==types.at(1)){on_actionExport_PSX_triggered();}
            else if(result ==types.at(2)){on_actionExport_MC_triggered();}
            else if(result ==types.at(3)){QMessageBox::information(this,tr("Black Chocobo"),tr("Can Not Export This Format"));}
            else if(result ==types.at(4)){QMessageBox::information(this,tr("Black Chocobo"),tr("Can Not Export This Format"));}
            else if(result ==types.at(5)){on_actionExport_DEX_triggered();}
            else if(result ==types.at(6)){on_actionExport_VGS_triggered();}
            else{}
    return;//leave this function.
    }

    if(!ff7->fileName().isEmpty())
    {
        if(ff7->type()=="PSP")
        {
            if(ff7->type()=="PSP"){QMessageBox::information(this,tr("PSP Save Notice"),tr("This File Does Not Have An Updated Checksum.It will not work on your PSP."));}
        }

        else if(ff7->type()=="PSV")
        {
            QMessageBox::information(this,tr("PSV Save Notice"),tr("This File Does Not Have An Updated Checksum.It will not work on your PS3."));
        }

        saveFileFull(ff7->fileName());
    }
    else{on_actionSave_File_As_triggered();return;}//there is no filename we should get one from save as..
}

void MainWindow::on_actionSave_File_As_triggered()
{QString fileName;
// check for the type of save loaded and set the output type so we don't save the wrong type, all conversion opperations should be done via an Export function.
    if(ff7->type() == "PC")
    {
        fileName = QFileDialog::getSaveFileName(this,
        tr("Save Final Fantasy 7 PC SaveGame"), settings->value("save_pc_path").toString(),
        tr("FF7 PC SaveGame(*.ff7)"));
    }
    else if(ff7->type() == "PSX")
    {
        fileName = QFileDialog::getSaveFileName(this,
        tr("Save Final Fantasy 7 PSX SaveGame"), QDir::homePath(),
        tr("FF7 Raw PSX SaveGame(*-S*)"));
    }
    else if(ff7->type() == "MC")
    {
        fileName = QFileDialog::getSaveFileName(this,
        tr("Save Final Fantasy 7 MC SaveGame"), settings->value("save_emu_path").toString(),
        tr("FF7 MC SaveGame(*.mcr *.mcd *.mci *.mc *.ddf *.ps *.psm *.VM1 *.bin)"));
    }
    else if(ff7->type() == "PSV")
    {
        fileName = QFileDialog::getSaveFileName(this,
        tr("Save Final Fantasy 7 PSV SaveGame"), QDir::homePath(),
        tr("FF7 PSV SaveGame(*.psv)"));
        QMessageBox::information(this,tr("PSV Save Notice"),tr("This File Does Not Have An Updated Checksum.It will not work on your PS3."));
    }
    else if(ff7->type() == "PSP")
    {
        fileName = QFileDialog::getSaveFileName(this,
        tr("Save Final Fantasy 7  PSP SaveGame"), QDir::homePath(),
        tr("FF7 PSP SaveGame(*.vmp)"));
        QMessageBox::information(this,tr("PSP Save Notice"),tr("This File Does Not Have An Updated Checksum.It will not work on your PSP."));
    }
    else if(ff7->type() == "VGS")
    {
        fileName = QFileDialog::getSaveFileName(this,
        tr("Save Final Fantasy 7  VGS SaveGame"), settings->value("save_emu_path").toString(),
        tr("FF7 VGS SaveGame(*.vgs *.mem)"));
    }
    else if(ff7->type() == "DEX")
    {
        fileName = QFileDialog::getSaveFileName(this,
        tr("Save Final Fantasy 7  Dex-Drive SaveGame"), settings->value("save_emu_path").toString(),
        tr("FF7 Dex SaveGame(*.gme)"));
    }
    else
    {//mystery type. make the user tell us user maybe used new game or made save manually.
        QStringList types;
        types << tr("PC")<<tr("Raw Psx Save")<<tr("Generic Emulator Memorycard")<<tr("PSP")<<tr("PS3")<<tr("Dex Drive Memorycard")<<tr("VGS Memorycard");
        QString result = QInputDialog::getItem(this,tr("Save Error"),tr("Please Select A File Type To Save"),types,-1,0,0,0);
        //check the string. and assign a type
        if(result ==types.at(0)){ff7->setType("PC");}
        else if(result ==types.at(1)){ff7->setType("PSX");}
        else if(result ==types.at(2)){ff7->setType("MC");}
        else if(result ==types.at(3)){ff7->setType("PSP");}
        else if(result ==types.at(4)){ff7->setType("PSV");}
        else if(result ==types.at(5)){ff7->setType("DEX");}
        else if(result ==types.at(6)){ff7->setType("VGS");}
        else{return;}
            on_actionSave_File_As_triggered(); //now that we have a type do again.
    }
    if(fileName.isEmpty()){return;}
    saveFileFull(fileName); //reguardless save the file of course if its has a string.
}
/*~~~~~~~~~~~SHORT SAVE~~~~~~~~~~~~*/
void MainWindow::saveFileFull(QString fileName)
{
    if((ff7->type() =="PC") && !(settings->value("skip_slot_mask").toBool())){ff7->fix_pc_bytemask(s);}//fix starting slot on pc

    if(ff7->saveFile(fileName))
    {
        if(_init)
        {//if no save was loaded and new game was clicked be sure to act like a game was loaded.
            _init=false;
        }
        file_modified(false);
        guirefresh(0);
    }
    else{QMessageBox::information(this,tr("Save Error"),tr("Failed to save file\n%1").arg(fileName));}
}
/*~~~~~~~~~~~~~~~New_Game~~~~~~~~~~~*/
void MainWindow::on_actionNew_Game_triggered()
{
    QString save_name ="";
    if(settings->value("override_default_save").toBool()){save_name = settings->value("default_save_file").toString();}
    ff7->newGame(s,save_name);//call the new game function
    if(save_name =="")    {ui->statusBar->showMessage(tr("New Game Created"),2000);}
    else{ui->statusBar->showMessage(tr("New Game Created - File:%1").arg(save_name,2000));}
    //detect region and fix names if needed.
    _init=false;
    guirefresh(1);
}
/*~~~~~~~~~~End New_Game~~~~~~~~~~~*/
/*~~~~~~~~~~New Game + ~~~~~~~~~~~~*/
void MainWindow::on_actionNew_Game_Plus_triggered()
{
    QString save_name ="";
    if(settings->value("override_default_save").toBool()){save_name = settings->value("default_save_file").toString();}
    ff7->newGamePlus(s,ff7->fileName(),save_name);
    if(save_name==""){ui->statusBar->showMessage(tr("New Game Plus Created"),2000);}
    else{ui->statusBar->showMessage(tr("New Game Created - File:%1").arg(save_name),2000);}
    guirefresh(0);
}
/*~~~~~~~~~~End New_Game +~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~EXPORT PC~~~~~~~~~~~~~~~~~~~*/
void MainWindow::on_actionExport_PC_Save_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
    tr("Save Final Fantasy 7 SaveGame"),  settings->value("export_pc").toString() ,
    tr("FF7 SaveGame(*.ff7)")); // Only Allow PC save Since we are going to make one
    if(fileName.isEmpty()){return;}// catch if Cancel is pressed
    else
    {
        if(ff7->exportPC(fileName)){ui->statusBar->showMessage(tr("Export Successful"),1000);}
        else{ui->statusBar->showMessage(tr("Export Failed"),2000);}
        file_modified(false);
    }
}
/*~~~~~~~~~~~~~~~~~EXPORT PSX~~~~~~~~~~~~~~~~~~*/
void MainWindow::on_actionExport_PSX_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
    tr("Save Final Fantasy 7 SaveGame"), ff7->region(s),
    tr("BASCUS-94163FF7-Sxx(*-S*);;BESCES-00867FF7-Sxx(*-S*);;BESCES-00868FF7-Sxx(*-S*);;BESCES-00869FF7-Sxx(*-S*);;BESCES-00900FF7-Sxx(*-S*);;BISLPS-00700FF7-Sxx(*-S*);;BISLPS-01057FF7-Sxx(*-S*)"));
    if (fileName.isEmpty()){return;}// catch if Cancel is pressed
    else
    {
        if(ff7->exportPSX(s,fileName)){ui->statusBar->showMessage(tr("Export Successful"),1000);}
        else{ui->statusBar->showMessage(tr("Export Failed"),2000);}
        file_modified(false);
    }
}
/*~~~~~Export Mcr/Mcd~~~~~~*/
void MainWindow::on_actionExport_MC_triggered()
{

    QString fileName = QFileDialog::getSaveFileName(this,
    tr("Save Final Fantasy 7 MC SaveGame"), settings->value("save_emu_path").toString(),
    tr("MCR File(*.mcr);;MCD File(*.mcd);;MCI File(*.mci);;MC File(*.mc);;DDF File(*.ddf);;PS File(*.ps);;PSM File(*.psm);;BIN File(*.bin);;PS3 Virtual Memory Card(*.VM1)"));
    if(fileName.isEmpty()){return;}
    else
    {
        if(ff7->exportVMC(fileName)){ui->statusBar->showMessage(tr("Export Successful"),1000);}
        else{ui->statusBar->showMessage(tr("Export Failed"),2000);}
        file_modified(false);
    }
}
void MainWindow::on_actionExport_VGS_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
    tr("Save Final Fantasy 7 VGS SaveGame"), settings->value("save_emu_path").toString(),
    tr("VGS File(*.vgs);;MEM File(*.mem)"));
    if(fileName.isEmpty()){return;}
    else
    {
        if(ff7->exportVGS(fileName)){ui->statusBar->showMessage(tr("Export Successful"),1000);}
        else{ui->statusBar->showMessage(tr("Export Failed"),2000);}
        file_modified(false);
    }
}
void MainWindow::on_actionExport_DEX_triggered()
{

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Final Fantasy 7 Dex-Drive SaveGame"), settings->value("save_emu_path").toString(),tr("FF7 Dex SaveGame(*.gme)"));
    if(fileName.isEmpty()){return;}
    else
    {
        if(ff7->exportDEX(fileName)){ui->statusBar->showMessage(tr("Export Successful"),1000);}
        else{ui->statusBar->showMessage(tr("Export Failed"),2000);}
        file_modified(false);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~END LOAD/SAVE FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MENU ACTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~Simple Menu Stuff~~~~~~~~~~~~~~~~*/
void MainWindow::on_actionSlot_01_triggered(){s=0; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_02_triggered(){s=1; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_03_triggered(){s=2; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_04_triggered(){s=3; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_05_triggered(){s=4; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_06_triggered(){s=5; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_07_triggered(){s=6; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_08_triggered(){s=7; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_09_triggered(){s=8; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_10_triggered(){s=9; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_11_triggered(){s=10; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_12_triggered(){s=11; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_13_triggered(){s=12; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_14_triggered(){s=13; CheckGame(); guirefresh(0);}
void MainWindow::on_actionSlot_15_triggered(){s=14; CheckGame(); guirefresh(0);}
void MainWindow::on_actionClear_Slot_triggered(){ff7->clearSlot(s);  guirefresh(0);}

void MainWindow::on_actionShow_Selection_Dialog_triggered(){SlotSelect slotselect(0,ff7);slotselect.setStyleSheet(this->styleSheet());s=slotselect.exec();CheckGame(); guirefresh(0);}
void MainWindow::on_actionPrevious_Slot_triggered(){if(ff7->type()==""){return;}else{if (s > 0) {s--; CheckGame(); guirefresh(0);}}}
void MainWindow::on_actionNext_Slot_triggered(){if(ff7->type()==""){return;}else{if (s<14){s++; CheckGame(); guirefresh(0);}}}
void MainWindow::on_actionAbout_triggered(){about adialog;  adialog.setStyleSheet(this->styleSheet()); adialog.exec();}
void MainWindow::on_actionCopy_Slot_triggered(){ff7->copySlot(s);}
void MainWindow::on_actionPaste_Slot_triggered(){ff7->pasteSlot(s); guirefresh(0);}
void MainWindow::on_actionShow_Options_triggered(){Options odialog(0,settings); odialog.setStyleSheet(this->styleSheet()); odialog.exec(); init_settings(); }
void MainWindow::on_actionCreateNewMetadata_triggered(){ MetadataCreator mdata(this,ff7); mdata.setStyleSheet(this->styleSheet()); mdata.exec();}

void MainWindow::on_action_auto_char_growth_triggered(bool checked)
{
    if(checked)
    {
        settings->setValue("autochargrowth",1);
        if(!load){char_editor->setAutoLevel(1); char_editor->setAutoStatCalc(1);}
    }
    else
    {
        settings->setValue("autochargrowth",0);
        char_editor->setAutoLevel(0); char_editor->setAutoStatCalc(0);
    }
}
void MainWindow::on_action_show_debug_toggled(bool checked)
{
    if(checked)
    {
        ui->actionNew_Window->setVisible(1);
        ui->tabWidget->setTabEnabled(9,1);
        ui->cb_Region_Slot->setEnabled(true);
        ui->bm_unknown->setVisible(true);
        ui->bh_id->setVisible(true);
        ui->leader_id->setVisible(true);
        if(ff7->type() == "PC"){optionsWidget->setControllerMappingVisible(true);}
        settings->setValue("show_test",1);
        ui->cb_farm_items_1->setVisible(true);
        ui->cb_farm_items_2->setVisible(true);
        ui->cb_farm_items_3->setVisible(true);
        ui->cb_farm_items_4->setVisible(true);
        ui->cb_farm_items_5->setVisible(true);
        ui->cb_farm_items_6->setVisible(true);
        ui->cb_farm_items_7->setVisible(true);
        ui->cb_farm_items_8->setVisible(true);
        testdata_refresh();
    }

    else
    {
        ui->actionNew_Window->setVisible(0);
        ui->tabWidget->setTabEnabled(9,0);
        ui->cb_Region_Slot->setEnabled(false);
        ui->bm_unknown->setVisible(false);
        ui->bh_id->setVisible(false);
        ui->leader_id->setVisible(false);
        if(ff7->type() =="PC"){optionsWidget->setControllerMappingVisible(false);}
        settings->setValue("show_test",0);
        ui->cb_farm_items_1->setVisible(false);
        ui->cb_farm_items_2->setVisible(false);
        ui->cb_farm_items_3->setVisible(false);
        ui->cb_farm_items_4->setVisible(false);
        ui->cb_farm_items_5->setVisible(false);
        ui->cb_farm_items_6->setVisible(false);
        ui->cb_farm_items_7->setVisible(false);
        ui->cb_farm_items_8->setVisible(false);
    }
}

/*~~~~~~~~~~~~LANGUAGE & REGION ACTIONS~~~~~~~~~~~~~~*/
void MainWindow::on_action_Lang_en_triggered()
{
    //clear other lang
    ui->action_Lang_es->setChecked(Qt::Unchecked);
    ui->action_Lang_fr->setChecked(Qt::Unchecked);
    ui->action_Lang_jp->setChecked(Qt::Unchecked);
    ui->action_Lang_de->setChecked(Qt::Unchecked);
    settings->setValue("lang","en");
    QMessageBox::information(this,"Language Changed","You Must Restart For The Language to Change");
}
void MainWindow::on_action_Lang_es_triggered()
{
    ui->action_Lang_en->setChecked(Qt::Unchecked);
    ui->action_Lang_fr->setChecked(Qt::Unchecked);
    ui->action_Lang_jp->setChecked(Qt::Unchecked);
    ui->action_Lang_de->setChecked(Qt::Unchecked);
    settings->setValue("lang","es");
    QMessageBox::information(this,QString::fromUtf8("Idioma Cambiado"),QString::fromUtf8("Debe reiniciar Para el cambio de idioma"));
}
void MainWindow::on_action_Lang_fr_triggered()
{
    ui->action_Lang_en->setChecked(Qt::Unchecked);
    ui->action_Lang_es->setChecked(Qt::Unchecked);
    ui->action_Lang_jp->setChecked(Qt::Unchecked);
    ui->action_Lang_de->setChecked(Qt::Unchecked);
    settings->setValue("lang","fr");
    QMessageBox::information(this,QString::fromUtf8("Langue Modifiée"),QString::fromUtf8("Vous Devez Redemarrer Pour Changer la Langue"));
}
void MainWindow::on_action_Lang_de_triggered()
{
    ui->action_Lang_en->setChecked(Qt::Unchecked);
    ui->action_Lang_es->setChecked(Qt::Unchecked);
    ui->action_Lang_fr->setChecked(Qt::Unchecked);
    settings->setValue("lang","de");
    QMessageBox::information(this,QString::fromUtf8("Sprache geändert"),QString::fromUtf8("Neustarten um Sprache zu ändern"));
}
void MainWindow::on_action_Lang_jp_triggered()
{
    ui->action_Lang_en->setChecked(Qt::Unchecked);
    ui->action_Lang_es->setChecked(Qt::Unchecked);
    ui->action_Lang_fr->setChecked(Qt::Unchecked);
    ui->action_Lang_de->setChecked(Qt::Unchecked);
    settings->setValue("lang","ja");
    QMessageBox::information(this,QString::fromUtf8("言語の変更"),QString::fromUtf8("プログラムを再起動して言語の変更を適用してください"));
}
/*~~~~~~~~~~~~~SET USA MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::on_action_Region_USA_triggered(bool checked)
{if(!load){
    if(!checked)
    {
        ff7->setRegion(s,"");
        ui->lbl_sg_region->clear();
        itemlist->setMaximumItemQty(127);
    }
    else
    {
        if(ff7->isPAL(s)){set_ntsc_time();}//Convert Time?
        ff7->setRegion(s,"NTSC-U");
        itemlist->setMaximumItemQty(127);
        ui->action_Region_PAL_Generic->setChecked(false);
        ui->action_Region_PAL_French->setChecked(false);
        ui->action_Region_PAL_German->setChecked(false);
        ui->action_Region_PAL_Spanish->setChecked(false);
        ui->action_Region_JPN->setChecked(false);
        ui->action_Region_JPN_International->setChecked(false);
        ui->lbl_sg_region->setText(ff7->region(s).mid(0,ff7->region(s).lastIndexOf("-")+1));
        ui->cb_Region_Slot->setCurrentIndex(ff7->region(s).mid(ff7->region(s).lastIndexOf("S")+1,2).toInt()-1);
    }
}}
/*~~~~~~~~~~~~~SET PAL MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::on_action_Region_PAL_Generic_triggered(bool checked)
{if(!load){
    if(!checked)
    {
        ff7->setRegion(s,"");
        ui->lbl_sg_region->clear();
        itemlist->setMaximumItemQty(127);
    }
    else
    {
        if(ff7->isNTSC(s)){set_pal_time();}//Call RegionTime Convertor
        ff7->setRegion(s,"PAL-E");
        itemlist->setMaximumItemQty(127);
        ui->action_Region_USA->setChecked(false);
        ui->action_Region_PAL_German->setChecked(false);
        ui->action_Region_PAL_French->setChecked(false);
        ui->action_Region_PAL_Spanish->setChecked(false);
        ui->action_Region_JPN->setChecked(false);
        ui->action_Region_JPN_International->setChecked(false);
        ui->lbl_sg_region->setText(ff7->region(s).mid(0,ff7->region(s).lastIndexOf("-")+1));
        ui->cb_Region_Slot->setCurrentIndex(ff7->region(s).mid(ff7->region(s).lastIndexOf("S")+1,2).toInt()-1);
        //Text.init(0);
    }
}}
/*~~~~~~~~~~~~~SET PAL_German MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::on_action_Region_PAL_German_triggered(bool checked)
{if(!load){
    if(!checked)
    {
        ff7->setRegion(s,"");
        ui->lbl_sg_region->clear();
        itemlist->setMaximumItemQty(127);
    }
    else
    {
        if(ff7->isNTSC(s)){set_pal_time();}//Call RegionTime Convertor
        ff7->setRegion(s,"PAL-DE");
        itemlist->setMaximumItemQty(127);
        ui->action_Region_USA->setChecked(false);
        ui->action_Region_PAL_Generic->setChecked(false);
        ui->action_Region_PAL_French->setChecked(false);
        ui->action_Region_PAL_Spanish->setChecked(false);
        ui->action_Region_JPN->setChecked(false);
        ui->action_Region_JPN_International->setChecked(false);
        ui->lbl_sg_region->setText(ff7->region(s).mid(0,ff7->region(s).lastIndexOf("-")+1));
        ui->cb_Region_Slot->setCurrentIndex(ff7->region(s).mid(ff7->region(s).lastIndexOf("S")+1,2).toInt()-1);
    }
}}
/*~~~~~~~~~~~~~SET PAL_Spanish MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::on_action_Region_PAL_Spanish_triggered(bool checked)
{if(!load){
    if(!checked)
    {
        ff7->setRegion(s,"");
        ui->lbl_sg_region->clear();
        itemlist->setMaximumItemQty(127);
    }
    else
    {
        if(ff7->isNTSC(s)){set_pal_time();}//Call RegionTime Convertor
        ff7->setRegion(s,"PAL-ES");
        itemlist->setMaximumItemQty(127);
        ui->action_Region_USA->setChecked(false);
        ui->action_Region_PAL_Generic->setChecked(false);
        ui->action_Region_PAL_French->setChecked(false);
        ui->action_Region_PAL_German->setChecked(false);
        ui->action_Region_JPN->setChecked(false);
        ui->action_Region_JPN_International->setChecked(false);
        ui->lbl_sg_region->setText(ff7->region(s).mid(0,ff7->region(s).lastIndexOf("-")+1));
        ui->cb_Region_Slot->setCurrentIndex(ff7->region(s).mid(ff7->region(s).lastIndexOf("S")+1,2).toInt()-1);
    }
}}
/*~~~~~~~~~~~~~SET PAL_French MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::on_action_Region_PAL_French_triggered(bool checked)
{if(!load){
    if(!checked)
    {
        ff7->setRegion(s,"");
        ui->lbl_sg_region->clear();
        itemlist->setMaximumItemQty(127);
    }
    else
    {
        if(ff7->isNTSC(s)){set_pal_time();}//Call RegionTime Convertor
        ff7->setRegion(s,"PAL-FR");
        itemlist->setMaximumItemQty(127);
        ui->action_Region_USA->setChecked(false);
        ui->action_Region_PAL_Generic->setChecked(false);
        ui->action_Region_PAL_Spanish->setChecked(false);
        ui->action_Region_PAL_German->setChecked(false);
        ui->action_Region_JPN->setChecked(false);
        ui->action_Region_JPN_International->setChecked(false);
        ui->lbl_sg_region->setText(ff7->region(s).mid(0,ff7->region(s).lastIndexOf("-")+1));
        ui->cb_Region_Slot->setCurrentIndex(ff7->region(s).mid(ff7->region(s).lastIndexOf("S")+1,2).toInt()-1);
    }
}}
/*~~~~~~~~~~~~~SET JPN MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::on_action_Region_JPN_triggered(bool checked)
{if(!load){
    if(!checked)
    {
        ff7->setRegion(s,"");
        ui->lbl_sg_region->clear();
        itemlist->setMaximumItemQty(127);
    }
    else
    {//First Check If Coming From PAL
        if(ff7->isPAL(s)){set_ntsc_time();}//Convert Time?
        ff7->setRegion(s,"NTSC-J");
        itemlist->setMaximumItemQty(99);
        ui->action_Region_USA->setChecked(false);
        ui->action_Region_PAL_Generic->setChecked(false);
        ui->action_Region_PAL_French->setChecked(false);
        ui->action_Region_PAL_German->setChecked(false);
        ui->action_Region_PAL_Spanish->setChecked(false);
        ui->action_Region_JPN_International->setChecked(false);
        ui->lbl_sg_region->setText(ff7->region(s).mid(0,ff7->region(s).lastIndexOf("-")+1));
        ui->cb_Region_Slot->setCurrentIndex(ff7->region(s).mid(ff7->region(s).lastIndexOf("S")+1,2).toInt()-1);
    }
}}
/*~~~~~~~~~~~~~SET JPN_International MC HEADER~~~~~~~~~~~~~~~~*/
void MainWindow::on_action_Region_JPN_International_triggered(bool checked)
{if(!load){
    if(!checked)
    {
        ff7->setRegion(s,"");
        ui->lbl_sg_region->clear();
        itemlist->setMaximumItemQty(127);
    }
    else
    {
        if(ff7->isPAL(s)){set_ntsc_time();}//Convert Time?
        ff7->setRegion(s,"NTSC-JI");
        itemlist->setMaximumItemQty(127);
        ui->action_Region_USA->setChecked(false);
        ui->action_Region_PAL_Generic->setChecked(false);
        ui->action_Region_PAL_French->setChecked(false);
        ui->action_Region_PAL_German->setChecked(false);
        ui->action_Region_PAL_Spanish->setChecked(false);
        ui->action_Region_JPN->setChecked(false);
        ui->lbl_sg_region->setText(ff7->region(s).mid(0,ff7->region(s).lastIndexOf("-")+1));
        ui->cb_Region_Slot->setCurrentIndex(ff7->region(s).mid(ff7->region(s).lastIndexOf("S")+1,2).toInt()-1);
    }
}}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~END MENU ACTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~GUI FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~Set Menu Items~~~~~~~~~~*/
void MainWindow::setmenu(bool newgame)
{
    load=true;
    /*~~Disable All Items that are dependent on File Type~~*/
    ui->actionSlot_01->setChecked(Qt::Unchecked);    ui->actionSlot_02->setChecked(Qt::Unchecked);    ui->actionSlot_03->setChecked(Qt::Unchecked);    ui->actionSlot_04->setChecked(Qt::Unchecked);
    ui->actionSlot_05->setChecked(Qt::Unchecked);    ui->actionSlot_06->setChecked(Qt::Unchecked);    ui->actionSlot_07->setChecked(Qt::Unchecked);    ui->actionSlot_08->setChecked(Qt::Unchecked);
    ui->actionSlot_09->setChecked(Qt::Unchecked);    ui->actionSlot_10->setChecked(Qt::Unchecked);    ui->actionSlot_11->setChecked(Qt::Unchecked);    ui->actionSlot_12->setChecked(Qt::Unchecked);
    ui->actionSlot_13->setChecked(Qt::Unchecked);    ui->actionSlot_14->setChecked(Qt::Unchecked);    ui->actionSlot_15->setChecked(Qt::Unchecked);     ui->actionClear_Slot->setEnabled(0);
    ui->action_Region_USA->setChecked(Qt::Unchecked);    ui->action_Region_PAL_Generic->setChecked(Qt::Unchecked);  ui->action_Region_PAL_German->setChecked(Qt::Unchecked);
    ui->action_Region_PAL_French->setChecked(Qt::Unchecked);ui->action_Region_PAL_Spanish->setChecked(Qt::Unchecked);    ui->action_Region_JPN->setChecked(Qt::Unchecked);
    ui->action_Region_JPN_International->setChecked(Qt::Unchecked);    ui->actionNext_Slot->setEnabled(0);ui->actionPrevious_Slot->setEnabled(0);
    ui->actionShow_Selection_Dialog->setEnabled(0);ui->actionSlot_01->setEnabled(0);ui->actionSlot_02->setEnabled(0);
    ui->actionSlot_03->setEnabled(0);ui->actionSlot_04->setEnabled(0);ui->actionSlot_05->setEnabled(0);
    ui->actionSlot_06->setEnabled(0);ui->actionSlot_07->setEnabled(0);ui->actionSlot_08->setEnabled(0);
    ui->actionSlot_09->setEnabled(0);ui->actionSlot_10->setEnabled(0);ui->actionSlot_11->setEnabled(0);
    ui->actionSlot_12->setEnabled(0);ui->actionSlot_13->setEnabled(0);ui->actionSlot_14->setEnabled(0);
    ui->actionSlot_15->setEnabled(0);ui->actionNew_Game->setEnabled(0);ui->compare_table->setEnabled(0);
    ui->lbl_current_slot_num->clear(); ui->lbl_current_slot_txt->clear();
    /*~~End Clear Menu Items~~*/
    /*~~~~~~Current Slot~~~~~~*/
    switch(s)
    {
        case 0:ui->actionSlot_01->setChecked(Qt::Checked);break;
        case 1:ui->actionSlot_02->setChecked(Qt::Checked);break;
        case 2:ui->actionSlot_03->setChecked(Qt::Checked);break;
        case 3:ui->actionSlot_04->setChecked(Qt::Checked);break;
        case 4:ui->actionSlot_05->setChecked(Qt::Checked);break;
        case 5:ui->actionSlot_06->setChecked(Qt::Checked);break;
        case 6:ui->actionSlot_07->setChecked(Qt::Checked);break;
        case 7:ui->actionSlot_08->setChecked(Qt::Checked);break;
        case 8:ui->actionSlot_09->setChecked(Qt::Checked);break;
        case 9:ui->actionSlot_10->setChecked(Qt::Checked);break;
        case 10:ui->actionSlot_11->setChecked(Qt::Checked);break;
        case 11:ui->actionSlot_12->setChecked(Qt::Checked);break;
        case 12:ui->actionSlot_13->setChecked(Qt::Checked);break;
        case 13:ui->actionSlot_14->setChecked(Qt::Checked);break;
        case 14:ui->actionSlot_15->setChecked(Qt::Checked);break;
    }
    /*~~~~End Current Slot~~~~~*/
    /*~~~~~~~Set Actions By Type~~~~~~~*/
    //For first file load.Don't Bother to disable these again.
    //new game should always be exported. no header...
    if(!newgame)
    {
        ui->actionSave_File_As->setEnabled(1);
        ui->actionReload->setEnabled(1);
    }
    ui->actionExport_PC_Save->setEnabled(1);    ui->actionExport_PSX->setEnabled(1);
    ui->actionExport_MC->setEnabled(1);         ui->actionExport_VGS->setEnabled(1);
    ui->actionExport_DEX->setEnabled(1);        ui->actionExport_char->setEnabled(1);
    ui->actionImport_char->setEnabled(1);       ui->action_Save->setEnabled(1);

    if(!_init)
    {//we haven't loaded a file yet.
        ui->actionNew_Game_Plus->setEnabled(1); ui->actionFrom_PSV_Slot->setEnabled(1);
        ui->actionFrom_PSX_Slot->setEnabled(1); ui->actionCopy_Slot->setEnabled(1);
        ui->actionPaste_Slot->setEnabled(1);
    }

    if ( (ff7->type()!= "PSX" && ff7->type() !="PSV" && (!_init)) && (ff7->type()!="") ) //more then one slot, or unknown Type
    {
        ui->actionSlot_01->setEnabled(1);   ui->actionNext_Slot->setEnabled(1);
        ui->actionSlot_02->setEnabled(1);   ui->actionPrevious_Slot->setEnabled(1);
        ui->actionSlot_03->setEnabled(1);   ui->actionShow_Selection_Dialog->setEnabled(1);
        ui->actionSlot_04->setEnabled(1);   ui->actionClear_Slot->setEnabled(1);
        ui->actionSlot_05->setEnabled(1);   ui->actionNew_Game->setEnabled(1);
        ui->actionSlot_06->setEnabled(1);   ui->actionSlot_07->setEnabled(1);
        ui->actionSlot_08->setEnabled(1);   ui->actionSlot_09->setEnabled(1);
        ui->actionSlot_10->setEnabled(1);   ui->actionSlot_11->setEnabled(1);
        ui->actionSlot_12->setEnabled(1);   ui->actionSlot_13->setEnabled(1);
        ui->actionSlot_14->setEnabled(1);   ui->actionSlot_15->setEnabled(1);
        ui->compare_table->setEnabled(1);   ui->lbl_current_slot_txt->setText(tr("Current Slot:"));
        ui->lbl_current_slot_num->setNum(s+1); ui->actionClear_Slot->setEnabled(1);
    }
    /*~~~End Set Actions By Type~~~*/
    /*~~Set Detected Region ~~*/
    if(ff7->region(s).contains("94163")){ui->action_Region_USA->setChecked(Qt::Checked);}
    else if(ff7->region(s).contains("00867")){ui->action_Region_PAL_Generic->setChecked(Qt::Checked);}
    else if(ff7->region(s).contains("00868")){ui->action_Region_PAL_French->setChecked(Qt::Checked);}
    else if(ff7->region(s).contains("00869")){ui->action_Region_PAL_German->setChecked(Qt::Checked);}
    else if(ff7->region(s).contains("00900")){ui->action_Region_PAL_Spanish->setChecked(Qt::Checked);}
    else if(ff7->region(s).contains("00700")){ui->action_Region_JPN->setChecked(Qt::Checked);}
    else if(ff7->region(s).contains("01057")){ui->action_Region_JPN_International->setChecked(Qt::Checked);}
    else if(ff7->region(s).isEmpty()){/*do nothing*/}
    /*~~End Detected Region~~*/
    else
    {//not FF7 unset some menu options.
            ui->actionNew_Game_Plus->setEnabled(0); ui->actionCopy_Slot->setEnabled(0);
            ui->actionExport_PC_Save->setEnabled(0);    ui->actionExport_char->setEnabled(0);
            ui->actionImport_char->setEnabled(0);
    }
   load=false;
}
void MainWindow::file_modified(bool changed)
{
    ff7->setFileModified(changed,s);
    ui->lbl_fileName->setText(ff7->fileName());

    if(changed)
    {    
        hexEditorRefresh();
        ui->lbl_fileName->setText(ui->lbl_fileName->text().append("*"));
    }
}

/*~~~~~~~~~End Set Menu~~~~~~~~~~~*/
void MainWindow::set_ntsc_time(void)
{
    int result;
    QMessageBox fixtime(this);
    fixtime.setIconPixmap(QPixmap(":/icon/fix_time"));
    fixtime.setText(tr("Would you like to correct the play time?"));
    fixtime.setInformativeText(tr("In this region the game runs 60hz"));
    fixtime.setWindowTitle(tr("PAL -> NTSC Conversion"));
    fixtime.addButton(QMessageBox::Yes);
    fixtime.addButton(QMessageBox::No);
    result=fixtime.exec();

    switch(result)
    {
        case QMessageBox::Yes:
            ff7->setTime(s,ff7->time(s)*1.2);
            load=true;
            ui->sb_time_hour->setValue(ff7->time(s) / 3600);
            ui->sb_time_min->setValue(ff7->time(s)/60%60);
            ui->sb_time_sec->setValue(ff7->time(s) -((ui->sb_time_hour->value()*3600)+ui->sb_time_min->value()*60));
            load=false;
        break;
        case QMessageBox::Cancel:break;
    }
}
void MainWindow::set_pal_time(void)
{
    int result;
    QMessageBox fixtime(this);
    fixtime.setIconPixmap(QPixmap(":/icon/fix_time"));
    fixtime.setText(tr("Would you like to correct the play time?"));
    fixtime.setInformativeText(tr("In this region the game runs 50hz"));
    fixtime.setWindowTitle(tr("NTSC -> PAL Conversion"));
    fixtime.addButton(QMessageBox::Yes);
    fixtime.addButton(QMessageBox::No);
    result=fixtime.exec();

    switch(result)
    {
        case QMessageBox::Yes:
        ff7->setTime(s,ff7->time(s)/1.2);
            load=true;
            ui->sb_time_hour->setValue(ff7->time(s) / 3600);
            ui->sb_time_min->setValue(ff7->time(s)/60%60);
            ui->sb_time_sec->setValue(ff7->time(s) -((ui->sb_time_hour->value()*3600)+ui->sb_time_min->value()*60));
            load=false;
        break;
        case QMessageBox::Cancel:break;
    }
}
void MainWindow::materiaupdate(void)
{
    load=true;
    QTableWidgetItem *newItem;
    int j= ui->tbl_materia->currentRow();
    ui->tbl_materia->reset();
    ui->tbl_materia->clearContents();
    ui->tbl_materia->setColumnWidth(0,(ui->tbl_materia->width()*.65));
    ui->tbl_materia->setColumnWidth(1,(ui->tbl_materia->width()*.25));
    ui->tbl_materia->setRowCount(200);

    for(int mat=0;mat<200;mat++)// partys materias
    {
        qint32 current_ap= ff7->partyMateriaAp(s,mat);
        quint8 current_id= ff7->partyMateriaId(s,mat);
        QString ap;

        if(current_id == FF7Materia::EnemySkill)
        {
            newItem = new QTableWidgetItem(Materias.icon(current_id),Materias.name(current_id),0);
            ui->tbl_materia->setItem(mat,0,newItem);
            if (current_ap == FF7Materia::MaxMateriaAp){newItem =new QTableWidgetItem(tr("Master"));ui->tbl_materia->setItem(mat,1,newItem);}
            else{newItem =new QTableWidgetItem(QString("N/A"),0);ui->tbl_materia->setItem(mat,1,newItem);}
        }
        else if (current_id !=FF7Materia::EmptyId)
        {
            newItem = new QTableWidgetItem(Materias.icon(current_id),Materias.name(current_id),0);
            ui->tbl_materia->setItem(mat,0,newItem);
            if (current_ap == FF7Materia::MaxMateriaAp){newItem =new QTableWidgetItem(tr("Master"));ui->tbl_materia->setItem(mat,1,newItem);}
            else{newItem =new QTableWidgetItem(ap.setNum(current_ap));ui->tbl_materia->setItem(mat,1,newItem);}
        }
        else
        {
            //We need to clear invalid to prevent data issues. to keep file changes correct we back up our change vars and replace later.
            bool fileTemp=ff7->isFileModified();
            bool slotTemp[15];
            for(int i=0;i<15;i++){slotTemp[i] = ff7->isSlotModified(i);}

            ff7->setPartyMateria(s,mat,FF7Materia::EmptyId,FF7Materia::MaxMateriaAp);//invalid insure its clear.
            newItem = new QTableWidgetItem(tr("===Empty Slot==="),0);
            ui->tbl_materia->setItem(mat,0,newItem);
            newItem = new QTableWidgetItem("",0);
            ui->tbl_materia->setItem(mat,1,newItem);

            if(fileTemp)
            {//file was changed need to set what slots changed.
                for(int i=0;i<15;i++)
                {
                    if(slotTemp[i])
                    {//reset slots marked changed.
                        ff7->setFileModified(true,i);
                    }
                }
             }
            else{ff7->setFileModified(false,0);}
        }
    }
    if(ff7->partyMateriaId(s,j) == FF7Materia::EnemySkill){mat_spacer->changeSize(0,0,QSizePolicy::Fixed,QSizePolicy::Fixed);}
    else{mat_spacer->changeSize(0,0,QSizePolicy::Fixed,QSizePolicy::MinimumExpanding);}
    materia_editor->setMateria(ff7->partyMateriaId(s,j),ff7->partyMateriaAp(s,j));
    ui->tbl_materia->setCurrentCell(j,1);//so that right side is set correctly.
    load=false;
}
void MainWindow::materia_ap_changed(qint32 ap)
{if(!load){
    //if(ap>=materia_editor->MaxAP()){ap=16777215;}
    ff7->setPartyMateria(s,ui->tbl_materia->currentRow(),ff7->partyMateriaId(s,ui->tbl_materia->currentRow()),ap);
    materiaupdate();
}}
void MainWindow::materia_id_changed(qint8 id)
{if(!load){
    ff7->setPartyMateria(s,ui->tbl_materia->currentRow(),id,ff7->partyMateriaAp(s,ui->tbl_materia->currentRow()));
    materiaupdate();
}}

void MainWindow::itemupdate(void)
{
    load=true;
    //Field Items Picked up
    if(ff7->unknown(s,38).at(48) & (1<<0)){ui->cb_bm_items_1->setChecked(Qt::Checked);}    else{ui->cb_bm_items_1->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,38).at(48) & (1<<1)){ui->cb_bm_items_2->setChecked(Qt::Checked);}    else{ui->cb_bm_items_2->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,38).at(48) & (1<<2)){ui->cb_bm_items_3->setChecked(Qt::Checked);}    else{ui->cb_bm_items_3->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,38).at(48) & (1<<3)){ui->cb_bm_items_4->setChecked(Qt::Checked);}    else{ui->cb_bm_items_4->setChecked(Qt::Unchecked);}

    ui->cb_itemmask1_1->setChecked(ff7->itemMask1(s,0));
    ui->cb_itemmask1_2->setChecked(ff7->itemMask1(s,1));
    ui->cb_itemmask1_3->setChecked(ff7->itemMask1(s,2));
    ui->cb_itemmask1_4->setChecked(ff7->itemMask1(s,3));
    ui->cb_itemmask1_5->setChecked(ff7->itemMask1(s,4));
    ui->cb_itemmask1_6->setChecked(ff7->itemMask1(s,5));
    ui->cb_itemmask1_7->setChecked(ff7->itemMask1(s,6));
    ui->cb_itemmask1_8->setChecked(ff7->itemMask1(s,7));
    ui->cb_gaiin_1Javelin->setChecked(ff7->gaiin_1Javelin(s));
    ui->cb_gaiin_1Ribbon->setChecked(ff7->gaiin_1Ribbon(s));
    ui->cb_gaiin_3Elixir->setChecked(ff7->gaiin_3Elixir(s));
    ui->cb_gaiin_3SpeedSource->setChecked(ff7->gaiin_3SpeedSource(s));
    ui->cb_gaiin_4EnhanceSword->setChecked(ff7->gaiin_4EnhanceSword(s));
    ui->cb_gaiin_5Elixir->setChecked(ff7->gaiin_5Elixir(s));
    ui->cb_gaiin_5FireArmlet->setChecked(ff7->gaiin_5FireArmlet(s));
    ui->cb_sninn2XPotion->setChecked(ff7->sninn2XPotion(s));
    ui->cb_snmayorTurboEther->setChecked(ff7->snmayorTurboEther(s));
    ui->cb_snmin2HeroDrink->setChecked(ff7->snmin2HeroDrink(s));
    ui->cb_snmin2Vaccine->setChecked(ff7->snmin2Vaccine(s));
    ui->cb_trnad_4PoisonRing->setChecked(ff7->trnad_4PoisonRing(s));
    ui->cb_trnad_4MpTurbo->setChecked(ff7->trnad_4MpTurbo(s));
    ui->cb_trnad_3KaiserKnuckle->setChecked(ff7->trnad_3KaiserKnuckle(s));
    ui->cb_trnad_2NeoBahmut->setChecked(ff7->trnad_2NeoBahmut(s));

    if(ff7->unknown(s,9).at(4) & (1<<0)){ui->cb_s7tg_items_1->setChecked(Qt::Checked);}    else{ui->cb_s7tg_items_1->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,9).at(4) & (1<<1)){ui->cb_s7tg_items_2->setChecked(Qt::Checked);}    else{ui->cb_s7tg_items_2->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,9).at(4) & (1<<2)){ui->cb_s7tg_items_3->setChecked(Qt::Checked);}    else{ui->cb_s7tg_items_3->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,9).at(4) & (1<<3)){ui->cb_s7tg_items_4->setChecked(Qt::Checked);}    else{ui->cb_s7tg_items_4->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,9).at(4) & (1<<4)){ui->cb_s7tg_items_5->setChecked(Qt::Checked);}    else{ui->cb_s7tg_items_5->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,9).at(4) & (1<<5)){ui->cb_s7tg_items_6->setChecked(Qt::Checked);}    else{ui->cb_s7tg_items_6->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,9).at(4) & (1<<6)){ui->cb_s7tg_items_7->setChecked(Qt::Checked);}    else{ui->cb_s7tg_items_7->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,9).at(4) & (1<<7)){ui->cb_s7tg_items_8->setChecked(Qt::Checked);}    else{ui->cb_s7tg_items_8->setChecked(Qt::Unchecked);}
    load=false;
}
void MainWindow::CheckGame()
{
    if((!ff7->isFF7(s) && !ff7->region(s).isEmpty()) ||
      ((!ff7->isFF7(s))&& (ff7->type() =="MC" || ff7->type() =="VGS" ||ff7->type() =="DEX" ||ff7->type() =="PSP")
                       && (ff7->psx_block_type(s) !=0xA0)))
    {// NOT FF7
        errbox error(0,ff7,s);
        error.setStyleSheet(this->styleSheet());
        switch(error.exec())
        {
        case 0://View Anyway..
            ui->tabWidget->setCurrentIndex(8);
            if(ui->combo_hexEditor->currentIndex()!=0){ui->combo_hexEditor->setCurrentIndex(0);}
            hexEditorRefresh();
            ui->lbl_sg_region->setText(ff7->region(s));
            ui->lbl_slot_icon->setPixmap(SaveIcon(ff7->slot_header(s).mid(96,160)).icon().scaledToHeight(64,Qt::SmoothTransformation));
            setmenu(0);
        break;

        case 1://Previous Clicked
            s--;
            guirefresh(0);
        break;

        case 2://Next Clicked
            s++;
            guirefresh(0);
        break;

        case 3://exported as psx
            on_actionShow_Selection_Dialog_triggered();
        break;
        }
    }
}
/*~~~~~~~~~~~~~~~~~~~~~GUIREFRESH~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::guirefresh(bool newgame)
{
    load=true;

    /*~~~~Check for SG type and ff7~~~~*/
    if((!ff7->isFF7(s) && !ff7->region(s).isEmpty()) ||
      ((!ff7->isFF7(s))&& (ff7->type() =="MC" || ff7->type() =="VGS" ||ff7->type() =="DEX" ||ff7->type() =="PSP")
                       && (ff7->psx_block_type(s) !=0xA0)))
    {
        // NOT FF7 Do Nothing , Handled By CheckGame()
    }
    else
    {//IS FF7 Slot
        if((ff7->type()=="PC") || (ff7->type()=="")){if(ui->combo_hexEditor->currentIndex()!=1){ui->combo_hexEditor->setCurrentIndex(1);}}
        QByteArray text;
        if(ff7->region(s).isEmpty()
           && (ff7->type() =="MC" || ff7->type() =="VGS" ||ff7->type() =="DEX" ||ff7->type() =="PSP")
           && ff7->psx_block_type(s)==0xA0)
        {//if empty region string and a virtual memcard format and dir frame says empty.
            ff7->clearSlot(s); //file_modified(false);//checking only
        }
        //Clear all check boxes and index's
        ui->cb_replay->setCurrentIndex(0);
        ui->cb_bombing_int->setChecked(Qt::Unchecked);
        ui->cb_ruby_dead->setChecked(Qt::Unchecked);
        ui->cb_emerald_dead->setChecked(Qt::Unchecked);
        ui->cb_visible_bronco->setChecked(Qt::Unchecked);
        ui->cb_visible_buggy->setChecked(Qt::Unchecked);
        ui->cb_visible_highwind->setChecked(Qt::Unchecked);
        ui->cb_visible_wild_chocobo->setChecked(Qt::Unchecked);
        ui->cb_visible_yellow_chocobo->setChecked(Qt::Unchecked);
        ui->cb_visible_green_chocobo->setChecked(Qt::Unchecked);
        ui->cb_visible_blue_chocobo->setChecked(Qt::Unchecked);
        ui->cb_visible_black_chocobo->setChecked(Qt::Unchecked);
        ui->cb_visible_gold_chocobo->setChecked(Qt::Unchecked);
        if((ff7->slot[s].ruby_emerald) &(1<<3)){ui->cb_ruby_dead->setChecked(Qt::Checked);}
        if((ff7->slot[s].ruby_emerald)& (1<<4)){ui->cb_emerald_dead->setChecked(Qt::Checked);}
        //item_preview->setItem(-1);//reset.
        /*~~~~Set Region info and icon~~~~*/

        ui->lbl_sg_region->setText(ff7->region(s).mid(0,ff7->region(s).lastIndexOf("-")+1));
        ui->cb_Region_Slot->setCurrentIndex(ff7->region(s).mid(ff7->region(s).lastIndexOf("S")+1,2).toInt()-1);
        if (ff7->type() != "PC") //we Display an icon. for all formats except for pc
        {
            ui->lbl_slot_icon->setPixmap(SaveIcon(ff7->slot_header(s).mid(96,160)).icon().scaledToHeight(64,Qt::SmoothTransformation));
        }
        /*~~~~~Load Game Options~~~~~*/
        optionsWidget->setFieldHelp(ff7->fieldHelp(s));
        optionsWidget->setBattleHelp(ff7->battleHelp(s));
        optionsWidget->setBattleTargets(ff7->battleTargets(s));
        optionsWidget->setSoundMode(ff7->soundMode(s));

        optionsWidget->setControllerMode(ff7->controlMode(s));
        optionsWidget->setCursor(ff7->cursorMode(s));
        optionsWidget->setCamera(ff7->cameraMode(s));
        optionsWidget->setAtb(ff7->atbMode(s));
        optionsWidget->setMagicOrder(ff7->magicOrder(s));

        optionsWidget->setBattleSpeed(ff7->battleSpeed(s));
        optionsWidget->setBattleMessageSpeed(ff7->battleMessageSpeed(s));
        optionsWidget->setFieldMessageSpeed(ff7->messageSpeed(s));


        //CONTROLLER MAPPING
        optionsWidget->setBtnCamera(ff7->controllerMapping(s,FF7Save::ACTION_CAMERA));
        optionsWidget->setBtnTarget(ff7->controllerMapping(s,FF7Save::ACTION_TARGET));
        optionsWidget->setBtnPgUp(ff7->controllerMapping(s,FF7Save::ACTION_PAGEUP));
        optionsWidget->setBtnPgDn(ff7->controllerMapping(s,FF7Save::ACTION_PAGEDOWN));
        optionsWidget->setBtnMenu(ff7->controllerMapping(s,FF7Save::ACTION_MENU));
        optionsWidget->setBtnOk(ff7->controllerMapping(s,FF7Save::ACTION_OK));
        optionsWidget->setBtnCancel(ff7->controllerMapping(s,FF7Save::ACTION_CANCEL));
        optionsWidget->setBtnSwitch(ff7->controllerMapping(s,FF7Save::ACTION_SWITCH));
        optionsWidget->setBtnHelp(ff7->controllerMapping(s,FF7Save::ACTION_HELP));
        optionsWidget->setBtn9(ff7->controllerMapping(s,FF7Save::ACTION_UNKNOWN1));
        optionsWidget->setBtn10(ff7->controllerMapping(s,FF7Save::ACTION_UNKNOWN2));
        optionsWidget->setBtnPause(ff7->controllerMapping(s,FF7Save::ACTION_PAUSE));
        optionsWidget->setBtnUp(ff7->controllerMapping(s,FF7Save::ACTION_UP));
        optionsWidget->setBtnDown(ff7->controllerMapping(s,FF7Save::ACTION_DOWN));
        optionsWidget->setBtnLeft(ff7->controllerMapping(s,FF7Save::ACTION_LEFT));
        optionsWidget->setBtnRight(ff7->controllerMapping(s,FF7Save::ACTION_RIGHT));
        //hide buttons config if not debug or non pc save
        /*

        if(ff7->type() !="PC" || ui->action_show_debug->isChecked()){optionsWidget->hideControllMapping(false);}
        else{optonsWidget->hideControllMapping(true);}
        */
        /*~~~~~End Options Loading~~~~~*/
        ui->sb_coster_1->setValue(ff7->speedScore(s,1));
        ui->sb_coster_2->setValue(ff7->speedScore(s,2));
        ui->sb_coster_3->setValue(ff7->speedScore(s,3));
        /*~~~~~~~Set up location Data~~~~~~~*/
        ui->sb_coordx->setValue(ff7->locationX(s));
        ui->sb_coordy->setValue(ff7->locationY(s));
        ui->sb_coordz->setValue(ff7->locationZ(s));
        ui->line_location->clear();
        ui->line_location->setText(ff7->location(s));
        ui->sb_map_id->setValue(ff7->mapId(s));
        ui->sb_loc_id->setValue(ff7->locationId(s));


        ui->lbl_fieldFile->setText(QString("%1").arg(Locations.fileName(ff7->mapId(s),ff7->locationId(s))));
        ui->lbl_locationPreview->setPixmap(QString("://locations/%1_%2").arg(QString::number(ff7->mapId(s)),QString::number(ff7->locationId(s))));
        switch(ui->combo_map_controls->currentIndex())
        {
        case 0: ui->slide_world_x->setValue(ff7->slot[s].l_world & 0x7FFFF);
                ui->slide_world_y->setValue(ff7->slot[s].l_world2& 0x3FFFF);
                break;
        case 1: ui->slide_world_x->setValue(ff7->slot[s].tc_world & 0x7FFFF);
                ui->slide_world_y->setValue(ff7->slot[s].tc_world2& 0x3FFFF);
                break;
        case 2: ui->slide_world_x->setValue(ff7->slot[s].bh_world & 0x7FFFF);
                ui->slide_world_y->setValue(ff7->slot[s].bh_world2& 0x3FFFF);
                break;
        case 3: ui->slide_world_x->setValue(ff7->slot[s].sub_world & 0x7FFFF);
                ui->slide_world_y->setValue(ff7->slot[s].sub_world2& 0x3FFFF);
                break;
        case 4: ui->slide_world_x->setValue(ff7->slot[s].uw_world & 0x7FFFF);
                ui->slide_world_y->setValue(ff7->slot[s].uw_world2& 0x3FFFF);
                break;
        case 5: ui->slide_world_x->setValue(ff7->slot[s].durw_world & 0x7FFFF);
                ui->slide_world_y->setValue(ff7->slot[s].durw_world2& 0x3FFFF);
                break;
        case 6: /* Do Nothing. Don't know emerald weapon Coords
                ui->slide_world_x->setValue(ff7->slot[s].ew_world & 0x7FFFF);
                ui->slide_world_y->setValue(ff7->slot[s].ew_world2& 0x3FFFF);
                */
                break;
        }
        //WORLD TAB
        ui->leader_x->setValue((ff7->slot[s].l_world) & 0x7FFFF);
        ui->leader_id->setValue((ff7->slot[s].l_world >> 19)&0x1F);
        ui->leader_angle->setValue((ff7->slot[s].l_world) >> 24);
        ui->leader_y->setValue((ff7->slot[s].l_world2) & 0x3FFFF);
        ui->leader_z->setValue((ff7->slot[s].l_world2) >> 18);

        ui->durw_x->setValue((ff7->slot[s].durw_world) & 0x7FFFF);
        ui->durw_id->setValue((ff7->slot[s].durw_world >> 19)&0x1F);
        ui->durw_angle->setValue((ff7->slot[s].durw_world) >> 24);
        ui->durw_y->setValue((ff7->slot[s].durw_world2) & 0x3FFFF);
        ui->durw_z->setValue((ff7->slot[s].durw_world2) >> 18);

        ui->uw_x->setValue((ff7->slot[s].uw_world) & 0x7FFFF);
        ui->uw_id->setValue((ff7->slot[s].uw_world >> 19)&0x1F);
        ui->uw_angle->setValue((ff7->slot[s].uw_world) >> 24);
        ui->uw_y->setValue((ff7->slot[s].uw_world2) & 0x3FFFF);
        ui->uw_z->setValue((ff7->slot[s].uw_world2) >> 18);

        ui->tc_x->setValue((ff7->slot[s].tc_world) & 0x7FFFF);
        ui->tc_id->setValue((ff7->slot[s].tc_world >> 19)&0x1F);
        ui->tc_angle->setValue((ff7->slot[s].tc_world) >> 24);
        ui->tc_y->setValue((ff7->slot[s].tc_world2) & 0x3FFFF);
        ui->tc_z->setValue((ff7->slot[s].tc_world2) >> 18);

        ui->bh_x->setValue((ff7->slot[s].bh_world) & 0x7FFFF);
        ui->bh_id->setValue((ff7->slot[s].bh_world >> 19)&0x1F);

        switch(ui->bh_id->value())
        {
            case 0:ui->combo_highwind_buggy->setCurrentIndex(0);break;//empty
            case 6:ui->combo_highwind_buggy->setCurrentIndex(1);break;//buggy
            case 3:ui->combo_highwind_buggy->setCurrentIndex(2);break;//highwind
            default:QMessageBox::information(this,tr("Black Chocobo"),tr("Unknown Id in Buggy/Highwind Location"));break;
        }
        ui->bh_angle->setValue((ff7->slot[s].bh_world) >> 24);
        ui->bh_y->setValue((ff7->slot[s].bh_world2) & 0x3FFFF);
        ui->bh_z->setValue((ff7->slot[s].bh_world2) >> 18);

        ui->sub_x->setValue((ff7->slot[s].sub_world) & 0x7FFFF);
        ui->sub_id->setValue((ff7->slot[s].sub_world >> 19)&0x1F);
        ui->sub_angle->setValue((ff7->slot[s].sub_world) >> 24);
        ui->sub_y->setValue((ff7->slot[s].sub_world2) & 0x3FFFF);
        ui->sub_z->setValue((ff7->slot[s].sub_world2) >> 18);


        if((1 << 0) & ff7->slot[s].world_map_vehicles){ui->cb_visible_buggy->setChecked(Qt::Checked);}
        if((1 << 2) & ff7->slot[s].world_map_vehicles){ui->cb_visible_bronco->setChecked(Qt::Checked);}
        if((1 << 4) & ff7->slot[s].world_map_vehicles){ui->cb_visible_highwind->setChecked(Qt::Checked);}

        if((1 << 0) & ff7->slot[s].world_map_chocobos){ui->cb_visible_wild_chocobo->setChecked(Qt::Checked);}
        if((1 << 2) & ff7->slot[s].world_map_chocobos){ui->cb_visible_yellow_chocobo->setChecked(Qt::Checked);}
        if((1 << 3) & ff7->slot[s].world_map_chocobos){ui->cb_visible_green_chocobo->setChecked(Qt::Checked);}
        if((1 << 4) & ff7->slot[s].world_map_chocobos){ui->cb_visible_blue_chocobo->setChecked(Qt::Checked);}
        if((1 << 5) & ff7->slot[s].world_map_chocobos){ui->cb_visible_black_chocobo->setChecked(Qt::Checked);}
        if((1 << 6) & ff7->slot[s].world_map_chocobos){ui->cb_visible_gold_chocobo->setChecked(Qt::Checked);}


        for (int i=0;i<7;i++)//flyers
        {
            if (ff7->turtleParadiseFlyerSeen(s,i)){ui->list_flyers->item(i)->setCheckState(Qt::Checked);}
            else{ui->list_flyers->item(i)->setCheckState(Qt::Unchecked);}
        }

        for (int i=0;i<9;i++)//Allowed in Phs
        {
            if(ff7->phsAllowed(s,i)){phsList->setChecked(i,1,false);}
            else{phsList->setChecked(i,1,true);}
        }
        for (int i=0;i<9;i++)//Visible
        {
            if(ff7->phsVisible(s,i)){phsList->setChecked(i,2,true);}
            else{phsList->setChecked(i,2,false);}
        }
        for (int i=0;i<10;i++)//visible_menu
        {
            if(ff7->menuVisible(s,i)){menuList->setChecked(i,1,true);}
            else{menuList->setChecked(i,1,false);}
        }
        for (int i=0;i<10;i++)//menu_locked
        {
            if(ff7->menuLocked(s,i)){menuList->setChecked(i,2,true);}
            else{menuList->setChecked(i,2,false);}
        }
        for (int i=0;i<51;i++)// key items
        {
            if (ff7->keyItem(s,i)){ui->list_keyitems->item(i)->setCheckState(Qt::Checked);}
            else{ ui->list_keyitems->item(i)->setCheckState(Qt::Unchecked);}
        }
        /*~~~~~party combo boxes (checking for empty slots)~~~*/
        if (ff7->party(s,0) >= 0x0C){ui->combo_party1->setCurrentIndex(12);}
        else{ui->combo_party1->setCurrentIndex(ff7->party(s,0));}
        if (ff7->party(s,1) >= 0x0C){ui->combo_party2->setCurrentIndex(12);}
        else{ui->combo_party2->setCurrentIndex(ff7->party(s,1));}
        if (ff7->party(s,2) >= 0x0C){ui->combo_party3->setCurrentIndex(12);}
        else{ui->combo_party3->setCurrentIndex(ff7->party(s,2));}

        ui->sb_gil->setValue(ff7->gil(s));
        ui->sb_gp->setValue(ff7->gp(s));
        ui->sb_runs->setValue(ff7->runs(s));
        ui->sb_battles->setValue(ff7->battles(s));
        ui->sb_steps->setValue(ff7->slot[s].steps);
        ui->sb_love_barret->setValue(ff7->love(s,false,FF7Save::LOVE_BARRET));
        ui->sb_love_tifa->setValue(ff7->love(s,false,FF7Save::LOVE_TIFA));
        ui->sb_love_aeris->setValue(ff7->love(s,false,FF7Save::LOVE_AERIS));
        ui->sb_love_yuffie->setValue(ff7->love(s,false,FF7Save::LOVE_YUFFIE));
        ui->sb_time_hour->setValue(ff7->time(s) / 3600);
        ui->sb_time_min->setValue(ff7->time(s)/60%60);
        ui->sb_time_sec->setValue(ff7->time(s) -((ui->sb_time_hour->value()*3600)+ui->sb_time_min->value()*60));

        optionsWidget->setDialogColors(ff7->dialogColorUL(s),ff7->dialogColorUR(s),ff7->dialogColorLL(s),ff7->dialogColorLR(s));


        if(ff7->materiaCave(s,FF7Save::CAVE_MIME)){ui->cb_materiacave_1->setChecked(Qt::Checked);}        else{ui->cb_materiacave_1->setChecked(Qt::Unchecked);}
        if(ff7->materiaCave(s,FF7Save::CAVE_HPMP)){ui->cb_materiacave_2->setChecked(Qt::Checked);}        else{ui->cb_materiacave_2->setChecked(Qt::Unchecked);}
        if(ff7->materiaCave(s,FF7Save::CAVE_QUADMAGIC)){ui->cb_materiacave_3->setChecked(Qt::Checked);}        else{ui->cb_materiacave_3->setChecked(Qt::Unchecked);}
        if(ff7->materiaCave(s,FF7Save::CAVE_KOTR)){ui->cb_materiacave_4->setChecked(Qt::Checked);}        else{ui->cb_materiacave_4->setChecked(Qt::Unchecked);}
        if((ff7->slot[s].yuffieforest)& (1<<0)){ui->cb_yuffieforest->setChecked(Qt::Checked);}        else{ui->cb_yuffieforest->setChecked(Qt::Unchecked);}

        /*~~~~~Stolen Materia~~~~~~~*/
        QTableWidgetItem *newItem;
        ui->tbl_materia_2->reset();
        ui->tbl_materia_2->clearContents();
        ui->tbl_materia_2->setColumnWidth(0,(ui->tbl_materia_2->width()*.65));
        ui->tbl_materia_2->setColumnWidth(1,(ui->tbl_materia_2->width()*.25));
        ui->tbl_materia_2->setRowCount(48);
        for(int mat=0;mat<48;mat++) //materias stolen by yuffie
        {
            QString ap;
            quint8 current_id = ff7->stolenMateriaId(s,mat);
            if (current_id !=FF7Materia::EmptyId)
            {
                newItem = new QTableWidgetItem(QPixmap::fromImage(Materias.image(current_id)),Materias.name(current_id),0);
                ui->tbl_materia_2->setItem(mat,0,newItem);
                qint32 current_ap = ff7->stolenMateriaAp(s,mat);
                if (current_ap == FF7Materia::MaxMateriaAp){newItem =new QTableWidgetItem(tr("Master"));ui->tbl_materia_2->setItem(mat,1,newItem);}
                else{newItem =new QTableWidgetItem(ap.setNum(current_ap));ui->tbl_materia_2->setItem(mat,1,newItem);}
            }
            else
            {
                newItem = new QTableWidgetItem(tr("===Empty Slot==="),0);
                ui->tbl_materia_2->setItem(mat,0,newItem);
            }
        }
        //SnowBoard Mini Game Data.
        ui->sbSnowBegMin->setValue((ff7->snowboardTime(s,0)/1000)/60%60);
        ui->sbSnowBegSec->setValue((ff7->snowboardTime(s,0)/1000)-(ui->sbSnowBegMin->value()*60));
        ui->sbSnowBegMsec->setValue(ff7->snowboardTime(s,0)%1000);
        ui->sbSnowBegScore->setValue(ff7->snowboardScore(s,0));

        ui->sbSnowExpMin->setValue((ff7->snowboardTime(s,1)/1000)/60%60);
        ui->sbSnowExpSec->setValue((ff7->snowboardTime(s,1)/1000)-(ui->sbSnowExpMin->value()*60));
        ui->sbSnowExpMsec->setValue(ff7->snowboardTime(s,1)%1000);
        ui->sbSnowExpScore->setValue(ff7->snowboardScore(s,1));

        ui->sbSnowCrazyMin->setValue((ff7->snowboardTime(s,2)/1000)/60%60);
        ui->sbSnowCrazySec->setValue((ff7->snowboardTime(s,2)/1000)-(ui->sbSnowCrazyMin->value()*60));
        ui->sbSnowCrazyMsec->setValue(ff7->snowboardTime(s,2)%1000);
        ui->sbSnowCrazyScore->setValue(ff7->snowboardScore(s,2));

        ui->sb_BikeHighScore->setValue(ff7->bikeHighScore(s));
        ui->sb_BattlePoints->setValue(ff7->battlePoints(s));

        load=false;
        // all functions should set load on their own.
        /*~~~~~Call External Functions~~~~~~~*/
        itemlist->setItems(ff7->items(s));
        setmenu(newgame);
        itemupdate();
        chocobo_refresh();
        materiaupdate();
        progress_update();
        set_char_buttons();
        hexEditorRefresh();
        if(ui->action_show_debug->isChecked()){testdata_refresh();}

        switch(curchar)
        {
            case 0: ui->btn_cloud->click();break;
            case 1: ui->btn_barret->click();break;
            case 2: ui->btn_tifa->click();break;
            case 3: ui->btn_aeris->click();break;
            case 4: ui->btn_red->click();break;
            case 5: ui->btn_yuffie->click();break;
            case 6: ui->btn_cait->click();break;
            case 7: ui->btn_vincent->click();break;
            case 8: ui->btn_cid->click();break;
        }
    }
}/*~~~~~~~~~~~~~~~~~~~~End GUIREFRESH ~~~~~~~~~~~~~~~~~*/
void MainWindow::set_char_buttons()
{
    ui->btn_cloud->setIcon(Chars.icon(ff7->charID(s,0)));
    ui->btn_barret->setIcon(Chars.icon(ff7->charID(s,1)));
    ui->btn_tifa->setIcon(Chars.icon(ff7->charID(s,2)));
    ui->btn_aeris->setIcon(Chars.icon(ff7->charID(s,3)));
    ui->btn_red->setIcon(Chars.icon(ff7->charID(s,4)));
    ui->btn_yuffie->setIcon(Chars.icon(ff7->charID(s,5)));
    ui->btn_cait->setIcon(Chars.icon(ff7->charID(s,6)));
    ui->btn_vincent->setIcon(Chars.icon(ff7->charID(s,7)));
    ui->btn_cid->setIcon(Chars.icon(ff7->charID(s,8)));
}
void MainWindow::progress_update()
{
    load=true;

    ui->sb_curdisc->setValue(ff7->disc(s));
    ui->sb_turkschruch->setValue(ff7->slot[s].aeris_chruch);
    ui->sb_donprog->setValue(ff7->slot[s].donprogress);
    ui->sb_mprogress->setValue(ff7->slot[s].mprogress);
    ui->combo_s7_slums->setCurrentIndex(0);
    if(ff7->slot[s].intbombing == 0x14){ui->cb_bombing_int->setChecked(Qt::Checked);}

    if((ff7->slot[s].bm_progress1)& (1<<0)){ui->cb_bm1_1->setChecked(Qt::Checked);}     else{ui->cb_bm1_1->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress1)& (1<<1)){ui->cb_bm1_2->setChecked(Qt::Checked);}     else{ui->cb_bm1_2->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress1)& (1<<2)){ui->cb_bm1_3->setChecked(Qt::Checked);}     else{ui->cb_bm1_3->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress1)& (1<<3)){ui->cb_bm1_4->setChecked(Qt::Checked);}     else{ui->cb_bm1_4->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress1)& (1<<4)){ui->cb_bm1_5->setChecked(Qt::Checked);}     else{ui->cb_bm1_5->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress1)& (1<<5)){ui->cb_bm1_6->setChecked(Qt::Checked);}     else{ui->cb_bm1_6->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress1)& (1<<6)){ui->cb_bm1_7->setChecked(Qt::Checked);}     else{ui->cb_bm1_7->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress1)& (1<<7)){ui->cb_bm1_8->setChecked(Qt::Checked);}     else{ui->cb_bm1_8->setChecked(Qt::Unchecked);}

    if((ff7->slot[s].bm_progress2)& (1<<0)){ui->cb_bm2_1->setChecked(Qt::Checked);}     else{ui->cb_bm2_1->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress2)& (1<<1)){ui->cb_bm2_2->setChecked(Qt::Checked);}     else{ui->cb_bm2_2->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress2)& (1<<2)){ui->cb_bm2_3->setChecked(Qt::Checked);}     else{ui->cb_bm2_3->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress2)& (1<<3)){ui->cb_bm2_4->setChecked(Qt::Checked);}     else{ui->cb_bm2_4->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress2)& (1<<4)){ui->cb_bm2_5->setChecked(Qt::Checked);}     else{ui->cb_bm2_5->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress2)& (1<<5)){ui->cb_bm2_6->setChecked(Qt::Checked);}     else{ui->cb_bm2_6->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress2)& (1<<6)){ui->cb_bm2_7->setChecked(Qt::Checked);}     else{ui->cb_bm2_7->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress2)& (1<<7)){ui->cb_bm2_8->setChecked(Qt::Checked);}     else{ui->cb_bm2_8->setChecked(Qt::Unchecked);}

    if((ff7->slot[s].bm_progress3)& (1<<0)){ui->cb_bm3_1->setChecked(Qt::Checked);}     else{ui->cb_bm3_1->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress3)& (1<<1)){ui->cb_bm3_2->setChecked(Qt::Checked);}     else{ui->cb_bm3_2->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress3)& (1<<2)){ui->cb_bm3_3->setChecked(Qt::Checked);}     else{ui->cb_bm3_3->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress3)& (1<<3)){ui->cb_bm3_4->setChecked(Qt::Checked);}     else{ui->cb_bm3_4->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress3)& (1<<4)){ui->cb_bm3_5->setChecked(Qt::Checked);}     else{ui->cb_bm3_5->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress3)& (1<<5)){ui->cb_bm3_6->setChecked(Qt::Checked);}     else{ui->cb_bm3_6->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress3)& (1<<6)){ui->cb_bm3_7->setChecked(Qt::Checked);}     else{ui->cb_bm3_7->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].bm_progress3)& (1<<7)){ui->cb_bm3_8->setChecked(Qt::Checked);}     else{ui->cb_bm3_8->setChecked(Qt::Unchecked);}

    if(ff7->unknown(s,26).at(0) & (1<<0)){ui->cb_s7pl_1->setChecked(Qt::Checked);}    else{ui->cb_s7pl_1->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(0) & (1<<1)){ui->cb_s7pl_2->setChecked(Qt::Checked);}    else{ui->cb_s7pl_2->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(0) & (1<<2)){ui->cb_s7pl_3->setChecked(Qt::Checked);}    else{ui->cb_s7pl_3->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(0) & (1<<3)){ui->cb_s7pl_4->setChecked(Qt::Checked);}    else{ui->cb_s7pl_4->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(0) & (1<<4)){ui->cb_s7pl_5->setChecked(Qt::Checked);}    else{ui->cb_s7pl_5->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(0) & (1<<5)){ui->cb_s7pl_6->setChecked(Qt::Checked);}    else{ui->cb_s7pl_6->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(0) & (1<<6)){ui->cb_s7pl_7->setChecked(Qt::Checked);}    else{ui->cb_s7pl_7->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(0) & (1<<7)){ui->cb_s7pl_8->setChecked(Qt::Checked);}    else{ui->cb_s7pl_8->setChecked(Qt::Unchecked);}

    if(ff7->unknown(s,26).at(8) & (1<<0)){ui->cb_s7ts_1->setChecked(Qt::Checked);}    else{ui->cb_s7ts_1->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(8) & (1<<1)){ui->cb_s7ts_2->setChecked(Qt::Checked);}    else{ui->cb_s7ts_2->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(8) & (1<<2)){ui->cb_s7ts_3->setChecked(Qt::Checked);}    else{ui->cb_s7ts_3->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(8) & (1<<3)){ui->cb_s7ts_4->setChecked(Qt::Checked);}    else{ui->cb_s7ts_4->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(8) & (1<<4)){ui->cb_s7ts_5->setChecked(Qt::Checked);}    else{ui->cb_s7ts_5->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(8) & (1<<5)){ui->cb_s7ts_6->setChecked(Qt::Checked);}    else{ui->cb_s7ts_6->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(8) & (1<<6)){ui->cb_s7ts_7->setChecked(Qt::Checked);}    else{ui->cb_s7ts_7->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,26).at(8) & (1<<7)){ui->cb_s7ts_8->setChecked(Qt::Checked);}    else{ui->cb_s7ts_8->setChecked(Qt::Unchecked);}

    if(ff7->unknown(s,23).at(26) & (1<<0)){ui->cb_s5_1->setChecked(Qt::Checked);}      else{ui->cb_s5_1->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,23).at(26) & (1<<1)){ui->cb_s5_2->setChecked(Qt::Checked);}      else{ui->cb_s5_2->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,23).at(26) & (1<<2)){ui->cb_s5_3->setChecked(Qt::Checked);}      else{ui->cb_s5_3->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,23).at(26) & (1<<3)){ui->cb_s5_4->setChecked(Qt::Checked);}      else{ui->cb_s5_4->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,23).at(26) & (1<<4)){ui->cb_s5_5->setChecked(Qt::Checked);}      else{ui->cb_s5_5->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,23).at(26) & (1<<5)){ui->cb_s5_6->setChecked(Qt::Checked);}      else{ui->cb_s5_6->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,23).at(26) & (1<<6)){ui->cb_s5_7->setChecked(Qt::Checked);}      else{ui->cb_s5_7->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,23).at(26) & (1<<7)){ui->cb_s5_8->setChecked(Qt::Checked);}      else{ui->cb_s5_8->setChecked(Qt::Unchecked);}

    if((ff7->slot[s].midgartrainflags)& (1<<0)){ui->cb_midgartrain_1->setChecked(Qt::Checked);}    else{ui->cb_midgartrain_1->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].midgartrainflags)& (1<<1)){ui->cb_midgartrain_2->setChecked(Qt::Checked);}    else{ui->cb_midgartrain_2->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].midgartrainflags)& (1<<2)){ui->cb_midgartrain_3->setChecked(Qt::Checked);}    else{ui->cb_midgartrain_3->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].midgartrainflags)& (1<<3)){ui->cb_midgartrain_4->setChecked(Qt::Checked);}    else{ui->cb_midgartrain_4->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].midgartrainflags)& (1<<4)){ui->cb_midgartrain_5->setChecked(Qt::Checked);}    else{ui->cb_midgartrain_5->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].midgartrainflags)& (1<<5)){ui->cb_midgartrain_6->setChecked(Qt::Checked);}    else{ui->cb_midgartrain_6->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].midgartrainflags)& (1<<6)){ui->cb_midgartrain_7->setChecked(Qt::Checked);}    else{ui->cb_midgartrain_7->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].midgartrainflags)& (1<<7)){ui->cb_midgartrain_8->setChecked(Qt::Checked);}    else{ui->cb_midgartrain_8->setChecked(Qt::Unchecked);}

    //When using A char for comparison to an int value such as below be sure to static_cast<unsigned char> the value
    //to avoid possible build warning and failed run time check  due to possible return of signed char  and int >127

    if( static_cast<unsigned char>(ff7->unknown(s,26).at(1)) ==0x00 &&
         static_cast<unsigned char>(ff7->unknown(s,26).at(2)) ==0x00 &&
         static_cast<unsigned char>(ff7->unknown(s,26).at(3)) ==0x00 &&
         static_cast<unsigned char>(ff7->unknown(s,26).at(4)) ==0x00 &&
         static_cast<unsigned char>(ff7->unknown(s,26).at(5)) ==0x00 &&
         static_cast<unsigned char>(ff7->unknown(s,26).at(6)) ==0x00   )
         {ui->combo_s7_slums->setCurrentIndex(1);}

    else if(
        (static_cast<unsigned char>(ff7->unknown(s,26).at(1)) == 0xFF || static_cast<unsigned char>(ff7->unknown(s,26).at(1)) == 0xBF) &&
        (static_cast<unsigned char>(ff7->unknown(s,26).at(2)) == 0x03 || static_cast<unsigned char>(ff7->unknown(s,26).at(2)) == 0x51) &&
        (static_cast<unsigned char>(ff7->unknown(s,26).at(3)) == 0x04 || static_cast<unsigned char>(ff7->unknown(s,26).at(3)) == 0x05) &&
        (static_cast<unsigned char>(ff7->unknown(s,26).at(4)) == 0x0F || static_cast<unsigned char>(ff7->unknown(s,26).at(4)) == 0x17) &&
        (static_cast<unsigned char>(ff7->unknown(s,26).at(5)) == 0x1F || static_cast<unsigned char>(ff7->unknown(s,26).at(5)) == 0x5D) &&
        (static_cast<unsigned char>(ff7->unknown(s,26).at(6)) == 0x6F || static_cast<unsigned char>(ff7->unknown(s,26).at(6)) == 0xEF)   )
        {ui->combo_s7_slums->setCurrentIndex(2);}

    else if ( static_cast<unsigned char>(ff7->unknown(s,26).at(2))== 0x13){ui->combo_s7_slums->setCurrentIndex(3);}
    else {ui->combo_s7_slums->setCurrentIndex(0);}
    load=false;
}
/*~~~~~~~~~~~~~~~~~~~~Chocobo Refresh~~~~~~~~~~~~~~~~*/
void MainWindow::chocobo_refresh()
{
    load=true;
    ui->sb_stables_owned->setValue(ff7->slot[s].stables);
    ui->lcd_stables_occupied->display(ff7->slot[s].stablesoccupied);

    if((ff7->slot[s].chocobomask)& (1<<0)){ui->box_stable1->setChecked(Qt::Checked);}    else{ui->box_stable1->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].chocobomask)& (1<<1)){ui->box_stable2->setChecked(Qt::Checked);}    else{ui->box_stable2->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].chocobomask)& (1<<2)){ui->box_stable3->setChecked(Qt::Checked);}    else{ui->box_stable3->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].chocobomask)& (1<<3)){ui->box_stable4->setChecked(Qt::Checked);}    else{ui->box_stable4->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].chocobomask)& (1<<4)){ui->box_stable5->setChecked(Qt::Checked);}    else{ui->box_stable5->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].chocobomask)& (1<<5)){ui->box_stable6->setChecked(Qt::Checked);}    else{ui->box_stable6->setChecked(Qt::Unchecked);}

    chocobo_stable_1->SetChocobo(ff7->chocobo(s,0),ff7->chocoName(s,0),ff7->chocoCantMate(s,0),ff7->chocoStamina(s,0));
    chocobo_stable_2->SetChocobo(ff7->chocobo(s,1),ff7->chocoName(s,1),ff7->chocoCantMate(s,1),ff7->chocoStamina(s,1));
    chocobo_stable_3->SetChocobo(ff7->chocobo(s,2),ff7->chocoName(s,2),ff7->chocoCantMate(s,2),ff7->chocoStamina(s,2));
    chocobo_stable_4->SetChocobo(ff7->chocobo(s,3),ff7->chocoName(s,3),ff7->chocoCantMate(s,3),ff7->chocoStamina(s,3));
    chocobo_stable_5->SetChocobo(ff7->chocobo(s,4),ff7->chocoName(s,4),ff7->chocoCantMate(s,4),ff7->chocoStamina(s,4));
    chocobo_stable_6->SetChocobo(ff7->chocobo(s,5),ff7->chocoName(s,5),ff7->chocoCantMate(s,5),ff7->chocoStamina(s,5));
    //set the penned chocobos
    ui->combo_pen1->setCurrentIndex(ff7->chocoboPen(s,0));
    ui->combo_pen2->setCurrentIndex(ff7->chocoboPen(s,1));
    ui->combo_pen3->setCurrentIndex(ff7->chocoboPen(s,2));
    ui->combo_pen4->setCurrentIndex(ff7->chocoboPen(s,3));
    load=false;
}/*~~~~~~~~~~~End Chocobo Slots~~~~~~~~~*/

/*~~~~~~~~~Test Data~~~~~~~~~~~*/
void MainWindow::testdata_refresh()
{
    load=true;

   //TEST TAB

    ui->cb_tut_sub->setChecked(Qt::Unchecked);
    ui->sb_timer_time_hour->setValue(ff7->slot[s].timer[0]);
    ui->sb_timer_time_min->setValue(ff7->slot[s].timer[1]);
    ui->sb_timer_time_sec->setValue(ff7->slot[s].timer[2]);

    ui->sb_b_love_aeris->setValue(ff7->love(s,true,FF7Save::LOVE_AERIS));
    ui->sb_b_love_tifa->setValue(ff7->love(s,true,FF7Save::LOVE_TIFA));
    ui->sb_b_love_yuffie->setValue(ff7->love(s,true,FF7Save::LOVE_YUFFIE));
    ui->sb_b_love_barret->setValue(ff7->love(s,true,FF7Save::LOVE_BARRET));
    ui->sb_u_weapon_hp->setValue(ff7->slot[s].u_weapon_hp[0] |(ff7->slot[s].u_weapon_hp[1] << 8) | (ff7->slot[s].u_weapon_hp[2] << 16));

    if((ff7->slot[s].tut_sub)&(1<<2)){ui->cb_tut_sub->setChecked(Qt::Checked);}

    ui->lcdNumber_6->display(ff7->slot[s].tut_sub);

    if(ff7->slot[s].tut_save == 0x3A){ui->cb_tut_worldsave->setCheckState(Qt::Checked);}
    else if(ff7->slot[s].tut_save ==0x32){ui->cb_tut_worldsave->setCheckState(Qt::PartiallyChecked);}
    else{ui->cb_tut_worldsave->setCheckState(Qt::Unchecked);}
    ui->lcdNumber_7->display(ff7->slot[s].tut_save);

    ui->cb_reg_vinny->setChecked(Qt::Unchecked);
    if(ff7->slot[s].reg_vinny == 0xFF){ui->cb_reg_vinny->setChecked(Qt::Checked);}
    ui->lcdNumber_8->display(ff7->slot[s].reg_vinny);

    ui->cb_reg_yuffie->setChecked(Qt::Unchecked);
    if(ff7->slot[s].reg_yuffie == 0x6F){ui->cb_reg_yuffie->setChecked(Qt::Checked);}
    ui->lcdNumber_9->display(ff7->slot[s].reg_yuffie);

    if(ff7->unknown(s,11).at(3)&(1<<0)){ui->cb_farm_items_1->setChecked(Qt::Checked);}    else{ui->cb_farm_items_1->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,11).at(3)&(1<<1)){ui->cb_farm_items_2->setChecked(Qt::Checked);}    else{ui->cb_farm_items_2->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,11).at(3)&(1<<2)){ui->cb_farm_items_3->setChecked(Qt::Checked);}    else{ui->cb_farm_items_3->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,11).at(3)&(1<<3)){ui->cb_farm_items_4->setChecked(Qt::Checked);}    else{ui->cb_farm_items_4->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,11).at(3)&(1<<4)){ui->cb_farm_items_5->setChecked(Qt::Checked);}    else{ui->cb_farm_items_5->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,11).at(3)&(1<<5)){ui->cb_farm_items_6->setChecked(Qt::Checked);}    else{ui->cb_farm_items_6->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,11).at(3)&(1<<6)){ui->cb_farm_items_7->setChecked(Qt::Checked);}    else{ui->cb_farm_items_7->setChecked(Qt::Unchecked);}
    if(ff7->unknown(s,11).at(3)&(1<<7)){ui->cb_farm_items_8->setChecked(Qt::Checked);}    else{ui->cb_farm_items_8->setChecked(Qt::Unchecked);}


    if((ff7->slot[s].tut_sub)& (1<<0)){ui->cb_tut_sub_1->setChecked(Qt::Checked);}    else{ui->cb_tut_sub_1->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].tut_sub)& (1<<1)){ui->cb_tut_sub_2->setChecked(Qt::Checked);}    else{ui->cb_tut_sub_2->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].tut_sub)& (1<<2)){ui->cb_tut_sub_3->setChecked(Qt::Checked);}    else{ui->cb_tut_sub_3->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].tut_sub)& (1<<3)){ui->cb_tut_sub_4->setChecked(Qt::Checked);}    else{ui->cb_tut_sub_4->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].tut_sub)& (1<<4)){ui->cb_tut_sub_5->setChecked(Qt::Checked);}    else{ui->cb_tut_sub_5->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].tut_sub)& (1<<5)){ui->cb_tut_sub_6->setChecked(Qt::Checked);}    else{ui->cb_tut_sub_6->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].tut_sub)& (1<<6)){ui->cb_tut_sub_7->setChecked(Qt::Checked);}    else{ui->cb_tut_sub_7->setChecked(Qt::Unchecked);}
    if((ff7->slot[s].tut_sub)& (1<<7)){ui->cb_tut_sub_8->setChecked(Qt::Checked);}    else{ui->cb_tut_sub_8->setChecked(Qt::Unchecked);}
    ui->lcd_tut_sub->display(ff7->slot[s].tut_sub);

    //Snowboard Times.
    load=false;

    unknown_refresh(ui->combo_z_var->currentIndex());
}
/*~~~~~~~~~Char Buttons.~~~~~~~~~~~*/
void MainWindow::on_btn_cloud_clicked()     {curchar=0; char_editor->setChar(ff7->character(s,0),ff7->charName(s,0));ui->btn_cloud->setIcon(Chars.icon(ff7->charID(s,curchar)));}
void MainWindow::on_btn_barret_clicked()    {curchar=1; char_editor->setChar(ff7->character(s,1),ff7->charName(s,1));ui->btn_barret->setIcon(Chars.icon(ff7->charID(s,curchar)));}
void MainWindow::on_btn_tifa_clicked()      {curchar=2; char_editor->setChar(ff7->character(s,2),ff7->charName(s,2));ui->btn_tifa->setIcon(Chars.icon(ff7->charID(s,curchar)));}
void MainWindow::on_btn_aeris_clicked()     {curchar=3; char_editor->setChar(ff7->character(s,3),ff7->charName(s,3));ui->btn_aeris->setIcon(Chars.icon(ff7->charID(s,curchar)));}
void MainWindow::on_btn_red_clicked()       {curchar=4; char_editor->setChar(ff7->character(s,4),ff7->charName(s,4));ui->btn_red->setIcon(Chars.icon(ff7->charID(s,curchar)));}
void MainWindow::on_btn_yuffie_clicked()    {curchar=5; char_editor->setChar(ff7->character(s,5),ff7->charName(s,5));ui->btn_yuffie->setIcon(Chars.icon(ff7->charID(s,curchar)));}
void MainWindow::on_btn_cait_clicked()      {curchar=6; char_editor->setChar(ff7->character(s,6),ff7->charName(s,6));ui->btn_cait->setIcon(Chars.icon(ff7->charID(s,curchar)));}
void MainWindow::on_btn_vincent_clicked()   {curchar=7; char_editor->setChar(ff7->character(s,7),ff7->charName(s,7));ui->btn_vincent->setIcon(Chars.icon(ff7->charID(s,curchar)));}
void MainWindow::on_btn_cid_clicked()       {curchar=8; char_editor->setChar(ff7->character(s,8),ff7->charName(s,8));ui->btn_cid->setIcon(Chars.icon(ff7->charID(s,curchar)));}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Party TAB~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::on_sb_gil_valueChanged(int value){if(!load){ ff7->setGil(s,value);}}
void MainWindow::on_sb_gp_valueChanged(int value){if(!load){ ff7->setGp(s,value);}}
void MainWindow::on_sb_battles_valueChanged(int value){if(!load){ ff7->setBattles(s,value);}}
void MainWindow::on_sb_runs_valueChanged(int value){if(!load){ ff7->setRuns(s,value);}}
void MainWindow::on_combo_party1_currentIndexChanged(int index)
{if(!load){
    if(index == 0x0C) //empty char slot (index 12)
    {
        ff7->setParty(s,0,0xFF);
        //wipe all desc data if noone is there
        ff7->setDescParty(s,0,ff7->party(s,0));
        ff7->setDescCurHP(s,0);
        ff7->setDescMaxHP(s,0);
        ff7->setDescCurMP(s,0);
        ff7->setDescMaxMP(s,0);
        ff7->setDescLevel(s,0);
        ff7->setDescName(s,QString(QByteArray(16,0xFF)));
    }
    else
    {
        ff7->setParty(s,0,index);
        ff7->setDescParty(s,0,ff7->party(s,0));
        // IF ID >8 no char slot so for 9, 10, 11 Use slot 6,7,8 char data.
        if(ff7->party(s,0)== FF7Char::YoungCloud)
        {
            ff7->setDescCurHP(s,ff7->charCurrentHp(s,6));
            ff7->setDescMaxHP(s,ff7->charMaxHp(s,6));
            ff7->setDescCurMP(s,ff7->charCurrentMp(s,6));
            ff7->setDescMaxMP(s,ff7->charMaxMp(s,6));
            ff7->setDescLevel(s,ff7->charLevel(s,6));
            ff7->setDescName(s,ff7->charName(s,6));
        }
        else if(ff7->party(s,0)== FF7Char::Sephiroth)
        {
            ff7->setDescCurHP(s,ff7->charCurrentHp(s,7));
            ff7->setDescMaxHP(s,ff7->charMaxHp(s,7));
            ff7->setDescCurMP(s,ff7->charCurrentMp(s,7));
            ff7->setDescMaxMP(s,ff7->charMaxMp(s,7));
            ff7->setDescLevel(s,ff7->charLevel(s,7));
            ff7->setDescName(s,ff7->charName(s,7));
        }
        else if(ff7->party(s,0)== 11)
        {//chocobo? that never really works.
            ff7->setDescCurHP(s,ff7->charCurrentHp(s,8));
            ff7->setDescMaxHP(s,ff7->charMaxHp(s,8));
            ff7->setDescCurMP(s,ff7->charCurrentMp(s,8));
            ff7->setDescMaxMP(s,ff7->charMaxMp(s,8));
            ff7->setDescLevel(s,ff7->charLevel(s,8));
            ff7->setDescName(s,ff7->charName(s,8));
        }
        else
        {
            ff7->setDescCurHP(s,ff7->charCurrentHp(s,ff7->party(s,0)));
            ff7->setDescMaxHP(s,ff7->charMaxHp(s,ff7->party(s,0)));
            ff7->setDescCurMP(s,ff7->charCurrentMp(s,ff7->party(s,0)));
            ff7->setDescMaxMP(s,ff7->charMaxMp(s,ff7->party(s,0)));
            ff7->setDescLevel(s,ff7->charLevel(s,ff7->party(s,0)));
            ff7->setDescName(s,ff7->charName(s,ff7->party(s,0)));
        }
    }
}}
void MainWindow::on_combo_party2_currentIndexChanged(int index)
{if(!load){
    if(index == 12){ff7->setParty(s,1,FF7Char::Empty);}
    else{ff7->setParty(s,1,index);}
    //either way set the desc
    ff7->setDescParty(s,1,ff7->party(s,1));
}}
void MainWindow::on_combo_party3_currentIndexChanged(int index)
{if(!load){
        if(index == 12){ff7->setParty(s,2,FF7Char::Empty);}
        else{ff7->setParty(s,2,index);}
        //either way set the desc
        ff7->setDescParty(s,2,ff7->party(s,2));
}}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~Chocobo Tab~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

//set data for stables inside
void MainWindow::on_sb_stables_owned_valueChanged(int value)
{
    if(!load){file_modified(true); ff7->slot[s].stables = value;}
    ui->box_stable1->setEnabled(false);
    ui->box_stable2->setEnabled(false);
    ui->box_stable3->setEnabled(false);
    ui->box_stable4->setEnabled(false);
    ui->box_stable5->setEnabled(false);
    ui->box_stable6->setEnabled(false);
    switch(value)
    {//No Breaks On Purpose.
        case 6:ui->box_stable6->setEnabled(true);
        case 5:ui->box_stable5->setEnabled(true);
        case 4:ui->box_stable4->setEnabled(true);
        case 3:ui->box_stable3->setEnabled(true);
        case 2:ui->box_stable2->setEnabled(true);
        case 1:ui->box_stable1->setEnabled(true);
    }
}
/*~~~~~~~~~Occupied~~~~~~~~~~~*/
void MainWindow::on_box_stable1_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].chocobomask |= (1<<0);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()+1);}
    else{ff7->slot[s].chocobomask &= ~(1<<0);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()-1);}
    ff7->slot[s].stablesoccupied=ui->lcd_stables_occupied->value();
}}
void MainWindow::on_box_stable2_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].chocobomask |= (1<<1);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()+1);}
    else{ff7->slot[s].chocobomask &= ~(1<<1);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()-1);}
    ff7->slot[s].stablesoccupied=ui->lcd_stables_occupied->value();
}}
void MainWindow::on_box_stable3_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].chocobomask |= (1<<2);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()+1);}
    else{ff7->slot[s].chocobomask &= ~(1<<2);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()-1);}
    ff7->slot[s].stablesoccupied=ui->lcd_stables_occupied->value();
}}
void MainWindow::on_box_stable4_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].chocobomask |= (1<<3);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()+1);}
    else{ff7->slot[s].chocobomask &= ~(1<<3);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()-1);}
    ff7->slot[s].stablesoccupied=ui->lcd_stables_occupied->value();
}}
void MainWindow::on_box_stable5_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].chocobomask |= (1<<4);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()+1);}
    else{ff7->slot[s].chocobomask &= ~(1<<4);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()-1);}
    ff7->slot[s].stablesoccupied=ui->lcd_stables_occupied->value();
}}
void MainWindow::on_box_stable6_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].chocobomask |= (1<<5);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()+1);}
    else{ff7->slot[s].chocobomask &= ~(1<<5);ui->lcd_stables_occupied->display(ui->lcd_stables_occupied->value()-1);}
    ff7->slot[s].stablesoccupied=ui->lcd_stables_occupied->value();
}}
/*~~~~~ChocoboStats~~~~~*/
void MainWindow::c1_nameChanged(QString text){if(!load){ff7->setChocoName(s,0,text);}}
void MainWindow::c1_staminaChanged(quint16 value){if(!load){ ff7->setChocoStamina(s,0,value);}}
void MainWindow::c1_speedChanged(quint16 value){if(!load){ ff7->setChocoSpeed(s,0,value);}}
void MainWindow::c1_maxspeedChanged(quint16 value){if(!load){ ff7->setChocoMaxSpeed(s,0,value);}}
void MainWindow::c1_sprintChanged(quint16 value){if(!load){ ff7->setChocoSprintSpeed(s,0,value);}}
void MainWindow::c1_maxsprintChanged(quint16 value){if(!load){ ff7->setChocoMaxSprintSpeed(s,0,value);}}
void MainWindow::c1_sexChanged(quint8 index){if(!load){ ff7->setChocoSex(s,0,index);}}
void MainWindow::c1_typeChanged(quint8 index){if(!load){ ff7->setChocoType(s,0,index);}}
void MainWindow::c1_coopChanged(quint8 value){if(!load){ ff7->setChocoCoop(s,0,value);}}
void MainWindow::c1_accelChanged(quint8 value){if(!load){ ff7->setChocoAccel(s,0,value);}}
void MainWindow::c1_intelChanged(quint8 value){if(!load){ ff7->setChocoIntelligence(s,0,value);}}
void MainWindow::c1_raceswonChanged(quint8 value){if(!load){ ff7->setChocoRaceswon(s,0,value);}}
void MainWindow::c1_pcountChanged(quint8 value){if(!load){ ff7->setChocoPCount(s,0,value);}}
void MainWindow::c1_personalityChanged(quint8 value){if(!load){ ff7->setChocoPersonality(s,0,value);}}
void MainWindow::c1_mated_toggled(bool checked){if(!load){ff7->setChocoCantMate(s,0,checked);}}

void MainWindow::c2_nameChanged(QString text){if(!load){ff7->setChocoName(s,1,text);}}
void MainWindow::c2_staminaChanged(quint16 value){if(!load){ ff7->setChocoStamina(s,1,value);}}
void MainWindow::c2_speedChanged(quint16 value){if(!load){ ff7->setChocoSpeed(s,1,value);}}
void MainWindow::c2_maxspeedChanged(quint16 value){if(!load){ ff7->setChocoMaxSpeed(s,1,value);}}
void MainWindow::c2_sprintChanged(quint16 value){if(!load){ff7->setChocoSprintSpeed(s,1,value);}}
void MainWindow::c2_maxsprintChanged(quint16 value){if(!load){ ff7->setChocoMaxSprintSpeed(s,1,value);}}
void MainWindow::c2_sexChanged(quint8 index){if(!load){ ff7->setChocoSex(s,1,index);}}
void MainWindow::c2_typeChanged(quint8 index){if(!load){ ff7->setChocoType(s,1,index);}}
void MainWindow::c2_coopChanged(quint8 value){if(!load){ ff7->setChocoCoop(s,1,value);}}
void MainWindow::c2_accelChanged(quint8 value){if(!load){ ff7->setChocoAccel(s,1,value);}}
void MainWindow::c2_intelChanged(quint8 value){if(!load){ ff7->setChocoIntelligence(s,1,value);}}
void MainWindow::c2_raceswonChanged(quint8 value){if(!load){ ff7->setChocoRaceswon(s,1,value);}}
void MainWindow::c2_pcountChanged(quint8 value){if(!load){ ff7->setChocoPCount(s,1,value);}}
void MainWindow::c2_personalityChanged(quint8 value){if(!load){ ff7->setChocoPersonality(s,1,value);}}
void MainWindow::c2_mated_toggled(bool checked){if(!load){ff7->setChocoCantMate(s,1,checked);}}

void MainWindow::c3_nameChanged(QString text){if(!load){ff7->setChocoName(s,2,text);}}
void MainWindow::c3_staminaChanged(quint16 value){if(!load){ ff7->setChocoStamina(s,2,value);}}
void MainWindow::c3_speedChanged(quint16 value){if(!load){ ff7->setChocoSpeed(s,2,value);}}
void MainWindow::c3_maxspeedChanged(quint16 value){if(!load){ ff7->setChocoMaxSpeed(s,2,value);}}
void MainWindow::c3_sprintChanged(quint16 value){if(!load){ ff7->setChocoSprintSpeed(s,2,value);}}
void MainWindow::c3_maxsprintChanged(quint16 value){if(!load){ ff7->setChocoMaxSprintSpeed(s,2,value);}}
void MainWindow::c3_sexChanged(quint8 index){if(!load){ ff7->setChocoSex(s,2,index);}}
void MainWindow::c3_typeChanged(quint8 index){if(!load){ ff7->setChocoType(s,2,index);}}
void MainWindow::c3_coopChanged(quint8 value){if(!load){ ff7->setChocoCoop(s,2,value);}}
void MainWindow::c3_accelChanged(quint8 value){if(!load){ ff7->setChocoAccel(s,2,value);}}
void MainWindow::c3_intelChanged(quint8 value){if(!load){ ff7->setChocoIntelligence(s,2,value);}}
void MainWindow::c3_raceswonChanged(quint8 value){if(!load){ ff7->setChocoRaceswon(s,2,value);}}
void MainWindow::c3_pcountChanged(quint8 value){if(!load){ ff7->setChocoPCount(s,2,value);}}
void MainWindow::c3_personalityChanged(quint8 value){if(!load){ ff7->setChocoPersonality(s,2,value);}}
void MainWindow::c3_mated_toggled(bool checked){if(!load){ff7->setChocoCantMate(s,2,checked);}}

void MainWindow::c4_nameChanged(QString text){if(!load){ff7->setChocoName(s,3,text);}}
void MainWindow::c4_staminaChanged(quint16 value){if(!load){ ff7->setChocoStamina(s,3,value);}}
void MainWindow::c4_speedChanged(quint16 value){if(!load){ ff7->setChocoSpeed(s,3,value);}}
void MainWindow::c4_maxspeedChanged(quint16 value){if(!load){ ff7->setChocoMaxSpeed(s,3,value);}}
void MainWindow::c4_sprintChanged(quint16 value){if(!load){ ff7->setChocoSprintSpeed(s,3,value);}}
void MainWindow::c4_maxsprintChanged(quint16 value){if(!load){ ff7->setChocoMaxSprintSpeed(s,3,value);}}
void MainWindow::c4_sexChanged(quint8 index){if(!load){ ff7->setChocoSex(s,3,index);}}
void MainWindow::c4_typeChanged(quint8 index){if(!load){ ff7->setChocoType(s,3,index);}}
void MainWindow::c4_coopChanged(quint8 value){if(!load){ ff7->setChocoCoop(s,3,value);}}
void MainWindow::c4_accelChanged(quint8 value){if(!load){ ff7->setChocoAccel(s,3,value);}}
void MainWindow::c4_intelChanged(quint8 value){if(!load){ ff7->setChocoIntelligence(s,3,value);}}
void MainWindow::c4_raceswonChanged(quint8 value){if(!load){ ff7->setChocoRaceswon(s,3,value);}}
void MainWindow::c4_pcountChanged(quint8 value){if(!load){ ff7->setChocoPCount(s,3,value);}}
void MainWindow::c4_personalityChanged(quint8 value){if(!load){ ff7->setChocoPersonality(s,3,value);}}
void MainWindow::c4_mated_toggled(bool checked){if(!load){ff7->setChocoCantMate(s,3,checked);}}

void MainWindow::c5_nameChanged(QString text){if(!load){ff7->setChocoName(s,4,text);}}
void MainWindow::c5_staminaChanged(quint16 value){if(!load){ ff7->setChocoStamina(s,4,value);}}
void MainWindow::c5_speedChanged(quint16 value){if(!load){ ff7->setChocoSpeed(s,4,value);}}
void MainWindow::c5_maxspeedChanged(quint16 value){if(!load){ ff7->setChocoMaxSpeed(s,4,value);}}
void MainWindow::c5_sprintChanged(quint16 value){if(!load){ ff7->setChocoSprintSpeed(s,4,value);}}
void MainWindow::c5_maxsprintChanged(quint16 value){if(!load){ ff7->setChocoMaxSprintSpeed(s,4,value);}}
void MainWindow::c5_sexChanged(quint8 index){if(!load){ ff7->setChocoSex(s,4,index);}}
void MainWindow::c5_typeChanged(quint8 index){if(!load){ ff7->setChocoType(s,4,index);}}
void MainWindow::c5_coopChanged(quint8 value){if(!load){ ff7->setChocoCoop(s,4,value);}}
void MainWindow::c5_accelChanged(quint8 value){if(!load){ ff7->setChocoAccel(s,4,value);}}
void MainWindow::c5_intelChanged(quint8 value){if(!load){ ff7->setChocoIntelligence(s,4,value);}}
void MainWindow::c5_raceswonChanged(quint8 value){if(!load){ ff7->setChocoRaceswon(s,4,value);}}
void MainWindow::c5_pcountChanged(quint8 value){if(!load){ ff7->setChocoPCount(s,4,value);}}
void MainWindow::c5_personalityChanged(quint8 value){if(!load){ ff7->setChocoPersonality(s,4,value);}}
void MainWindow::c5_mated_toggled(bool checked){if(!load){ff7->setChocoCantMate(s,4,checked);}}

void MainWindow::c6_nameChanged(QString text){if(!load){ff7->setChocoName(s,5,text);}}
void MainWindow::c6_staminaChanged(quint16 value){if(!load){ ff7->setChocoStamina(s,5,value);}}
void MainWindow::c6_speedChanged(quint16 value){if(!load){ ff7->setChocoSpeed(s,5,value);}}
void MainWindow::c6_maxspeedChanged(quint16 value){if(!load){ ff7->setChocoMaxSpeed(s,5,value);}}
void MainWindow::c6_sprintChanged(quint16 value){if(!load){ ff7->setChocoSprintSpeed(s,5,value);}}
void MainWindow::c6_maxsprintChanged(quint16 value){if(!load){ ff7->setChocoMaxSprintSpeed(s,5,value);}}
void MainWindow::c6_sexChanged(quint8 index){if(!load){ ff7->setChocoSex(s,5,index);}}
void MainWindow::c6_typeChanged(quint8 index){if(!load){ ff7->setChocoType(s,5,index);}}
void MainWindow::c6_coopChanged(quint8 value){if(!load){ ff7->setChocoCoop(s,5,value);}}
void MainWindow::c6_accelChanged(quint8 value){if(!load){ ff7->setChocoAccel(s,5,value);}}
void MainWindow::c6_intelChanged(quint8 value){if(!load){ ff7->setChocoIntelligence(s,5,value);}}
void MainWindow::c6_raceswonChanged(quint8 value){if(!load){ ff7->setChocoRaceswon(s,5,value);}}
void MainWindow::c6_pcountChanged(quint8 value){if(!load){ ff7->setChocoPCount(s,5,value);}}
void MainWindow::c6_personalityChanged(quint8 value){if(!load){ ff7->setChocoPersonality(s,5,value);}}
void MainWindow::c6_mated_toggled(bool checked){if(!load){ff7->setChocoCantMate(s,5,checked);}}

//set data for pens outside
void MainWindow::on_combo_pen1_currentIndexChanged(int index){if(!load){ ff7->setChocoboPen(s,0,index);}}
void MainWindow::on_combo_pen2_currentIndexChanged(int index){if(!load){ ff7->setChocoboPen(s,1,index);}}
void MainWindow::on_combo_pen3_currentIndexChanged(int index){if(!load){ ff7->setChocoboPen(s,2,index);}}
void MainWindow::on_combo_pen4_currentIndexChanged(int index){if(!load){ ff7->setChocoboPen(s,3,index);}}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OTHERS TAB~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void MainWindow::on_sb_love_barret_valueChanged(int value){if(!load){file_modified(true); ff7->setLove(s,false,FF7Save::LOVE_BARRET,value);}}
void MainWindow::on_sb_love_aeris_valueChanged(int value){if(!load){file_modified(true); ff7->setLove(s,false,FF7Save::LOVE_AERIS,value);}}
void MainWindow::on_sb_love_tifa_valueChanged(int value){if(!load){file_modified(true); ff7->setLove(s,false,FF7Save::LOVE_TIFA,value);}}
void MainWindow::on_sb_love_yuffie_valueChanged(int value){if(!load){file_modified(true); ff7->setLove(s,false,FF7Save::LOVE_YUFFIE,value);}}

void MainWindow::on_sb_time_hour_valueChanged(int value){if(!load){ff7->setTime(s,((value*3600) + (ui->sb_time_min->value()*60) + (ui->sb_time_sec->value())));}}
void MainWindow::on_sb_time_min_valueChanged(int value){if(!load){ff7->setTime (s,( (ui->sb_time_hour->value()*3600) + ((value*60)) + (ui->sb_time_sec->value())));}}
void MainWindow::on_sb_time_sec_valueChanged(int value){if(!load){ff7->setTime(s,((ui->sb_time_hour->value()*3600) + (ui->sb_time_min->value()*60) + (value)));}}

void MainWindow::on_sb_steps_valueChanged(int value){if(!load){file_modified(true);  ff7->slot[s].steps = value;}}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Item Tab~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void MainWindow::on_list_flyers_clicked(const QModelIndex &index)
{if(!load){file_modified(true);
    ff7->setTurtleParadiseFlyerSeen(s,index.row(),ui->list_flyers->item(index.row())->checkState());
}}

void MainWindow::on_list_keyitems_clicked(const QModelIndex &index)
{if(!load){file_modified(true);
    ff7->setKeyItem(s,index.row(),ui->list_keyitems->item(index.row())->checkState());
}}

// Field Items Combos
void MainWindow::on_cb_bm_items_1_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,38);  char t = temp.at(48);
        if(checked){t |= (1<<0);}
        else{t &= ~(1<<0);}
        temp[48]=t;
        ff7->setUnknown(s,38,temp);
 }}
void MainWindow::on_cb_bm_items_2_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,38); char t = temp .at(48);
        if(checked){t |= (1<<1);}
        else{t &= ~(1<<1);}
        temp[48]=t;
        ff7->setUnknown(s,38,temp);
}}
void MainWindow::on_cb_bm_items_3_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,38); char t = temp .at(48);
        if(checked){t |= (1<<2);}
        else{t &= ~(1<<2);}
        temp[48]=t;
        ff7->setUnknown(s,38,temp);
}}
void MainWindow::on_cb_bm_items_4_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,38); char t = temp .at(48);
        if(checked){t |= (1<<3);}
        else{t &= ~(1<<3);}
        temp[48]=t;
        ff7->setUnknown(s,38,temp);
}}

void MainWindow::on_cb_s7tg_items_1_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,9);char t = temp.at(4);
        if(checked){t |= (1<<0);}
        else{t &= ~(1<<0);}
        temp[4]=t;
        ff7->setUnknown(s,9,temp);
}}

void MainWindow::on_cb_s7tg_items_2_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,9);char t = temp.at(4);
        if(checked){t |= (1<<1);}
        else{t &= ~(1<<1);}
        temp[4]=t;
        ff7->setUnknown(s,9,temp);
}}

void MainWindow::on_cb_s7tg_items_3_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,9);char t = temp.at(4);
        if(checked){t |= (1<<2);}
        else{t &= ~(1<<2);}
        temp[4]=t;
        ff7->setUnknown(s,9,temp);
}}

void MainWindow::on_cb_s7tg_items_4_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,9);char t = temp.at(4);
        if(checked){t |= (1<<3);}
        else{t &= ~(1<<3);}
        temp[4]=t;
        ff7->setUnknown(s,9,temp);
}}

void MainWindow::on_cb_s7tg_items_5_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,9);char t = temp.at(4);
        if(checked){t |= (1<<4);}
        else{t &= ~(1<<4);}
        temp[4]=t;
        ff7->setUnknown(s,9,temp);
}}

void MainWindow::on_cb_s7tg_items_6_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,9);char t = temp.at(4);
        if(checked){t |= (1<<5);}
        else{t &= ~(1<<5);}
        temp[4]=t;
        ff7->setUnknown(s,9,temp);
}}

void MainWindow::on_cb_s7tg_items_7_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,9);char t = temp.at(4);
        if(checked){t |= (1<<6);}
        else{t &= ~(1<<6);}
        temp[4]=t;
        ff7->setUnknown(s,9,temp);
}}

void MainWindow::on_cb_s7tg_items_8_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,9);char t = temp.at(4);
        if(checked){t |= (1<<7);}
        else{t &= ~(1<<7);}
        temp[4]=t;
        ff7->setUnknown(s,9,temp);
}}

void MainWindow::on_cb_farm_items_1_toggled(bool checked)
{if(!load)
    {
        file_modified(true);
        QByteArray temp = ff7->unknown(s,11); char t = temp.at (3);
        if(checked){t |= (1<<0);}
        else{t &= ~(1<<0);}
        temp[3]=t;
        ff7->setUnknown(s,11,temp);
    }
    ui->lcd_farm_items->display(ff7->unknown(s,11).at(3));
}

void MainWindow::on_cb_farm_items_2_toggled(bool checked)
{if(!load)
    {
        file_modified(true);
        QByteArray temp = ff7->unknown(s,11); char t = temp.at (3);
        if(checked){t |= (1<<1);}
        else{t &= ~(1<<1);}
        temp[3]=t;
        ff7->setUnknown(s,11,temp);
    }
    ui->lcd_farm_items->display(ff7->unknown(s,11).at(3));
}

void MainWindow::on_cb_farm_items_3_toggled(bool checked)
{if(!load)
    {
        file_modified(true);
        QByteArray temp = ff7->unknown(s,11); char t = temp.at (3);
        if(checked){t |= (1<<2);}
        else{t &= ~(1<<2);}
        temp[3]=t;
        ff7->setUnknown(s,11,temp);
    }
    ui->lcd_farm_items->display(ff7->unknown(s,11).at(3));
}

void MainWindow::on_cb_farm_items_4_toggled(bool checked)
{if(!load)
    {
        file_modified(true);
        QByteArray temp = ff7->unknown(s,11); char t = temp.at (3);
        if(checked){t |= (1<<3);}
        else{t &= ~(1<<3);}
        temp[3]=t;
        ff7->setUnknown(s,11,temp);
    }
    ui->lcd_farm_items->display(ff7->unknown(s,11).at(3));
}

void MainWindow::on_cb_farm_items_5_toggled(bool checked)
{if(!load)
    {
        file_modified(true);
        QByteArray temp = ff7->unknown(s,11); char t = temp.at (3);
        if(checked){t |= (1<<4);}
        else{t &= ~(1<<4);}
        temp[3]=t;
        ff7->setUnknown(s,11,temp);
    }
    ui->lcd_farm_items->display(ff7->unknown(s,11).at(3));
}

void MainWindow::on_cb_farm_items_6_toggled(bool checked)
{if(!load)
    {
        file_modified(true);
        QByteArray temp = ff7->unknown(s,11); char t = temp.at (3);
        if(checked){t |= (1<<5);}
        else{t &= ~(1<<5);}
        temp[3]=t;
        ff7->setUnknown(s,11,temp);
    }
    ui->lcd_farm_items->display(ff7->unknown(s,11).at(3));
}

void MainWindow::on_cb_farm_items_7_toggled(bool checked)
{if(!load)
    {
        file_modified(true);
        QByteArray temp = ff7->unknown(s,11); char t = temp.at (3);
        if(checked){t |= (1<<6);}
        else{t &= ~(1<<6);}
        temp[3]=t;
        ff7->setUnknown(s,11,temp);
    }
    ui->lcd_farm_items->display(ff7->unknown(s,11).at(3));
}

void MainWindow::on_cb_farm_items_8_toggled(bool checked)
{if(!load)
    {
        file_modified(true);
        QByteArray temp = ff7->unknown(s,11); char t = temp.at (3);
        if(checked){t |= (1<<7);}
        else{t &= ~(1<<7);}
        temp[3]=t;
        ff7->setUnknown(s,11,temp);
    }
    ui->lcd_farm_items->display(ff7->unknown(s,11).at(3));
}

void MainWindow::on_btn_clear_keyitems_clicked()
{if(!load){file_modified(true);}//used in other functions
    for(int i=0;i<51;i++)
    {
        ui->list_keyitems->item(i)->setCheckState(Qt::Unchecked);
    }
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MATERIA TAB~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void MainWindow::on_tbl_materia_currentCellChanged(int row){if(!load){materia_editor->setMateria(ff7->partyMateriaId(s,row),ff7->partyMateriaAp(s,row));}}


void MainWindow::on_btn_add_all_materia_clicked()
{
    //place one of each at lowest ossible point
    for(int i=117;i<142;i++)
    {//Starting With Magic Materia
        if(i<132){ff7->setPartyMateria(s,i,(i-68),FF7Materia::MaxMateriaAp);}
        else if((i>=132) && (i<136)){ff7->setPartyMateria(s,(i-1),(i-68),FF7Materia::MaxMateriaAp);}
        else if((i>=136) && (i<142)){ff7->setPartyMateria(s,(i-3),(i-68),FF7Materia::MaxMateriaAp);}
    }
    // Then Support
    for(int i=139;i<152;i++){ff7->setPartyMateria(s,i,(i-116),FF7Materia::MaxMateriaAp);}

    for(int i=152;i<166;i++)
    {//Then Command
        if(i<154){ff7->setPartyMateria(s,i,(i-138),FF7Materia::MaxMateriaAp);}
        else if(i<157){ff7->setPartyMateria(s,i,(i-135),FF7Materia::MaxMateriaAp);}
        else if(i<159){ff7->setPartyMateria(s,i,(i-121),FF7Materia::MaxMateriaAp);}
        else if(i<165){ff7->setPartyMateria(s,i,(i-120),FF7Materia::MaxMateriaAp);}
        else {ff7->setPartyMateria(s,i,0x30,FF7Materia::MaxMateriaAp);}
    }
    for(int i=166;i<183;i++)
    {//And Independent
        if(i<180){ff7->setPartyMateria(s,i,(i-166),FF7Materia::MaxMateriaAp);}
        else{ff7->setPartyMateria(s,i,(i-164),FF7Materia::MaxMateriaAp);}
    }
    //Finish With Summons
    for(int i=183;i<200;i++){ff7->setPartyMateria(s,i,(i-109),FF7Materia::MaxMateriaAp);}
    materiaupdate();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SAVE LOCATION TAB~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::on_tbl_location_field_itemSelectionChanged()
{
    ui->tbl_location_field->setCurrentCell(ui->tbl_location_field->currentRow(),0);
    ui->line_location->setText(ui->tbl_location_field->currentItem()->text());
    ui->tbl_location_field->setCurrentCell(ui->tbl_location_field->currentRow(),1);
    ui->sb_map_id->setValue(ui->tbl_location_field->currentItem()->text().toInt());
    ui->tbl_location_field->setCurrentCell(ui->tbl_location_field->currentRow(),2);
    ui->sb_loc_id->setValue(ui->tbl_location_field->currentItem()->text().toInt());
    ui->tbl_location_field->setCurrentCell(ui->tbl_location_field->currentRow(),3);
    ui->sb_coordx->setValue(ui->tbl_location_field->currentItem()->text().toInt());
    ui->tbl_location_field->setCurrentCell(ui->tbl_location_field->currentRow(),4);
    ui->sb_coordy->setValue(ui->tbl_location_field->currentItem()->text().toInt());
    ui->tbl_location_field->setCurrentCell(ui->tbl_location_field->currentRow(),5);
    ui->sb_coordz->setValue(ui->tbl_location_field->currentItem()->text().toInt());
    ui->lbl_fieldFile->setText(QString("%1").arg(Locations.fileName(ff7->mapId(s),ff7->locationId(s))));
    ui->lbl_locationPreview->setPixmap(QString("://locations/%1_%2").arg(QString::number(ff7->mapId(s)),QString::number(ff7->locationId(s))));
}
void MainWindow::on_sb_map_id_valueChanged(int value)
{if(!load){file_modified(true);
        ff7->setMapId(s, value);
        ui->lbl_fieldFile->setText(QString("%1").arg(Locations.fileName(ff7->mapId(s),ff7->locationId(s))));
        ui->lbl_locationPreview->setPixmap(QString("://locations/%1_%2").arg(QString::number(ff7->mapId(s)),QString::number(ff7->locationId(s))));
 }}
void MainWindow::on_sb_loc_id_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->setLocationId(s,value);
    ui->lbl_fieldFile->setText(QString("%1").arg(Locations.fileName(ff7->mapId(s),ff7->locationId(s))));
    ui->lbl_locationPreview->setPixmap(QString("://locations/%1_%2").arg(QString::number(ff7->mapId(s)),QString::number(ff7->locationId(s))));
}}
void MainWindow::on_sb_coordx_valueChanged(int value){if(!load){file_modified(true); ff7->setLocationX(s,value);}}
void MainWindow::on_sb_coordy_valueChanged(int value){if(!load){file_modified(true); ff7->setLocationY(s,value);}}
void MainWindow::on_sb_coordz_valueChanged(int value){if(!load){file_modified(true); ff7->setLocationZ(s,value);}}

void MainWindow::on_line_location_textChanged(QString text)
{
    if (!load)
    {
        QString lang = QCoreApplication::applicationDirPath() +"/"+ "lang/bchoco_";// base path and name for translation files.
        QTranslator Translator;// will do the translating.
        QString region = ff7->region(s);//get region
        region.chop(7);// remove trailing  FF7-SXX
        if(region =="BASLUS-94163" || region =="BESLES-00867"){lang.append("en.qm");}
        else if(region =="BESCES-00868"){lang.append("fr.qm");}
        else if(region =="BESCES-00869"){lang.append("de.qm");}
        else if(region =="BESCES-00900"){lang.append("es.qm");}
        else if(region =="BISLPS-00700" || region =="BISLPS-01057"){lang.append("ja.qm");}
        else{}//unknown language.
        Translator.load(lang);
        QString newText = Translator.translate("Locations",text.toUtf8());
        if(newText.isEmpty()){ff7->setLocation(s,text);}
        else{ff7->setLocation(s,newText);}
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~CHARACTER TAB~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~ Game Options~~~~~~~~~~~~~~~~~~*/
void MainWindow::setDialogColorUL(QColor color){if(!load){ff7->setDialogColorUL(s,color);}}
void MainWindow::setDialogColorUR(QColor color){if(!load){ff7->setDialogColorUR(s,color);}}
void MainWindow::setDialogColorLL(QColor color){if(!load){ff7->setDialogColorLL(s,color);}}
void MainWindow::setDialogColorLR(QColor color){if(!load){ff7->setDialogColorLR(s,color);}}

void MainWindow::setBattleSpeed(int value){if(!load){file_modified(true);ff7->setBattleSpeed(s,value);}}
void MainWindow::setBattleMessageSpeed(int value){if(!load){file_modified(true); ff7->setBattleMessageSpeed(s,value);}}
void MainWindow::setFieldMessageSpeed(int value){if(!load){file_modified(true); ff7->setMessageSpeed(s, value);}}
void MainWindow::setBattleHelp(bool checked){if(!load){file_modified(true); ff7->setBattleHelp(s,checked);}}
void MainWindow::setFieldHelp(bool checked){if(!load){file_modified(true);ff7->setFieldHelp(s,checked);}}
void MainWindow::setBattleTargets(bool checked){if(!load){file_modified(true);ff7->setBattleTargets(s,checked);}}


void MainWindow::setControlMode(int mode){if(!load){file_modified(true);ff7->setControlMode(s,mode);}}
void MainWindow::setSoundMode(int mode){if(!load){file_modified(true); ff7->setSoundMode(s,mode);}}
void MainWindow::setCursorMode(int mode){if(!load){file_modified(true);ff7->setCursorMode(s,mode);}}
void MainWindow::setAtbMode(int mode){if(!load){file_modified(true); ff7->setAtbMode(s,mode);}}
void MainWindow::setCameraMode(int mode){if(!load){file_modified(true);ff7->setCameraMode(s,mode);}}
void MainWindow::setMagicOrder(int order){if(!load){file_modified(true);ff7->setMagicOrder(s,order);}}

void MainWindow::setButtonCamera(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_CAMERA,index);}}
void MainWindow::setButtonTarget(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_TARGET,index);}}
void MainWindow::setButtonPageUp(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_PAGEUP,index);}}
void MainWindow::setButtonPageDown(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_PAGEDOWN,index);}}
void MainWindow::setButtonMenu(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_MENU,index);}}
void MainWindow::setButtonOk(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_OK,index);}}
void MainWindow::setButtonCancel(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_CANCEL,index);}}
void MainWindow::setButtonSwitch(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_SWITCH,index);}}
void MainWindow::setButtonHelp(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_HELP,index);}}
void MainWindow::setButtonUnknown1(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_UNKNOWN1,index);}}
void MainWindow::setButtonUnknown2(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_UNKNOWN2,index);}}
void MainWindow::setButtonPause(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_PAUSE,index);}}
void MainWindow::setButtonUp(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_UP,index);}}
void MainWindow::setButtonDown(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_DOWN,index);}}
void MainWindow::setButtonLeft(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_LEFT,index);}}
void MainWindow::setButtonRight(int index){if(!load){file_modified(true); ff7->setControllerMapping(s,FF7Save::ACTION_RIGHT,index);}}

/*--------GAME PROGRESS-------*/
void MainWindow::on_sb_curdisc_valueChanged(int value){if(!load){file_modified(true); ff7->setDisc(s,value);}}
void MainWindow::on_sb_mprogress_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].mprogress = value;}}
void MainWindow::on_sb_turkschruch_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].aeris_chruch=value;}}
void MainWindow::on_sb_donprog_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].donprogress=value;}}

void MainWindow::on_cb_bm1_1_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress1 |= (1<<0);}else{ff7->slot[s].bm_progress1 &= ~(1<<0);}}}
void MainWindow::on_cb_bm1_2_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress1 |= (1<<1);}else{ff7->slot[s].bm_progress1 &= ~(1<<1);}}}
void MainWindow::on_cb_bm1_3_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress1 |= (1<<2);}else{ff7->slot[s].bm_progress1 &= ~(1<<2);}}}
void MainWindow::on_cb_bm1_4_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress1 |= (1<<3);}else{ff7->slot[s].bm_progress1 &= ~(1<<3);}}}
void MainWindow::on_cb_bm1_5_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress1 |= (1<<4);}else{ff7->slot[s].bm_progress1 &= ~(1<<4);}}}
void MainWindow::on_cb_bm1_6_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress1 |= (1<<5);}else{ff7->slot[s].bm_progress1 &= ~(1<<5);}}}
void MainWindow::on_cb_bm1_7_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress1 |= (1<<6);}else{ff7->slot[s].bm_progress1 &= ~(1<<6);}}}
void MainWindow::on_cb_bm1_8_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress1 |= (1<<7);}else{ff7->slot[s].bm_progress1 &= ~(1<<7);}}}

void MainWindow::on_cb_bm2_1_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress2 |= (1<<0);}else{ff7->slot[s].bm_progress2 &= ~(1<<0);}}}
void MainWindow::on_cb_bm2_2_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress2 |= (1<<1);}else{ff7->slot[s].bm_progress2 &= ~(1<<1);}}}
void MainWindow::on_cb_bm2_3_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress2 |= (1<<2);}else{ff7->slot[s].bm_progress2 &= ~(1<<2);}}}
void MainWindow::on_cb_bm2_4_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress2 |= (1<<3);}else{ff7->slot[s].bm_progress2 &= ~(1<<3);}}}
void MainWindow::on_cb_bm2_5_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress2 |= (1<<4);}else{ff7->slot[s].bm_progress2 &= ~(1<<4);}}}
void MainWindow::on_cb_bm2_6_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress2 |= (1<<5);}else{ff7->slot[s].bm_progress2 &= ~(1<<5);}}}
void MainWindow::on_cb_bm2_7_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress2 |= (1<<6);}else{ff7->slot[s].bm_progress2 &= ~(1<<6);}}}
void MainWindow::on_cb_bm2_8_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress2 |= (1<<7);}else{ff7->slot[s].bm_progress2 &= ~(1<<7);}}}

void MainWindow::on_cb_bm3_1_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress3 |= (1<<0);}else{ff7->slot[s].bm_progress3 &= ~(1<<0);}}}
void MainWindow::on_cb_bm3_2_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress3 |= (1<<1);}else{ff7->slot[s].bm_progress3 &= ~(1<<1);}}}
void MainWindow::on_cb_bm3_3_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress3 |= (1<<2);}else{ff7->slot[s].bm_progress3 &= ~(1<<2);}}}
void MainWindow::on_cb_bm3_4_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress3 |= (1<<3);}else{ff7->slot[s].bm_progress3 &= ~(1<<3);}}}
void MainWindow::on_cb_bm3_5_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress3 |= (1<<4);}else{ff7->slot[s].bm_progress3 &= ~(1<<4);}}}
void MainWindow::on_cb_bm3_6_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress3 |= (1<<5);}else{ff7->slot[s].bm_progress3 &= ~(1<<5);}}}
void MainWindow::on_cb_bm3_7_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress3 |= (1<<6);}else{ff7->slot[s].bm_progress3 &= ~(1<<6);}}}
void MainWindow::on_cb_bm3_8_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].bm_progress3 |= (1<<7);}else{ff7->slot[s].bm_progress3 &= ~(1<<7);}}}

void MainWindow::on_cb_s7pl_1_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(0);
        if(checked){t |= (1<<0);}
        else{t &= ~(1<<0);}
        temp[0]=t;
        ff7->setUnknown(s,26,temp);
}}
void MainWindow::on_cb_s7pl_2_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(0);
        if(checked){t |= (1<<1);}
        else{t &= ~(1<<1);}
        temp[0]=t;
        ff7->setUnknown(s,26,temp);
}}
void MainWindow::on_cb_s7pl_3_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(0);
        if(checked){t |= (1<<2);}
        else{t &= ~(1<<2);}
        temp[0]=t;
        ff7->setUnknown(s,26,temp);
}}
void MainWindow::on_cb_s7pl_4_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(0);
        if(checked){t |= (1<<3);}
        else{t &= ~(1<<3);}
        temp[0]=t;
        ff7->setUnknown(s,26,temp);
}}
void MainWindow::on_cb_s7pl_5_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(0);
        if(checked){t |= (1<<4);}
        else{t &= ~(1<<4);}
        temp[0]=t;
        ff7->setUnknown(s,26,temp);
}}
void MainWindow::on_cb_s7pl_6_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(0);
        if(checked){t |= (1<<5);}
        else{t &= ~(1<<5);}
        temp[0]=t;
        ff7->setUnknown(s,26,temp);
}}
void MainWindow::on_cb_s7pl_7_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(0);
        if(checked){t |= (1<<6);}
        else{t &= ~(1<<6);}
        temp[0]=t;
        ff7->setUnknown(s,26,temp);
}}
void MainWindow::on_cb_s7pl_8_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(0);
        if(checked){t |= (1<<7);}
        else{t &= ~(1<<7);}
        temp[0]=t;
        ff7->setUnknown(s,26,temp);
}}

void MainWindow::on_cb_s7ts_1_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(8);
        if(checked){t |= (1<<0);}
        else{t &= ~(1<<0);}
        temp[8]=t;
        ff7->setUnknown(s,26,temp);
}}

void MainWindow::on_cb_s7ts_2_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(8);
        if(checked){t |= (1<<1);}
        else{t &= ~(1<<1);}
        temp[8]=t;
        ff7->setUnknown(s,26,temp);
}}

void MainWindow::on_cb_s7ts_3_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(8);
        if(checked){t |= (1<<2);}
        else{t &= ~(1<<2);}
        temp[8]=t;
        ff7->setUnknown(s,26,temp);
}}

void MainWindow::on_cb_s7ts_4_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(8);
        if(checked){t |= (1<<3);}
        else{t &= ~(1<<3);}
        temp[8]=t;
        ff7->setUnknown(s,26,temp);
}}

void MainWindow::on_cb_s7ts_5_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(8);
        if(checked){t |= (1<<4);}
        else{t &= ~(1<<4);}
        temp[8]=t;
        ff7->setUnknown(s,26,temp);
}}

void MainWindow::on_cb_s7ts_6_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(8);
        if(checked){t |= (1<<5);}
        else{t &= ~(1<<5);}
        temp[8]=t;
        ff7->setUnknown(s,26,temp);
}}

void MainWindow::on_cb_s7ts_7_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(8);
        if(checked){t |= (1<<6);}
        else{t &= ~(1<<6);}
        temp[8]=t;
        ff7->setUnknown(s,26,temp);
}}

void MainWindow::on_cb_s7ts_8_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,26); char t=temp.at(8);
        if(checked){t |= (1<<7);}
        else{t &= ~(1<<7);}
        temp[8]=t;
        ff7->setUnknown(s,26,temp);
}}

void MainWindow::on_cb_s5_1_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,23); char t=temp.at(26);
        if(checked){t |= (1<<0);}
        else{t &= ~(1<<0);}
        temp[26]=t;
        ff7->setUnknown(s,23,temp);
}}

void MainWindow::on_cb_s5_2_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,23); char t=temp.at(26);
        if(checked){t |= (1<<1);}
        else{t &= ~(1<<1);}
        temp[26]=t;
        ff7->setUnknown(s,23,temp);
}}

void MainWindow::on_cb_s5_3_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,23); char t=temp.at(26);
        if(checked){t |= (1<<2);}
        else{t &= ~(1<<2);}
        temp[26]=t;
        ff7->setUnknown(s,23,temp);
}}

void MainWindow::on_cb_s5_4_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,23); char t=temp.at(26);
        if(checked){t |= (1<<3);}
        else{t &= ~(1<<3);}
        temp[26]=t;
        ff7->setUnknown(s,23,temp);
}}

void MainWindow::on_cb_s5_5_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,23); char t=temp.at(26);
        if(checked){t |= (1<<4);}
        else{t &= ~(1<<4);}
        temp[26]=t;
        ff7->setUnknown(s,23,temp);
}}

void MainWindow::on_cb_s5_6_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,23); char t=temp.at(26);
        if(checked){t |= (1<<5);}
        else{t &= ~(1<<5);}
        temp[26]=t;
        ff7->setUnknown(s,23,temp);
}}

void MainWindow::on_cb_s5_7_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,23); char t=temp.at(26);
        if(checked){t |= (1<<6);}
        else{t &= ~(1<<6);}
        temp[26]=t;
        ff7->setUnknown(s,23,temp);
}}

void MainWindow::on_cb_s5_8_toggled(bool checked)
{if(!load){file_modified(true);
        QByteArray temp = ff7->unknown(s,23); char t=temp.at(26);
        if(checked){t |= (1<<7);}
        else{t &= ~(1<<7);}
        temp[26]=t;
        ff7->setUnknown(s,23,temp);
}}

void MainWindow::on_cb_bombing_int_stateChanged(int checked)
{if(!load){file_modified(true);
    if(checked == Qt::Checked){ff7->slot[s].intbombing =0x14;}
    else{ff7->slot[s].intbombing =0x56;}
}}

void MainWindow::on_cb_replay_currentIndexChanged(int index)
{
    if(index == 1) // bombing mission
    {
        ui->sb_curdisc->setValue(1);
        ui->sb_mprogress->setValue(1);
        ff7->slot[s].bm_progress1=0;
        ff7->slot[s].bm_progress2=0;
        ff7->slot[s].bm_progress3=0;
        ui->cb_bombing_int->setChecked(Qt::Checked);
        ui->cb_midgartrain_1->setChecked(Qt::Unchecked);
        ui->cb_midgartrain_2->setChecked(Qt::Unchecked);
        ui->cb_midgartrain_3->setChecked(Qt::Unchecked);
        ui->cb_midgartrain_4->setChecked(Qt::Unchecked);
        ui->cb_midgartrain_5->setChecked(Qt::Unchecked);
        ui->cb_midgartrain_6->setChecked(Qt::Unchecked);
        ui->cb_midgartrain_7->setChecked(Qt::Unchecked);
        ui->cb_midgartrain_8->setChecked(Qt::Unchecked);
        ui->combo_s7_slums->setCurrentIndex(1);
        ui->cb_s5_7->setChecked(Qt::Unchecked);//show aeris on roof of chruch durring script
        ui->cb_s5_8->setChecked(Qt::Unchecked);//not after chruch scene.
        ui->sb_turkschruch->setValue(0); // reset turks.
        ui->line_location->setText(tr("Platform"));
        ui->sb_map_id->setValue(1);
        ui->sb_loc_id->setValue(116);
        ui->sb_coordx->setValue(3655);
        ui->sb_coordy->setValue(27432);
        ui->sb_coordz->setValue(25);
        ui->label_replaynote->setText(tr("Replay the bombing mission from right after you get off the train."));
    }
    else if(index == 2) // The Church In The Slums
    {
        ui->sb_curdisc->setValue(1);
        ui->sb_mprogress->setValue(130);
        ui->sb_turkschruch->setValue(0);
        ff7->slot[s].bm_progress1=120;
        ff7->slot[s].bm_progress2=198;
        ff7->slot[s].bm_progress3=3;
        ui->cb_bombing_int->setChecked(Qt::Unchecked);
        ui->cb_s5_7->setChecked(Qt::Unchecked);//show aeris on roof of chruch durring script
        ui->cb_s5_8->setChecked(Qt::Unchecked);//not after chruch scene.
        ui->line_location->setText(tr("Church in the Slums"));
        ui->sb_map_id->setValue(1);
        ui->sb_loc_id->setValue(183);
        ui->sb_coordx->setValue(65463);
        ui->sb_coordy->setValue(400);
        ui->sb_coordz->setValue(8);
        ui->combo_party1->setCurrentIndex(0);
        ui->combo_party2->setCurrentIndex(12);
        ui->combo_party3->setCurrentIndex(12);
        ui->label_replaynote->setText(tr("Meeting Aeris"));

    }
    else if (index ==3)// Flash back
    {
        ui->sb_curdisc->setValue(1);
        ui->sb_mprogress->setValue(341);
        ff7->slot[s].bm_progress1=120;
        ff7->slot[s].bm_progress2=198;
        ff7->slot[s].bm_progress3=3;
        ui->cb_bombing_int->setChecked(Qt::Unchecked);
        ui->line_location->setText(tr("Kalm Inn"));
        ui->sb_map_id->setValue(1);
        ui->sb_loc_id->setValue(332);
        ui->sb_coordx->setValue(267);
        ui->sb_coordy->setValue(65429);
        ui->sb_coordz->setValue(15);
        // set up young cloud, Copy Cloud Change ID to young Cloud
        ff7->setCharacter(s,6,ff7->character(s,0));
        ff7->setCharID(s,6,FF7Char::YoungCloud);
        //set up Sephiroth
        ff7->setCharID(s,7,FF7Char::Sephiroth);
        if(ff7->region(s).contains("00700") || ff7->region(s).contains("01057")){ff7->setCharName(s,7,QString::fromUtf8("セフィロス"));}
        else{ff7->setCharName(s,7,QString::fromUtf8("Sephiroth"));}
        set_char_buttons();
        if(curchar == FF7Char::CaitSith){char_editor->setChar(ff7->character(s,6),ff7->charName(s,6));}
        else if(curchar ==FF7Char::Vincent){char_editor->setChar(ff7->character(s,7),ff7->charName(s,7));}
        ui->label_replaynote->setText(tr("Setting This Will Copy Cloud as is to young cloud (caitsith's slot). sephiroth's stats will come directly from vincent."));
    }

    else if(index == 4) // The Date Scene
    {
        ui->sb_curdisc->setValue(1);
        ui->sb_mprogress->setValue(583);
        ff7->slot[s].bm_progress1=120;
        ff7->slot[s].bm_progress2=198;
        ff7->slot[s].bm_progress3=3;
        ui->cb_bombing_int->setChecked(Qt::Unchecked);
        ui->line_location->setText(tr("Ropeway Station"));
        ui->sb_map_id->setValue(1);
        ui->sb_loc_id->setValue(496);
        ui->sb_coordx->setValue(64767);
        ui->sb_coordy->setValue(95);
        ui->sb_coordz->setValue(26);
        ui->label_replaynote->setText(tr("Replay the Date Scene, Your Location will be set To The Ropeway Station Talk to man by the Tram to start event. If Your Looking for a special Date be sure to set your love points too."));
    }

    else if (index == 5)//Aeris Death
    {
        ui->sb_curdisc->setValue(1);
        ui->sb_mprogress->setValue(664);
        ff7->slot[s].bm_progress1=120;
        ff7->slot[s].bm_progress2=198;
        ff7->slot[s].bm_progress3=3;
        ui->cb_bombing_int->setChecked(Qt::Unchecked);
        ui->line_location->setText(tr("Forgotten City"));
        ui->sb_map_id->setValue(1);
        ui->sb_loc_id->setValue(646);
        ui->sb_coordx->setValue(641);
        ui->sb_coordy->setValue(793);
        ui->sb_coordz->setValue(243);
        phsList->setChecked(3,1,false);
        phsList->setChecked(3,2,false);
        //ui->list_chars_unlocked->item(3)->setCheckState(Qt::Unchecked);
        //ui->list_phs_chars->item(3)->setCheckState(Qt::Unchecked);
        ui->label_replaynote->setText(tr("Replay the death of Aeris.This option Will remove Aeris from your PHS"));
    }

    else {ui->label_replaynote->setText(tr("         INFO ON CURRENTLY SELECTED REPLAY MISSION"));}
    if(!load){file_modified(true); progress_update();}
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FUNCTIONS FOR TESTING~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void MainWindow::on_btn_remove_all_items_clicked() //used for testing
{
    for(int i=0;i<320;i++){ff7->setItem(s,i,FF7Item::EmptyItemData);}
    itemlist->setItems(ff7->items(s));
}

void MainWindow::on_btn_remove_all_materia_clicked()
{
    for (int i=0;i<200;i++){ff7->setPartyMateria(s,i,FF7Materia::EmptyId,FF7Materia::MaxMateriaAp);}
    materiaupdate();
}

void MainWindow::on_btn_remove_all_stolen_clicked()
{
    for(int i=0;i<48;i++){ff7->setStolenMateria(s,i,FF7Materia::EmptyId,FF7Materia::MaxMateriaAp);}
    guirefresh(0);
}

void MainWindow::on_sb_b_love_aeris_valueChanged(int value){if(!load){file_modified(true); ff7->setLove(s,true,FF7Save::LOVE_AERIS,value);}}
void MainWindow::on_sb_b_love_tifa_valueChanged(int value){if(!load){file_modified(true); ff7->setLove(s,true,FF7Save::LOVE_TIFA,value);}}
void MainWindow::on_sb_b_love_yuffie_valueChanged(int value){if(!load){file_modified(true); ff7->setLove(s,true,FF7Save::LOVE_YUFFIE,value);}}
void MainWindow::on_sb_b_love_barret_valueChanged(int value){if(!load){file_modified(true); ff7->setLove(s,true,FF7Save::LOVE_BARRET,value);}}
void MainWindow::on_sb_coster_1_valueChanged(int value){if(!load){file_modified(true); ff7->setSpeedScore(s,1,value);}}
void MainWindow::on_sb_coster_2_valueChanged(int value){if(!load){file_modified(true); ff7->setSpeedScore(s,2,value);}}
void MainWindow::on_sb_coster_3_valueChanged(int value){if(!load){file_modified(true); ff7->setSpeedScore(s,3,value);}}
void MainWindow::on_sb_timer_time_hour_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].timer[0] = value;}}
void MainWindow::on_sb_timer_time_min_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].timer[1] = value;}}
void MainWindow::on_sb_timer_time_sec_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].timer[2] = value;}}

void MainWindow::on_sb_u_weapon_hp_valueChanged(int value)
{if(!load){file_modified(true);
    load=true;
    int a = (value & 0xff);
    int b = (value & 0xff00) >> 8;
    int c = (value & 0xff0000) >> 16;
    ff7->slot[s].u_weapon_hp[0] = a;
    ff7->slot[s].u_weapon_hp[1] = b;
    ff7->slot[s].u_weapon_hp[2] = c;
    load=false;
}}

void MainWindow::on_cb_reg_vinny_toggled(bool checked)
{if(!load){file_modified(true);
    if (checked){ff7->slot[s].reg_vinny =0xFF;}
    else{ff7->slot[s].reg_vinny =0xFB;}
    testdata_refresh();
}}

void MainWindow::on_cb_itemmask1_1_toggled(bool checked){if(!load){file_modified(true);   ff7->setItemMask1(s,0,checked);}}
void MainWindow::on_cb_itemmask1_2_toggled(bool checked){if(!load){file_modified(true);   ff7->setItemMask1(s,1,checked);}}
void MainWindow::on_cb_itemmask1_3_toggled(bool checked){if(!load){file_modified(true);   ff7->setItemMask1(s,2,checked);}}
void MainWindow::on_cb_itemmask1_4_toggled(bool checked){if(!load){file_modified(true);   ff7->setItemMask1(s,3,checked);}}
void MainWindow::on_cb_itemmask1_5_toggled(bool checked){if(!load){file_modified(true);   ff7->setItemMask1(s,4,checked);}}
void MainWindow::on_cb_itemmask1_6_toggled(bool checked){if(!load){file_modified(true);   ff7->setItemMask1(s,5,checked);}}
void MainWindow::on_cb_itemmask1_7_toggled(bool checked){if(!load){file_modified(true);   ff7->setItemMask1(s,6,checked);}}
void MainWindow::on_cb_itemmask1_8_toggled(bool checked){if(!load){file_modified(true);   ff7->setItemMask1(s,7,checked);}}

void MainWindow::on_cb_gaiin_1Javelin_toggled(bool checked){if(!load){file_modified(true); ff7->setGaiin_1Javelin(s,checked);}}
void MainWindow::on_cb_gaiin_1Ribbon_toggled(bool checked){if(!load){file_modified(true);ff7->setGaiin_1Ribbon(s,checked);}}
void MainWindow::on_cb_gaiin_3Elixir_toggled(bool checked){if(!load){file_modified(true);ff7->setGaiin_3Elixir(s,checked);}}
void MainWindow::on_cb_gaiin_3SpeedSource_toggled(bool checked){if(!load){file_modified(true);ff7->setGaiin_3SpeedSource(s,checked);}}
void MainWindow::on_cb_gaiin_4EnhanceSword_toggled(bool checked){if(!load){file_modified(true);ff7->setGaiin_4EnhanceSword(s,checked);}}
void MainWindow::on_cb_gaiin_5FireArmlet_toggled(bool checked){if(!load){file_modified(true);ff7->setGaiin_5FireArmlet(s,checked);}}
void MainWindow::on_cb_gaiin_5Elixir_toggled(bool checked){if(!load){file_modified(true);ff7->setGaiin_5Elixir(s,checked);}}
void MainWindow::on_cb_snmayorTurboEther_toggled(bool checked){if(!load){file_modified(true);ff7->setSnmayorTurboEther(s,checked);}}
void MainWindow::on_cb_sninn2XPotion_toggled(bool checked){if(!load){file_modified(true);ff7->setSninn2XPotion(s,checked);}}
void MainWindow::on_cb_snmin2Vaccine_toggled(bool checked){if(!load){file_modified(true);ff7->setSnmin2Vaccine(s,checked);}}
void MainWindow::on_cb_snmin2HeroDrink_toggled(bool checked){if(!load){file_modified(true);ff7->setSnmin2HeroDrink(s,checked);}}
void MainWindow::on_cb_ncoin3Catastrophe_toggled(bool checked){if(!load){file_modified(true);ff7->setNcoin3Catastrophe(s,checked);}}
void MainWindow::on_cb_ncoin1Ether_toggled(bool checked){if(!load){file_modified(true);ff7->setNcoin1Ether(s,checked);}}
void MainWindow::on_cb_trnad_4PoisonRing_toggled(bool checked){if(!load){file_modified(true);ff7->setTrnad_4PoisonRing(s,checked);}}
void MainWindow::on_cb_trnad_4MpTurbo_toggled(bool checked){if(!load){file_modified(true);ff7->setTrnad_4MpTurbo(s,checked);}}
void MainWindow::on_cb_trnad_3KaiserKnuckle_toggled(bool checked){if(!load){file_modified(true);ff7->setTrnad_3KaiserKnuckle(s,checked);}}
void MainWindow::on_cb_trnad_2NeoBahmut_toggled(bool checked){if(!load){file_modified(true);ff7->setTrnad_2NeoBahmut(s,checked);}}

void MainWindow::on_cb_materiacave_1_toggled(bool checked){if(!load){file_modified(true);ff7->setMateriaCave(s,FF7Save::CAVE_MIME,checked);}}
void MainWindow::on_cb_materiacave_2_toggled(bool checked){if(!load){file_modified(true);ff7->setMateriaCave(s,FF7Save::CAVE_HPMP,checked);}}
void MainWindow::on_cb_materiacave_3_toggled(bool checked){if(!load){file_modified(true);ff7->setMateriaCave(s,FF7Save::CAVE_QUADMAGIC,checked);}}
void MainWindow::on_cb_materiacave_4_toggled(bool checked){if(!load){file_modified(true);ff7->setMateriaCave(s,FF7Save::CAVE_KOTR,checked);}}

void MainWindow::on_cb_reg_yuffie_toggled(bool checked)
{if(!load){file_modified(true);
        if (checked){ff7->slot[s].reg_yuffie =0x6F;}
        else{ff7->slot[s].reg_yuffie =0x6E;}
        testdata_refresh();
}}

void MainWindow::on_cb_yuffieforest_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].yuffieforest |= (1<<0);}
    else{ff7->slot[s].yuffieforest &= ~(1<<0);}
}}

void MainWindow::on_cb_midgartrain_1_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].midgartrainflags |= (1<<0);}else{ff7->slot[s].midgartrainflags &= ~(1<<0);}}}
void MainWindow::on_cb_midgartrain_2_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].midgartrainflags |= (1<<1);}else{ff7->slot[s].midgartrainflags &= ~(1<<1);}}}
void MainWindow::on_cb_midgartrain_3_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].midgartrainflags |= (1<<2);}else{ff7->slot[s].midgartrainflags &= ~(1<<2);}}}
void MainWindow::on_cb_midgartrain_4_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].midgartrainflags |= (1<<3);}else{ff7->slot[s].midgartrainflags &= ~(1<<3);}}}
void MainWindow::on_cb_midgartrain_5_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].midgartrainflags |= (1<<4);}else{ff7->slot[s].midgartrainflags &= ~(1<<4);}}}
void MainWindow::on_cb_midgartrain_6_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].midgartrainflags |= (1<<5);}else{ff7->slot[s].midgartrainflags &= ~(1<<5);}}}
void MainWindow::on_cb_midgartrain_7_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].midgartrainflags |= (1<<6);}else{ff7->slot[s].midgartrainflags &= ~(1<<6);}}}
void MainWindow::on_cb_midgartrain_8_toggled(bool checked){if(!load){file_modified(true); if(checked){ff7->slot[s].midgartrainflags |= (1<<7);}else{ff7->slot[s].midgartrainflags &= ~(1<<7);}}}

void MainWindow::on_cb_tut_worldsave_stateChanged(int value)
{if(!load){file_modified(true);
    if (value == 0){ff7->slot[s].tut_save =0x00;}
    else if(value ==1){ff7->slot[s].tut_save =0x32;}
    else if(value ==2){ff7->slot[s].tut_save=0x3A;}
    testdata_refresh();
}}

void MainWindow::on_cb_Region_Slot_currentIndexChanged()
{if(!load){file_modified(true); if(!ff7->region(s).isEmpty()){
    QString new_regionString = ff7->region(s).mid(0,ff7->region(s).lastIndexOf("-")+1);
    new_regionString.append(ui->cb_Region_Slot->currentText().toLocal8Bit());
    ff7->setRegion(s,new_regionString);
    if(ff7->type()== "MC"|| ff7->type()=="PSP"|| ff7->type()=="VGS" || ff7->type() =="DEX"){guirefresh(0);}
}}}

void MainWindow::on_cb_tut_sub_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].tut_sub |= (1<<2);}
    else{ff7->slot[s].tut_sub &= ~(1<<2);}
    testdata_refresh();
}}
void MainWindow::on_cb_tut_sub_1_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].tut_sub |= (1<<0);}
    else{ff7->slot[s].tut_sub &= ~(1<<0);}
    testdata_refresh();
}}

void MainWindow::on_cb_tut_sub_2_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].tut_sub |= (1<<1);}
    else{ff7->slot[s].tut_sub &= ~(1<<1);}
    testdata_refresh();
}}

void MainWindow::on_cb_tut_sub_3_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].tut_sub |= (1<<2);}
    else{ff7->slot[s].tut_sub &= ~(1<<2);}
    testdata_refresh();
}}

void MainWindow::on_cb_tut_sub_4_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].tut_sub |= (1<<3);}
    else{ff7->slot[s].tut_sub &= ~(1<<3);}
    testdata_refresh();
}}

void MainWindow::on_cb_tut_sub_5_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].tut_sub |= (1<<4);}
    else{ff7->slot[s].tut_sub &= ~(1<<4);}
    testdata_refresh();
}}

void MainWindow::on_cb_tut_sub_6_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].tut_sub |= (1<<5);}
    else{ff7->slot[s].tut_sub &= ~(1<<5);}
    testdata_refresh();
}}

void MainWindow::on_cb_tut_sub_7_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].tut_sub |= (1<<6);}
    else{ff7->slot[s].tut_sub &= ~(1<<6);}
    testdata_refresh();
}}

void MainWindow::on_cb_tut_sub_8_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].tut_sub |= (1<<7);}
    else{ff7->slot[s].tut_sub &= ~(1<<7);}
    testdata_refresh();
}}

void MainWindow::on_cb_ruby_dead_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].ruby_emerald |= (1<<3);}
    else{ff7->slot[s].ruby_emerald &= ~(1<<3);}
}}

void MainWindow::on_cb_emerald_dead_toggled(bool checked)
{if(!load){file_modified(true);
    if(checked){ff7->slot[s].ruby_emerald |= (1<<4);}
    else{ff7->slot[s].ruby_emerald &= ~(1<<4);}
}}

void MainWindow::on_combo_highwind_buggy_currentIndexChanged(int index)
{if(!load){file_modified(true);
  switch(index)
  {
  case 1: ui->bh_id->setValue(0x06);ui->cb_visible_buggy->setChecked(Qt::Checked);break;//buggy
  case 2: ui->bh_id->setValue(0x03);ui->cb_visible_highwind->setChecked(Qt::Checked);break;//highwind
  default: break;
  }
}}
void MainWindow::on_cb_visible_buggy_toggled(bool checked)
{if(!load){file_modified(true);
        if(checked){ff7->slot[s].world_map_vehicles |= (1<<0);}
        else{ff7->slot[s].world_map_vehicles &= ~(1<<0);}
}}
void MainWindow::on_cb_visible_bronco_toggled(bool checked)
{if(!load){file_modified(true);
        if(checked){ff7->slot[s].world_map_vehicles |= (1<<2);}
        else{ff7->slot[s].world_map_vehicles &= ~(1<<2);}
}}
void MainWindow::on_cb_visible_highwind_toggled(bool checked)
{if(!load){file_modified(true);
        if(checked){ff7->slot[s].world_map_vehicles |= (1<<4);}
        else{ff7->slot[s].world_map_vehicles &= ~(1<<4);}
}}
void MainWindow::on_cb_visible_wild_chocobo_toggled(bool checked)
{if(!load){file_modified(true);
        if(checked){ff7->slot[s].world_map_chocobos |= (1<<0);}
        else{ff7->slot[s].world_map_chocobos &= ~(1<<0);}
}}
void MainWindow::on_cb_visible_yellow_chocobo_toggled(bool checked)
{if(!load){file_modified(true);
        if(checked){
            ff7->slot[s].world_map_chocobos |= (1<<2);
            ui->cb_visible_green_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_blue_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_black_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_gold_chocobo->setChecked(Qt::Unchecked);
        }
        else{ff7->slot[s].world_map_chocobos &= ~(1<<2);}
}}
void MainWindow::on_cb_visible_green_chocobo_toggled(bool checked)
{if(!load){file_modified(true);
        if(checked){
            ff7->slot[s].world_map_chocobos |= (1<<3);
            ui->cb_visible_yellow_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_blue_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_black_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_gold_chocobo->setChecked(Qt::Unchecked);
        }
        else{ff7->slot[s].world_map_chocobos &= ~(1<<3);}
}}
void MainWindow::on_cb_visible_blue_chocobo_toggled(bool checked)
{if(!load){file_modified(true);
        if(checked){
            ff7->slot[s].world_map_chocobos |= (1<<4);
            ui->cb_visible_yellow_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_green_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_black_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_gold_chocobo->setChecked(Qt::Unchecked);
        }
        else{ff7->slot[s].world_map_chocobos &= ~(1<<4);}
}}

void MainWindow::on_cb_visible_black_chocobo_toggled(bool checked)
{if(!load){file_modified(true);
        if(checked){
            ff7->slot[s].world_map_chocobos |= (1<<5);
            ui->cb_visible_yellow_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_green_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_blue_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_gold_chocobo->setChecked(Qt::Unchecked);
        }
        else{ff7->slot[s].world_map_chocobos &= ~(1<<5);}
}}

void MainWindow::on_cb_visible_gold_chocobo_toggled(bool checked)
{if(!load){file_modified(true);
        if(checked){
            ff7->slot[s].world_map_chocobos |= (1<<6);
            ui->cb_visible_yellow_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_green_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_blue_chocobo->setChecked(Qt::Unchecked);
            ui->cb_visible_black_chocobo->setChecked(Qt::Unchecked);
        }
        else{ff7->slot[s].world_map_chocobos &= ~(1<<6);}
}}
// Leader's world map stuff. 0
void MainWindow::on_leader_id_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].l_world = (ui->leader_x->value()  | value << 19 | ui->leader_angle->value() <<24);}}
void MainWindow::on_leader_angle_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].l_world = (ui->leader_x->value()  | ui->leader_id->value() << 19 | value <<24);}}
void MainWindow::on_leader_z_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].l_world2 = (ui->leader_y->value() | value << 18);}}
void MainWindow::on_leader_x_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].l_world = (value | ui->leader_id->value() << 19 | ui->leader_angle->value() << 24);
    if(ui->combo_map_controls->currentIndex()==0){load=true;ui->slide_world_x->setValue(value);load=false;}
}}

void MainWindow::on_leader_y_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].l_world2 = (value | ui->leader_z->value() << 18);
    if(ui->combo_map_controls->currentIndex()==0){load=true;ui->slide_world_y->setValue(value);load=false;}
}}

//Tiny bronco / chocobo world 1
void MainWindow::on_tc_id_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].tc_world = (ui->tc_x->value()  | value << 19 | ui->tc_angle->value() <<24);}}
void MainWindow::on_tc_angle_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].tc_world = (ui->tc_x->value()  | ui->tc_id->value() << 19 | value <<24);}}
void MainWindow::on_tc_z_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].tc_world2 = (ui->tc_y->value() | value << 18);}}
void MainWindow::on_tc_x_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].tc_world = (value | ui->tc_id->value() << 19 | ui->tc_angle->value() << 24);
    if(ui->combo_map_controls->currentIndex()==1){load=true;ui->slide_world_x->setValue(value);load=false;}
}}
void MainWindow::on_tc_y_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].tc_world2 = (value | ui->tc_z->value() << 18);
    if(ui->combo_map_controls->currentIndex()==1){load=true;ui->slide_world_y->setValue(value);load=false;}
}}

//buggy / highwind world 2
void MainWindow::on_bh_id_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].bh_world = (ui->bh_x->value()  | value << 19 | ui->bh_angle->value() <<24);}}
void MainWindow::on_bh_angle_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].bh_world = (ui->bh_x->value()  | ui->bh_id->value() << 19 | value <<24);}}
void MainWindow::on_bh_z_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].bh_world2 = (ui->bh_y->value() | value << 18);}}
void MainWindow::on_bh_x_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].bh_world = (value | ui->bh_id->value() << 19 | ui->bh_angle->value() << 24);
    if(ui->combo_map_controls->currentIndex()==2){load=true;ui->slide_world_x->setValue(value);load=false;}
}}
void MainWindow::on_bh_y_valueChanged(int value)
{if(!load){file_modified(true);
        ff7->slot[s].bh_world2 = (value | ui->bh_z->value() << 18);
        if(ui->combo_map_controls->currentIndex()==2){load=true;ui->slide_world_y->setValue(value);load=false;}
}}
// sub world 3
void MainWindow::on_sub_id_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].sub_world = (ui->sub_x->value()  | value << 19 | ui->sub_angle->value() <<24);}}
void MainWindow::on_sub_angle_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].sub_world = (ui->sub_x->value()  | ui->sub_id->value() << 19 | value <<24);}}
void MainWindow::on_sub_z_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].sub_world2 = (ui->sub_y->value() | value << 18);}}
void MainWindow::on_sub_x_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].sub_world = (value | ui->sub_id->value() << 19 | ui->sub_angle->value() << 24);
     if(ui->combo_map_controls->currentIndex()==3){load=true;ui->slide_world_x->setValue(value);load=false;}
}}
void MainWindow::on_sub_y_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].sub_world2 = (value | ui->sub_z->value() << 18);
    if(ui->combo_map_controls->currentIndex()==3){load=true;ui->slide_world_y->setValue(value);load=false;}
}}

//Ruby world stuff 4
void MainWindow::on_durw_id_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].durw_world = (ui->durw_x->value()  | value << 19 | ui->durw_angle->value() <<24);}}
void MainWindow::on_durw_angle_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].durw_world = (ui->durw_x->value()  | ui->durw_id->value() << 19 | value <<24);}}
void MainWindow::on_durw_z_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].durw_world2 = (ui->durw_y->value() | value << 18);}}
void MainWindow::on_durw_x_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].durw_world = (value | ui->durw_id->value() << 19 | ui->durw_angle->value() << 24);
     if(ui->combo_map_controls->currentIndex()==4){load=true;ui->slide_world_x->setValue(value);load=false;}
}}
void MainWindow::on_durw_y_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].durw_world2 = (value | ui->durw_z->value() << 18);
     if(ui->combo_map_controls->currentIndex()==4){load=true;ui->slide_world_y->setValue(value);load=false;}
}}
//ultimate weapon 5?
void MainWindow::on_uw_id_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].uw_world = (ui->uw_x->value()  | value << 19 | ui->uw_angle->value() <<24);}}
void MainWindow::on_uw_angle_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].uw_world = (ui->uw_x->value()  | ui->uw_id->value() << 19 | value <<24);}}
void MainWindow::on_uw_z_valueChanged(int value){if(!load){file_modified(true); ff7->slot[s].uw_world2 = (ui->uw_y->value() | value << 18);}}
void MainWindow::on_uw_x_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].uw_world = (value | ui->uw_id->value() << 19 | ui->uw_angle->value() << 24);
     if(ui->combo_map_controls->currentIndex()==6){load=true;ui->slide_world_x->setValue(value);load=false;}
}}
void MainWindow::on_uw_y_valueChanged(int value)
{if(!load){file_modified(true);
    ff7->slot[s].uw_world2 = (value | ui->uw_z->value() << 18);
    if(ui->combo_map_controls->currentIndex()==6){load=true;ui->slide_world_y->setValue(value);load=false;}
}}

void MainWindow::on_combo_map_controls_currentIndexChanged(int index)
{
    load=true;
    switch(index)
    {
        case 0: ui->slide_world_x->setValue(ff7->slot[s].l_world  & 0x7FFFF);            ui->slide_world_y->setValue(ff7->slot[s].l_world2 & 0x3FFFF);           break;
        case 1: ui->slide_world_x->setValue(ff7->slot[s].tc_world  & 0x7FFFF);         ui->slide_world_y->setValue(ff7->slot[s].tc_world2 & 0x3FFFF);       break;
        case 2: ui->slide_world_x->setValue(ff7->slot[s].bh_world  & 0x7FFFF);         ui->slide_world_y->setValue(ff7->slot[s].bh_world2 & 0x3FFFF);      break;
        case 3: ui->slide_world_x->setValue(ff7->slot[s].sub_world  & 0x7FFFF);       ui->slide_world_y->setValue(ff7->slot[s].sub_world2 & 0x3FFFF);    break;
        case 4: ui->slide_world_x->setValue(ff7->slot[s].uw_world  & 0x7FFFF);        ui->slide_world_y->setValue(ff7->slot[s].uw_world2 & 0x3FFFF);      break;
        case 5: ui->slide_world_x->setValue(ff7->slot[s].durw_world  & 0x7FFFF);    ui->slide_world_y->setValue(ff7->slot[s].durw_world2 & 0x3FFFF);  break;
    }
    load=false;
}

void MainWindow::on_slide_world_x_valueChanged(int value)
{if(!load){file_modified(true);
    switch(ui->combo_map_controls->currentIndex())
    {
        case 0: ui->leader_x->setValue(value);  break;
        case 1: ui->tc_x->setValue(value);      break;
        case 2: ui->bh_x->setValue(value);      break;
        case 3: ui->sub_x->setValue(value);     break;
        case 4: ui->uw_x->setValue(value);      break;
        case 5: ui->durw_x->setValue(value);    break;
    }
}}

void MainWindow::on_slide_world_y_valueChanged(int value)
{if(!load){file_modified(true);
    switch(ui->combo_map_controls->currentIndex())
    {
        case 0: ui->leader_y->setValue(value);  break;
        case 1: ui->tc_y->setValue(value);      break;
        case 2: ui->bh_y->setValue(value);      break;
        case 3: ui->sub_y->setValue(value);     break;
        case 4: ui->uw_y->setValue(value);      break;
        case 5: ui->durw_y->setValue(value);    break;
    }

}}

void MainWindow::on_world_map_view_customContextMenuRequested(QPoint pos)
{//Need to create a Paint System Here To put Dots where Chars Are Placed.
    QMenu menu(this);
    QAction *sel;
    menu.addAction(tr("Place Leader"));
    menu.addAction(tr("Place Tiny Bronco/Chocobo"));
    menu.addAction(tr("Place Buggy/Highwind"));
    menu.addAction(tr("Place Sub"));
    menu.addAction(tr("Place Wild Chocobo"));
    menu.addAction(tr("Place Diamond/Ultimate/Ruby Weapon"));
    /* Do Nothing. Don't know emerald weapon Coords
    menu.addAction(tr("Place Emerald Weapon?"));
    */
    sel = menu.exec(ui->world_map_view->mapToGlobal(pos));
    if(sel==0){return;}
    file_modified(true);
    if(sel->text()==tr("Place Leader"))
    {
        ui->leader_x->setValue(pos.x() *( 295000/ ui->world_map_view->width()));
        ui->leader_y->setValue(pos.y() *( 230000/ ui->world_map_view->height()));
    }
    else if(sel->text()==tr("Place Tiny Bronco/Chocobo"))
    {
        ui->tc_x->setValue(pos.x() *( 295000/ ui->world_map_view->width()));
        ui->tc_y->setValue(pos.y() *( 230000/ ui->world_map_view->height()));
    }
    else if(sel->text()==tr("Place Buggy/Highwind"))
    {
         ui->bh_x->setValue(pos.x() *( 295000/ ui->world_map_view->width()));
         ui->bh_y->setValue(pos.y() *( 230000/ ui->world_map_view->height()));
    }
    else if(sel->text()==tr("Place Sub"))
    {
         ui->sub_x->setValue(pos.x() *( 295000/ ui->world_map_view->width()));
         ui->sub_y->setValue(pos.y() *( 230000/ ui->world_map_view->height()));
    }
    else if(sel->text()==tr("Place Wild Chocobo"))
    {
         ui->uw_x->setValue(pos.x() *( 295000/ ui->world_map_view->width()));
         ui->uw_y->setValue(pos.y() *( 230000/ ui->world_map_view->height()));
    }
    else if(sel->text()==tr("Place Diamond/Ultimate/Ruby Weapon"))
    {
         ui->durw_x->setValue(pos.x() *( 295000/ ui->world_map_view->width()));
         ui->durw_y->setValue(pos.y() *( 230000/ ui->world_map_view->height()));
    }
    else{return;}
}//End Of Map Context Menu

void MainWindow::on_btn_item_add_each_item_clicked()
{
    if(!load){file_modified(true); }
    ui->btn_remove_all_items->click();
    for(int i=0;i<320;i++)
    {
      //Replaced by new item engine. (Vegeta_Ss4)
        if(Items.name(i)!=tr("DON'T USE"))
        {
            if(i<106)
            {
                ff7->setItem(s,i,i,127);
            }
            else// after the block of empty items shift up 23 spots.
            {
                ff7->setItem(s,(i-23),i,127);
            }
        }
        else{ff7->setItem(s,i,0x1FF,0x7F);}//exclude the test items
        if(i>296){ff7->setItem(s,i,0x1FF,0x7F);}//replace the shifted ones w/ empty slots
    }
    //guirefresh(0)
            itemlist->setItems(ff7->items(s));
}

void MainWindow::unknown_refresh(int z)//remember to add/remove case statments in all 3 switches when number of z vars changes.
{//for updating the unknown table(s)
    load=true;

    QString text;
    int rows=0;
    QTableWidgetItem *newItem;
    quint8 value=0;
    QByteArray temp,temp2;
    int s2;

    ui->tbl_unknown->reset();
    ui->tbl_unknown->setColumnWidth(0,40);
    ui->tbl_unknown->setColumnWidth(1,40);
    ui->tbl_unknown->setColumnWidth(2,40);
    ui->tbl_unknown->setColumnWidth(3,70);
    ui->tbl_unknown->setColumnWidth(4,20);

    ui->tbl_compare_unknown->reset();
    ui->tbl_compare_unknown->setColumnWidth(0,40);
    ui->tbl_compare_unknown->setColumnWidth(1,40);
    ui->tbl_compare_unknown->setColumnWidth(2,40);
    ui->tbl_compare_unknown->setColumnWidth(3,70);
    ui->tbl_compare_unknown->setColumnWidth(4,20);

    if(ui->combo_compare_slot->currentIndex()==0){ui->btn_all_z_diffs->setEnabled(0);}
    else {ui->btn_all_z_diffs->setEnabled(1);}

    if(z <= unknown_zmax){temp = ff7->unknown(s,z);}
    else if(z == unknown_zmax+1){temp = ff7->slotFF7Data(s);}

    rows=temp.size();

    ui->tbl_unknown->setRowCount(rows);
    if(ui->combo_compare_slot->currentIndex()!=0){ui->tbl_compare_unknown->setRowCount(rows);}
    for(int i=0;i<rows;i++)
    {
        if(ui->combo_z_var->currentText()=="SLOT")
        {
            QString hex_str = QString("%1").arg(i,4,16,QChar('0')).toUpper(); //format ex: 000C
            newItem = new QTableWidgetItem(hex_str,0);
            ui->tbl_unknown->setItem(i,0,newItem);
        }
        else
        {
            text.setNum(i);
            newItem = new QTableWidgetItem(text,0);
            ui->tbl_unknown->setItem(i,0,newItem);
        }

        value = temp.at(i);

        //Write Hex
        QString hex_str = QString("%1").arg(value,2,16,QChar('0')).toUpper(); //Format: 000C
        newItem = new QTableWidgetItem(hex_str,0);
        ui->tbl_unknown->setItem(i,1,newItem);
        //Write Dec
        newItem = new QTableWidgetItem(text.number(value,10),0);
        ui->tbl_unknown->setItem(i,2,newItem);
        //Write Bin
        QString binary_str = QString("%1").arg(value,8,2,QChar('0')); //New format ex: 00000111 | Vegeta_Ss4 Bin mod
        newItem = new QTableWidgetItem(binary_str,0);
        ui->tbl_unknown->setItem(i,3,newItem);
        //Write Char
        newItem = new QTableWidgetItem(QChar(value),0);
        ui->tbl_unknown->setItem(i,4,newItem);

        if(ui->combo_compare_slot->currentIndex()!=0)
        {//do the same for the compare slot if one has been selected.
            if(ui->combo_z_var->currentText()=="SLOT")
            {
                QString hex_str = QString("%1").arg(i,4,16,QChar('0')).toUpper();
                newItem = new QTableWidgetItem(hex_str,0);
                ui->tbl_compare_unknown->setItem(i,0,newItem);
            }
            else
            {
                newItem = new QTableWidgetItem(text,0);
                ui->tbl_compare_unknown->setItem(i,0,newItem);
            }

            s2 = ui->combo_compare_slot->currentIndex()-1;
            if(z <= unknown_zmax){temp2 = ff7->unknown(s2,z);}
            else if(z == unknown_zmax+1){temp2 = ff7->slotFF7Data(s2);}

            //rows=temp2.size();
            value = temp2.at(i);

            //Write Hex
            QString hex_str = QString("%1").arg(value,2,16,QChar('0')).toUpper(); //New format ex: 0C | Vegeta_Ss4 Hex mod
            newItem = new QTableWidgetItem(hex_str,0);
            ui->tbl_compare_unknown->setItem(i,1,newItem);
            //Write Dec
            newItem = new QTableWidgetItem(text.number(value,10),0);
            ui->tbl_compare_unknown->setItem(i,2,newItem);
            //Write Bin
            QString binary_str = QString("%1").arg(value,8,2,QChar('0')); //New format ex: 00000111 | Vegeta_Ss4 Bin mod
            newItem = new QTableWidgetItem(binary_str,0);
            ui->tbl_compare_unknown->setItem(i,3,newItem);
            //Write Char
            newItem = new QTableWidgetItem(QChar(value),0);
            ui->tbl_compare_unknown->setItem(i,4,newItem);

            if(ui->tbl_compare_unknown->item(i,1)->text()!=ui->tbl_unknown->item(i,1)->text())
            {
                for (int c=0;c<5;c++)
                {//color the diffs ;)
                    ui->tbl_compare_unknown->item(i,c)->setBackgroundColor(Qt::yellow);
                    ui->tbl_compare_unknown->item(i,c)->setTextColor(Qt::red);
                    ui->tbl_unknown->item(i,c)->setBackgroundColor(Qt::yellow);
                    ui->tbl_unknown->item(i,c)->setTextColor(Qt::red);
                }
            }
        }
    }
    for(int i=0;i<rows;i++)//set up the item flags
    {
        ui->tbl_unknown->item(i,0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->tbl_unknown->item(i,1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable| Qt::ItemIsEditable);
        ui->tbl_unknown->item(i,2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable| Qt::ItemIsEditable);
        ui->tbl_unknown->item(i,3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable| Qt::ItemIsEditable);
        ui->tbl_unknown->item(i,4)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        if(ui->combo_compare_slot->currentIndex()!=0)
        {
            ui->tbl_compare_unknown->item(i,0)->setFlags(Qt::ItemIsEnabled);
            ui->tbl_compare_unknown->item(i,1)->setFlags(Qt::ItemIsEnabled);
            ui->tbl_compare_unknown->item(i,2)->setFlags(Qt::ItemIsEnabled);
            ui->tbl_compare_unknown->item(i,3)->setFlags(Qt::ItemIsEnabled);
            ui->tbl_compare_unknown->item(i,4)->setFlags(Qt::ItemIsEnabled);
        }
    }
    load=false;
}
void MainWindow::on_combo_z_var_currentIndexChanged(int z){unknown_refresh(z);}
void MainWindow::on_combo_compare_slot_currentIndexChanged(void)
{
    if(ui->combo_compare_slot->currentIndex()==0)
    {
        ui->tbl_compare_unknown->clearContents();
        ui->tbl_compare_unknown->setRowCount(0);
        ui->tbl_diff->clearContents();
        ui->tbl_diff->setRowCount(0);
        ui->btn_all_z_diffs->setEnabled(0);
    }
    else{unknown_refresh(ui->combo_z_var->currentIndex());}
    ui->tbl_diff->setVisible(0);
}
void MainWindow::on_tbl_unknown_itemChanged(QTableWidgetItem* item)
{if(!load){file_modified(true);
    QByteArray temp;

    int z = ui->combo_z_var->currentIndex();
    if(z <= unknown_zmax){temp = ff7->unknown(s,z);}
    else if(z == unknown_zmax+1){temp = ff7->slotFF7Data(s);}

    switch(item->column())
    {
    case 1: temp[item->row()] = item->text().toInt(0,16);  break;
    case 2: temp[item->row()] = item->text().toInt();      break;
    case 3: temp[item->row()] = item->text().toInt(0,2);   break;
    };

    if(z <= unknown_zmax){ff7->setUnknown(s,z,temp);}
    else if(z == unknown_zmax+1){ff7->setSlotFF7Data(s,temp);}

    unknown_refresh(z);
}}
void MainWindow::on_btn_all_z_diffs_clicked()
{
    ui->tbl_diff->reset();

    int num_diff=0;
    qint16 diff =0;
    QString text;
    QTableWidgetItem *newItem;
    int z_index= ui->combo_z_var->currentIndex();
    if(z_index==ui->combo_z_var->count()-1)
    {//if last item in list (SLOT mode)
        for(int i=0;i<ui->tbl_unknown->rowCount();i++)
        {
            if(ui->tbl_compare_unknown->item(i,1)->text()!=ui->tbl_unknown->item(i,1)->text())
            {
                num_diff++;
                ui->tbl_diff->setRowCount(num_diff);
                text.clear();
                //Offset
                QString hex_str = QString("%1").arg(i,4,16,QChar('0')).toUpper(); //Format: 0000C
                newItem = new QTableWidgetItem(hex_str,0);
                ui->tbl_diff->setItem(num_diff-1,0,newItem);
                //Decimal
                diff= ui->tbl_unknown->item(i,2)->text().toInt() - ui->tbl_compare_unknown->item(i,2)->text().toInt() ;
                newItem = new QTableWidgetItem(text.number(diff,10),0);
                ui->tbl_diff->setItem(num_diff-1,1,newItem);

                //Write Bin
                QString binary_str = QString("%1").arg(qAbs(diff),8,2,QChar('0')); //New format ex: 00000111 | Vegeta_Ss4 Bin mod
                newItem = new QTableWidgetItem(binary_str,0);
                ui->tbl_diff->setItem(num_diff-1,2,newItem);

                //set properites for the tableitems
                ui->tbl_diff->setVisible(1);
                ui->tbl_diff->item(num_diff-1,0)->setFlags(Qt::ItemIsEnabled);
                ui->tbl_diff->item(num_diff-1,1)->setFlags(Qt::ItemIsEnabled);
                ui->tbl_diff->item(num_diff-1,2)->setFlags(Qt::ItemIsEnabled);
                ui->tbl_diff->setRowHeight(num_diff-1,20);
            }
        }
    }
    else
    {
        for(int z=0;z<ui->combo_z_var->count()-1;z++)
        {
            ui->combo_z_var->setCurrentIndex(z);
            for(int i=0;i<ui->tbl_unknown->rowCount();i++)
            {
                if(ui->tbl_compare_unknown->item(i,1)->text()!=ui->tbl_unknown->item(i,1)->text())
                {
                    num_diff++;
                    ui->tbl_diff->setRowCount(num_diff);
                    text.clear();
                    text.append("z_");  text.append(QString::number(z));
                    text.append(":");   text.append(QString::number(i));
                    newItem = new QTableWidgetItem(text,0);
                    ui->tbl_diff->setItem(num_diff-1,0,newItem);
                    diff= ui->tbl_unknown->item(i,2)->text().toInt() - ui->tbl_compare_unknown->item(i,2)->text().toInt() ;
                    newItem = new QTableWidgetItem(text.number(diff,10),0);
                    ui->tbl_diff->setItem(num_diff-1,1,newItem);
                    //Write Bin
                    QString binary_str = QString("%1").arg(qAbs(diff),8,2,QChar('0')); //New format ex: 00000111 | Vegeta_Ss4 Bin mod
                    newItem = new QTableWidgetItem(binary_str,0);
                    ui->tbl_diff->setItem(num_diff-1,2,newItem);

                    //set properites for the tableitems
                    ui->tbl_diff->setVisible(1);
                    ui->tbl_diff->item(num_diff-1,0)->setFlags(Qt::ItemIsEnabled);
                    ui->tbl_diff->item(num_diff-1,1)->setFlags(Qt::ItemIsEnabled);
                    ui->tbl_diff->item(num_diff-1,2)->setFlags(Qt::ItemIsEnabled);
                    ui->tbl_diff->setRowHeight(num_diff-1,20);
                }
            }
        }
    }
    ui->tbl_diff->setColumnWidth(0,70);
    ui->tbl_diff->setColumnWidth(1,40);
    ui->tbl_diff->setColumnWidth(2,70);
    ui->tbl_diff->setVisible(1);
    if(num_diff<16){ui->tbl_diff->setFixedHeight((num_diff*21)+20);ui->tbl_diff->setFixedWidth(185);}
    else{ui->tbl_diff->setFixedHeight((15*21)+23);ui->tbl_diff->setFixedWidth(200);}
    ui->combo_z_var->setCurrentIndex(z_index);
    if(num_diff ==0){ui->tbl_diff->clearContents();ui->tbl_diff->setRowCount(0);ui->tbl_diff->setVisible(0);}
}


void MainWindow::on_combo_s7_slums_currentIndexChanged(int index)
{if(!load){file_modified(true);
        QByteArray temp;
        switch(index)
        {
        default: break; //do nothing
        case 1: //initial slums setting
            temp.setRawData("\x00\x00\x00\x00\x00\x00",6);
            ff7->setUnknown(s,26,temp);
            break;

        case 2://after first scene. needs game global progress set to 105
            temp.setRawData("\xBF\x03\x05\x17\x5D\xEF",6);
            ff7->setUnknown(s,26,temp);
            break;

        case 3://plate falling
            temp.setRawData("\xBF\x13\x05\x17\x5D\xEF",6);
            ff7->setUnknown(s,26,temp);
            break;
        }
}}
void MainWindow::char_materia_changed(materia mat){ ff7->setCharMateria(s,curchar,mslotsel,mat);}
void MainWindow::char_accessory_changed(quint8 accessory){ff7->setCharAccessory(s,curchar,accessory);}
void MainWindow::char_armor_changed(quint8 armor){ff7->setCharArmor(s,curchar,armor);}
void MainWindow::char_baseHp_changed(quint16 hp){ff7->setCharBaseHp(s,curchar,hp);}
void MainWindow::char_baseMp_changed(quint16 mp){ff7->setCharBaseMp(s,curchar,mp);}
void MainWindow::char_curHp_changed(quint16 hp)
{
    ff7->setCharCurrentHp(s,curchar,hp);
    if(curchar==ff7->party(s,0)){ff7->setDescCurHP(s,hp);}
}
void MainWindow::char_curMp_changed(quint16 mp)
{
    ff7->setCharCurrentMp(s,curchar,mp);
    if(curchar==ff7->party(s,0)){ff7->setDescCurMP(s,mp);}
}
void MainWindow::char_id_changed(qint8 id)
{
    ff7->setCharID(s,curchar,id);
    set_char_buttons();
}
void MainWindow::char_level_changed(qint8 level)
{
    ff7->setCharLevel(s,curchar,level);
    if(curchar==ff7->party(s,0)){ff7->setDescLevel(s,level);}
}
void MainWindow::char_str_changed(quint8 str){ff7->setCharStr(s,curchar,str);}\
void MainWindow::char_vit_changed(quint8 vit){ff7->setCharVit(s,curchar,vit);}
void MainWindow::char_mag_changed(quint8 mag){ff7->setCharMag(s,curchar,mag);}
void MainWindow::char_spi_changed(quint8 spi){ff7->setCharSpi(s,curchar,spi);}
void MainWindow::char_dex_changed(quint8 dex){ff7->setCharDex(s,curchar,dex);}
void MainWindow::char_lck_changed(quint8 lck){ff7->setCharLck(s,curchar,lck);}
void MainWindow::char_strBonus_changed(quint8 value){ff7->setCharStrBonus(s,curchar,value);}
void MainWindow::char_vitBonus_changed(quint8 value){ff7->setCharVitBonus(s,curchar,value);}
void MainWindow::char_magBonus_changed(quint8 value){ff7->setCharMagBonus(s,curchar,value);}
void MainWindow::char_spiBonus_changed(quint8 value){ff7->setCharSpiBonus(s,curchar,value);}
void MainWindow::char_dexBonus_changed(quint8 value){ff7->setCharDexBonus(s,curchar,value);}
void MainWindow::char_lckBonus_changed(quint8 value){ff7->setCharLckBonus(s,curchar,value);}
void MainWindow::char_limitLevel_changed(qint8 value){ff7->setCharLimitLevel(s,curchar,value);}
void MainWindow::char_limitBar_changed(quint8 value){ff7->setCharLimitBar(s,curchar,value);}
void MainWindow::char_weapon_changed(quint8 value){ff7->setCharWeapon(s,curchar,value);}
void MainWindow::char_kills_changed(quint16 value){ff7->setCharKills(s,curchar,value);}
void MainWindow::char_row_changed(quint8 value){ff7->setCharFlag(s,curchar,1,value);}
void MainWindow::char_levelProgress_changed(quint8 value){ff7->setCharFlag(s,curchar,2,value);}
void MainWindow::char_sadnessfury_changed(quint8 value){ff7->setCharFlag(s,curchar,0,value);}
void MainWindow::char_limits_changed(quint16 value){ff7->setCharLimits(s,curchar,value);}
void MainWindow::char_timesused1_changed(quint16 value){ff7->setCharTimeLimitUsed(s,curchar,1,value);}
void MainWindow::char_timeused2_changed(quint16 value){ff7->setCharTimeLimitUsed(s,curchar,2,value);}
void MainWindow::char_timeused3_changed(quint16 value){ff7->setCharTimeLimitUsed(s,curchar,3,value);}
void MainWindow::char_exp_changed(quint32 value){ff7->setCharCurrentExp(s,curchar,value);}
void MainWindow::char_expNext_changed(quint32 value){ff7->setCharNextExp(s,curchar,value);}
void MainWindow::char_mslot_changed(int slot){mslotsel=slot;}

void MainWindow::char_name_changed(QString name)
{
    ff7->setCharName(s,curchar,name);
    if(curchar==ff7->party(s,0)){ff7->setDescName(s,name);}
}

void MainWindow::char_maxHp_changed(quint16 value)
{
    ff7->setCharMaxHp(s,curchar,value);
    if(curchar==ff7->party(s,0)){ff7->setDescMaxHP(s,value);}
}
void MainWindow::char_maxMp_changed(quint16 value)
{
    ff7->setCharMaxMp(s,curchar,value);
    if(curchar==ff7->party(s,0)){ff7->setDescMaxMP(s,value);}
}

void MainWindow::on_btn_maxChar_clicked()
{
    if(ff7->charID(s,curchar)==FF7Char::YoungCloud || ff7->charID(s,curchar) == FF7Char::Sephiroth  ||  _init){return;}//no char selected, sephiroth and young cloud.
    int result = QMessageBox::question(this,tr("Black Chococbo"),tr("Replace %1's Materia and Equipment").arg(ff7->charName(s,curchar)),QMessageBox::Yes,QMessageBox::No);
    switch(result)
    {
        case QMessageBox::Yes:char_editor->MaxStats();char_editor->MaxEquip();break;
        case QMessageBox::No: char_editor->MaxStats();break;
    }
    switch(curchar)
    {
        case 0: on_btn_cloud_clicked();break;
        case 1: on_btn_barret_clicked();break;
        case 2: on_btn_tifa_clicked();break;
        case 3: on_btn_aeris_clicked();break;
        case 4: on_btn_red_clicked();break;
        case 5: on_btn_yuffie_clicked();break;
        case 6: on_btn_cait_clicked();break;
        case 7: on_btn_vincent_clicked();break;
        case 8: on_btn_cid_clicked();break;
    }
}
void MainWindow::Items_Changed(QList<quint16> items){ff7->setItems(s,items);}
void MainWindow::on_sbSnowBegScore_valueChanged(int value){ff7->setSnowboardScore(s,0,value);}
void MainWindow::on_sbSnowExpScore_valueChanged(int value){ff7->setSnowboardScore(s,1,value);}
void MainWindow::on_sbSnowCrazyScore_valueChanged(int value){ff7->setSnowboardScore(s,2,value);}

void MainWindow::on_sbSnowBegMin_valueChanged(int value)
{if(!load){
    quint32 time;
    time = ((value*60)*1000) + (ui->sbSnowBegSec->value()*1000) + ui->sbSnowBegMsec->value();
    ff7->setSnowboardTime(s,0,time);
}}

void MainWindow::on_sbSnowBegSec_valueChanged(int value)
{if(!load){
        quint32 time;
        time = ((ui->sbSnowBegMin->value()*60)*1000) + (value*1000) + ui->sbSnowBegMsec->value();
        ff7->setSnowboardTime(s,0,time);
}}

void MainWindow::on_sbSnowBegMsec_valueChanged(int value)
{if(!load){
        quint32 time;
        time = ((ui->sbSnowBegMin->value()*60)*1000) + (ui->sbSnowBegSec->value()*1000) + value;
        ff7->setSnowboardTime(s,0,time);
}}

void MainWindow::on_sbSnowExpMin_valueChanged(int value)
{if(!load){
    quint32 time;
    time = ((value*60)*1000) + (ui->sbSnowExpSec->value()*1000) + ui->sbSnowExpMsec->value();
    ff7->setSnowboardTime(s,1,time);
}}

void MainWindow::on_sbSnowExpSec_valueChanged(int value)
{if(!load){
        quint32 time;
        time = ((ui->sbSnowExpMin->value()*60)*1000) + (value*1000) + ui->sbSnowExpMsec->value();
        ff7->setSnowboardTime(s,1,time);
}}

void MainWindow::on_sbSnowExpMsec_valueChanged(int value)
{if(!load){
        quint32 time;
        time = ((ui->sbSnowExpMin->value()*60)*1000) + (ui->sbSnowExpSec->value()*1000) + value;
        ff7->setSnowboardTime(s,1,time);
}}

void MainWindow::on_sbSnowCrazyMin_valueChanged(int value)
{if(!load){
    quint32 time;
    time = ((value*60)*1000) + (ui->sbSnowCrazySec->value()*1000) + ui->sbSnowCrazyMsec->value();
    ff7->setSnowboardTime(s,2,time);
}}

void MainWindow::on_sbSnowCrazySec_valueChanged(int value)
{if(!load){
        quint32 time;
        time = ((ui->sbSnowCrazyMin->value()*60)*1000) + (value*1000) + ui->sbSnowCrazyMsec->value();
        ff7->setSnowboardTime(s,2,time);
}}

void MainWindow::on_sbSnowCrazyMsec_valueChanged(int value)
{if(!load){
        quint32 time;
        time = ((ui->sbSnowCrazyMin->value()*60)*1000) + (ui->sbSnowCrazySec->value()*1000) + value;
        ff7->setSnowboardTime(s,2,time);
}}
void MainWindow::on_sb_BikeHighScore_valueChanged(int arg1){if(!load){ff7->setBikeHighScore(s,arg1);}}
void MainWindow::on_sb_BattlePoints_valueChanged(int arg1){if(!load){ff7->setBattlePoints(s,arg1);}}

void MainWindow::hexEditorRefresh()
{
    switch(ui->combo_hexEditor->currentIndex())
    {
        case 0:hexEditor->setData(ff7->slotPsxRawData(s)); break;
        case 1:hexEditor->setData(ff7->slotFF7Data(s)); break;
    }
}
void MainWindow::on_combo_hexEditor_currentIndexChanged(){hexEditorRefresh();}

void MainWindow::hexEditorChanged(void)
{
    switch(ui->combo_hexEditor->currentIndex())
    {
        case 0: ff7->setSlotPsxRawData(s,hexEditor->data()); break;
        case 1: ff7->setSlotFF7Data(s,hexEditor->data());   break;
    }
}

void MainWindow::phsList_box_allowed_toggled(int row, bool checked){if(!load){file_modified(true);ff7->setPhsAllowed(s,row,!checked);}}
void MainWindow::phsList_box_visible_toggled(int row, bool checked){if(!load){file_modified(true);ff7->setPhsVisible(s,row,checked);}}
void MainWindow::menuList_box_locked_toggled(int row, bool checked){if(!load){file_modified(true);ff7->setMenuLocked(s,row,checked);}}
void MainWindow::menuList_box_visible_toggled(int row, bool checked){if(!load){file_modified(true);ff7->setMenuVisible(s,row,checked);}}


void MainWindow::on_tabWidget_currentChanged(){guirefresh(0);}
