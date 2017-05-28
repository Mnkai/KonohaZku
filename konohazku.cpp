#include <KLocalizedString>
#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>

#include <QApplication>
#include <QClipboard>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <QtCore/QtCore>

#include "konohazku.h"

#define MAIN_PORT 3329
#define BACKUP_PORT 5829

using namespace std;
using namespace boost::asio;

io_service ioservice;
ip::tcp::socket tcp_socket{ioservice};
array<char, 4096> bytes;

string sendString;
string recvString;

void boost::throw_exception(std::exception const& exception)
{
    // TODO: Do exception handling
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

KonohaZku::~KonohaZku() {}

void KonohaZku::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;

    // Cancel if there is any ongoing request
    if (tcp_socket.is_open())
    {
        tcp_socket.close();
    }

    const QString enteredKey = context.query();
    sendString = enteredKey.toStdString();
    recvString = string("");

    ip::tcp::endpoint server_endpoint (ip::address::from_string("127.0.0.1"), MAIN_PORT);
    boost::system::error_code ec;
    tcp_socket.connect(server_endpoint, ec);
    if ( ec ) { return; }
    write(tcp_socket, buffer(sendString));
    do
    {
        tcp_socket.read_some(buffer(bytes), ec);
        recvString.append(bytes.data());
    } while ( !ec );

    // Process response
    vector<std::string> responseVector;
    boost::split(responseVector, recvString, boost::is_any_of("\n"));

    string actualContent;
    for (int i=0; i<responseVector.size(); i++)
    {
        if ( boost::starts_with(responseVector[i], "response") )
        {
            string::size_type equalPosition = responseVector[i].find("=", 0);
            if ( equalPosition == string::npos )
            {
                // Cannot do anything
                return;
            }
            actualContent = responseVector[i].substr(equalPosition, responseVector[i].size());

        }
    }

    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::ExactMatch);
    match.setData("zku");
    match.setText(actualContent.data());
    match.setSubtext(sendString.data());
    match.setIcon(QIcon::fromTheme("preferences-desktop-font"));

    context.addMatch(match);

    tcp_socket.close();
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

    } else if (match.data().toString().compare("execute") == 0) {
        // Execute a command
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = match.text().remove(">_ ").toStdString() + " &";
        system(command.c_str());

    } else {
        // Copy the result to clipboard
        QApplication::clipboard()->setText(match.text());
    }
}


K_EXPORT_PLASMA_RUNNER(konohazku, KonohaZku)

#include "konohazku.moc"