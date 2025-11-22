#include <QtWidgets/QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QWidget window;
    window.setWindowTitle("LanChat");
    window.resize(800, 600);
    
    QVBoxLayout* layout = new QVBoxLayout(&window);
    QLabel* label = new QLabel("LanChat is working!", &window);
    label->setAlignment(Qt::AlignCenter);
    
    QFont font = label->font();
    font.setPointSize(14);
    font.setBold(true);
    label->setFont(font);
    
    layout->addWidget(label);
    
    window.show();
    
    return app.exec();
}
