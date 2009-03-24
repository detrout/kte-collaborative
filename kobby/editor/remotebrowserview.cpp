#include "remotebrowserview.h"
#include "itemfactory.h"
#include "createitemdialog.h"

#include <libqinfinity/noteplugin.h>
#include <libqinfinity/browsermodel.h>

#include <KAction>
#include <KIcon>
#include <KLocalizedString>
#include <KToolBar>
#include <KMenu>

#include <QTreeView>
#include <QVBoxLayout>
#include <QItemSelection>
#include <QModelIndexList>
#include <QDebug>
#include <QContextMenuEvent>

#include "remotebrowserview.moc"

namespace Kobby
{

RemoteBrowserView::RemoteBrowserView( QInfinity::NotePlugin &plugin,
    QInfinity::BrowserModel &model,
    QWidget *parent )
    : QWidget( parent )
    , m_treeView( new QTreeView( this ) )
    , m_plugin( &plugin )
    , browserModel( &model )
    , contextMenu( 0 )
{
    m_treeView->setModel( &model );
    setupActions();
    setupToolbar();

    QVBoxLayout *vertLayout = new QVBoxLayout( this );
    vertLayout->addWidget( toolBar );
    vertLayout->addWidget( m_treeView );

    setLayout( vertLayout );
}

void RemoteBrowserView::contextMenuEvent( QContextMenuEvent *e )
{
    if( !e )
        return;

    if( !contextMenu )
    {
        contextMenu = new KMenu( this );
        contextMenu->addAction( createConnectionAction );
        contextMenu->addAction( createDocumentAction );
        contextMenu->addAction( createFolderAction );
        contextMenu->addSeparator();
        contextMenu->addAction( openAction );
        contextMenu->addAction( deleteAction );
    }

    contextMenu->popup( e->globalPos() );
}

void RemoteBrowserView::slotNewConnection()
{
    emit(createConnection());
}

void RemoteBrowserView::slotNewDocument()
{
    QList<QModelIndex> indexes = getSelection();
    CreateItemDialog *dialog;
    if( canCreateDocument( indexes ) )
    {
        dialog = new CreateItemDialog( "Create Note",
            "Note name:", this );
        if( dialog->exec() )
            browserModel->createNote( indexes[0],
                *m_plugin, dialog->name() );
        delete dialog;
    }
    else
        qDebug() << "Create document handler called but we have invalid selection.";
}

void RemoteBrowserView::slotNewFolder()
{
    QList<QModelIndex> indexes = getSelection();
    CreateItemDialog *dialog;
    if( canCreateFolder( indexes ) )
    {
        dialog = new CreateItemDialog( "Create Folder",
            "Folder Name:", this );
        if( dialog->exec() )
            browserModel->createDirectory( indexes[0], dialog->name() );
        delete dialog;
    }
    else
        qDebug() << "Create folder handler called but we have invalid selection.";
}

void RemoteBrowserView::slotOpen()
{
    QList<QModelIndex> indexes = getSelection();
    QList<QModelIndex>::ConstIterator itr;
    if( canOpenItem( indexes ) )
    {
        for( itr = indexes.begin(); itr != indexes.end(); itr++ )
            slotOpen( *itr );
    }
    else
        qDebug() << "Open handler called but we have invalid selection.";
}

void RemoteBrowserView::slotOpen( const QModelIndex &index )
{
    emit( openItem( index ) );
}

void RemoteBrowserView::slotDelete()
{
    if( !m_treeView->selectionModel()->hasSelection() )
        return;
    QList<QModelIndex> indexes = m_treeView->selectionModel()->selectedIndexes();
    QList<QModelIndex>::Iterator indexItr;
    QStandardItem *item;
    for( indexItr = indexes.begin(); indexItr != indexes.end(); indexItr++ )
    {
        item = browserModel->itemFromIndex( *indexItr );
        browserModel->removeRow( indexItr->row(), indexItr->parent() );
    }
}

void RemoteBrowserView::slotSelectionChanged( const QItemSelection &selected,
    const QItemSelection &deselected )
{
    Q_UNUSED(deselected)
    createDocumentAction->setEnabled( canCreateDocument( selected.indexes() ) );
    createFolderAction->setEnabled( canCreateDocument( selected.indexes() ) );
    openAction->setEnabled( canOpenItem( selected.indexes() ) );
    deleteAction->setEnabled( canDeleteItem( selected.indexes() ) );
}

void RemoteBrowserView::setupActions()
{
    createConnectionAction = new KAction( i18n("New Connection"), this );
    createDocumentAction = new KAction( i18n("New Document"), this );
    createFolderAction = new KAction( i18n("New Folder"), this );
    openAction = new KAction( i18n("Open Document"), this );
    deleteAction = new KAction( i18n("Delete"), this );

    createConnectionAction->setIcon( KIcon("network-connect.png") );
    createDocumentAction->setIcon( KIcon("document-new.png") );
    createFolderAction->setIcon( KIcon("folder-new.png") );
    openAction->setIcon( KIcon("document-open.png") );
    deleteAction->setIcon( KIcon("user-trash.png") );

    createConnectionAction->setEnabled( true );
    createDocumentAction->setEnabled( false );
    createFolderAction->setEnabled( false );
    openAction->setEnabled( false );
    deleteAction->setEnabled( false );

    connect( createConnectionAction, SIGNAL(triggered(bool)),
        this, SLOT(slotNewConnection()) );
    connect( createDocumentAction, SIGNAL(triggered(bool)),
        this, SLOT(slotNewDocument()) );
    connect( createFolderAction, SIGNAL(triggered(bool)),
        this, SLOT(slotNewFolder()) );
    connect( openAction, SIGNAL(triggered(bool)),
        this, SLOT(slotOpen()) );
    connect( deleteAction, SIGNAL(triggered(bool)),
        this, SLOT(slotDelete()) );
    connect( m_treeView, SIGNAL(expanded(const QModelIndex&)),
        browserModel, SLOT(itemActivated(const QModelIndex&)) );
    connect( m_treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)) );
    connect( m_treeView, SIGNAL(doubleClicked(const QModelIndex&)),
        this, SLOT(slotOpen(const QModelIndex&)) );
}

void RemoteBrowserView::setupToolbar()
{
    toolBar = new KToolBar( this );
    toolBar->setIconDimensions( 16 );
    toolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
    toolBar->addAction( createConnectionAction );
    toolBar->addAction( createDocumentAction );
    toolBar->addAction( createFolderAction );
    toolBar->addAction( openAction );
    toolBar->addAction( deleteAction );
}

bool RemoteBrowserView::canCreateDocument( QModelIndexList selected )
{
    QStandardItem *item;
    QInfinity::NodeItem *nodeItem;
    if( selected.size() != 1 )
        return false;
    item = browserModel->itemFromIndex( selected[0] );
    if( !item )
        return false;
    if( item->type() == QInfinity::BrowserItemFactory::NodeItem )
    {
        nodeItem = dynamic_cast<QInfinity::NodeItem*>(item);
        return nodeItem->isDirectory();
    }
    return false;
}

bool RemoteBrowserView::canCreateFolder( QModelIndexList selected )
{
    return canCreateDocument( selected );
}

bool RemoteBrowserView::canOpenItem( QModelIndexList selected )
{
    QStandardItem *item;
    QModelIndexList::Iterator itr;
    if( selected.size() == 0 )
        return false;

    for( itr = selected.begin(); itr != selected.end(); itr++ )
    {
        item = browserModel->itemFromIndex( *itr );
        if( !item ||
            !(item->type() == QInfinity::BrowserItemFactory::NodeItem) )
            return false;
    }

    return true;
}

bool RemoteBrowserView::canDeleteItem( QModelIndexList selected )
{
    return selected.size() != 0;
}

QList<QModelIndex> RemoteBrowserView::getSelection()
{
    return m_treeView->selectionModel()->selectedRows();
}

}
