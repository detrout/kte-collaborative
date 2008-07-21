#include <libinfinitymm/common/xmppconnection.h>

#include "connectionmanagerlistitem.h"

namespace Kobby
{

ConnectionManagerListItem::ConnectionManagerListItem( Infinity::XmppConnection &conn,
    const QString &hostname, QListWidget *parent )
    : QListWidgetItem( hostname, parent )
    , connection( &conn )
    , hostname( hostname )
    , has_connected( false )
{
    conn.property_tcp_connection().get_value()->property_status().signal_changed().connect( 
        sigc::mem_fun( this, &ConnectionManagerListItem::onConnectionStatusChanged )
    );

    setStatusDisplay();
}

void ConnectionManagerListItem::onConnectionStatusChanged()
{
    setStatusDisplay();
}

void ConnectionManagerListItem::setStatusDisplay()
{
    QString statusLine;

    switch( connection->property_tcp_connection().get_value()->property_status() )
    {
        case Infinity::TCP_CONNECTION_CONNECTING:
            statusLine = "Connecting...";
            break;
        case Infinity::TCP_CONNECTION_CONNECTED:
            has_connected = true;
            statusLine = "Connected.";
            break;
        case Infinity::TCP_CONNECTION_CLOSED:
            if( !has_connected )
                statusLine = "Could not connect to server.";
            else
                statusLine = "Closed.";
    }

    setText( hostname + QString( ":" ) + QString::number( connection->property_tcp_connection().get_value()->getRemotePort() ) + "\n" + statusLine );
}

}
