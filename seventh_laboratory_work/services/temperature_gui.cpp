#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QComboBox>
#include "temperature_graphs.hpp"
#include "database_manager.hpp"
#include <pqxx/pqxx>

std::vector<std::string> getAvailableDates(pqxx::connection& conn, const std::string& table) {
    pqxx::work txn(conn);
    pqxx::result r = txn.exec("SELECT DISTINCT timestamp::date FROM " + table);
    std::vector<std::string> dates;
    for (const auto& row : r) {
        dates.push_back(row[0].as<std::string>());
    }
    return dates;
}

bool hasData(pqxx::connection& conn, const std::string& table) {
    pqxx::work txn(conn);
    pqxx::result r = txn.exec("SELECT 1 FROM " + table + " LIMIT 1");
    return !r.empty();
}

void addDatesToComboBox(QComboBox* comboBox, const std::vector<std::string>& dates) {
    for (const auto& date : dates) {
        comboBox->addItem(QString::fromStdString(date));
    }
}

void connectComboBoxToButton(QComboBox* startComboBox, QComboBox* endComboBox, QPushButton* button) {
    QObject::connect(startComboBox, &QComboBox::currentTextChanged, [=](const QString &text) {
        button->setEnabled(!text.isEmpty() && !endComboBox->currentText().isEmpty());
    });
    QObject::connect(endComboBox, &QComboBox::currentTextChanged, [=](const QString &text) {
        button->setEnabled(!text.isEmpty() && !startComboBox->currentText().isEmpty());
    });
}

void setupButtonAvailability(pqxx::connection* conn, QPushButton* averageTempButton, QPushButton* hourlyAverageTempButton) {
    if (conn) {
        averageTempButton->setEnabled(hasData(*conn, "daily_average_log"));
        hourlyAverageTempButton->setEnabled(hasData(*conn, "hourly_average_log"));
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Temperature Graphs");

    QVBoxLayout *layout = new QVBoxLayout;

    QPushButton *currentTempButton = new QPushButton("Show Current Temperature Graph");
    QPushButton *averageTempButton = new QPushButton("Show Average Temperature Graph");
    QPushButton *hourlyAverageTempButton = new QPushButton("Show Hourly Average Temperature Graph");

    layout->addWidget(currentTempButton);
    layout->addWidget(averageTempButton);
    layout->addWidget(hourlyAverageTempButton);

    QObject::connect(currentTempButton, &QPushButton::clicked, []() {
        generateCurrentTemperatureGraph();
    });

    QComboBox *startDateComboBox = new QComboBox;
    QComboBox *endDateComboBox = new QComboBox;
    QComboBox *hourlyStartDateComboBox = new QComboBox;
    QComboBox *hourlyEndDateComboBox = new QComboBox;

    auto conn = ConnectToDatabase();

    setupButtonAvailability(conn.get(), averageTempButton, hourlyAverageTempButton);

    if (conn) {
        auto dates = getAvailableDates(*conn, "daily_average_log");
        addDatesToComboBox(startDateComboBox, dates);
        addDatesToComboBox(endDateComboBox, dates);
    }

    connectComboBoxToButton(startDateComboBox, endDateComboBox, averageTempButton);

    if (conn) {
        auto hourlyDates = getAvailableDates(*conn, "hourly_average_log");
        addDatesToComboBox(hourlyStartDateComboBox, hourlyDates);
        addDatesToComboBox(hourlyEndDateComboBox, hourlyDates);
    }

    connectComboBoxToButton(hourlyStartDateComboBox, hourlyEndDateComboBox, hourlyAverageTempButton);

    layout->addWidget(startDateComboBox);
    layout->addWidget(endDateComboBox);

    QObject::connect(averageTempButton, &QPushButton::clicked, [=]() {
        QString startDate = startDateComboBox->currentText();
        QString endDate = endDateComboBox->currentText();
        generateDailyAverageTemperatureGraph(startDate.toStdString(), endDate.toStdString());
    });

    layout->addWidget(hourlyStartDateComboBox);
    layout->addWidget(hourlyEndDateComboBox);

    QObject::connect(hourlyAverageTempButton, &QPushButton::clicked, [=]() {
        QString startDate = hourlyStartDateComboBox->currentText();
        QString endDate = hourlyEndDateComboBox->currentText();
        generateHourlyAverageTemperatureGraph(startDate.toStdString(), endDate.toStdString());
    });

    window.setLayout(layout);
    window.show();

    return app.exec();
} 