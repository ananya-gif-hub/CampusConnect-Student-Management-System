#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QVector>
#include <QTime>
#include <QDate>
#include <vector>

class QWidget;
class QLineEdit;
class QStackedWidget;
class QLabel;
class QVBoxLayout;
class QComboBox;
class QTableWidget;
class QDoubleSpinBox;

struct AttendanceRecord {
    QString code;
    QString subject;
    int attended;
    int conducted;
    double percentage() const {
        return conducted == 0 ? 0.0 : 100.0 * attended / conducted;
    }
};

class CourseRecord {
    QString code_;
    QString name_;
    int credits_;
    int semester_;
public:
    CourseRecord(const QString &code, const QString &name, int credits, int semester)
        : code_(code), name_(name), credits_(credits), semester_(semester) {}
    QString code() const { return code_; }
    QString name() const { return name_; }
    int credits() const { return credits_; }
    int semester() const { return semester_; }
};

struct TimetableEntry {
    int id;
    QString courseCode;
    int day;
    QTime start;
    QTime end;
    QString room;
};

struct ExamRecord {
    int id;
    QString courseCode;
    QString title;
    QDate date;
    QTime time;
    QString venue;
    double maximum;
    double scored;  // -1 means result pending
};

struct AnnouncementRecord {
    int id;
    QString title;
    QString category;
    QString message;
    QDate date;
    bool important;
    bool read;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;
private:
    QLineEdit *username = nullptr;
    QLineEdit *password = nullptr;
    QStackedWidget *pageStack = nullptr;
    QLabel *overallLabel = nullptr;
    QVBoxLayout *attendanceList = nullptr;
    QLabel *courseCountLabel = nullptr;
    QLabel *creditCountLabel = nullptr;
    QLineEdit *courseSearch = nullptr;
    QVBoxLayout *courseList = nullptr;
    QComboBox *dayFilter = nullptr;
    QTableWidget *timetableTable = nullptr;
    QLabel *timetableCountLabel = nullptr;
    QVector<AttendanceRecord> records;
    std::vector<CourseRecord> courses;
    std::vector<TimetableEntry> timetable;
    int nextTimetableId = 1;
    std::vector<ExamRecord> exams;
    int nextExamId = 1;
    std::vector<AnnouncementRecord> announcements;
    int nextAnnouncementId = 1;
    QTableWidget *announcementTable = nullptr;
    QComboBox *announcementFilter = nullptr;
    QLabel *announcementSummary = nullptr;
    QTableWidget *examTable = nullptr;
    QComboBox *examFilter = nullptr;
    QLabel *examSummary = nullptr;

    QString dataFilePath() const;
    void loadData();
    void saveData() const;
    void showLoginPage();
    void showStudentDashboard();
    void switchPage(int index);
    void refreshDashboard();
    QWidget *createDashboardPage();
    QWidget *createProfilePage();
    QWidget *createAttendancePage();
    QWidget *createCoursesPage();
    QWidget *createTimetablePage();
    QWidget *createExaminationsPage();
    QWidget *createAnnouncementsPage();
    double overallAttendance() const;
    void updateAttendance(int index, bool present);
    void refreshAttendance();
    int totalCredits() const;
    void addCourse(const CourseRecord &course);
    void showAddCourseDialog();
    void removeCourse(const QString &code);
    void refreshCourses();
    QString courseName(const QString &code) const;
    bool hasClash(const TimetableEntry &entry, int ignoreId = -1) const;
    void showTimetableDialog(int editId = -1);
    void deleteTimetableEntry(int id);
    void refreshTimetable();
    void showExamDialog(int editId = -1);
    void deleteExam(int id);
    void refreshExams();
    static QString examGrade(double percentage);
    void showAnnouncementDialog(int editId = -1);
    void deleteAnnouncement(int id);
    void toggleAnnouncementRead(int id);
    void refreshAnnouncements();
};

#endif // MAINWINDOW_H