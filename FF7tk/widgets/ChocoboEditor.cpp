/****************************************************************************/
//    copyright 2012  Chris Rizzitello <sithlord48@gmail.com>               //
//                                                                          //
//    This file is part of FF7tk                                            //
//                                                                          //
//    FF7tk is free software: you can redistribute it and/or modify         //
//    it under the terms of the GNU General Public License as published by  //
//    the Free Software Foundation, either version 3 of the License, or     //
//    (at your option) any later version.                                   //
//                                                                          //
//    FF7tk is distributed in the hope that it will be useful,              //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of        //
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//    GNU General Public License for more details.                          //
/****************************************************************************/
#include "ChocoboEditor.h"
// Pull in Icons, Be Sure Paths Are Correct
#include"../static_data/icons/Chocobo_Icons/black_choco.xpm"
#include"../static_data/icons/Chocobo_Icons/blue_choco.xpm"
#include"../static_data/icons/Chocobo_Icons/green_choco.xpm"
#include"../static_data/icons/Chocobo_Icons/yellow_choco.xpm"
#include"../static_data/icons/Chocobo_Icons/gold_choco.xpm"

ChocoboEditor::ChocoboEditor(QWidget *parent) :
    QWidget(parent)
{
    init_display();
    init_connections();
}
//Simple Data Return Functions.
quint16 ChocoboEditor::sprint(void){return choco_data.sprintspd;}
quint16 ChocoboEditor::mSprint(void){return choco_data.maxsprintspd;}
quint16 ChocoboEditor::speed(void){return choco_data.speed;}
quint16 ChocoboEditor::mSpeed(void){return choco_data.maxspeed;}
quint16 ChocoboEditor::stamina(void){return choco_stamina;}
bool ChocoboEditor::cantMate(void){return choco_cant_mate;}
QString ChocoboEditor::name(void){return choco_name;}
quint8 ChocoboEditor::accel(void){return choco_data.accel;}
quint8 ChocoboEditor::coop(void){return choco_data.coop;}
quint8 ChocoboEditor::intelligence(void){return choco_data.intelligence;}
quint8 ChocoboEditor::personality(void){return choco_data.personality;}
quint8 ChocoboEditor::pCount(void){return choco_data.pcount;}
quint8 ChocoboEditor::wins(void){return choco_data.raceswon;}
quint8 ChocoboEditor::sex(void){return choco_data.sex;}
quint8 ChocoboEditor::type(void){return choco_data.type;}
//Data Set Functions.

void ChocoboEditor::setSprint(int sprint_speed)
{
    if(sprint_speed <0){sprint_speed =0;}
    else if (sprint_speed > 9999){sprint_speed = 9999;}
    choco_data.sprintspd = sprint_speed;
    emit sprint_changed(choco_data.sprintspd);
    //QMessageBox::information(this,"Sprint Changed",QString("Sprint: %1").arg(choco_data.sprintspd));
}
void ChocoboEditor::setMsprint(int max_sprint_speed)
{
    if(max_sprint_speed <0){max_sprint_speed =0;}
    else if (max_sprint_speed > 9999){max_sprint_speed = 9999;}
    choco_data.maxsprintspd = max_sprint_speed;
    emit mSprint_changed(choco_data.maxsprintspd);
    //QMessageBox::information(this,"Max Sprint Changed",QString("Max Sprint: %1").arg(choco_data.maxsprintspd));
}
void ChocoboEditor::setSpeed(int speed)
{
    if(speed <0){speed =0;}
    else if (speed > 9999){speed = 9999;}
    choco_data.speed = speed;
    emit speed_changed(choco_data.speed);
    //QMessageBox::information(this,"Speed Changed",QString("Speed: %1").arg(choco_data.speed));
}
void ChocoboEditor::setMspeed(int max_speed)
{
    if(max_speed <0){max_speed =0;}
    else if (max_speed > 9999){max_speed = 9999;}
    choco_data.maxspeed = max_speed;
    emit mSprint_changed(choco_data.maxspeed);
    //QMessageBox::information(this,"Max Speed Changed",QString("Max Speed: %1").arg(choco_data.maxspeed));
}
void ChocoboEditor::setStamina(int stamina)
{
    if(stamina <0){stamina =0;}
    else if (stamina > 9999){stamina = 9999;}
    choco_stamina = stamina;
    emit stamina_changed(choco_stamina);
    //QMessageBox::information(this,"Stamina Changed",QString("Stamina: %1").arg(choco_stamina));
}
void ChocoboEditor::setAccel(int accel)
{
    if(accel <0){accel =0;}
    else if (accel > 255){accel =255;}
    choco_data.accel = accel;
    emit accel_changed(choco_data.accel);
    //QMessageBox::information(this,"Accel Changed",QString("Accel: %1").arg(choco_data.accel));
}
void ChocoboEditor::setCoop(int coop)
{
    if(coop < 0){coop =0;}
    else if (coop >255){coop =255;}
    choco_data.coop = coop;
    emit coop_changed(choco_data.coop);
    //QMessageBox::information(this,"Coop Changed",QString("Coop: %1").arg(choco_data.coop));
}
void ChocoboEditor::setIntelligence(int intel)
{
    if(intel <0){intel =0;}
    else if(intel >255){intel = 255;}
    choco_data.intelligence = intel;
    emit intelligence_changed(choco_data.intelligence);
    //QMessageBox::information(this,"Intel Changed",QString("Intel: %1").arg(choco_data.intelligence));
}
void ChocoboEditor::setPersonality (int personality)
{//need to be researched further.
    if(personality <0){personality =0;}
    else if (personality >255){personality = 255;}
    choco_data.personality = personality;
    emit personality_changed(choco_data.personality);
    //QMessageBox::information(this,"Personality Changed",QString("Personality: %1").arg(choco_data.personality));
}
void ChocoboEditor::setPcount(int pCount)
{
    if(pCount <0){pCount =0;}
    else if (pCount >255){pCount =255;}
    choco_data.pcount=pCount;
    emit pCount_changed(choco_data.pcount);
    //QMessageBox::information(this,"pCount Changed",QString("pCount: %1").arg(choco_data.pcount));
}
void ChocoboEditor::setWins(int wins)
{
    if(wins <0){wins = 0;}
    else if (wins>255){wins = 255;}
    choco_data.raceswon = wins;
    emit wins_changed(choco_data.raceswon);
    //QMessageBox::information(this,"Wins Changed",QString("Wins: %1").arg(choco_data.raceswon));
}
void ChocoboEditor::setSex(int sex)
{
    if(sex<0){sex=0;}
    else if(sex > 1){sex = 1;}
    choco_data.sex = sex;
    emit sex_changed(choco_data.sex);
    //QMessageBox::information(this,"Sex Changed",QString("Sex: %1").arg(choco_data.sex));
}
void ChocoboEditor::setType(int type)
{
    if(type<0){type=0;}
    else if(type > 4){type = 4;}
    choco_data.type = type;
    emit type_changed(choco_data.type);
    //QMessageBox::information(this,"Type Changed",QString("Type: %1").arg(choco_data.type));
}
void ChocoboEditor::setName(QString new_name)
{
    if(new_name.length()>6){new_name.truncate(6);}
    choco_name = new_name;
    emit name_changed(new_name);
    //QMessageBox::information(this,"Name Changed",QString("Name: %1").arg(choco_name));
}
void ChocoboEditor::setCantMate(bool cantMate)
{
    choco_cant_mate=cantMate;
    emit cantMate_changed(choco_cant_mate);
    //QMessageBox::information(this,"Cant Mate' Changed",QString("Cant Mate: %1").arg(choco_cant_mate));
}

void ChocoboEditor::init_display(void)
{
    //create Gui Widgets.
    //QSpacerItem *spacer = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum);
    combo_sex = new QComboBox;
    combo_type = new QComboBox;
    line_name = new QLineEdit;
    sb_speed = new QSpinBox;
    sb_mSpeed = new QSpinBox;
    sb_sprint = new QSpinBox;
    sb_mSprint = new QSpinBox;
    sb_stamina = new QSpinBox;
    sb_accel = new QSpinBox;
    sb_wins = new QSpinBox;
    sb_coop = new QSpinBox;
    sb_pCount = new QSpinBox;
    sb_intel = new QSpinBox;
    sb_personality = new QSpinBox;
    cb_cantMate = new QCheckBox(tr("Unable To Mate"));

    lbl_stamina  = new QLabel(tr("Stamina"));
    lbl_sex  = new QLabel(tr("Sex"));
    lbl_type = new QLabel(tr("Type"));
    lbl_name = new QLabel(tr("Name"));
    lbl_speed = new QLabel(tr("Run Speed"));
    lbl_div_speed = new QLabel("/");
    lbl_div_speed->setAlignment(Qt::AlignHCenter);
    lbl_sprint = new QLabel(tr("Sprint Speed"));
    lbl_div_sprint = new QLabel("/");
    lbl_div_sprint->setAlignment(Qt::AlignHCenter);
    lbl_accel = new QLabel(tr("Accel"));
    lbl_wins = new QLabel(tr("Wins"));
    lbl_coop = new QLabel(tr("Coop"));
    lbl_pCount = new QLabel(tr("pCount"));
    lbl_intel = new QLabel(tr("Intel"));
    lbl_personality = new QLabel(tr("Personality"));
    //Set Widgets up
    line_name->setMaxLength(6);
    sb_speed->setMaximum(9999);
    sb_speed->setAlignment(Qt::AlignHCenter);
    sb_mSpeed->setMaximum(9999);
    sb_mSpeed->setAlignment(Qt::AlignHCenter);
    sb_sprint->setMaximum(9999);
    sb_sprint->setAlignment(Qt::AlignHCenter);
    sb_mSprint->setMaximum(9999);
    sb_mSprint->setAlignment(Qt::AlignHCenter);
    sb_stamina->setMaximum(9999);
    sb_stamina->setAlignment(Qt::AlignHCenter);
    sb_coop->setMaximum(255);
    sb_coop->setAlignment(Qt::AlignHCenter);
    sb_accel->setMaximum(255);
    sb_accel->setAlignment(Qt::AlignHCenter);
    sb_wins->setMaximum(255);
    sb_wins->setAlignment(Qt::AlignHCenter);
    sb_pCount->setMaximum(255);
    sb_pCount->setAlignment(Qt::AlignHCenter);
    sb_intel->setMaximum(255);
    sb_intel->setAlignment(Qt::AlignHCenter);
    sb_personality->setMaximum(255);
    sb_personality->setAlignment(Qt::AlignHCenter);

    //Fill Combos.
    combo_sex->addItem(tr("Male"));
    combo_sex->addItem(tr("Female"));
    combo_type->addItem(QIcon(QPixmap(yellow_choco_xpm)),tr("Yellow"));
    combo_type->addItem(QIcon(QPixmap(green_choco_xpm)),tr("Green"));
    combo_type->addItem(QIcon(QPixmap(blue_choco_xpm)),tr("Blue"));
    combo_type->addItem(QIcon(QPixmap(black_choco_xpm)),tr("Black"));
    combo_type->addItem(QIcon(QPixmap(gold_choco_xpm)),tr("Gold"));
    //Make Layouts
    QHBoxLayout *name_layout = new QHBoxLayout;
    name_layout->setContentsMargins(0,0,0,0);
    name_layout->addWidget(lbl_name);
    name_layout->addWidget(line_name);

    QHBoxLayout *type_sex_layout = new QHBoxLayout;
    type_sex_layout->setContentsMargins(0,0,0,0);

    type_sex_layout->addWidget(lbl_type);
    type_sex_layout->addWidget(combo_type);
    type_sex_layout->addWidget(lbl_sex);
    type_sex_layout->addWidget(combo_sex);

    QHBoxLayout *speed_layout = new QHBoxLayout;
    speed_layout->setContentsMargins(0,0,0,0);
    //speed_layout->setAlignment(Qt::AlignHCenter);
    speed_layout->addWidget(lbl_speed);
    speed_layout->addWidget(sb_speed);
    speed_layout->addWidget(lbl_div_speed);
    speed_layout->addWidget(sb_mSpeed);

    QHBoxLayout *sprint_layout = new QHBoxLayout;
    sprint_layout->setContentsMargins(0,0,0,0);
    //sprint_layout->setAlignment(Qt::AlignHCenter);
    sprint_layout->addWidget(lbl_sprint);
    sprint_layout->addWidget(sb_sprint);
    sprint_layout->addWidget(lbl_div_sprint);
    sprint_layout->addWidget(sb_mSprint);

    QHBoxLayout *cantMate_layout = new QHBoxLayout;
    cantMate_layout->setContentsMargins(0,0,0,0);
    //cantMate_layout->setAlignment(Qt::AlignHCenter);
    cantMate_layout->addWidget(cb_cantMate);

    QHBoxLayout *accel_layout = new QHBoxLayout;
    accel_layout->setContentsMargins(0,0,0,0);
    accel_layout->addWidget(lbl_accel);
    accel_layout->addWidget(sb_accel);

    QHBoxLayout *wins_layout = new QHBoxLayout;
    wins_layout->setContentsMargins(0,0,0,0);
    wins_layout->addWidget(lbl_wins);
    wins_layout->addWidget(sb_wins);

    QHBoxLayout *accel_wins_layout = new QHBoxLayout;
    accel_wins_layout->setContentsMargins(0,0,0,0);
    //accel_wins_layout->setAlignment(Qt::AlignHCenter);
    accel_wins_layout->addLayout(accel_layout);
    accel_wins_layout->addLayout(wins_layout);

    QHBoxLayout *stamina_layout = new QHBoxLayout;
    stamina_layout->setContentsMargins(0,0,0,0);
    stamina_layout->addWidget(lbl_stamina);
    stamina_layout->addWidget(sb_stamina);

    QHBoxLayout *pCount_layout = new QHBoxLayout;
    pCount_layout->setContentsMargins(0,0,0,0);
    pCount_layout->addWidget(lbl_pCount);
    pCount_layout->addWidget(sb_pCount);

    QHBoxLayout *stamina_pCount_layout = new QHBoxLayout;
    stamina_pCount_layout->setContentsMargins(0,0,0,0);
    stamina_pCount_layout->addLayout(stamina_layout);
    stamina_pCount_layout->addLayout(pCount_layout);

    QHBoxLayout *coop_layout = new QHBoxLayout;
    coop_layout->setContentsMargins(0,0,0,0);
    coop_layout->addWidget(lbl_coop);
    coop_layout->addWidget(sb_coop);

    QHBoxLayout *intel_layout = new QHBoxLayout;
    intel_layout->setContentsMargins(0,0,0,0);
    intel_layout->addWidget(lbl_intel);
    intel_layout->addWidget(sb_intel);

    QHBoxLayout *coop_intel_layout = new QHBoxLayout;
    coop_intel_layout->setContentsMargins(0,0,0,0);
    coop_intel_layout->addLayout(coop_layout);
    coop_intel_layout->addLayout(intel_layout);

    QHBoxLayout *personality_layout = new QHBoxLayout;
    personality_layout->setContentsMargins(0,0,0,0);
    personality_layout->addWidget(lbl_personality);
    personality_layout->addWidget(sb_personality);

    //Final Layout
    QVBoxLayout *Final = new QVBoxLayout;
    Final->setContentsMargins(0,0,0,0);
    Final->addLayout(name_layout);
    Final->addLayout(type_sex_layout);
    Final->addLayout(cantMate_layout);
    Final->addLayout(speed_layout);
    Final->addLayout(sprint_layout);
    Final->addLayout(accel_wins_layout);
    Final->addLayout(stamina_pCount_layout);
    Final->addLayout(coop_intel_layout);
    Final->addLayout(personality_layout);
    this->setLayout(Final);
}
void ChocoboEditor::init_connections(void)
{
    if(QT_VERSION <0x050000)
    {//QT4 Style Connect Statements
        connect(combo_type,SIGNAL(currentIndexChanged(int)),this,SLOT(setType(int)));
        connect(combo_sex,SIGNAL(currentIndexChanged(int)),this,SLOT(setSex(int)));
        connect(cb_cantMate,SIGNAL(toggled(bool)),this,SLOT(setCantMate(bool)));
        connect(line_name,SIGNAL(textChanged(QString)),this,SLOT(setName(QString)));
        connect(sb_sprint,SIGNAL(valueChanged(int)),this,SLOT(setSprint(int)));
        connect(sb_mSprint,SIGNAL(valueChanged(int)),this,SLOT(setMsprint(int)));
        connect(sb_speed,SIGNAL(valueChanged(int)),this,SLOT(setSpeed(int)));
        connect(sb_mSpeed,SIGNAL(valueChanged(int)),this,SLOT(setMspeed(int)));
        connect(sb_stamina,SIGNAL(valueChanged(int)),this,SLOT(setStamina(int)));
        connect(sb_accel,SIGNAL(valueChanged(int)),this,SLOT(setAccel(int)));
        connect(sb_coop,SIGNAL(valueChanged(int)),this,SLOT(setCoop(int)));
        connect(sb_intel,SIGNAL(valueChanged(int)),this,SLOT(setIntelligence(int)));
        connect(sb_personality,SIGNAL(valueChanged(int)),this,SLOT(setPersonality(int)));
        connect(sb_pCount,SIGNAL(valueChanged(int)),this,SLOT(setPcount(int)));
        connect(sb_wins,SIGNAL(valueChanged(int)),this,SLOT(setWins(int)));
    }
    else
    {//QT5 Style Connect Statements
    /*
        connect(combo_type::currentIndexChanged(int),this::setType(int));
        connect(combo_sex::currentIndexChanged(int),this::setSex(int));
        connect(cb_cantMate::toggled(bool),this::setCantMate(bool));
        connect(line_name::textChanged(QString),this::setName(QString));
        connect(sb_sprint::valueChanged(int),this::setSprint(int));
        connect(sb_mSprint::valueChanged(int),this::setMsprint(int));
        connect(sb_speed::valueChanged(int),this::setSpeed(int));
        connect(sb_mSpeed::valueChanged(int),this::setMspeed(int));
        connect(sb_stamina::valueChanged(int),this::setStamina(int));
        connect(sb_accel::valueChanged(int),this::setAccel(int));
        connect(sb_coop::valueChanged(int),this::setCoop(int));
        connect(sb_intel::valueChanged(int),this::setIntelligence(int));
        connect(sb_personality::valueChanged(int),this::setPersonality(int));
        connect(sb_pCount::valueChanged(int),this::setPcount(int));
        connect(sb_wins::valueChanged(int),this::setWins(int));
    */
    }
}
void ChocoboEditor::disconnectAll(void)
{
    if(QT_VERSION <0x050000)
    {//QT4 Style Connect Statements
        disconnect(combo_type,SIGNAL(currentIndexChanged(int)),this,SLOT(setType(int)));
        disconnect(combo_sex,SIGNAL(currentIndexChanged(int)),this,SLOT(setSex(int)));
        disconnect(cb_cantMate,SIGNAL(toggled(bool)),this,SLOT(setCantMate(bool)));
        disconnect(line_name,SIGNAL(textChanged(QString)),this,SLOT(setName(QString)));
        disconnect(sb_sprint,SIGNAL(valueChanged(int)),this,SLOT(setSprint(int)));
        disconnect(sb_mSprint,SIGNAL(valueChanged(int)),this,SLOT(setMsprint(int)));
        disconnect(sb_speed,SIGNAL(valueChanged(int)),this,SLOT(setSpeed(int)));
        disconnect(sb_mSpeed,SIGNAL(valueChanged(int)),this,SLOT(setMspeed(int)));
        disconnect(sb_stamina,SIGNAL(valueChanged(int)),this,SLOT(setStamina(int)));
        disconnect(sb_accel,SIGNAL(valueChanged(int)),this,SLOT(setAccel(int)));
        disconnect(sb_coop,SIGNAL(valueChanged(int)),this,SLOT(setCoop(int)));
        disconnect(sb_intel,SIGNAL(valueChanged(int)),this,SLOT(setIntelligence(int)));
        disconnect(sb_personality,SIGNAL(valueChanged(int)),this,SLOT(setPersonality(int)));
        disconnect(sb_pCount,SIGNAL(valueChanged(int)),this,SLOT(setPcount(int)));
        disconnect(sb_wins,SIGNAL(valueChanged(int)),this,SLOT(setWins(int)));
    }
    else
    {//QT5 Style Connect Statements
    /*
        disconnect(combo_type::currentIndexChanged(int),this::setType(int));
        disconnect(combo_sex::currentIndexChanged(int),this::setSex(int));
        disconnect(cb_cantMate::toggled(bool),this::setCantMate(bool));
        disconnect(line_name::textChanged(QString),this::setName(QString));
        disconnect(sb_sprint::valueChanged(int),this::setSprint(int));
        disconnect(sb_mSprint::valueChanged(int),this::setMsprint(int));
        disconnect(sb_speed::valueChanged(int),this::setSpeed(int));
        disconnect(sb_mSpeed::valueChanged(int),this::setMspeed(int));
        disconnect(sb_stamina::valueChanged(int),this::setStamina(int));
        disconnect(sb_accel::valueChanged(int),this::setAccel(int));
        disconnect(sb_coop::valueChanged(int),this::setCoop(int));
        disconnect(sb_intel::valueChanged(int),this::setIntelligence(int));
        disconnect(sb_personality::valueChanged(int),this::setPersonality(int));
        disconnect(sb_pCount::valueChanged(int),this::setPcount(int));
        disconnect(sb_wins::valueChanged(int),this::setWins(int));
    */
    }

}
void ChocoboEditor::SetChocobo(FF7CHOCOBO choco, QString Processed_Name, bool cant_mate, quint16 stamina)
{
    choco_data = choco;
    choco_name = Processed_Name;
    choco_cant_mate = cant_mate;
    choco_stamina = stamina;

    disconnectAll();
    line_name->setText(choco_name);
    combo_type->setCurrentIndex(choco_data.type);
    combo_sex->setCurrentIndex(choco_data.sex);
    if(choco_cant_mate){cb_cantMate->setCheckState(Qt::Checked);}
    else{cb_cantMate->setCheckState(Qt::Unchecked);}
    sb_speed->setValue(choco_data.speed);
    sb_mSpeed->setValue(choco_data.maxspeed);
    sb_sprint->setValue(choco_data.sprintspd);
    sb_mSprint->setValue(choco_data.maxsprintspd);
    sb_stamina->setValue(choco_stamina);
    sb_accel->setValue(choco_data.accel);
    sb_wins->setValue(choco_data.raceswon);
    sb_pCount->setValue(choco_data.pcount);
    sb_coop->setValue(choco_data.coop);
    sb_intel->setValue(choco_data.intelligence);
    sb_personality->setValue(choco_data.personality);
    init_connections();
}

