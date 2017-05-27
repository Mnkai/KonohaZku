#include <iostream>

#include <KLocalizedString>
#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>

#include <QApplication>
#include <QClipboard>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "konohazku.h"

#define  BUFF_SIZE   1024

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

KonohaZku::~KonohaZku()
{}

void KonohaZku::match(Plasma::RunnerContext &context)
{
  if (!context.isValid()) return;

  const QString enteredKey = context.query();
  QByteArray tempString;
  tempString.append(enteredKey);

  int client_socket;
  struct sockaddr_in server_addr;

  char buff[BUFF_SIZE+5];
  client_socket = socket(PF_INET, SOCK_STREAM, 0);
  if ( client_socket == -1 )
    return;

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(4000);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if ( connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)))
    return;

  write(client_socket, tempString.data(), strlen(tempString.data()+1));
  read(client_socket, buff, BUFF_SIZE);

  QList <Plasma::QueryMatch> matches;

  Plasma::QueryMatch match(this);
  match.setType(Plasma::QueryMatch::ExactMatch);
  match.setData("zku");
  match.setText(buff);
  match.setSubtext(tempString.data());
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