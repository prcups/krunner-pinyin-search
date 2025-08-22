#include "pinyinmatch.h"

PinyinMatch::PinyinMatch()
{
    QFile f(QString::fromUtf8(":pinyintable.txt"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream fin(&f);
    QString hanzi, pinyin;
    while (!(fin >> hanzi >> pinyin).atEnd())
        pinyinMap.insert(hanzi[0], pinyin);

    f.close();
}

bool PinyinMatch::MatchStr(const QStringView &hanziStr, const QStringView &pinyinStr)
{
    if (pinyinStr.empty())
        return true;
    if (hanziStr.empty())
        return false;
    auto hanzi = hanziStr[0];
    if (hanzi == pinyinStr[0]) {
        if (MatchStr(hanziStr.sliced(1), pinyinStr.sliced(1)))
            return true;
    }
    auto correspondPinyinList = pinyinMap.values(hanzi);
    for (auto &correspondPinyin : correspondPinyinList) {
        if (pinyinStr[0] == correspondPinyin[0]) {
            if (MatchStr(hanziStr.sliced(1), pinyinStr.sliced(1)))
                return true;
        }

        if (pinyinStr.size() <= correspondPinyin.size()) {
            if (correspondPinyin.startsWith(pinyinStr))
                return true;
        } else if (pinyinStr.startsWith(correspondPinyin)) {
            if (MatchStr(hanziStr.sliced(1), pinyinStr.sliced(correspondPinyin.size())))
                return true;
        }
    }
    return false;
}
