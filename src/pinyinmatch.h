#ifndef PINYINMATCH_H
#define PINYINMATCH_H

#include <QObject>
#include <QMultiHash>
#include <QFile>
#include <QTextStream>
#include <QStringView>

class PinyinMatch : public QObject
{
    Q_OBJECT
    QMultiHash <QChar, QString> pinyinMap;
public:
    PinyinMatch();
    bool MatchStr(const QStringView & hanziStr, const QStringView &pinyinStr);
};

#endif // PINYINMATCH_H
