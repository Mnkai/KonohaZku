#ifndef SYMBOLS_H
#define SYMBOLS_H

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
