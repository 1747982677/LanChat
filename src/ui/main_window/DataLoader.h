#ifndef DATALOADER_H
#define DATALOADER_H

#include <QList>
#include <QString>
#include "SessionInfo.h"

class DataLoader
{
public:
    static QList<SessionInfo> loadFromFile(const QString& filePath);
};

#endif // DATALOADER_H
