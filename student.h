
#ifndef STUDENT_H
#define STUDENT_H

#include <QString>
#include <vector>
#include <stdexcept>

// Course structure stores academic information
struct Course {
    QString code;
    QString name;
    int attended;
    int conducted;
    QString grade;

    // Calculate attendance percentage
    double attendance() const {
        if (conducted == 0)
            return 0;

        return (100.0 * attended) / conducted;
    }
};

// Student class demonstrates OOP concepts
class Student {
private:
    // Encapsulation: private data members
    QString name;
    QString registerNumber;
    QString department;
    int semester;
    double cgpa;

    // STL vector stores multiple courses
    std::vector<Course> courses;

public:
    // Parameterized constructor
    Student(QString n, QString reg, QString dept,
            int sem, double g)
        : name(n),
        registerNumber(reg),
        department(dept),
        semester(sem),
        cgpa(g) {}

    // Getter functions
    QString getName() const {
        return name;
    }

    QString getRegisterNumber() const {
        return registerNumber;
    }

    QString getDepartment() const {
        return department;
    }

    int getSemester() const {
        return semester;
    }

    double getCGPA() const {
        return cgpa;
    }

    // Add a course to the student's list
    void addCourse(const Course &course) {
        for (const Course &c : courses) {
            if (c.code == course.code) {
                throw std::runtime_error(
                    "Course already registered"
                    );
            }
        }

        courses.push_back(course);
    }

    // Return all registered courses
    const std::vector<Course>& getCourses() const {
        return courses;
    }

    // Calculate overall attendance
    double getOverallAttendance() const {
        int totalAttended = 0;
        int totalConducted = 0;

        for (const Course &c : courses) {
            totalAttended += c.attended;
            totalConducted += c.conducted;
        }

        if (totalConducted == 0)
            return 0;

        return (100.0 * totalAttended) / totalConducted;
    }
};

#endif // STUDENT_H
