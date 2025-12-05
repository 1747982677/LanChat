#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>
#include <QVariant>

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }
    
    void load(const QString& filename = "config.ini");
    
    QString getString(const QString& key, const QString& defaultValue = QString()) const;
    int getInt(const QString& key, int defaultValue = 0) const;
    bool getBool(const QString& key, bool defaultValue = false) const;
    
    void setString(const QString& key, const QString& value);
    void setInt(const QString& key, int value);
    void setBool(const QString& key, bool value);
    
    void remove(const QString& key); // Added remove method declaration
    
    void save();
    
private:
    Config() : settings(nullptr) {}
    ~Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    QSettings* settings;
};

#endif // CONFIG_H
