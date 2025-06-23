#include "bookingreview.h"
#include "qsqlquery.h"
#include "ui_bookingreview.h"
#include <QFile>
#include <QTextCodec>

//实现自定义代理模型
TimeSlotProxyModel::TimeSlotProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

QVariant TimeSlotProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole && index.column() == 5) { // 时间段列
        int timeSlotId = sourceModel()->data(index, Qt::EditRole).toInt();
        if (timeSlotId > 0) {
            QSqlQuery query;
            query.prepare("SELECT start_time, end_time FROM time_slots WHERE id = :id");
            query.bindValue(":id", timeSlotId);
            if (query.exec() && query.next()) {
                return query.value(0).toString() + " - " + query.value(1).toString();
            } else {
                qDebug() << "Query failed for time slot:" << query.lastError().text();
            }
        }
    }
    return QSortFilterProxyModel::data(index, role);
}

BookingReview::BookingReview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BookingReview)
{
    ui->setupUi(this);
    currentAdminId = 1;

    initDatabase();
    initTableView();
    loadBookingData();

    // 使用新语法连接信号槽
    connect(ui->statusComboBox, &QComboBox::currentIndexChanged, this, &BookingReview::handleStatusChanged);
}

BookingReview::~BookingReview()
{
    delete ui;
}

void BookingReview::initDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/Users/18319/Desktop/labbooking.db");

    if (!db.open()) {
        QMessageBox::critical(this, "数据库错误", "无法连接到数据库: " + db.lastError().text());
        return;
    }

    // 检查数据库是否已经初始化
    QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table' AND name='users'");
    if (!query.next()) {
        // 执行SQL脚本初始化数据库结构
        if (!executeSqlScript("C:/Users/18319/Desktop/schema.sql")) {
            QMessageBox::critical(this, "数据库错误", "初始化数据库失败");
        }
    }
}

void BookingReview::initTableView()
{
    bookingModel = new QSqlRelationalTableModel(this);
    bookingModel->setTable("bookings");

    // 设置外键关联
    bookingModel->setRelation(1, QSqlRelation("users", "id", "real_name"));
    bookingModel->setRelation(2, QSqlRelation("labs", "id", "name"));
    bookingModel->setRelation(3, QSqlRelation("booking_status", "id", "status"));

    // 设置表头
    bookingModel->setHeaderData(0, Qt::Horizontal, "预约ID");
    bookingModel->setHeaderData(1, Qt::Horizontal, "申请人");
    bookingModel->setHeaderData(2, Qt::Horizontal, "实验室");
    bookingModel->setHeaderData(3, Qt::Horizontal, "状态");
    bookingModel->setHeaderData(4, Qt::Horizontal, "预约日期");
    bookingModel->setHeaderData(5, Qt::Horizontal, "时间段");
    bookingModel->setHeaderData(6, Qt::Horizontal, "使用目的");
    bookingModel->setHeaderData(7, Qt::Horizontal, "申请时间");

    // 使用自定义代理模型
    proxyModel = new TimeSlotProxyModel(this);
    proxyModel->setSourceModel(bookingModel);

    ui->bookingTableView->setModel(proxyModel);
    ui->bookingTableView->setSortingEnabled(true);
    ui->bookingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->bookingTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置时间段列宽
    ui->bookingTableView->setColumnWidth(5, 150); // 设置合适的列宽
}

void BookingReview::loadBookingData(const QString &filter)
{
    qDebug() << "Loading data with filter:" << filter;

    bookingModel->setFilter(filter);
    if (!bookingModel->select()) {
        QMessageBox::warning(this, "数据加载失败", "加载预约数据失败: " + bookingModel->lastError().text());
        qDebug() << "Select query failed:" << bookingModel->lastError().text();
        return;
    }

    // 隐藏不需要显示的列
    ui->bookingTableView->hideColumn(0);
    ui->bookingTableView->hideColumn(8);
    ui->bookingTableView->hideColumn(9);
    ui->bookingTableView->hideColumn(10);
}

QString BookingReview::getTimeSlotDetails(int timeSlotId)
{
    QSqlQuery query;
    query.prepare("SELECT start_time, end_time FROM time_slots WHERE id = :timeSlotId");
    query.bindValue(":timeSlotId", timeSlotId);
    if (query.exec() && query.next()) {
        QString startTime = query.value(0).toString();
        QString endTime = query.value(1).toString();
        return startTime + " - " + endTime;
    }
    qDebug() << "Failed to get time slot details:" << query.lastError().text();
    return "";
}

void BookingReview::on_searchBtn_clicked()
{
    QString status = ui->statusComboBox->currentText();
    QString date = ui->dateEdit->date().toString("yyyy-MM-dd");
    QString keyword = ui->keywordLineEdit->text();

    QString filter;
    QStringList conditions;

    if (!status.isEmpty() && status != "全部") {
        conditions.append(QString("booking_status.status = '%1'").arg(status));
    }

    if (!date.isEmpty() && date != "2000-01-01") {
        conditions.append(QString("bookings.booking_date = '%1'").arg(date));
    }

    if (!keyword.isEmpty()) {
        conditions.append(QString("(users.real_name LIKE '%%1%' OR labs.name LIKE '%%1%' OR bookings.use_purpose LIKE '%%1%')").arg(keyword));
    }

    if (!conditions.isEmpty()) {
        filter = conditions.join(" AND ");
    }

    qDebug() << "Generated filter:" << filter; // 打印过滤条件进行检查

    loadBookingData(filter);
}

void BookingReview::handleStatusChanged(int index)
{
    qDebug() << "Status changed to index:" << index;
    on_searchBtn_clicked(); // 触发搜索
}

void BookingReview::on_approveBtn_clicked()
{
    QModelIndexList selectedRows = ui->bookingTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "提示", "请选择要审批的预约");
        return;
    }

    QStringList bookingIds;
    for (const auto &index : selectedRows) {
        int bookingId = proxyModel->data(proxyModel->index(index.row(), 0)).toInt();
        bookingIds.append(QString::number(bookingId));
    }

    QSqlQuery query;
    query.prepare("UPDATE bookings SET status_id = 2, review_time = CURRENT_TIMESTAMP, reviewer_id = :reviewer_id WHERE id = :id");
    query.bindValue(":reviewer_id", currentAdminId);

    for (const QString &id : bookingIds) {
        query.bindValue(":id", id);
        if (!query.exec()) {
            QMessageBox::warning(this, "审批失败", "审批预约失败: " + query.lastError().text());
            qDebug() << "Approve query failed:" << query.lastError().text();
            return;
        }
    }

    loadBookingData();
    QMessageBox::information(this, "成功", "预约已成功审批");
}

void BookingReview::on_rejectBtn_clicked()
{
    QModelIndexList selectedRows = ui->bookingTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "提示", "请选择要拒绝的预约");
        return;
    }

    bool ok;
    QString reason = QInputDialog::getText(this, "拒绝理由", "请输入拒绝理由:", QLineEdit::Normal, "", &ok);
    if (!ok || reason.isEmpty()) {
        return;
    }

    QStringList bookingIds;
    for (const auto &index : selectedRows) {
        int bookingId = proxyModel->data(proxyModel->index(index.row(), 0)).toInt();
        bookingIds.append(QString::number(bookingId));
    }

    QSqlQuery query;
    query.prepare("UPDATE bookings SET status_id = 3, review_time = CURRENT_TIMESTAMP, reviewer_id = :reviewer_id, remark = :reason WHERE id = :id");
    query.bindValue(":reviewer_id", currentAdminId);
    query.bindValue(":reason", reason);

    for (const QString &id : bookingIds) {
        query.bindValue(":id", id);
        if (!query.exec()) {
            QMessageBox::warning(this, "拒绝失败", "拒绝预约失败: " + query.lastError().text());
            qDebug() << "Reject query failed:" << query.lastError().text();
            return;
        }
    }

    loadBookingData();
    QMessageBox::information(this, "成功", "预约已成功拒绝");
}

void BookingReview::on_refreshBtn_clicked()
{
    ui->statusComboBox->setCurrentIndex(0);
    ui->dateEdit->setDate(QDate(2000, 1, 1));
    ui->keywordLineEdit->clear();
    loadBookingData();
}

void BookingReview::on_importBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入文件", "", "CSV文件 (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }

    if (fileName.contains("labs", Qt::CaseInsensitive)) {
        if (importLabsFromCsv(fileName)) {
            QMessageBox::information(this, "成功", "实验室数据导入成功");
        } else {
            QMessageBox::warning(this, "失败", "实验室数据导入失败");
        }
    } else if (fileName.contains("devices", Qt::CaseInsensitive)) {
        if (importDevicesFromCsv(fileName)) {
            QMessageBox::information(this, "成功", "设备数据导入成功");
        } else {
            QMessageBox::warning(this, "失败", "设备数据导入失败");
        }
    } else {
        QMessageBox::warning(this, "错误", "无法识别的文件类型，请导入实验室或设备CSV文件");
    }

    loadBookingData();
}

void BookingReview::on_exportBtn_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出文件", "", "CSV文件 (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }

    if (exportBookingsToCsv(fileName)) {
        QMessageBox::information(this, "成功", "预约数据导出成功");
    } else {
        QMessageBox::warning(this, "失败", "预约数据导出失败");
    }
}

bool BookingReview::importLabsFromCsv(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << fileName;
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    // 跳过标题行
    if (!in.atEnd()) {
        in.readLine();
    }

    QSqlQuery query;
    query.prepare("INSERT INTO labs (name, location, capacity, device_types, description, open_time, create_time) "
                  "VALUES (:name, :location, :capacity, :device_types, :description, :open_time, CURRENT_TIMESTAMP)");

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() < 6) {
            continue; // 跳过不完整的行
        }

        query.bindValue(":name", fields[0].trimmed());
        query.bindValue(":location", fields[1].trimmed());
        query.bindValue(":capacity", fields[2].trimmed().toInt());
        query.bindValue(":device_types", fields[3].trimmed());
        query.bindValue(":description", fields[4].trimmed());
        query.bindValue(":open_time", fields[5].trimmed());

        if (!query.exec()) {
            qDebug() << "导入实验室数据失败:" << query.lastError().text();
            return false;
        }
    }

    file.close();
    return true;
}

bool BookingReview::importDevicesFromCsv(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << fileName;
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    // 跳过标题行
    if (!in.atEnd()) {
        in.readLine();
    }

    QSqlQuery query;
    query.prepare("INSERT INTO lab_devices (lab_id, device_name, model, quantity) "
                  "VALUES (:lab_id, :device_name, :model, :quantity)");

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() < 4) {
            continue; // 跳过不完整的行
        }

        query.bindValue(":lab_id", fields[0].trimmed().toInt());
        query.bindValue(":device_name", fields[1].trimmed());
        query.bindValue(":model", fields[2].trimmed());
        query.bindValue(":quantity", fields[3].trimmed().toInt());

        if (!query.exec()) {
            qDebug() << "导入设备数据失败:" << query.lastError().text();
            return false;
        }
    }

    file.close();
    return true;
}

bool BookingReview::exportBookingsToCsv(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << fileName;
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // 写入标题行
    out << "预约ID,申请人,实验室,状态,预约日期,时间段,使用目的,申请时间,审核时间,审核人,备注\n";

    // 遍历模型数据
    for (int i = 0; i < bookingModel->rowCount(); ++i) {
        QStringList fields;
        for (int j = 0; j < bookingModel->columnCount(); ++j) {
            fields.append("\"" + bookingModel->data(bookingModel->index(i, j)).toString().replace("\"", "\"\"") + "\"");
        }
        out << fields.join(',') << "\n";
    }

    file.close();
    return true;
}

bool BookingReview::executeSqlScript(const QString &scriptPath)
{
    QFile file(scriptPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开SQL脚本文件:" << scriptPath;
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString script = in.readAll();
    file.close();

    // 按分号分割SQL语句
    QStringList statements = script.split(';', Qt::SkipEmptyParts);

    QSqlQuery query;
    query.exec("BEGIN TRANSACTION");

    for (const QString &statement : statements) {
        QString trimmedStmt = statement.trimmed();
        if (trimmedStmt.isEmpty()) {
            continue;
        }

        if (!query.exec(trimmedStmt)) {
            qDebug() << "执行SQL语句失败:" << trimmedStmt;
            qDebug() << "错误信息:" << query.lastError().text();
            query.exec("ROLLBACK");
            return false;
        }
    }

    query.exec("COMMIT");
    return true;
}

// 定义槽函数
void BookingReview::on_statusComboBox_currentIndexChanged(int index)
{
    on_searchBtn_clicked();
}
