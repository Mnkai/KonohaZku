#include <iostream>

#include <KLocalizedString>
#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>

#include <QApplication>
#include <QClipboard>

#include "konohazku.h"

using namespace std;

KonohaZku::KonohaZku(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    
    // General runner configuration
    setObjectName(QLatin1String("KonohaZku"));
    setHasRunOptions(true);
    setIgnoredTypes(Plasma::RunnerContext::Directory |
                    Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);
    setSpeed(AbstractRunner::NormalSpeed);
    setPriority(HighestPriority);
    setDefaultSyntax(
        Plasma::RunnerSyntax(
            QString::fromLatin1(":q:"),
            i18n("Sends :q: query to Zku, and retrieves data.")
        )
    );
}

KonohaZku::~KonohaZku() {}

void KonohaZku::match(Plasma::RunnerContext &context)
{
    if (!context.isValid()) return;

    const QString enteredKey = context.query();
    
    QList<Plasma::QueryMatch> matches;

    // Sends enteredKey to Zku

    // Found a key
    // Plasma::QueryMatch match(this);

    // match.setType(Plasma::QueryMatch::ExactMatch);
    // match.setData("zku");
    // match.setText(it.value());
    // match.setSubtext("[" + foundKey + "]");
    // match.setIcon(QIcon::fromTheme("preferences-desktop-font"));

    // matches.append(match);

    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::ExactMatch);
    match.setData("zku");
    match.setText("This is test response from Zku");
    match.setSubtext("This is original input: " + enteredKey);
    match.setIcon(QIcon::fromTheme("preferences-desktop-font"));

    matches.append(match);

    // Feed the framework with the calculated results
    context.addMatches(matches);
}

/**
 * Perform an action when a user chooses one of the previously found matches.
 * Either some string gets copied to the clipboard, a file/path/URL is being opened, 
 * or a command is being executed.
 */
void KonohaZku::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);

    if (match.data().toString().compare("open") == 0)
    {
        // Open a file or a URL in a (file/web) browser
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = "kde-open " + match.text().remove("→ ").toStdString() + " &";
        system(command.c_str());
        
    } else if (match.data().toString().compare("execute") == 0) 
    {
        // Execute a command
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = match.text().remove(">_ ").toStdString() + " &";
        system(command.c_str());
        
    } else 
    {
        // Copy the result to clipboard
        QApplication::clipboard()->setText(match.text());
    }
}


K_EXPORT_PLASMA_RUNNER(konohazku, KonohaZku)

#include "konohazku.moc"