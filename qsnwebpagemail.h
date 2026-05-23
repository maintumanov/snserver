#ifndef QSNWEBPAGEMAIL_H
#define QSNWEBPAGEMAIL_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "qsnweb.h"
#include "QTcpSocket"
#include "QSslSocket"
#include "qhostaddress.h"
#include "qsnbsshapes.h"

/**
 * @brief Web widget for sending email notifications based on system log events.
 *
 * This class integrates with the global service bus to receive log/alert messages,
 * queues them per user (based on permissions), and sends HTML-formatted emails
 * via SMTP (with optional SSL/TLS support).
 *
 * It provides a web UI for manual testing and status monitoring.
 */
class QsnWebPageMail : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageMail(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageMail();

    /**
     * @brief Generates JavaScript functions for the web UI (e.g., button click handlers).
     *
     * @param functions Output list of JS code snippets.
     * @param accountIndex Index of currently logged-in user.
     * @param pageURL Unused URL parameter.
     */
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");

    /**
     * @brief Updates UI elements from JSON data received via AJAX.
     *
     * @param fjson Output list of jQuery expressions to update DOM.
     * @param accountIndex Current user index.
     */
    void getFunctionsJSON(QStringList *fjson, int accountIndex);

    /**
     * @brief Renders the HTML content of the mail widget page.
     *
     * @param contents Output list of HTML snippets.
     * @param accountIndex Current user index.
     */
    void getContents(QStringList *contents, int accountIndex);

    /**
     * @brief Handles AJAX actions from the web UI (e.g., manual send requests).
     *
     * @param URL Unused URL.
     * @param options Map of action parameters.
     * @param returnItems Output list of JSON key-value pairs.
     * @param uid Unused unique ID.
     */
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);

    /**
     * @brief Restores widget state from persistent storage stream.
     *
     * Loads SMTP configuration and admin email.
     *
     * @param stream Input QDataStream.
     */
    void fromStream(QDataStream *stream);

    /**
     * @brief Called when the web session switches to a different user account.
     *
     * @param accountIndex New active user index.
     */
    void urlChanged(int accountIndex);

    /**
     * @brief Returns the current operational state of the mail widget for display.
     *
     * @return QString Human-readable status (e.g., "waiting for messages").
     */
    QString widgetState();

public slots:
    /**
     * @brief Receives messages from the global service bus.
     *
     * Listens for log/alert messages and enqueues them for email delivery
     * based on user permissions ("mailinfo", "mailicident").
     *
     * @param container Incoming message container.
     * @param sender Unused sender object.
     */
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private slots:
    // Mail queue & SMTP protocol handlers
    /**
     * @brief Slot called when data arrives from the SMTP server.
     */
    void slotReadyRead();

    /**
     * @brief Slot called when SMTP connection is closed.
     */
    void disconnected();
    /**
      * @brief Slot called when SMTP connection is established.
      */
    void slotError(QAbstractSocket::SocketError error);
    void slotConnected();

private:
    // SMTP configuration
    QString SenderName;        ///< Display name of the sender (e.g., "Smart home").
    QString SenderEmail;       ///< Email address used in "From" field.
    QString Login;             ///< SMTP login (if authentication required).
    QString Password;          ///< SMTP password (if authentication required).
    QString Server;            ///< SMTP server hostname or IP.
    quint16 Port;              ///< SMTP server port (e.g., 25, 465, 587).
    quint32 SendTimeout;       ///< Retry timeout in seconds after send failure.
    bool SSL;                  ///< True if SSL/TLS encryption is enabled.

    // Runtime state
    QsnGlobalModules *mds;           ///< Pointer to global application modules.
    QString mailState;               ///< Current high-level state (e.g., "Sending").
    QString mailCurOp;               ///< Current SMTP operation (e.g., "MAIL FROM").
    QString wState;                  ///< Widget status shown in main UI.
    int currentAccountIndex;         ///< Index of currently logged-in user.
    quint8 sendState;                ///< SMTP state machine state (0–13, 100=idle).
    int currentItemIndex;            ///< Index in mailqueue being sent.
    QString localAddress;            ///< Local IP used in HELO/EHLO command.
    QDateTime sendBeginTime;         ///< Timestamp when current send started (unused).

    // Network sockets
    QTcpSocket *clientSocket;        ///< Plain TCP socket (if SSL disabled).
    QSslSocket *clientSocketSSL;     ///< SSL-enabled socket (if SSL supported).
    quint32 dataBlockSize;           ///< Unused legacy field.

    // Email queue structures
    /**
      * @brief Represents a single styled log entry within an email.
      */
    struct contentItem {
        QString color;             ///< Main text color (CSS name).
        QString color_border;      ///< Border color (hex).
        QString color_background;  ///< Background color (hex).
        QString color_note;        ///< Note text background (hex).
        QDateTime time;            ///< Timestamp of the log event.
        QString content;           ///< Main log message body.
        QString note;              ///< Optional note in braces {like this}.
    };

    /**
       * @brief Represents a pending email to a specific user.
       */
    struct queueItem {
        QString name;                     ///< Recipient's display name.
        QString email;                    ///< Recipient's email address.
        QList<contentItem> content;       ///< List of log entries to include.
        QDateTime waitingUntil;            ///< Earliest time to attempt sending.
        quint8 state;                     ///< 0=ready, 1=connected, 2=sending.
    };

    bool isLog;                  ///< Unused legacy flag.
    QList<queueItem> mailqueue;  ///< Queue of emails waiting to be sent.

    // Private methods
    /**
     * @brief Adds a log message to the email queue for a specific user.
     *
     * @param text Log message body (with optional {note}).
     * @param user Account index (default: all users).
     */
    void addToQueue(QString text, int user = 65535);

    /**
     * @brief Parses a structured log message into a styled content item.
     *
     * @param text Input log string (e.g., "(!) Error {check cable}").
     * @return contentItem Structured representation for templating.
     */
    contentItem contentFromText(QString text);

    /**
     * @brief Periodically checks the mail queue for messages ready to send.
     */
    void queueCheck();

    /**
     * @brief Resets the SMTP state machine to idle.
     */
    void clearState();

    /**
     * @brief Handles SMTP protocol errors during sending.
     *
     * @param log Error description from server.
     * @param operation Current SMTP command (e.g., "MAIL FROM").
     */
    void senderror(QString log, QString operation = QString());

    // Low-level SMTP I/O
    void readAnswer(QString *answer);   ///< Reads full server response.
    void sendRequest(QString request);  ///< Sends raw SMTP command.

    // SMTP state handlers
    void connect_answer();
    void helo();
    void helo_answer();
    void mail_from();
    void mail_from_answer();
    void rcpt_to();
    void rcpt_to_answer();
    void data();
    void data_answer();
    void data_send();
    void data_send_answer();
    void quit();
    void ehlo();
    void ehlo_answer();
    void auth_login();
    void auth_login_answer();
    void auth_username();
    void auth_username_answer();
    void auth_password();
    void auth_password_answer();

    /**
     * @brief Initiates SMTP connection to send a queued message.
     *
     * @param itemIndex Index in mailqueue to send.
     */
    void beginSend(int itemIndex);

    /**
     * @brief Closes and deletes the active network socket.
     */
    void closeSocket();

    /**
     * @brief Returns the active socket (SSL or plain) for I/O operations.
     *
     * @return QIODevice* Active socket or nullptr.
     */
    QIODevice *socket();

    /**
     * @brief Counts total number of log entries waiting in the email queue.
     *
     * @return int Total messages across all recipients.
     */
    int getQueueItems();

    /**
     * @brief Counts number of users with configured email addresses.
     *
     * @return int Recipient count.
     */
    int getRecipientsCount();

    /**
     * @brief Forces immediate sending of all queued messages (manual trigger).
     */
    void sendNow();

    /**
     * @brief Sends a test email to the current user.
     */
    void sendTest();

    /**
     * @brief Encodes a UTF-8 string for use in email headers (RFC 2047).
     *
     * @param text Input string.
     * @return QString Encoded header (e.g., "=?UTF-8?B?...?=").
     */
    QString encodeHeader(const QString &text);

};

#endif // QSNWEBPAGEMAIL_H
