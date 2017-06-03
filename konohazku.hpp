#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <KLocalizedString>
#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>

#include <QApplication>
#include <QClipboard>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <QtCore/QtCore>

#include <zkutils/include/zku/zmsg.h>
#include "zkutils/include/zku/zsmp/guest.h"

#include <KRunner/AbstractRunner>

class KonohaZku : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    KonohaZku(QObject *parent, const QVariantList &args);
    ~KonohaZku();

    void match(Plasma::RunnerContext &);
    void run(const Plasma::RunnerContext &, const Plasma::QueryMatch &);
};

#endif
