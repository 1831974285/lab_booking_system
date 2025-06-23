# LabBookingSystem.pro

# 项目基本信息
TEMPLATE = app
TARGET = LabBookingSystem
VERSION = 1.0.0

# 配置选项
CONFIG += c++17
CONFIG += console # 调试时显示控制台，发布时可移除

# 编译目录设置
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
UI_DIR = $$PWD/build/ui
RCC_DIR = $$PWD/build/rcc

# 包含路径
INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/src/common
INCLUDEPATH += $$PWD/src/database
INCLUDEPATH += $$PWD/src/account
INCLUDEPATH += $$PWD/src/appointment
INCLUDEPATH += $$PWD/src/schedule
INCLUDEPATH += $$PWD/src/reminder
INCLUDEPATH += $$PWD/src/admin

# 模块源文件
SOURCES += $$PWD/src/main.cpp
SOURCES += $$PWD/src/MainWindow.cpp

# 公共模块
SOURCES += $$PWD/src/common/utils/*.cpp
HEADERS += $$PWD/src/common/utils/*.h
HEADERS += $$PWD/src/common/constants.h

# 数据库模块
SOURCES += $$PWD/src/database/DatabaseManager.cpp
SOURCES += $$PWD/src/database/DbInitializer.cpp
SOURCES += $$PWD/src/database/models/*.cpp
HEADERS += $$PWD/src/database/*.h
HEADERS += $$PWD/src/database/models/*.h

# 账号系统模块
SOURCES += $$PWD/src/account/AccountManager.cpp
SOURCES += $$PWD/src/account/LoginWidget.cpp
SOURCES += $$PWD/src/account/RegisterWidget.cpp
HEADERS += $$PWD/src/account/*.h
FORMS += $$PWD/src/account/*.ui

# 预约系统模块
SOURCES += $$PWD/src/appointment/AppointmentSystem.cpp
SOURCES += $$PWD/src/appointment/LabListView.cpp
SOURCES += $$PWD/src/appointment/BookingForm.cpp
SOURCES += $$PWD/src/appointment/BookingManager.cpp
HEADERS += $$PWD/src/appointment/*.h
FORMS += $$PWD/src/appointment/*.ui

# 行程展示模块
SOURCES += $$PWD/src/schedule/ScheduleModel.cpp
SOURCES += $$PWD/src/schedule/CalendarView.cpp
SOURCES += $$PWD/src/schedule/ScheduleList.cpp
SOURCES += $$PWD/src/schedule/ScheduleDetail.cpp
HEADERS += $$PWD/src/schedule/*.h
FORMS += $$PWD/src/schedule/*.ui

# 提醒系统模块
SOURCES += $$PWD/src/reminder/ReminderManager.cpp
SOURCES += $$PWD/src/reminder/ReminderDialog.cpp
SOURCES += $$PWD/src/reminder/TrayIcon.cpp
HEADERS += $$PWD/src/reminder/*.h
FORMS += $$PWD/src/reminder/*.ui

# 管理员模块
SOURCES += $$PWD/src/admin/LabManagement.cpp
SOURCES += $$PWD/src/admin/BookingReview.cpp
SOURCES += $$PWD/src/admin/UserManagement.cpp
SOURCES += $$PWD/src/admin/Statistics.cpp
HEADERS += $$PWD/src/admin/*.h
FORMS += $$PWD/src/admin/*.ui

# 资源文件
RESOURCES += $$PWD/resources/resources.qrc

# Qt模块依赖
QT += core gui widgets sql network