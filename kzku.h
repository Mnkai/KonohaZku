#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <KRunner/AbstractRunner>

class KZku : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    KZku(QObject *parent, const QVariantList &args);
    ~KZku();

    void match(Plasma::RunnerContext &);
    void run(const Plasma::RunnerContext &, const Plasma::QueryMatch &);
};

#endif
