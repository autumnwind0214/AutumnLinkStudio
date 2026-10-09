#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenuBar>
#include <QPainter>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTabWidget>

namespace {
QIcon symbol(const QString &kind, const QColor &color)
{
    QPixmap pix(24, 24);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.8));
    if (kind == "folder") {
        p.drawPolyline(QPolygonF{QPointF(3,7), QPointF(3,20), QPointF(21,20), QPointF(21,9), QPointF(11,9), QPointF(9,6), QPointF(3,6)});
    } else if (kind == "file") {
        p.drawPolyline(QPolygonF{QPointF(15,20), QPointF(4,20), QPointF(4,3), QPointF(14,3), QPointF(18,7), QPointF(18,13)});
        p.drawLine(14,3,14,8); p.drawLine(14,8,18,8);
        p.drawLine(19,15,19,23); p.drawLine(15,19,23,19);
    } else if (kind == "mqtt") {
        p.drawArc(QRectF(3,3,35,35),90*16,90*16);
        p.drawArc(QRectF(3,9,23,23),90*16,90*16);
        p.drawArc(QRectF(3,15,11,11),90*16,90*16);
        p.setBrush(color); p.drawEllipse(QPointF(4,20),1.5,1.5);
    } else if (kind == "play") {
        p.setBrush(color); p.drawPolygon(QPolygonF{QPointF(5,3),QPointF(21,12),QPointF(5,21)});
    } else if (kind == "globe") {
        p.drawEllipse(3,3,18,18); p.drawEllipse(8,3,8,18);
        p.drawLine(3,12,21,12); p.drawLine(5,7,19,7); p.drawLine(5,17,19,17);
    } else if (kind == "trash") {
        p.drawLine(4,6,20,6); p.drawLine(9,3,15,3);
        p.drawRect(6,7,12,14); p.drawLine(10,10,10,18); p.drawLine(14,10,14,18);
    } else {
        p.drawRoundedRect(3,4,18,16,2,2);
        p.drawPolyline(QPolygonF{QPointF(5,14),QPointF(9,14),QPointF(9,8),QPointF(14,8),QPointF(14,16),QPointF(19,16)});
    }
    return QIcon(pix);
}
QPushButton *button(const QString &text, const QString &icon, const QColor &color, QWidget *parent)
{
    auto *b = new QPushButton(symbol(icon,color),text,parent);
    b->setCursor(Qt::PointingHandCursor);
    b->setIconSize(QSize(22,22));
    return b;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    buildWorkspace();
}

void MainWindow::buildWorkspace()
{
    const QColor teal("#167f8c");
    setWindowTitle("Autumn Studio");
    resize(1500, 820);
    setMinimumSize(1000, 620);
    setStyleSheet(QString::fromUtf8(R"(
        QMainWindow, QWidget { background: #101318; color: #e4e6ec; font-family: 'Microsoft YaHei UI', 'Segoe UI'; font-size: 14px; }
        QMenuBar { background: #101114; padding: 5px; font-weight: 600; font-size: 15px; }
        QMenuBar::item { padding: 4px 18px; } QMenuBar::item:selected, QMenu::item:selected { background: #263039; }
        QMenu { border: 1px solid #343a44; padding: 6px; } QMenu::item { padding: 7px 24px; }
        QFrame#explorer, QFrame#workspace { background: #181c22; border: 2px solid #303640; border-radius: 3px; }
        QFrame#explorer QWidget, QFrame#workspace QWidget { background: transparent; }
        QPushButton { border: 0; padding: 7px 10px; text-align: left; }
        QPushButton:hover { background: #253039; border-radius: 3px; } QPushButton:pressed { background: #30424b; }
        QPushButton#link { color: #168594; font-weight: 600; padding-left: 0; }
        QPushButton#guide { background: #101419; border: 1px solid #303844; border-radius: 3px; padding: 12px; text-align: left; }
        QPushButton#guide:hover { border-color: #168594; background: #1d2930; }
        QLabel#muted { color: #a0a7b4; } QLabel#heading { font-size: 20px; font-weight: 600; }
        QTreeWidget { background: #181c22; border: 0; outline: none; font-weight: 600; }
        QTreeWidget::item { height: 33px; } QTreeWidget::item:selected { background: #253944; } QTreeWidget::item:hover { background: #222b34; }
        QSplitter::handle { background: #101318; width: 9px; }
        QStatusBar { background: #101318; } QStatusBar::item { border: 0; }
        QLabel#badge { background: #20252d; border: 1px solid #303844; border-radius: 3px; padding: 5px 10px; font-size: 12px; }
        QLineEdit, QPlainTextEdit { background: #101419; border: 1px solid #343e48; border-radius: 3px; padding: 7px; }
        QTabWidget::pane { border: 0; } QTabBar::tab { background: #20252d; padding: 9px 18px; } QTabBar::tab:selected { color: #35b9c4; }
    )"));
    menuBar()->clear();
    auto *brand = new QLabel("  ◉  Autumn Studio  ",this);
    brand->setStyleSheet("color: #168594; font-weight: 600; font-size: 16px;");
    menuBar()->setCornerWidget(brand,Qt::TopLeftCorner);
    auto *fileMenu = menuBar()->addMenu("文件(&F)");
    auto *newAction = fileMenu->addAction("新建连接文件…");
    auto *openAction = fileMenu->addAction("打开文件夹…");
    fileMenu->addSeparator();
    connect(fileMenu->addAction("退出"), &QAction::triggered,this,&QWidget::close);
    auto *connectionMenu = menuBar()->addMenu("连接(&C)");
    auto *connectionAction = connectionMenu->addAction("新建连接配置…");
    auto *debugMenu = menuBar()->addMenu("调试");
    auto *debugAction = debugMenu->addAction("打开选中的调试页面");
    auto *toolsMenu = menuBar()->addMenu("工具(&T)");
    auto *homeAction = toolsMenu->addAction("返回欢迎页");
    menuBar()->addMenu("帮助(&H)")->addAction("NexusLink Studio · 连接调试工作台");

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *outer = new QHBoxLayout(central);
    outer->setContentsMargins(8,0,5,0); outer->setSpacing(8);
    auto *rail = new QWidget(central);
    rail->setFixedWidth(42);
    auto *railLayout = new QVBoxLayout(rail);
    railLayout->setContentsMargins(0,0,0,0);
    auto *resources = button("","folder",QColor("#a2a5ad"),rail);
    resources->setToolTip("资源管理器");
    resources->setStyleSheet("border-left: 3px solid #20c970;");
    railLayout->addWidget(resources);
    auto *tools = button("⚒","",QColor("#999da6"),rail);
    tools->setIcon(QIcon()); tools->setToolTip("返回欢迎页"); railLayout->addWidget(tools);
    railLayout->addStretch();
    auto *settings = button("⚙","",teal,rail); settings->setIcon(QIcon()); settings->setToolTip("工作区设置"); railLayout->addWidget(settings);
    outer->addWidget(rail);
    auto *splitter = new QSplitter(Qt::Horizontal,central);
    outer->addWidget(splitter,1);
    auto *explorer = new QFrame(splitter); explorer->setObjectName("explorer"); explorer->setMinimumWidth(240);
    auto *explorerLayout = new QVBoxLayout(explorer); explorerLayout->setContentsMargins(8,6,8,8); explorerLayout->setSpacing(4);
    auto *toolbar = new QHBoxLayout;
    auto *newButton = button("","file",teal,explorer); newButton->setToolTip("新建连接文件");
    auto *openButton = button("","folder",teal,explorer); openButton->setToolTip("打开文件夹");
    auto *folderButton = button("","folder",QColor("#ffb000"),explorer); folderButton->setToolTip("新建分组");
    auto *deleteButton = button("","trash",QColor("#f15c63"),explorer); deleteButton->setToolTip("移除选中配置");
    for (auto *b : {newButton,openButton,folderButton,deleteButton}) { b->setFixedSize(38,36); toolbar->addWidget(b); }
    toolbar->addStretch(); explorerLayout->addLayout(toolbar);
    auto *tree = new QTreeWidget(explorer); tree->setHeaderHidden(true); tree->setIndentation(16); tree->setIconSize(QSize(20,20)); explorerLayout->addWidget(tree);
    auto *group = new QTreeWidgetItem(tree,QStringList{"新建文件夹"}); group->setIcon(0,symbol("folder",QColor("#ffb000"))); group->setExpanded(true);
    const QStringList names = {"串口","命名管道服务器","命名管道客户端","实时报表","通讯组态","HTTP 客户端","Modbus 从站","Modbus 从站（搜索）","Modbus 主站","Modbus 主站（搜索）","MQTT 客户端","MQTT Broker","TCP 服务端1","TCP 客户端1"};
    const QStringList colors = {"#ff8c16","#bb79ed","#ec59aa","#17c9e4","#22cd60","#579bec","#19cce3","#19cce3","#f1d600","#f1d600","#ff8c16","#20cd62","#ffc500","#20cd62"};
    for (int i=0;i<names.size();++i) {
        auto *item = new QTreeWidgetItem(group,QStringList{names[i]});
        item->setIcon(0,symbol(i==5 ? "globe" : (i==10 || i==11 ? "mqtt" : "protocol"),QColor(colors[i])));
    }
    auto *workspace = new QFrame(splitter); workspace->setObjectName("workspace");
    auto *workspaceLayout = new QVBoxLayout(workspace); workspaceLayout->setContentsMargins(0,0,0,0);
    auto *stack = new QStackedWidget(workspace); workspaceLayout->addWidget(stack);
    auto *welcome = new QWidget(stack); stack->addWidget(welcome);
    auto *welcomeLayout = new QHBoxLayout(welcome); welcomeLayout->setContentsMargins(60,54,60,30); welcomeLayout->setSpacing(50);
    auto *left = new QVBoxLayout; left->setSpacing(14);
    auto *title = new QLabel("NexusLink Studio",welcome); title->setStyleSheet("font-size: 38px; font-weight: 700;"); left->addWidget(title);
    auto *subtitle = new QLabel("连接调试工作台",welcome); subtitle->setObjectName("muted"); subtitle->setStyleSheet("font-size: 17px; font-weight: 600;"); left->addWidget(subtitle);
    left->addSpacing(28);
    auto heading = [welcome](const QString &text) { auto *l=new QLabel(text,welcome); l->setObjectName("heading"); return l; };
    left->addWidget(heading("启动"));
    auto *createLink = button("新建连接文件…","file",teal,welcome); createLink->setObjectName("link"); left->addWidget(createLink);
    auto *openLink = button("打开文件夹…","folder",teal,welcome); openLink->setObjectName("link"); left->addWidget(openLink);
    left->addSpacing(8);
    auto *hint = new QLabel("也可以从左侧资源管理器选择一个配置文件开始调试。",welcome); hint->setObjectName("muted"); hint->setWordWrap(true); left->addWidget(hint);
    left->addSpacing(10); left->addWidget(heading("最近使用"));
    auto *recent = button("MQTT Broker","mqtt",QColor("#20cd62"),welcome); recent->setObjectName("link"); left->addWidget(recent);
    auto *recentPath = new QLabel("尚未打开工作文件夹",welcome); recentPath->setObjectName("muted"); left->addWidget(recentPath);
    left->addStretch(); welcomeLayout->addLayout(left,1);
    auto *right = new QVBoxLayout; right->setSpacing(14); right->addWidget(heading("入门")); right->addSpacing(4);
    auto guide = [welcome,teal,right](const QString &title,const QString &detail,const QString &icon) {
        auto *b=button(title+"\n"+detail,icon,teal,welcome); b->setObjectName("guide"); b->setMinimumHeight(78); right->addWidget(b); return b;
    };
    auto *createGuide=guide("创建连接配置","选择协议类型，保存为可复用的连接文件。","file");
    auto *debugGuide=guide("打开调试页面","双击配置文件后，会在工作区打开对应的调试工具。","play");
    auto *layoutGuide=guide("整理工作区布局","拖动左侧分隔线，调整资源管理器宽度。","folder");
    right->addStretch(); welcomeLayout->addLayout(right,1); welcomeLayout->addStretch(1);
    auto *tabs = new QTabWidget(stack); tabs->setTabsClosable(true); stack->addWidget(tabs);
    splitter->setSizes({290,1150}); splitter->setChildrenCollapsible(false);
    auto openDebug = [this,tree,group,tabs,stack](QTreeWidgetItem *item) {
        if (!item || item==group || item->childCount()>0) return;
        for(int i=0;i<tabs->count();++i) if(tabs->tabText(i)==item->text(0)) { tabs->setCurrentIndex(i); stack->setCurrentWidget(tabs); return; }
        auto *page=new QWidget(tabs); auto *v=new QVBoxLayout(page); v->setContentsMargins(24,24,24,24);
        auto *row=new QHBoxLayout; row->addWidget(new QLabel("服务器地址:",page)); auto *host=new QLineEdit("127.0.0.1",page); row->addWidget(host); row->addWidget(new QLabel("端口:",page)); auto *port=new QLineEdit("8080",page); port->setMaximumWidth(100); row->addWidget(port); v->addLayout(row);
        auto *notice=new QLabel("调试界面预览 · 协议通信功能尚未接入",page); notice->setObjectName("muted"); v->addWidget(notice);
        v->addWidget(new QLabel("接收数据",page)); auto *receive=new QPlainTextEdit(page); receive->setReadOnly(true); v->addWidget(receive,1);
        v->addWidget(new QLabel("发送数据",page)); auto *send=new QPlainTextEdit(page); send->setPlaceholderText("在此输入发送内容…"); v->addWidget(send,1);
        tabs->setCurrentIndex(tabs->addTab(page,item->icon(0),item->text(0))); stack->setCurrentWidget(tabs);
        statusBar()->showMessage("已打开 " + item->text(0),3000);
    };
    connect(tree,&QTreeWidget::itemDoubleClicked,this,[openDebug](QTreeWidgetItem *item,int){openDebug(item);});
    connect(tabs,&QTabWidget::tabCloseRequested,this,[tabs,stack,welcome](int index){ auto *page=tabs->widget(index); tabs->removeTab(index); page->deleteLater(); if(tabs->count()==0) stack->setCurrentWidget(welcome); });
    auto create = [this,tree,group,teal] {
        bool ok=false; auto name=QInputDialog::getItem(this,"新建连接配置","选择协议类型",{"串口","HTTP 客户端","Modbus 主站","Modbus 从站","MQTT 客户端","MQTT Broker","TCP 服务端","TCP 客户端"},0,false,&ok);
        if(ok && !name.isEmpty()) { auto *item=new QTreeWidgetItem(group,QStringList{name}); item->setIcon(0,symbol("file",teal)); group->setExpanded(true); tree->setCurrentItem(item); }
    };
    auto openFolder = [this,group,recentPath] { auto path=QFileDialog::getExistingDirectory(this,"打开工作文件夹"); if(!path.isEmpty()) { group->setText(0,QFileInfo(path).fileName()); recentPath->setText(path); } };
    for(auto *b : {newButton,createLink,createGuide}) connect(b,&QPushButton::clicked,this,create);
    connect(newAction,&QAction::triggered,this,create); connect(connectionAction,&QAction::triggered,this,create);
    for(auto *b : {openButton,openLink}) connect(b,&QPushButton::clicked,this,openFolder); connect(openAction,&QAction::triggered,this,openFolder);
    connect(folderButton,&QPushButton::clicked,this,[this,tree]{ bool ok; auto name=QInputDialog::getText(this,"新建分组","分组名称",QLineEdit::Normal,"新建文件夹",&ok); if(ok && !name.trimmed().isEmpty()){ auto *item=new QTreeWidgetItem(tree,QStringList{name}); item->setIcon(0,symbol("folder",QColor("#ffb000"))); } });
    connect(deleteButton,&QPushButton::clicked,this,[tree,group]{ auto *item=tree->currentItem(); if(item && item!=group) delete item; });
    auto selectedDebug=[tree,group,openDebug]{ auto *item=tree->currentItem(); if(!item || item==group) item=group->child(0); openDebug(item); };
    connect(debugAction,&QAction::triggered,this,selectedDebug); connect(debugGuide,&QPushButton::clicked,this,selectedDebug);
    connect(recent,&QPushButton::clicked,this,[group,openDebug]{ for(int i=0;i<group->childCount();++i) if(group->child(i)->text(0)=="MQTT Broker") { openDebug(group->child(i)); break; } });
    auto home=[stack,welcome]{ stack->setCurrentWidget(welcome); };
    connect(homeAction,&QAction::triggered,this,home); connect(tools,&QPushButton::clicked,this,home); connect(settings,&QPushButton::clicked,this,home);
    connect(resources,&QPushButton::clicked,this,[explorer]{ explorer->setVisible(!explorer->isVisible()); });
    connect(layoutGuide,&QPushButton::clicked,this,[splitter]{ splitter->setSizes({290,1150}); });
    statusBar()->setSizeGripEnabled(true);
    auto badge=[this](const QString &text){ auto *l=new QLabel(text,this); l->setObjectName("badge"); return l; };
    statusBar()->addWidget(badge("状态  就绪"));
    statusBar()->addPermanentWidget(badge("资源  —"));
    statusBar()->addPermanentWidget(badge("版本  v0.1.0"));
    statusBar()->addPermanentWidget(badge("●  MCP 未连接"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
