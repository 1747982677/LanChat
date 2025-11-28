// Simple test version of main.cpp
// Use this if you want to test Qt setup first
// Replace src/main.cpp with this content temporarily

#include <QtWidgets/QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Create simple window
    QWidget window;
    window.setWindowTitle("LanChat - Simple Test");
    window.resize(800, 600);
    
    // Create layout
    QVBoxLayout* layout = new QVBoxLayout(&window);
    
    // Add title label
    QLabel* titleLabel = new QLabel("LanChat v1.0", &window);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    // Add status label
    QLabel* statusLabel = new QLabel("Application is running successfully!", &window);
    statusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel);
    
    // Add stretch
    layout->addStretch();
    
    // Add close button
    QPushButton* closeButton = new QPushButton("Close", &window);
    layout->addWidget(closeButton);
    
    // Connect close button
    QObject::connect(closeButton, &QPushButton::clicked, &app, &QApplication::quit);
    
    window.show();
    
    return app.exec();
}
