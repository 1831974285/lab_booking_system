#ifndef BOOKINGREVIEW_H
#define BOOKINGREVIEW_H

#include <QWidget>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDate>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>
#include <QInputDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class BookingReview; }
QT_END_NAMESPACE

// 自定义代理模型，用于显示时间段详情
class TimeSlotProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TimeSlotProxyModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};


class BookingReview : public QWidget
{
    Q_OBJECT

public:
    BookingReview(QWidget *parent = nullptr);
    ~BookingReview();

private slots:
    void on_searchBtn_clicked();
    void on_approveBtn_clicked();
    void on_rejectBtn_clicked();
    void on_refreshBtn_clicked();
    void on_importBtn_clicked();
    void on_exportBtn_clicked();
    void handleStatusChanged(int index);
    void on_statusComboBox_currentIndexChanged(int index);

private:
    Ui::BookingReview *ui;
    QSqlRelationalTableModel *bookingModel;
    QSortFilterProxyModel *proxyModel;
    int currentAdminId;

    void initDatabase();
    void initTableView();
    void loadBookingData(const QString &filter = "");
    bool importLabsFromCsv(const QString &fileName);
    bool importDevicesFromCsv(const QString &fileName);
    bool exportBookingsToCsv(const QString &fileName);
    bool executeSqlScript(const QString &scriptPath);
    QString getTimeSlotDetails(int timeSlotId);
};
#endif // BOOKINGREVIEW_H
