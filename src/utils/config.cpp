#include "config.h"

void Config::load(const QString& filename) {
    if (settings) {
        delete settings;
    }
    
    settings = new QSettings(filename, QSettings::IniFormat);
}

Config::~Config() {
    if (settings) {
        delete settings;
        settings = nullptr;
    }
}

QString Config::getString(const QString& key, const QString& defaultValue) const {
    if (!settings) return defaultValue;
    return settings->value(key, defaultValue).toString();
}

int Config::getInt(const QString& key, int defaultValue) const {
    if (!settings) return defaultValue;
    return settings->value(key, defaultValue).toInt();
}

bool Config::getBool(const QString& key, bool defaultValue) const {
    if (!settings) return defaultValue;
    return settings->value(key, defaultValue).toBool();
}

void Config::setString(const QString& key, const QString& value) {
    if (settings) {
        settings->setValue(key, value);
    }
}

void Config::setInt(const QString& key, int value) {
    if (settings) {
        settings->setValue(key, value);
    }
}

void Config::setBool(const QString& key, bool value) {
    if (settings) {
        settings->setValue(key, value);
    }
}

void Config::remove(const QString& key) {
    if (settings) {
        settings->remove(key);
    }
}

void Config::save() {
    if (settings) {
        settings->sync();
    }
}
