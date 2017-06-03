#include "konohazku.hpp"

using namespace std;

KonohaZku *application;
Plasma::RunnerContext gContext;

void boost::throw_exception(std::exception const &exception) {
    // TODO: Do exception handling
}

void receiveZmsgCallback(const zmsg_t *input) {
    string recvString((*input).msg);

    // Process response
    vector<std::string> responseVector;
    boost::split(responseVector, recvString, boost::is_any_of("\n"));
    std::string sender("");

    for (int i = 0; i < responseVector.size(); i++) {
        if (boost::starts_with(responseVector[i], "sender")) {
            string::size_type equalPosition = responseVector[i].find("=", 0);
            if (equalPosition == string::npos) {
                // Cannot do anything
                return;
            }
            sender = responseVector[i].substr(equalPosition + 1, string::npos);
        }
        else if (boost::starts_with(responseVector[i], "response")) {
            string::size_type equalPosition = responseVector[i].find("=", 0);
            if (equalPosition == string::npos) {
                // Cannot do anything
                return;
            }
            std::string actualContent = responseVector[i].substr(equalPosition + 1, string::npos);
            string::size_type colonPosition = actualContent.find(":", 0);
            if (colonPosition == string::npos) {
                // No numbering in response, possible old response protocol? (<=ZSMP 0.4)
                Plasma::QueryMatch match(application);
                match.setType(Plasma::QueryMatch::ExactMatch);
                match.setData("zku");
                match.setText(actualContent.data());
                match.setSubtext(sender.data());
                match.setIcon(QIcon::fromTheme("preferences-desktop-font"));

                gContext.addMatch(match);
            }
            else {
                std::string responseNumber = actualContent.substr(0, colonPosition);

                Plasma::QueryMatch match(application);
                match.setType(Plasma::QueryMatch::ExactMatch);
                match.setData("zku");
                match.setText(actualContent.data());
                match.setSubtext(sender.append(":").append(responseNumber).data());
                match.setIcon(QIcon::fromTheme("preferences-desktop-font"));

                gContext.addMatch(match);
            }
        }
    }

    zku::zsmp_guest_end();
}

KonohaZku::KonohaZku(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args) {
    Q_UNUSED(args);

    // General runner configuration
    setObjectName(QLatin1String("KonohaZku"));
    setHasRunOptions(true);
    setIgnoredTypes(Plasma::RunnerContext::Directory |
                    Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);
    setSpeed(Plasma::AbstractRunner::NormalSpeed);
    setPriority(HighestPriority);
    setDefaultSyntax(
        Plasma::RunnerSyntax(
            QString::fromLatin1(":q:"),
            i18n("Sends :q: query to Zku, and retrieves data.")
        )
    );
}

KonohaZku::~KonohaZku() {
}

void KonohaZku::match(Plasma::RunnerContext &context) {
    if (!context.isValid())
        return;

    application = this;
    gContext = context;
    string sendString;

    string ident("KonohaZku#");
    ident.append(to_string((int)getpid()));

    zku::zsmp_guest_start(ident.data());

    const QString enteredKey = context.query();
    sendString = enteredKey.toStdString();

    if (!zku::zsmp_guest_running())
        return;

    zmsg_t *sendZmsg = zku::zmsg_alloc();
    zku::zmsg_initwithcstr(sendZmsg, sendString.data());

    zku::zsmp_guest_setcallback(receiveZmsgCallback);

    zku::zsmp_guest_pushzmsg(sendZmsg);
}

/**
 * Perform an action when a user chooses one of the previously found matches.
 * Either some string gets copied to the clipboard, a file/path/URL is being opened, 
 * or a command is being executed.
 */
void KonohaZku::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context);

    if (match.data().toString().compare("open") == 0) {
        // Open a file or a URL in a (file/web) browser
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = "kde-open " + match.text().remove("→ ").toStdString() + " &";
        system(command.c_str());
    }
    else if (match.data().toString().compare("execute") == 0) {
        // Execute a command
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = match.text().remove(">_ ").toStdString() + " &";
        system(command.c_str());
    }
    else {
        // Copy the result to clipboard
        QApplication::clipboard()->setText(match.text());
    }
}


K_EXPORT_PLASMA_RUNNER(konohazku, KonohaZku)

#include "konohazku.moc"