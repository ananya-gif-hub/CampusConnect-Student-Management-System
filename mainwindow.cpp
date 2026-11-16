#include "mainwindow.h"

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QStackedWidget>
#include <QScrollArea>
#include <QProgressBar>
#include <QStringList>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QComboBox>
#include <QListView>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QTimeEdit>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QLayoutItem>
#include <QtGlobal>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QSaveFile>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <algorithm>
#include <stdexcept>

// CampusConnect Step 20 - Qt Widgets student portal.
static QString cardStyle() {
    return "QFrame {background:white;border:1px solid #E2E8F0;border-radius:12px;}"
           "QLabel {background:transparent;border:none;border-radius:0;}";
}
static QString greenStyle() {
    return "QPushButton {background:#10B981;color:white;border:0;border-radius:8px;padding:11px;font-weight:bold;}"
           "QPushButton:hover {background:#059669;}";
}
static QString redStyle() {
    return "QPushButton {background:#FEE2E2;color:#B91C1C;border:0;border-radius:8px;padding:10px;font-weight:bold;}"
           "QPushButton:hover {background:#FECACA;}";
}
static QString inputStyle() {
    return "QLineEdit,QComboBox,QTimeEdit,QDateEdit,QSpinBox,QDoubleSpinBox {"
           "background:#FFFFFF;color:#172033;"
           "border:1px solid #CBD5E1;border-radius:7px;"
           "padding:9px;font-size:14px;selection-background-color:#10B981;"
           "selection-color:white;}"
           "QDateEdit::drop-down,QTimeEdit::drop-down {"
           "subcontrol-origin:padding;subcontrol-position:top right;"
           "width:28px;background:#F1F5F9;border-left:1px solid #CBD5E1;}"
           "QDateEdit::down-arrow {image:none;border:solid #334155;"
           "border-width:0 1px 1px 0;padding:3px;}"
           "QDateEdit:disabled,QTimeEdit:disabled,QDoubleSpinBox:disabled {"
           "background:#F1F5F9;color:#64748B;}"
           "QComboBox {min-width:145px;padding-right:24px;}"
           "QComboBox::drop-down {subcontrol-origin:padding;"
           "subcontrol-position:top right;width:28px;"
           "border-left:1px solid #CBD5E1;background:#F1F5F9;}"
           "QComboBox QAbstractItemView {background:#FFFFFF;color:#172033;"
           "border:1px solid #CBD5E1;outline:0;"
           "selection-background-color:#D1FAE5;selection-color:#064E3B;}"
           "QLineEdit:focus,QComboBox:focus,QTimeEdit:focus,QDateEdit:focus,QSpinBox:focus,QDoubleSpinBox:focus {"
           "border:2px solid #10B981;}";
}

// Explicitly style the native popup list. This prevents invisible white-on-white
// choices when Windows is running in dark mode.
static void configureCombo(QComboBox *combo) {
    combo->setView(new QListView(combo));
    combo->setStyleSheet(inputStyle());
    QPalette palette = combo->palette();
    palette.setColor(QPalette::Base, QColor("#FFFFFF"));
    palette.setColor(QPalette::Text, QColor("#172033"));
    palette.setColor(QPalette::Window, QColor("#FFFFFF"));
    palette.setColor(QPalette::WindowText, QColor("#172033"));
    palette.setColor(QPalette::Highlight, QColor("#D1FAE5"));
    palette.setColor(QPalette::HighlightedText, QColor("#064E3B"));
    combo->setPalette(palette);
    combo->view()->setPalette(palette);
    combo->view()->setStyleSheet(
        "QListView {background:#FFFFFF;color:#172033;"
        "border:1px solid #CBD5E1;font-size:14px;outline:0;}"
        "QListView::item {min-height:30px;padding:5px 10px;color:#172033;}"
        "QListView::item:selected {background:#D1FAE5;color:#064E3B;}"
        "QListView::item:hover {background:#E2E8F0;color:#172033;}");
}

static QLabel *txt(const QString &value, int size = 14, bool bold = false) {
    QLabel *label = new QLabel(value);
    label->setWordWrap(true);
    label->setStyleSheet(QString("color:#172033;font-size:%1px;font-weight:%2;")
                             .arg(size).arg(bold ? "bold" : "normal"));
    return label;
}
static QFrame *card(const QString &title, const QString &value) {
    QFrame *frame = new QFrame;
    frame->setStyleSheet(cardStyle());
    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->addWidget(txt(title, 13));
    layout->addWidget(txt(value, 26, true));
    return frame;
}
static QScrollArea *scrollPage(QVBoxLayout *&layout) {
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget;
    content->setStyleSheet("background:#F4F7FB;");
    layout = new QVBoxLayout(content);
    layout->setContentsMargins(35, 35, 35, 35);
    layout->setSpacing(20);
    scroll->setWidget(content);
    return scroll;
}
static void clearLayout(QVBoxLayout *layout) {
    if (!layout) return;
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        delete item;
    }
}
static QStringList days() {
    return {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    records = {
        {"21CSC101T", "Object Oriented Design", 38, 42},
        {"21CSC201J", "Data Structures", 35, 40},
        {"APP", "Application Programming", 28, 40},
        {"COA", "Computer Organization", 36, 42},
        {"21MAB201T", "Mathematics", 32, 40}
    };
    courses = {
        CourseRecord("21CSC101T", "Object Oriented Design", 3, 3),
        CourseRecord("21CSC201J", "Data Structures", 4, 3),
        CourseRecord("APP", "Application Programming", 3, 3),
        CourseRecord("COA", "Computer Organization", 3, 3),
        CourseRecord("21MAB201T", "Mathematics", 4, 3)
    };
    timetable = {
        {1, "21CSC201J", 0, QTime(9, 0), QTime(10, 0), "A-201"},
        {2, "APP", 0, QTime(11, 0), QTime(12, 0), "Lab-3"},
        {3, "COA", 1, QTime(9, 0), QTime(10, 0), "B-104"},
        {4, "21MAB201T", 2, QTime(10, 0), QTime(11, 0), "A-205"},
        {5, "21CSC101T", 3, QTime(13, 0), QTime(14, 0), "C-102"},
        {6, "21CSC201J", 4, QTime(9, 0), QTime(10, 0), "A-201"}
    };
    nextTimetableId = 7;
    // Example exams; -1 indicates marks have not been entered.
    exams = {
        {1, "21CSC201J", "DSA Internal Test", QDate(2026, 12, 2), QTime(9, 0), "A-201", 50, -1},
        {2, "APP", "APP Practical", QDate(2026, 12, 4), QTime(10, 0), "Lab-3", 100, -1},
        {3, "COA", "COA Quiz", QDate(2026, 11, 10), QTime(14, 0), "B-104", 20, 17}
    };
    nextExamId = 4;
    // Sample notices are demonstration data, not actual university announcements.
    announcements = {
        {1, "Welcome to CampusConnect", "General", "Explore your courses, attendance, timetable, exams and announcements.", QDate(2026, 11, 1), false, false},
        {2, "Internal Assessment Reminder", "Examinations", "Check your examination schedule and verify the venue before each test.", QDate(2026, 11, 12), true, false},
        {3, "Attendance Review", "Academics", "Review attendance regularly and contact your course instructor about discrepancies.", QDate(2026, 11, 14), false, true}
    };
    nextAnnouncementId = 4;
    loadData(); // Load saved data before constructing any UI pages.
    showLoginPage();
}

void MainWindow::showLoginPage() {
    pageStack = nullptr;
    overallLabel = nullptr;
    attendanceList = nullptr;
    courseCountLabel = nullptr;
    creditCountLabel = nullptr;
    courseSearch = nullptr;
    courseList = nullptr;
    dayFilter = nullptr;
    timetableTable = nullptr;
    timetableCountLabel = nullptr;
    announcementTable = nullptr;
    announcementFilter = nullptr;
    announcementSummary = nullptr;
    examTable = nullptr;
    examFilter = nullptr;
    examSummary = nullptr;
    setWindowTitle("CampusConnect | SRM KTR");
    resize(1100, 700);
    setMinimumSize(850, 550);
    auto *central = new QWidget;
    auto *main = new QHBoxLayout(central);
    main->setContentsMargins(0, 0, 0, 0);
    main->setSpacing(0);
    auto *left = new QFrame;
    left->setStyleSheet("background:#101827;");
    auto *branding = new QVBoxLayout(left);
    branding->setContentsMargins(55, 65, 55, 65);
    auto *logo = new QLabel("CAMPUSCONNECT");
    logo->setStyleSheet("color:#34D399;font-size:27px;font-weight:bold;");
    auto *tagline = new QLabel("Your Campus.\nYour Future.");
    tagline->setStyleSheet("color:white;font-size:36px;font-weight:bold;");
    auto *description = new QLabel("A smarter way to manage your\nacademic journey at SRM KTR.");
    description->setStyleSheet("color:#CBD5E1;font-size:16px;");
    branding->addWidget(logo);
    branding->addStretch();
    branding->addWidget(tagline);
    branding->addSpacing(20);
    branding->addWidget(description);
    branding->addStretch();
    auto *right = new QFrame;
    right->setStyleSheet("background:#F8FAFC;");
    auto *form = new QVBoxLayout(right);
    form->setContentsMargins(70, 60, 70, 60);
    form->setSpacing(15);
    username = new QLineEdit;
    username->setPlaceholderText("Enter register number");
    username->setStyleSheet(inputStyle());
    username->setMinimumHeight(48);
    password = new QLineEdit;
    password->setPlaceholderText("Enter password");
    password->setEchoMode(QLineEdit::Password);
    password->setStyleSheet(inputStyle());
    password->setMinimumHeight(48);
    auto *login = new QPushButton("Sign In");
    login->setMinimumHeight(52);
    login->setStyleSheet(greenStyle());
    form->addStretch();
    form->addWidget(txt("Welcome Back!", 30, true));
    form->addWidget(txt("Sign in to your student portal", 15));
    form->addSpacing(20);
    form->addWidget(txt("Register Number"));
    form->addWidget(username);
    form->addWidget(txt("Password"));
    form->addWidget(password);
    form->addSpacing(12);
    form->addWidget(login);
    form->addStretch();
    main->addWidget(left, 1);
    main->addWidget(right, 1);
    connect(login, &QPushButton::clicked, this, [this]() {
        if (username->text() == "student" && password->text() == "1234")
            showStudentDashboard();
        else
            QMessageBox::warning(this, "Login Failed", "Invalid register number or password.");
    });
    connect(password, &QLineEdit::returnPressed, login, &QPushButton::click);
    setCentralWidget(central);
}

void MainWindow::showStudentDashboard() {
    setWindowTitle("CampusConnect | Student Portal");
    resize(1200, 750);
    setMinimumSize(900, 600);
    auto *central = new QWidget;
    auto *main = new QHBoxLayout(central);
    main->setContentsMargins(0, 0, 0, 0);
    main->setSpacing(0);
    auto *sidebar = new QFrame;
    sidebar->setFixedWidth(245);
    sidebar->setStyleSheet("background:#101827;");
    auto *nav = new QVBoxLayout(sidebar);
    nav->setContentsMargins(20, 35, 20, 30);
    nav->setSpacing(12);
    auto *brand = new QLabel("CAMPUSCONNECT");
    brand->setStyleSheet("color:#34D399;font-size:20px;font-weight:bold;");
    nav->addWidget(brand);
    nav->addSpacing(35);
    pageStack = new QStackedWidget;
    pageStack->addWidget(createDashboardPage());
    pageStack->addWidget(createProfilePage());
    pageStack->addWidget(createAttendancePage());
    pageStack->addWidget(createCoursesPage());
    pageStack->addWidget(createTimetablePage());
    pageStack->addWidget(createExaminationsPage());
    pageStack->addWidget(createAnnouncementsPage());
    QStringList menu = {"Dashboard", "My Profile", "Attendance", "My Courses", "Timetable", "Examinations", "Announcements"};
    QString navStyle = "QPushButton {background:transparent;color:#CBD5E1;border:0;text-align:left;"
                       "padding:14px;border-radius:8px;font-size:14px;}"
                       "QPushButton:hover {background:#243247;color:white;}";
    for (int i = 0; i < menu.size(); ++i) {
        auto *button = new QPushButton(menu[i]);
        button->setStyleSheet(navStyle);
        button->setCursor(Qt::PointingHandCursor);
        nav->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, i]() { switchPage(i); });
    }
    nav->addStretch();
    auto *logout = new QPushButton("Logout");
    logout->setMinimumHeight(45);
    logout->setStyleSheet("QPushButton {background:#EF4444;color:white;border-radius:8px;font-weight:bold;}"
                          "QPushButton:hover {background:#DC2626;}");
    nav->addWidget(logout);
    connect(logout, &QPushButton::clicked, this, [this]() { showLoginPage(); });
    main->addWidget(sidebar);
    main->addWidget(pageStack, 1);
    setCentralWidget(central);
    switchPage(0);
}

void MainWindow::switchPage(int index) {
    if (!pageStack) return;
    if (index == 0) refreshDashboard();
    if (index >= 0 && index < pageStack->count()) pageStack->setCurrentIndex(index);
}
void MainWindow::refreshDashboard() {
    if (!pageStack) return;
    QWidget *old = pageStack->widget(0);
    QWidget *updated = createDashboardPage();
    pageStack->insertWidget(0, updated);
    if (old) {
        pageStack->removeWidget(old);
        old->deleteLater();
    }
}

double MainWindow::overallAttendance() const {
    int attended = 0, conducted = 0;
    for (const AttendanceRecord &r : records) {
        attended += r.attended;
        conducted += r.conducted;
    }
    return conducted == 0 ? 0.0 : 100.0 * attended / conducted;
}
void MainWindow::updateAttendance(int index, bool present) {
    if (index < 0 || index >= records.size()) return;
    records[index].conducted++;
    if (present) records[index].attended++;
    saveData();
    refreshAttendance();
}

QWidget *MainWindow::createDashboardPage() {
    QVBoxLayout *layout;
    QScrollArea *scroll = scrollPage(layout);
    layout->addWidget(txt("Student Dashboard", 29, true));
    layout->addWidget(txt("Welcome back! Here's your academic overview.", 15));
    auto *grid = new QGridLayout;
    grid->setSpacing(15);
    grid->addWidget(card("Overall Attendance", QString::number(overallAttendance(), 'f', 1) + "%"), 0, 0);
    grid->addWidget(card("Current CGPA", "8.5"), 0, 1);
    grid->addWidget(card("Registered Courses", QString::number(courses.size())), 0, 2);
    grid->addWidget(card("Current Semester", "3"), 0, 3);
    layout->addLayout(grid);
    layout->addWidget(txt("My Courses", 22, true));
    if (courses.empty()) layout->addWidget(txt("No registered courses."));
    for (const CourseRecord &c : courses) {
        auto *frame = new QFrame;
        frame->setStyleSheet(cardStyle());
        auto *row = new QHBoxLayout(frame);
        row->setContentsMargins(20, 16, 20, 16);
        auto *info = new QVBoxLayout;
        info->addWidget(txt(c.name(), 15, true));
        info->addWidget(txt(c.code() + " | " + QString::number(c.credits()) + " Credits", 12));
        row->addLayout(info);
        row->addStretch();
        row->addWidget(txt("Enrolled", 13, true));
        layout->addWidget(frame);
    }
    layout->addWidget(txt("Latest Announcements", 22, true));
    std::vector<AnnouncementRecord> latest = announcements;
    std::sort(latest.begin(), latest.end(), [](const AnnouncementRecord &a, const AnnouncementRecord &b) {
        return a.date == b.date ? a.id > b.id : a.date > b.date;
    });
    if (latest.empty()) layout->addWidget(txt("No announcements yet."));
    for (int i = 0; i < static_cast<int>(latest.size()) && i < 3; ++i) {
        const AnnouncementRecord &a = latest[i];
        auto *notice = new QFrame;
        notice->setStyleSheet(cardStyle());
        auto *n = new QVBoxLayout(notice);
        n->setContentsMargins(20, 14, 20, 14);
        n->addWidget(txt((a.important ? "IMPORTANT  |  " : "") + a.title, 15, true));
        n->addWidget(txt(a.category + "  â€¢  " + a.date.toString("dd MMM yyyy") +
                             (a.read ? "  â€¢  Read" : "  â€¢  Unread"), 12));
        layout->addWidget(notice);
    }
    layout->addStretch();
    return scroll;
}

QWidget *MainWindow::createProfilePage() {
    QVBoxLayout *layout;
    QScrollArea *scroll = scrollPage(layout);
    layout->addWidget(txt("My Profile", 29, true));
    layout->addWidget(txt("View your personal and academic information.", 15));
    auto *header = new QFrame;
    header->setStyleSheet(cardStyle());
    auto *row = new QHBoxLayout(header);
    row->setContentsMargins(25, 25, 25, 25);
    auto *avatar = new QLabel("S");
    avatar->setFixedSize(80, 80);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("background:#10B981;color:white;border-radius:40px;font-size:34px;font-weight:bold;");
    auto *identity = new QVBoxLayout;
    identity->addWidget(txt("Demo Student", 24, true));
    identity->addWidget(txt("B.Tech - Artificial Intelligence", 15));
    identity->addWidget(txt("SRM Institute of Science and Technology", 13));
    row->addWidget(avatar);
    row->addSpacing(15);
    row->addLayout(identity);
    row->addStretch();
    layout->addWidget(header);
    layout->addWidget(txt("Student Information", 21, true));
    auto *details = new QFrame;
    details->setStyleSheet(cardStyle());
    auto *grid = new QGridLayout(details);
    grid->setContentsMargins(25, 25, 25, 25);
    grid->setSpacing(20);
    QStringList labels = {"Full Name", "Register Number", "Degree", "Department", "Current Year", "Semester", "Campus", "Academic Status"};
    QStringList values = {"Demo Student", "student", "B.Tech", "Artificial Intelligence", "Second Year", "3", "SRM KTR", "Active"};
    for (int i = 0; i < labels.size(); ++i) {
        auto *field = new QVBoxLayout;
        field->addWidget(txt(labels[i], 12));
        field->addWidget(txt(values[i], 16, true));
        grid->addLayout(field, i / 2, i % 2);
    }
    layout->addWidget(details);
    layout->addWidget(txt("Academic Summary", 21, true));
    auto *summary = new QHBoxLayout;
    summary->addWidget(card("CGPA", "8.5"));
    summary->addWidget(card("Semester", "3"));
    layout->addLayout(summary);
    layout->addStretch();
    return scroll;
}

QWidget *MainWindow::createAttendancePage() {
    QVBoxLayout *layout;
    QScrollArea *scroll = scrollPage(layout);
    layout->addWidget(txt("Attendance Management", 29, true));
    layout->addWidget(txt("Monitor attendance and plan upcoming classes.", 15));
    auto *summary = new QFrame;
    summary->setStyleSheet(cardStyle());
    auto *s = new QVBoxLayout(summary);
    s->setContentsMargins(25, 22, 25, 22);
    s->addWidget(txt("Overall Attendance", 15));
    overallLabel = txt("0%", 32, true);
    s->addWidget(overallLabel);
    layout->addWidget(summary);
    layout->addWidget(txt("Subject-wise Attendance", 22, true));
    auto *container = new QWidget;
    attendanceList = new QVBoxLayout(container);
    attendanceList->setContentsMargins(0, 0, 0, 0);
    attendanceList->setSpacing(15);
    layout->addWidget(container);
    auto *note = new QFrame;
    note->setStyleSheet(cardStyle());
    auto *n = new QVBoxLayout(note);
    n->setContentsMargins(20, 20, 20, 20);
    n->addWidget(txt("Attendance Calculator", 20, true));
    n->addWidget(txt("Calculates classes needed to reach 75%, or how many can be missed while maintaining at least 75%."));
    layout->addWidget(note);
    layout->addStretch();
    refreshAttendance();
    return scroll;
}

void MainWindow::refreshAttendance() {
    if (!attendanceList || !overallLabel) return;
    double overall = overallAttendance();
    overallLabel->setText(QString::number(overall, 'f', 1) + "%");
    overallLabel->setStyleSheet(QString("font-size:32px;font-weight:bold;color:%1;")
                                    .arg(overall >= 75 ? "#059669" : "#DC2626"));
    clearLayout(attendanceList);
    for (int i = 0; i < records.size(); ++i) {
        const AttendanceRecord &r = records[i];
        double pct = r.percentage();
        QString color = pct >= 75 ? "#10B981" : pct >= 65 ? "#F59E0B" : "#EF4444";
        auto *frame = new QFrame;
        frame->setStyleSheet(cardStyle());
        auto *box = new QVBoxLayout(frame);
        box->setContentsMargins(22, 20, 22, 20);
        box->setSpacing(12);
        auto *top = new QHBoxLayout;
        top->addWidget(txt(r.subject, 17, true));
        top->addStretch();
        auto *pctLabel = txt(QString::number(pct, 'f', 1) + "%", 19, true);
        pctLabel->setStyleSheet("font-size:19px;font-weight:bold;color:" + color + ";");
        top->addWidget(pctLabel);
        box->addLayout(top);
        box->addWidget(txt(r.code + " | " + QString::number(r.attended) + " / " +
                               QString::number(r.conducted) + " classes attended", 13));
        auto *bar = new QProgressBar;
        bar->setRange(0, 100);
        bar->setValue(qRound(pct));
        bar->setTextVisible(false);
        bar->setFixedHeight(12);
        bar->setStyleSheet("QProgressBar {background:#E2E8F0;border:0;border-radius:6px;}"
                           "QProgressBar::chunk {background:" + color + ";border-radius:6px;}");
        box->addWidget(bar);
        QString advice;
        if (pct < 75) {
            int needed = qMax(0, 3 * r.conducted - 4 * r.attended);
            advice = QString("Attend the next %1 classes to reach 75%.").arg(needed);
        } else {
            int canMiss = qMax(0, (4 * r.attended) / 3 - r.conducted);
            advice = QString("You can miss %1 upcoming classes and remain at or above 75%.").arg(canMiss);
        }
        box->addWidget(txt(advice, 13));
        auto *actions = new QHBoxLayout;
        auto *present = new QPushButton("Mark Present");
        auto *absent = new QPushButton("Mark Absent");
        present->setStyleSheet(greenStyle());
        absent->setStyleSheet(redStyle());
        connect(present, &QPushButton::clicked, this, [this, i]() { updateAttendance(i, true); });
        connect(absent, &QPushButton::clicked, this, [this, i]() { updateAttendance(i, false); });
        actions->addWidget(present);
        actions->addWidget(absent);
        actions->addStretch();
        box->addLayout(actions);
        attendanceList->addWidget(frame);
    }
}

int MainWindow::totalCredits() const {
    int total = 0;
    for (const CourseRecord &c : courses) total += c.credits();
    return total;
}
QString MainWindow::courseName(const QString &code) const {
    for (const CourseRecord &c : courses)
        if (c.code().compare(code, Qt::CaseInsensitive) == 0) return c.name();
    return code;
}
void MainWindow::addCourse(const CourseRecord &course) {
    auto it = std::find_if(courses.begin(), courses.end(), [&course](const CourseRecord &c) {
        return c.code().compare(course.code(), Qt::CaseInsensitive) == 0;
    });
    if (it != courses.end()) throw std::runtime_error("This course code is already registered.");
    courses.push_back(course);
    saveData();
    refreshCourses();
}
void MainWindow::removeCourse(const QString &code) {
    exams.erase(std::remove_if(exams.begin(), exams.end(),
                               [&code](const ExamRecord &e) {
                                   return e.courseCode.compare(code, Qt::CaseInsensitive) == 0;
                               }), exams.end());
    refreshExams();
    timetable.erase(std::remove_if(timetable.begin(), timetable.end(), [&code](const TimetableEntry &e) {
                        return e.courseCode.compare(code, Qt::CaseInsensitive) == 0;
                    }), timetable.end());
    courses.erase(std::remove_if(courses.begin(), courses.end(), [&code](const CourseRecord &c) {
                      return c.code().compare(code, Qt::CaseInsensitive) == 0;
                  }), courses.end());
    saveData();
    refreshCourses();
    refreshTimetable();
}

QWidget *MainWindow::createCoursesPage() {
    QVBoxLayout *layout;
    QScrollArea *scroll = scrollPage(layout);
    auto *heading = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    titles->addWidget(txt("My Courses", 29, true));
    titles->addWidget(txt("Manage your registered semester courses.", 15));
    auto *add = new QPushButton("+ Add Course");
    add->setStyleSheet(greenStyle());
    heading->addLayout(titles);
    heading->addStretch();
    heading->addWidget(add);
    layout->addLayout(heading);
    connect(add, &QPushButton::clicked, this, [this]() { showAddCourseDialog(); });
    auto *summary = new QHBoxLayout;
    auto *countCard = new QFrame;
    countCard->setStyleSheet(cardStyle());
    auto *countBox = new QVBoxLayout(countCard);
    countBox->setContentsMargins(20, 20, 20, 20);
    countBox->addWidget(txt("Registered Courses", 13));
    courseCountLabel = txt("0", 28, true);
    countBox->addWidget(courseCountLabel);
    auto *creditCard = new QFrame;
    creditCard->setStyleSheet(cardStyle());
    auto *creditBox = new QVBoxLayout(creditCard);
    creditBox->setContentsMargins(20, 20, 20, 20);
    creditBox->addWidget(txt("Total Credits", 13));
    creditCountLabel = txt("0", 28, true);
    creditBox->addWidget(creditCountLabel);
    summary->addWidget(countCard);
    summary->addWidget(creditCard);
    layout->addLayout(summary);
    courseSearch = new QLineEdit;
    courseSearch->setPlaceholderText("Search by course name or code...");
    courseSearch->setMinimumHeight(45);
    courseSearch->setStyleSheet(inputStyle());
    layout->addWidget(courseSearch);
    connect(courseSearch, &QLineEdit::textChanged, this, [this](const QString &) { refreshCourses(); });
    layout->addWidget(txt("Registered Subjects", 22, true));
    auto *container = new QWidget;
    courseList = new QVBoxLayout(container);
    courseList->setContentsMargins(0, 0, 0, 0);
    courseList->setSpacing(14);
    layout->addWidget(container);
    layout->addStretch();
    refreshCourses();
    return scroll;
}
void MainWindow::refreshCourses() {
    if (!courseList || !courseCountLabel || !creditCountLabel) return;
    courseCountLabel->setText(QString::number(courses.size()));
    creditCountLabel->setText(QString::number(totalCredits()));
    clearLayout(courseList);
    QString query = courseSearch ? courseSearch->text().trimmed() : QString();
    int matches = 0;
    for (const CourseRecord &c : courses) {
        if (!c.name().contains(query, Qt::CaseInsensitive) &&
            !c.code().contains(query, Qt::CaseInsensitive)) continue;
        ++matches;
        auto *frame = new QFrame;
        frame->setStyleSheet(cardStyle());
        auto *row = new QHBoxLayout(frame);
        row->setContentsMargins(20, 18, 20, 18);
        auto *info = new QVBoxLayout;
        info->addWidget(txt(c.code(), 12));
        info->addWidget(txt(c.name(), 17, true));
        info->addWidget(txt(QString("%1 Credits | Semester %2").arg(c.credits()).arg(c.semester()), 13));
        auto *remove = new QPushButton("Remove");
        remove->setStyleSheet(redStyle());
        QString code = c.code(), name = c.name();
        connect(remove, &QPushButton::clicked, this, [this, code, name]() {
            int linked = 0;
            for (const TimetableEntry &e : timetable)
                if (e.courseCode.compare(code, Qt::CaseInsensitive) == 0) ++linked;
            QString message = "Remove " + name + " (" + code + ")?";
            if (linked > 0) message += QString("\n\nThis also removes %1 linked timetable class(es).").arg(linked);
            if (QMessageBox::question(this, "Remove Course", message, QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No) == QMessageBox::Yes) removeCourse(code);
        });
        row->addLayout(info, 1);
        row->addWidget(txt("Enrolled", 13, true));
        row->addWidget(remove);
        courseList->addWidget(frame);
    }
    if (matches == 0)
        courseList->addWidget(txt(query.isEmpty() ? "No registered courses." : "No matching courses found.", 15));
}
void MainWindow::showAddCourseDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Add Course");
    dialog.setMinimumWidth(420);
    auto *main = new QVBoxLayout(&dialog);
    main->setContentsMargins(25, 25, 25, 25);
    main->addWidget(txt("Register New Course", 22, true));
    auto *form = new QFormLayout;
    form->setSpacing(15);
    auto *code = new QLineEdit;
    code->setPlaceholderText("Example: 21CSC202J");
    code->setStyleSheet(inputStyle());
    auto *name = new QLineEdit;
    name->setPlaceholderText("Course name");
    name->setStyleSheet(inputStyle());
    auto *credits = new QSpinBox;
    credits->setRange(1, 10);
    credits->setValue(3);
    auto *semester = new QSpinBox;
    semester->setRange(1, 12);
    semester->setValue(3);
    form->addRow("Course Code:", code);
    form->addRow("Course Name:", name);
    form->addRow("Credits:", credits);
    form->addRow("Semester:", semester);
    main->addLayout(form);
    main->addWidget(txt("Time format: 24-hour (09:00 = 9 AM, 14:00 = 2 PM). "
                        "Click hours or minutes to change them.", 12));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    main->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog,
            [this, &dialog, code, name, credits, semester]() {
                QString c = code->text().trimmed().toUpper();
                QString n = name->text().trimmed();
                if (c.isEmpty() || n.isEmpty()) {
                    QMessageBox::warning(&dialog, "Invalid Input", "Course code and name cannot be empty.");
                    return;
                }
                try {
                    addCourse(CourseRecord(c, n, credits->value(), semester->value()));
                    dialog.accept();
                } catch (const std::exception &e) {
                    QMessageBox::warning(&dialog, "Registration Failed", QString::fromUtf8(e.what()));
                }
            });
    dialog.exec();
}

bool MainWindow::hasClash(const TimetableEntry &entry, int ignoreId) const {
    for (const TimetableEntry &existing : timetable) {
        if (existing.id == ignoreId || existing.day != entry.day) continue;
        if (entry.start < existing.end && existing.start < entry.end) return true;
    }
    return false;
}

QWidget *MainWindow::createTimetablePage() {
    QVBoxLayout *layout;
    QScrollArea *scroll = scrollPage(layout);
    auto *heading = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    titles->addWidget(txt("Weekly Timetable", 29, true));
    titles->addWidget(txt("Organize classes, rooms and schedules.", 15));
    auto *add = new QPushButton("+ Add Class");
    add->setStyleSheet(greenStyle());
    add->setMinimumHeight(45);
    heading->addLayout(titles);
    heading->addStretch();
    heading->addWidget(add);
    layout->addLayout(heading);
    connect(add, &QPushButton::clicked, this, [this]() { showTimetableDialog(); });
    auto *filterRow = new QHBoxLayout;
    filterRow->addWidget(txt("View:", 15, true));
    dayFilter = new QComboBox;
    dayFilter->addItem("All Days", -1);
    QStringList names = days();
    for (int i = 0; i < names.size(); ++i) dayFilter->addItem(names[i], i);
    configureCombo(dayFilter);
    dayFilter->setMinimumWidth(190);
    dayFilter->setMinimumHeight(42);
    filterRow->addWidget(dayFilter);
    filterRow->addStretch();
    timetableCountLabel = txt("0 classes", 14, true);
    filterRow->addWidget(timetableCountLabel);
    layout->addLayout(filterRow);
    auto *tableCard = new QFrame;
    tableCard->setStyleSheet(cardStyle());
    auto *tableBox = new QVBoxLayout(tableCard);
    tableBox->setContentsMargins(15, 15, 15, 15);
    timetableTable = new QTableWidget;
    timetableTable->setColumnCount(6);
    timetableTable->setHorizontalHeaderLabels({"Day", "Time", "Course", "Room", "Edit", "Delete"});
    timetableTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    timetableTable->setSelectionMode(QAbstractItemView::NoSelection);
    timetableTable->setAlternatingRowColors(true);
    timetableTable->setShowGrid(false);
    timetableTable->verticalHeader()->setVisible(false);
    timetableTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    timetableTable->setStyleSheet("QTableWidget {background:white;color:#172033;border:0;"
                                  "alternate-background-color:#F8FAFC;font-size:13px;}"
                                  "QHeaderView::section {background:#E2E8F0;color:#172033;border:0;"
                                  "padding:10px;font-weight:bold;}");
    tableBox->addWidget(timetableTable);
    layout->addWidget(tableCard);
    auto *tip = new QFrame;
    tip->setStyleSheet(cardStyle());
    auto *tipBox = new QVBoxLayout(tip);
    tipBox->setContentsMargins(20, 20, 20, 20);
    tipBox->addWidget(txt("Smart Scheduling", 19, true));
    tipBox->addWidget(txt("Classes cannot overlap on the same day. New classes must use registered courses. "
                          "Back-to-back classes are allowed."));
    layout->addWidget(tip);
    layout->addStretch();
    connect(dayFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshTimetable(); });
    refreshTimetable();
    return scroll;
}

void MainWindow::showTimetableDialog(int editId) {
    if (courses.empty()) {
        QMessageBox::warning(this, "No Courses", "Register a course in My Courses first.");
        return;
    }
    TimetableEntry original{};
    bool editing = false;
    for (const TimetableEntry &e : timetable) {
        if (e.id == editId) { original = e; editing = true; break; }
    }
    if (editId != -1 && !editing) return;
    QDialog dialog(this);
    dialog.setObjectName("classDialog");
    dialog.setStyleSheet(
        "QDialog#classDialog {background:#F8FAFC;color:#172033;}"
        "QDialog#classDialog QLabel {color:#172033;background:transparent;"
        "border:none;font-size:14px;}"
        "QDialog#classDialog QDialogButtonBox QPushButton {"
        "background:#E2E8F0;color:#172033;border:0;"
        "border-radius:7px;padding:10px 22px;font-weight:bold;}"
        "QDialog#classDialog QDialogButtonBox QPushButton:hover {"
        "background:#CBD5E1;}");
    dialog.setWindowTitle(editing ? "Edit Class" : "Add Class");
    dialog.setMinimumWidth(440);
    auto *main = new QVBoxLayout(&dialog);
    main->setContentsMargins(25, 25, 25, 25);
    main->setSpacing(16);
    main->addWidget(txt(editing ? "Edit Timetable Class" : "Schedule New Class", 22, true));
    auto *form = new QFormLayout;
    form->setSpacing(14);
    auto *courseBox = new QComboBox;
    for (const CourseRecord &c : courses)
        courseBox->addItem(c.name() + " (" + c.code() + ")", c.code());
    auto *dayBox = new QComboBox;
    dayBox->addItems(days());
    auto *startBox = new QTimeEdit;
    startBox->setDisplayFormat("HH:mm");
    startBox->setTime(QTime(9, 0));
    auto *endBox = new QTimeEdit;
    endBox->setDisplayFormat("HH:mm");
    endBox->setTime(QTime(10, 0));
    auto *roomBox = new QLineEdit;
    roomBox->setPlaceholderText("Example: A-201");
    configureCombo(courseBox);
    configureCombo(dayBox);
    startBox->setStyleSheet(inputStyle());
    endBox->setStyleSheet(inputStyle());
    roomBox->setStyleSheet(inputStyle());
    startBox->setCalendarPopup(false);
    startBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    endBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    courseBox->setMinimumHeight(43);
    dayBox->setMinimumHeight(43);
    startBox->setMinimumHeight(43);
    endBox->setMinimumHeight(43);
    roomBox->setMinimumHeight(43);
    if (editing) {
        int idx = courseBox->findData(original.courseCode);
        if (idx >= 0) courseBox->setCurrentIndex(idx);
        dayBox->setCurrentIndex(original.day);
        startBox->setTime(original.start);
        endBox->setTime(original.end);
        roomBox->setText(original.room);
    }
    form->addRow("Course:", courseBox);
    form->addRow("Day:", dayBox);
    form->addRow("Start Time:", startBox);
    form->addRow("End Time:", endBox);
    form->addRow("Classroom:", roomBox);
    main->addLayout(form);
    main->addWidget(txt("Time format: 24-hour (09:00 = 9 AM, 14:00 = 2 PM). "
                        "Click hours or minutes to change them.", 12));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    main->addWidget(buttons);
    if (QPushButton *saveButton = buttons->button(QDialogButtonBox::Save))
        saveButton->setStyleSheet(greenStyle());
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog,
            [this, &dialog, editing, editId, courseBox, dayBox, startBox, endBox, roomBox]() {
                TimetableEntry entry;
                entry.id = editing ? editId : nextTimetableId;
                entry.courseCode = courseBox->currentData().toString();
                entry.day = dayBox->currentIndex();
                entry.start = startBox->time();
                entry.end = endBox->time();
                entry.room = roomBox->text().trimmed();
                if (entry.room.isEmpty()) {
                    QMessageBox::warning(&dialog, "Invalid Room", "Enter a classroom or laboratory.");
                    return;
                }
                if (entry.start >= entry.end) {
                    QMessageBox::warning(&dialog, "Invalid Time", "End time must be later than start time.");
                    return;
                }
                if (hasClash(entry, editing ? editId : -1)) {
                    QMessageBox::warning(&dialog, "Timetable Clash",
                                         "Another class already occupies this time slot on the selected day.");
                    return;
                }
                if (editing) {
                    for (TimetableEntry &e : timetable) {
                        if (e.id == editId) { e = entry; break; }
                    }
                } else {
                    timetable.push_back(entry);
                    ++nextTimetableId;
                }
                saveData();
                refreshTimetable();
                dialog.accept();
            });
    dialog.exec();
}

void MainWindow::deleteTimetableEntry(int id) {
    auto it = std::find_if(timetable.begin(), timetable.end(), [id](const TimetableEntry &e) {
        return e.id == id;
    });
    if (it == timetable.end()) return;
    QString message = "Delete this scheduled class?\n\n" + courseName(it->courseCode) + "\n" +
                      days().at(it->day) + " " + it->start.toString("hh:mm AP");
    if (QMessageBox::question(this, "Delete Class", message, QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes) return;
    timetable.erase(it);
    saveData();
    refreshTimetable();
}

void MainWindow::refreshTimetable() {
    if (!timetableTable || !dayFilter || !timetableCountLabel) return;
    int selectedDay = dayFilter->currentData().toInt();
    std::vector<TimetableEntry> visible;
    for (const TimetableEntry &e : timetable) {
        if (selectedDay == -1 || e.day == selectedDay) visible.push_back(e);
    }
    std::sort(visible.begin(), visible.end(), [](const TimetableEntry &a, const TimetableEntry &b) {
        if (a.day != b.day) return a.day < b.day;
        return a.start < b.start;
    });

    // Corrected class-count line, already integrated.
    timetableCountLabel->setText(
        QString::number(visible.size()) + " classes");

    timetableTable->setRowCount(0);
    for (const TimetableEntry &e : visible) {
        int row = timetableTable->rowCount();
        timetableTable->insertRow(row);
        timetableTable->setRowHeight(row, 55);
        auto put = [this, row](int column, const QString &value) {
            timetableTable->setItem(row, column, new QTableWidgetItem(value));
        };
        put(0, days().at(e.day));
        put(1, e.start.toString("hh:mm AP") + " - " + e.end.toString("hh:mm AP"));
        put(2, courseName(e.courseCode));
        put(3, e.room);
        auto *edit = new QPushButton("Edit");
        edit->setStyleSheet(greenStyle());
        auto *remove = new QPushButton("Delete");
        remove->setStyleSheet(redStyle());
        int id = e.id;
        connect(edit, &QPushButton::clicked, this, [this, id]() { showTimetableDialog(id); });
        connect(remove, &QPushButton::clicked, this, [this, id]() { deleteTimetableEntry(id); });
        timetableTable->setCellWidget(row, 4, edit);
        timetableTable->setCellWidget(row, 5, remove);
    }
}


// ======================================
// STEP 18: EXAMINATIONS AND RESULTS
// ======================================

QString MainWindow::examGrade(double percentage)
{
    // Demonstration grading scale; not an official university policy.
    if (percentage >= 90) return "O";
    if (percentage >= 80) return "A+";
    if (percentage >= 70) return "A";
    if (percentage >= 60) return "B+";
    if (percentage >= 50) return "B";
    return "F";
}

QWidget *MainWindow::createExaminationsPage()
{
    QVBoxLayout *layout;
    QScrollArea *scroll = scrollPage(layout);
    auto *heading = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    titles->addWidget(txt("Examinations & Results", 29, true));
    titles->addWidget(txt("Schedule exams, record marks and view grades.", 15));
    auto *add = new QPushButton("+ Add Exam");
    add->setStyleSheet(greenStyle());
    add->setMinimumHeight(45);
    connect(add, &QPushButton::clicked, this, [this]() { showExamDialog(); });
    heading->addLayout(titles);
    heading->addStretch();
    heading->addWidget(add);
    layout->addLayout(heading);

    auto *summaryCard = new QFrame;
    summaryCard->setStyleSheet(cardStyle());
    auto *summaryLayout = new QVBoxLayout(summaryCard);
    summaryLayout->setContentsMargins(20, 20, 20, 20);
    examSummary = txt("", 15, true);
    summaryLayout->addWidget(examSummary);
    layout->addWidget(summaryCard);

    auto *filterRow = new QHBoxLayout;
    filterRow->addWidget(txt("Show:", 15, true));
    examFilter = new QComboBox;
    examFilter->addItem("All Exams", 0);
    examFilter->addItem("Upcoming / Today", 1);
    examFilter->addItem("Results Entered", 2);
    examFilter->addItem("Results Pending", 3);
    configureCombo(examFilter);
    examFilter->setMinimumHeight(43);
    filterRow->addWidget(examFilter);
    filterRow->addStretch();
    layout->addLayout(filterRow);

    auto *frame = new QFrame;
    frame->setStyleSheet(cardStyle());
    auto *box = new QVBoxLayout(frame);
    box->setContentsMargins(14, 14, 14, 14);
    examTable = new QTableWidget;
    examTable->setColumnCount(8);
    examTable->setHorizontalHeaderLabels(
        {"Date", "Time", "Course / Exam", "Venue", "Marks", "Grade", "Edit", "Delete"});
    examTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    examTable->setSelectionMode(QAbstractItemView::NoSelection);
    examTable->setAlternatingRowColors(true);
    examTable->setShowGrid(false);
    examTable->verticalHeader()->setVisible(false);
    examTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    examTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    examTable->setMinimumHeight(320);
    examTable->setStyleSheet(
        "QTableWidget {background:#FFFFFF;color:#172033;border:0;"
        "alternate-background-color:#F8FAFC;font-size:13px;}"
        "QHeaderView::section {background:#E2E8F0;color:#172033;"
        "border:0;padding:8px;font-weight:bold;}");
    box->addWidget(examTable);
    layout->addWidget(frame);

    auto *note = new QFrame;
    note->setStyleSheet(cardStyle());
    auto *noteLayout = new QVBoxLayout(note);
    noteLayout->addWidget(txt("Demo grading: O 90+, A+ 80+, A 70+, B+ 60+, B 50+, F below 50.", 13));
    noteLayout->addWidget(txt("These are sample rules, not SRM's official grade or CGPA policy.", 12));
    layout->addWidget(note);
    layout->addStretch();

    connect(examFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshExams(); });
    refreshExams();
    return scroll;
}

void MainWindow::showExamDialog(int editId)
{
    if (courses.empty()) {
        QMessageBox::warning(this, "No Courses", "Register a course first.");
        return;
    }
    ExamRecord original{};
    bool editing = false;
    for (const ExamRecord &e : exams) {
        if (e.id == editId) { original = e; editing = true; break; }
    }
    if (editId != -1 && !editing) return;

    QDialog dialog(this);
    dialog.setObjectName("examDialog");
    dialog.setWindowTitle(editing ? "Edit Examination" : "Add Examination");
    dialog.setMinimumWidth(480);
    dialog.setStyleSheet(
        "QDialog#examDialog {background:#F8FAFC;color:#172033;}"
        "QDialog#examDialog QLabel {color:#172033;background:transparent;border:0;}"
        "QDialog#examDialog QDialogButtonBox QPushButton {background:#E2E8F0;"
        "color:#172033;border:0;border-radius:7px;padding:10px 22px;}"
        "QDialog#examDialog QDialogButtonBox QPushButton:hover {background:#CBD5E1;}");
    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(25, 25, 25, 25);
    layout->setSpacing(14);
    layout->addWidget(txt(editing ? "Edit Exam / Result" : "Add Exam / Result", 22, true));
    auto *form = new QFormLayout;
    form->setSpacing(12);

    auto *courseBox = new QComboBox;
    for (const CourseRecord &c : courses)
        courseBox->addItem(c.name() + " (" + c.code() + ")", c.code());
    configureCombo(courseBox);
    auto *titleBox = new QLineEdit;
    titleBox->setPlaceholderText("e.g. Internal Test 1");
    auto *dateBox = new QDateEdit;
    dateBox->setDisplayFormat("dd MMM yyyy");
    dateBox->setCalendarPopup(true);
    dateBox->setDate(QDate::currentDate());
    auto *timeBox = new QTimeEdit;
    timeBox->setDisplayFormat("HH:mm");
    timeBox->setTime(QTime(9, 0));
    auto *venueBox = new QLineEdit;
    venueBox->setPlaceholderText("e.g. A-201");
    auto *maxBox = new QDoubleSpinBox;
    maxBox->setRange(1, 10000);
    maxBox->setDecimals(1);
    maxBox->setValue(100);
    auto *resultCheck = new QCheckBox("Marks available (enter result)");
    resultCheck->setStyleSheet("color:#172033;background:transparent;");
    auto *scoredBox = new QDoubleSpinBox;
    scoredBox->setRange(0, 10000);
    scoredBox->setDecimals(1);
    scoredBox->setEnabled(false);
    QList<QWidget *> examInputs = {
        titleBox,
        dateBox,
        timeBox,
        venueBox,
        maxBox,
        scoredBox
    };

    for (QWidget *w : examInputs) {
        w->setStyleSheet(inputStyle());
        w->setMinimumHeight(42);
    }
    // Ensure Windows dark-mode palette cannot override editor text/background.
    QPalette examPalette = dateBox->palette();
    examPalette.setColor(QPalette::Base, QColor("#FFFFFF"));
    examPalette.setColor(QPalette::Text, QColor("#172033"));
    examPalette.setColor(QPalette::Window, QColor("#FFFFFF"));
    examPalette.setColor(QPalette::WindowText, QColor("#172033"));
    examPalette.setColor(QPalette::Button, QColor("#F1F5F9"));
    examPalette.setColor(QPalette::ButtonText, QColor("#172033"));
    examPalette.setColor(QPalette::Highlight, QColor("#10B981"));
    examPalette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    for (QWidget *w : {static_cast<QWidget *>(dateBox),
                       static_cast<QWidget *>(timeBox),
                       static_cast<QWidget *>(maxBox),
                       static_cast<QWidget *>(scoredBox)}) {
        w->setPalette(examPalette);
    }
    dateBox->setCalendarPopup(false); // Use clear date spin editor, not OS dark calendar.
    connect(resultCheck, &QCheckBox::toggled, scoredBox, &QWidget::setEnabled);
    if (editing) {
        int index = courseBox->findData(original.courseCode);
        if (index >= 0) courseBox->setCurrentIndex(index);
        titleBox->setText(original.title);
        dateBox->setDate(original.date);
        timeBox->setTime(original.time);
        venueBox->setText(original.venue);
        maxBox->setValue(original.maximum);
        resultCheck->setChecked(original.scored >= 0);
        if (original.scored >= 0) scoredBox->setValue(original.scored);
    }
    form->addRow("Course:", courseBox);
    form->addRow("Exam title:", titleBox);
    form->addRow("Exam date:", dateBox);
    form->addRow("Start time:", timeBox);
    form->addRow("Venue:", venueBox);
    form->addRow("Maximum marks:", maxBox);
    form->addRow("Result:", resultCheck);
    form->addRow("Marks scored:", scoredBox);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    if (auto *save = buttons->button(QDialogButtonBox::Save)) save->setStyleSheet(greenStyle());
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog,
            [this, &dialog, editing, editId, courseBox, titleBox, dateBox,
             timeBox, venueBox, maxBox, resultCheck, scoredBox]() {
                const QString title = titleBox->text().trimmed();
                const QString venue = venueBox->text().trimmed();
                if (title.isEmpty() || venue.isEmpty()) {
                    QMessageBox::warning(&dialog, "Missing Details", "Enter an exam title and venue.");
                    return;
                }
                if (resultCheck->isChecked() && scoredBox->value() > maxBox->value()) {
                    QMessageBox::warning(&dialog, "Invalid Marks", "Scored marks cannot exceed maximum marks.");
                    return;
                }
                ExamRecord updated{editing ? editId : nextExamId,
                                   courseBox->currentData().toString(), title, dateBox->date(),
                                   timeBox->time(), venue, maxBox->value(),
                                   resultCheck->isChecked() ? scoredBox->value() : -1.0};
                if (editing) {
                    for (ExamRecord &e : exams) {
                        if (e.id == editId) { e = updated; break; }
                    }
                } else {
                    exams.push_back(updated);
                    ++nextExamId;
                }
                saveData();
                refreshExams();
                dialog.accept();
            });
    dialog.exec();
}

void MainWindow::deleteExam(int id)
{
    auto it = std::find_if(exams.begin(), exams.end(),
                           [id](const ExamRecord &e) { return e.id == id; });
    if (it == exams.end()) return;
    if (QMessageBox::question(this, "Delete Exam",
                              "Delete '" + it->title + "' for " + courseName(it->courseCode) + "?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    exams.erase(it);
    saveData();
    refreshExams();
}

void MainWindow::refreshExams()
{
    if (!examTable || !examFilter || !examSummary) return;
    int graded = 0;
    double totalPercentage = 0;
    for (const ExamRecord &e : exams) {
        if (e.scored >= 0) {
            ++graded;
            totalPercentage += 100.0 * e.scored / e.maximum;
        }
    }
    examSummary->setText(QString("Total Exams: %1    |    Results Entered: %2    |    Average: %3")
                             .arg(exams.size()).arg(graded)
                             .arg(graded ? QString::number(totalPercentage / graded, 'f', 1) + "%" : "N/A"));

    std::vector<ExamRecord> visible;
    const int filter = examFilter->currentData().toInt();
    for (const ExamRecord &e : exams) {
        if (filter == 1 && e.date < QDate::currentDate()) continue;
        if (filter == 2 && e.scored < 0) continue;
        if (filter == 3 && e.scored >= 0) continue;
        visible.push_back(e);
    }
    std::sort(visible.begin(), visible.end(),
              [](const ExamRecord &a, const ExamRecord &b) {
                  return a.date == b.date ? a.time < b.time : a.date < b.date;
              });
    examTable->setRowCount(0);
    for (const ExamRecord &e : visible) {
        int row = examTable->rowCount();
        examTable->insertRow(row);
        examTable->setRowHeight(row, 56);
        auto put = [this, row](int col, const QString &value) {
            examTable->setItem(row, col, new QTableWidgetItem(value));
        };
        put(0, e.date.toString("dd MMM yyyy"));
        put(1, e.time.toString("HH:mm"));
        put(2, courseName(e.courseCode) + " / " + e.title);
        put(3, e.venue);
        put(4, e.scored < 0 ? "Pending" :
                   QString::number(e.scored, 'f', 1) + " / " + QString::number(e.maximum, 'f', 1));
        put(5, e.scored < 0 ? "-" : examGrade(100.0 * e.scored / e.maximum));
        auto *edit = new QPushButton("Edit");
        edit->setStyleSheet(greenStyle());
        auto *remove = new QPushButton("Delete");
        remove->setStyleSheet(redStyle());
        int id = e.id;
        connect(edit, &QPushButton::clicked, this, [this, id]() { showExamDialog(id); });
        connect(remove, &QPushButton::clicked, this, [this, id]() { deleteExam(id); });
        examTable->setCellWidget(row, 6, edit);
        examTable->setCellWidget(row, 7, remove);
    }
}


// ======================================
// STEP 19: ANNOUNCEMENTS AND NOTIFICATIONS
// ======================================
QWidget *MainWindow::createAnnouncementsPage()
{
    QVBoxLayout *layout;
    QScrollArea *scroll = scrollPage(layout);
    auto *heading = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    titles->addWidget(txt("Announcements & Notifications", 29, true));
    titles->addWidget(txt("Create notices, filter updates and mark them as read.", 15));
    heading->addLayout(titles);
    heading->addStretch();
    auto *add = new QPushButton("+ New Announcement");
    add->setStyleSheet(greenStyle());
    add->setMinimumHeight(44);
    connect(add, &QPushButton::clicked, this, [this]() { showAnnouncementDialog(); });
    heading->addWidget(add);
    layout->addLayout(heading);

    auto *summaryCard = new QFrame;
    summaryCard->setStyleSheet(cardStyle());
    auto *summaryLayout = new QVBoxLayout(summaryCard);
    summaryLayout->setContentsMargins(20, 20, 20, 20);
    announcementSummary = txt("", 15, true);
    summaryLayout->addWidget(announcementSummary);
    layout->addWidget(summaryCard);

    auto *filterRow = new QHBoxLayout;
    filterRow->addWidget(txt("Show:", 15, true));
    announcementFilter = new QComboBox;
    announcementFilter->addItem("All Announcements", 0);
    announcementFilter->addItem("Unread", 1);
    announcementFilter->addItem("Important", 2);
    announcementFilter->addItem("Read", 3);
    configureCombo(announcementFilter);
    announcementFilter->setMinimumHeight(43);
    filterRow->addWidget(announcementFilter);
    filterRow->addStretch();
    layout->addLayout(filterRow);

    auto *frame = new QFrame;
    frame->setStyleSheet(cardStyle());
    auto *box = new QVBoxLayout(frame);
    box->setContentsMargins(14, 14, 14, 14);
    announcementTable = new QTableWidget;
    announcementTable->setColumnCount(7);
    announcementTable->setHorizontalHeaderLabels(
        {"Date", "Category", "Announcement", "Priority", "Read / Unread", "Edit", "Delete"});
    announcementTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    announcementTable->setSelectionMode(QAbstractItemView::NoSelection);
    announcementTable->setAlternatingRowColors(true);
    announcementTable->setShowGrid(false);
    announcementTable->verticalHeader()->setVisible(false);
    announcementTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    announcementTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    announcementTable->setMinimumHeight(350);
    announcementTable->setStyleSheet(
        "QTableWidget {background:#FFFFFF;color:#172033;border:0;"
        "alternate-background-color:#F8FAFC;font-size:13px;}"
        "QHeaderView::section {background:#E2E8F0;color:#172033;"
        "border:0;padding:8px;font-weight:bold;}");
    box->addWidget(announcementTable);
    layout->addWidget(frame);
    layout->addWidget(txt("Demo notices only. New notices are stored in memory until the app closes.", 12));
    layout->addStretch();
    connect(announcementFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshAnnouncements(); });
    refreshAnnouncements();
    return scroll;
}

void MainWindow::showAnnouncementDialog(int editId)
{
    AnnouncementRecord original{};
    bool editing = false;
    for (const AnnouncementRecord &a : announcements) {
        if (a.id == editId) { original = a; editing = true; break; }
    }
    if (editId != -1 && !editing) return;

    QDialog dialog(this);
    dialog.setObjectName("announcementDialog");
    dialog.setWindowTitle(editing ? "Edit Announcement" : "New Announcement");
    dialog.setMinimumWidth(510);
    dialog.setStyleSheet(
        "QDialog#announcementDialog {background:#F8FAFC;color:#172033;}"
        "QDialog#announcementDialog QLabel {color:#172033;background:transparent;border:0;}"
        "QDialog#announcementDialog QCheckBox {color:#172033;background:transparent;}"
        "QDialog#announcementDialog QDialogButtonBox QPushButton {background:#E2E8F0;"
        "color:#172033;border:0;border-radius:7px;padding:10px 22px;}");
    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(25, 25, 25, 25);
    layout->setSpacing(14);
    layout->addWidget(txt(editing ? "Edit Announcement" : "Create Announcement", 22, true));
    auto *form = new QFormLayout;
    form->setSpacing(12);
    auto *titleBox = new QLineEdit;
    titleBox->setPlaceholderText("e.g. Library Timing Update");
    auto *categoryBox = new QComboBox;
    categoryBox->addItems({"General", "Academics", "Examinations", "Events", "Administration"});
    configureCombo(categoryBox);
    auto *dateBox = new QDateEdit;
    dateBox->setDisplayFormat("dd MMM yyyy");
    dateBox->setCalendarPopup(false);
    dateBox->setDate(QDate::currentDate());
    auto *messageBox = new QTextEdit;
    messageBox->setPlaceholderText("Write the announcement details...");
    messageBox->setMinimumHeight(110);
    messageBox->setStyleSheet(
        "QTextEdit {background:#FFFFFF;color:#172033;border:1px solid #CBD5E1;"
        "border-radius:7px;padding:8px;selection-background-color:#10B981;}"
        "QTextEdit:focus {border:2px solid #10B981;}");
    auto *importantCheck = new QCheckBox("Mark as important");
    for (QWidget *w : QList<QWidget *>{titleBox, dateBox}) {
        w->setStyleSheet(inputStyle());
        w->setMinimumHeight(43);
    }
    QPalette light = dateBox->palette();
    light.setColor(QPalette::Base, QColor("#FFFFFF"));
    light.setColor(QPalette::Text, QColor("#172033"));
    light.setColor(QPalette::Window, QColor("#FFFFFF"));
    light.setColor(QPalette::WindowText, QColor("#172033"));
    light.setColor(QPalette::Highlight, QColor("#10B981"));
    light.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    dateBox->setPalette(light);
    if (editing) {
        titleBox->setText(original.title);
        categoryBox->setCurrentText(original.category);
        dateBox->setDate(original.date);
        messageBox->setPlainText(original.message);
        importantCheck->setChecked(original.important);
    }
    form->addRow("Title:", titleBox);
    form->addRow("Category:", categoryBox);
    form->addRow("Date:", dateBox);
    form->addRow("Message:", messageBox);
    form->addRow("Priority:", importantCheck);
    layout->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    if (auto *save = buttons->button(QDialogButtonBox::Save)) save->setStyleSheet(greenStyle());
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog,
            [this, &dialog, editing, editId, titleBox, categoryBox, dateBox, messageBox, importantCheck]() {
                QString title = titleBox->text().trimmed();
                QString message = messageBox->toPlainText().trimmed();
                if (title.isEmpty() || message.isEmpty()) {
                    QMessageBox::warning(&dialog, "Missing Details", "Enter a title and message.");
                    return;
                }
                bool wasRead = false;
                if (editing) {
                    for (const AnnouncementRecord &a : announcements)
                        if (a.id == editId) { wasRead = a.read; break; }
                }
                AnnouncementRecord updated{editing ? editId : nextAnnouncementId,
                                           title, categoryBox->currentText(), message, dateBox->date(),
                                           importantCheck->isChecked(), wasRead};
                if (editing) {
                    for (AnnouncementRecord &a : announcements)
                        if (a.id == editId) { a = updated; break; }
                } else {
                    announcements.push_back(updated);
                    ++nextAnnouncementId;
                }
                saveData();
                refreshAnnouncements();
                dialog.accept();
            });
    dialog.exec();
}

void MainWindow::deleteAnnouncement(int id)
{
    auto it = std::find_if(announcements.begin(), announcements.end(),
                           [id](const AnnouncementRecord &a) { return a.id == id; });
    if (it == announcements.end()) return;
    if (QMessageBox::question(this, "Delete Announcement",
                              "Delete '" + it->title + "'?", QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes) return;
    announcements.erase(it);
    saveData();
    refreshAnnouncements();
}

void MainWindow::toggleAnnouncementRead(int id)
{
    for (AnnouncementRecord &a : announcements) {
        if (a.id == id) { a.read = !a.read; break; }
    }
    saveData();
    refreshAnnouncements();
}

void MainWindow::refreshAnnouncements()
{
    if (!announcementTable || !announcementFilter || !announcementSummary) return;
    int unread = 0, important = 0;
    for (const AnnouncementRecord &a : announcements) {
        if (!a.read) ++unread;
        if (a.important) ++important;
    }
    announcementSummary->setText(QString("Total: %1    |    Unread: %2    |    Important: %3")
                                     .arg(announcements.size()).arg(unread).arg(important));
    const int filter = announcementFilter->currentData().toInt();
    std::vector<AnnouncementRecord> visible;
    for (const AnnouncementRecord &a : announcements) {
        if (filter == 1 && a.read) continue;
        if (filter == 2 && !a.important) continue;
        if (filter == 3 && !a.read) continue;
        visible.push_back(a);
    }
    std::sort(visible.begin(), visible.end(),
              [](const AnnouncementRecord &a, const AnnouncementRecord &b) {
                  return a.date == b.date ? a.id > b.id : a.date > b.date;
              });
    announcementTable->setRowCount(0);
    for (const AnnouncementRecord &a : visible) {
        int row = announcementTable->rowCount();
        announcementTable->insertRow(row);
        announcementTable->setRowHeight(row, 78);
        auto put = [this, row](int col, const QString &value) {
            announcementTable->setItem(row, col, new QTableWidgetItem(value));
        };
        put(0, a.date.toString("dd MMM yyyy"));
        put(1, a.category);
        put(2, a.title + "\n" + a.message);
        put(3, a.important ? "IMPORTANT" : "Normal");
        auto *toggle = new QPushButton(a.read ? "Mark Unread" : "Mark Read");
        toggle->setStyleSheet(greenStyle());
        auto *edit = new QPushButton("Edit");
        edit->setStyleSheet(greenStyle());
        auto *remove = new QPushButton("Delete");
        remove->setStyleSheet(redStyle());
        int id = a.id;
        connect(toggle, &QPushButton::clicked, this, [this, id]() { toggleAnnouncementRead(id); });
        connect(edit, &QPushButton::clicked, this, [this, id]() { showAnnouncementDialog(id); });
        connect(remove, &QPushButton::clicked, this, [this, id]() { deleteAnnouncement(id); });
        announcementTable->setCellWidget(row, 4, toggle);
        announcementTable->setCellWidget(row, 5, edit);
        announcementTable->setCellWidget(row, 6, remove);
    }
}


// Step 20: atomic JSON persistence. File location is the user's writable
// application-data directory, not the Qt Creator build directory.
QString MainWindow::dataFilePath() const {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(dir).filePath("campusconnect_data.json");
}

void MainWindow::saveData() const {
    const QString path = dataFilePath();
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        qWarning("CampusConnect: could not create data directory");
        return;
    }
    QJsonObject root;
    root["version"] = 1;
    QJsonArray attendanceJson;
    for (const auto &r : records) {
        QJsonObject o;
        o["code"] = r.code; o["subject"] = r.subject;
        o["attended"] = r.attended; o["conducted"] = r.conducted;
        attendanceJson.append(o);
    }
    root["attendance"] = attendanceJson;
    QJsonArray coursesJson;
    for (const auto &c : courses) {
        QJsonObject o;
        o["code"] = c.code(); o["name"] = c.name();
        o["credits"] = c.credits(); o["semester"] = c.semester();
        coursesJson.append(o);
    }
    root["courses"] = coursesJson;
    QJsonArray timetableJson;
    for (const auto &t : timetable) {
        QJsonObject o;
        o["id"] = t.id; o["courseCode"] = t.courseCode; o["day"] = t.day;
        o["start"] = t.start.toString("HH:mm");
        o["end"] = t.end.toString("HH:mm"); o["room"] = t.room;
        timetableJson.append(o);
    }
    root["timetable"] = timetableJson;
    root["nextTimetableId"] = nextTimetableId;
    QJsonArray examsJson;
    for (const auto &e : exams) {
        QJsonObject o;
        o["id"] = e.id; o["courseCode"] = e.courseCode; o["title"] = e.title;
        o["date"] = e.date.toString(Qt::ISODate);
        o["time"] = e.time.toString("HH:mm"); o["venue"] = e.venue;
        o["maximum"] = e.maximum; o["scored"] = e.scored;
        examsJson.append(o);
    }
    root["exams"] = examsJson;
    root["nextExamId"] = nextExamId;
    QJsonArray noticesJson;
    for (const auto &a : announcements) {
        QJsonObject o;
        o["id"] = a.id; o["title"] = a.title; o["category"] = a.category;
        o["message"] = a.message; o["date"] = a.date.toString(Qt::ISODate);
        o["important"] = a.important; o["read"] = a.read;
        noticesJson.append(o);
    }
    root["announcements"] = noticesJson;
    root["nextAnnouncementId"] = nextAnnouncementId;
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("CampusConnect: unable to open data file for saving");
        return;
    }
    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        qWarning("CampusConnect: failed to save data file");
    }
}

void MainWindow::loadData() {
    QFile file(dataFilePath());
    if (!file.exists()) return; // First launch uses the sample records.
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("CampusConnect: unable to read saved data");
        return;
    }
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning("CampusConnect: invalid JSON; keeping sample data");
        return;
    }
    const QJsonObject root = doc.object();
    if (root.value("version").toInt() != 1 ||
        !root.value("attendance").isArray() || !root.value("courses").isArray() ||
        !root.value("timetable").isArray() || !root.value("exams").isArray() ||
        !root.value("announcements").isArray()) {
        qWarning("CampusConnect: unsupported or incomplete saved data");
        return;
    }
    QVector<AttendanceRecord> loadedAttendance;
    std::vector<CourseRecord> loadedCourses;
    std::vector<TimetableEntry> loadedTimetable;
    std::vector<ExamRecord> loadedExams;
    std::vector<AnnouncementRecord> loadedNotices;
    for (const auto &v : root.value("attendance").toArray()) {
        if (!v.isObject()) return;
        auto o = v.toObject();
        int attended = o.value("attended").toInt(-1);
        int conducted = o.value("conducted").toInt(-1);
        if (attended < 0 || conducted < attended) return;
        loadedAttendance.append({o.value("code").toString(), o.value("subject").toString(), attended, conducted});
    }
    for (const auto &v : root.value("courses").toArray()) {
        if (!v.isObject()) return;
        auto o = v.toObject();
        QString code = o.value("code").toString();
        if (code.isEmpty()) return;
        loadedCourses.emplace_back(code, o.value("name").toString(),
                                   o.value("credits").toInt(), o.value("semester").toInt());
    }
    int maxTimetableId = 0, maxExamId = 0, maxNoticeId = 0;
    for (const auto &v : root.value("timetable").toArray()) {
        if (!v.isObject()) return;
        auto o = v.toObject();
        TimetableEntry t{o.value("id").toInt(), o.value("courseCode").toString(),
                         o.value("day").toInt(-1), QTime::fromString(o.value("start").toString(), "HH:mm"),
                         QTime::fromString(o.value("end").toString(), "HH:mm"), o.value("room").toString()};
        if (t.id <= 0 || t.day < 0 || t.day > 5 || !t.start.isValid() ||
            !t.end.isValid() || t.start >= t.end) return;
        maxTimetableId = std::max(maxTimetableId, t.id);
        loadedTimetable.push_back(t);
    }
    for (const auto &v : root.value("exams").toArray()) {
        if (!v.isObject()) return;
        auto o = v.toObject();
        ExamRecord e{o.value("id").toInt(), o.value("courseCode").toString(),
                     o.value("title").toString(), QDate::fromString(o.value("date").toString(), Qt::ISODate),
                     QTime::fromString(o.value("time").toString(), "HH:mm"), o.value("venue").toString(),
                     o.value("maximum").toDouble(), o.value("scored").toDouble(-1)};
        if (e.id <= 0 || !e.date.isValid() || !e.time.isValid() || e.maximum <= 0 ||
            e.scored > e.maximum || e.scored < -1) return;
        maxExamId = std::max(maxExamId, e.id);
        loadedExams.push_back(e);
    }
    for (const auto &v : root.value("announcements").toArray()) {
        if (!v.isObject()) return;
        auto o = v.toObject();
        AnnouncementRecord a{o.value("id").toInt(), o.value("title").toString(),
                             o.value("category").toString(), o.value("message").toString(),
                             QDate::fromString(o.value("date").toString(), Qt::ISODate),
                             o.value("important").toBool(), o.value("read").toBool()};
        if (a.id <= 0 || !a.date.isValid()) return;
        maxNoticeId = std::max(maxNoticeId, a.id);
        loadedNotices.push_back(a);
    }
    records = std::move(loadedAttendance);
    courses = std::move(loadedCourses);
    timetable = std::move(loadedTimetable);
    exams = std::move(loadedExams);
    announcements = std::move(loadedNotices);
    nextTimetableId = std::max(maxTimetableId + 1, root.value("nextTimetableId").toInt(1));
    nextExamId = std::max(maxExamId + 1, root.value("nextExamId").toInt(1));
    nextAnnouncementId = std::max(maxNoticeId + 1, root.value("nextAnnouncementId").toInt(1));
}