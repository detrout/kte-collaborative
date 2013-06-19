/* This file is part of the Kobby
   Copyright (C) 2013 Sven Brauch <svenbrauch@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kobbyplugin.h"

#include <ktexteditor/document.h>
#include <ktexteditor/attribute.h>
#include <ktexteditor/searchinterface.h>
#include <ktexteditor/movinginterface.h>
#include <ktexteditor/movingrange.h>
#include <ktexteditor/configinterface.h>

#include <assert.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kfiledialog.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <KTextEditor/Editor>

#include "common/connection.h"
#include "common/document.h"
#include "editor/collabsession.h"
#include "common/itemfactory.h"
#include "common/documentbuilder.h"
#include "editor/remotebrowserview.h"
#include "common/noteplugin.h"
#include "common/documentmodel.h"
#include <createitemdialog.h>
#include <createconnectiondialog.h>
#include "kobbypluginview.h"

#include <libqinfinity/communicationjoinedgroup.h>
#include <libqinfinity/init.h>
#include <libqinfinity/user.h>
#include <libqinfinity/browsermodel.h>
#include <libqinfinity/browseritemfactory.h>
#include <libqinfinity/browser.h>
#include <libqinfinity/noteplugin.h>
#include <libqinfinity/xmlconnection.h>
#include <libqinfinity/xmppconnection.h>
#include <libqinfinity/userrequest.h>

#include <kparts/part.h>
#include <QApplication>
#include <QTimer>

// This is the default port for infinoted.
static int defaultPort = 6523;

K_PLUGIN_FACTORY( KobbyPluginFactory, registerPlugin<KobbyPlugin>(); )
K_EXPORT_PLUGIN( KobbyPluginFactory( KAboutData( "ktexteditor_kobby", "ktexteditor_plugins",
                                          ki18n( "Collaborative Editing" ), "1.0", ki18n("Collaborative Editing"), KAboutData::License_LGPL_V2 ) ) )

KobbyPlugin::KobbyPlugin( QObject *parent, const QVariantList& )
  : KTextEditor::Plugin ( parent )
{
    kDebug() << "loading kobby plugin" << this;
    QInfinity::init();
    // TODO this is for unit tests. I can't figure out how to get the plugin instance
    // from the KatePart we create there.
    QApplication::instance()->setProperty("KobbyPluginInstance", QVariant(reinterpret_cast<unsigned long long>(this)));
    m_browserModel = new QInfinity::BrowserModel( this );
    m_browserModel->setItemFactory( new Kobby::ItemFactory( this ) );
    m_textPlugin = new Kobby::NotePlugin( this );
    m_communicationManager = new QInfinity::CommunicationManager();
    m_browserModel->addPlugin( *m_textPlugin );
    kDebug() << "ok";
}

KobbyPlugin::~KobbyPlugin()
{
}

void KobbyPlugin::connectionPrepared(Connection* connection)
{
    kDebug() << "connection prepared, establishing connection";
    m_browserModel->addConnection(connection->xmppConnection(), connection->name());
    foreach ( QInfinity::Browser* browser, m_browserModel->browsers() ) {
        QObject::connect(browser, SIGNAL(connectionEstablished(const QInfinity::Browser*)),
                         this, SLOT(browserConnected(const QInfinity::Browser*)), Qt::UniqueConnection);
    }
    connection->open();
}

void KobbyPlugin::browserConnected(const QInfinity::Browser* browser)
{
    kDebug() << "browser connected, subscribing documents";
    subscribeNewDocuments();
}

void KobbyPlugin::subscribeNewDocuments()
{
    kDebug() << "subscribing new documents";
    foreach ( ManagedDocument* document, m_managedDocuments ) {
        QInfinity::Browser* browser = document->browser();
        if ( ! browser ) {
            continue;
        }
        if ( browser->connectionStatus() == INFC_BROWSER_CONNECTED && ! document->isSubscribed() ) {
            document->subscribe();
        }
    }
}

void KobbyPlugin::addDocument(KTextEditor::Document* document)
{
    if ( property("kobbyPluginDisabled").toBool() ) {
        // unit tests
        return;
    }
    kDebug() << "add document" << document << document->url() << "to plugin instance" << this;
    eventuallyManageDocument(document);
    connect(document, SIGNAL(aboutToClose(KTextEditor::Document*)),
            this, SLOT(removeDocument(KTextEditor::Document*)));
    connect(document, SIGNAL(aboutToReload(KTextEditor::Document*)),
            this, SLOT(removeDocument(KTextEditor::Document*)));
    connect(document, SIGNAL(documentUrlChanged(KTextEditor::Document*)),
            this, SLOT(eventuallyManageDocument(KTextEditor::Document*)));
}

void KobbyPlugin::removeDocument(KTextEditor::Document* document)
{
    kDebug() << "remove document:" << document->url().path();
    ManagedDocument* doc = m_managedDocuments.findDocument(document);
    if ( doc ) {
        doc->unsubscribe();
        m_managedDocuments.remove(document);
        delete doc;
    }
    else {
        kDebug() << "tried to remove document" << document << "which is not being managed";
    }
}

void KobbyPlugin::eventuallyManageDocument(KTextEditor::Document* document)
{
    kDebug() << "new url:" << document->url() << document;
    // the property() stuff is for unit tests (and only for unit tests!)
    bool manageAll = property("manageAllDocuments").toBool();
    if ( document->url().protocol() != "inf" && ! manageAll ) {
        kDebug() << "not a collaborative document:" << document->url().url();
        if ( m_managedDocuments.isManaged(document) ) {
            kDebug() << "removing document" << document << "from manager";
            ManagedDocument* doc = m_managedDocuments.findDocument(document);
            m_managedDocuments.remove(doc->document());
            delete doc;
        }
        return;
    }
    if ( m_managedDocuments.isManaged(document) ) {
        kDebug() << document->url() << "is already being managed.";
        return;
    }
    kDebug() << "initializing collaborative session for document" << document->url();

    Connection* connection = eventuallyAddConnection(document->url());

    ManagedDocument* managed = new ManagedDocument(document, m_browserModel, m_textPlugin, connection, this);
    m_managedDocuments[document] = managed;

    connect(document, SIGNAL(textInserted(KTextEditor::Document*, KTextEditor::Range)),
            this, SLOT(textInserted(KTextEditor::Document*, KTextEditor::Range)), Qt::UniqueConnection);
    connect(document, SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range)),
            this, SLOT(textRemoved(KTextEditor::Document*,KTextEditor::Range)), Qt::UniqueConnection);

    // add the existing views for this document
    foreach ( KTextEditor::View* view, document->views() ) {
        addView(view);
    }

    subscribeNewDocuments();
}

const ManagedDocumentList& KobbyPlugin::managedDocuments() const
{
    return m_managedDocuments;
}

const QString KobbyPlugin::connectionName(const KUrl& url)
{
    int port = url.port();
    port = port == -1 ? defaultPort : port;
    return url.host() + ":" + QString::number(port);
}

short unsigned int KobbyPlugin::portForUrl(const KUrl& url)
{
    return url.port() == -1 ? defaultPort : url.port();
}

Connection* KobbyPlugin::eventuallyAddConnection(const KUrl& documentUrl)
{
    int port = portForUrl(documentUrl);
    QString name = connectionName(documentUrl);
    if ( ! m_connections.contains(name) ) {
        kDebug() << "adding connection" << name << "because it doesn't exist";
        Connection* c = new Kobby::Connection(documentUrl.host(), port, name, this);
        c->setProperty("useSimulatedConnection", property("useSimulatedConnection"));
        connect(c, SIGNAL(ready(Connection*)),
                this, SLOT(connectionPrepared(Connection*)));
        connect(c, SIGNAL(disconnected(Connection*)),
                this, SLOT(connectionDisconnected(Connection*)));
        m_connections[name] = c;
        c->prepare();
        return c;
    }
    else {
        kDebug() << "connection" << name << "requested but it exists already";
    }
    return m_connections[name];
}

void KobbyPlugin::connectionDisconnected(Connection* connection)
{
    kDebug() << "disconnected:" << connection;
    m_connections.remove(connection->name());
}

void KobbyPlugin::addView(KTextEditor::View* view)
{
    ManagedDocument* doc = managedDocuments().findDocument(view->document());
    if ( doc ) {
        kDebug() << "adding view" << view;
        KobbyPluginView* kobbyView = new KobbyPluginView(view, doc);
    }
}

void KobbyPlugin::removeView(KTextEditor::View* view)
{
    kDebug() << "removing view" << view;
}

// Just for debugging purposes, the real handling happens in Kobby::InfTextDocument
void KobbyPlugin::textInserted(KTextEditor::Document* doc, KTextEditor::Range range)
{
    kDebug() << "text inserted:" << range << doc->textLines(range) << doc;
}

void KobbyPlugin::textRemoved(KTextEditor::Document* doc, KTextEditor::Range range)
{
    kDebug() << "text removed:" << range << doc->textLines(range) << doc;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
