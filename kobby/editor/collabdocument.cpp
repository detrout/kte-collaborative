#include <libinfinitymm/common/user.h>
#include <libinftextmm/textchunk.h>

#include "collabdocument.h"

#include <libqinfinitymm/document.h>

#include <KTextEditor/Document>
#include <KTextEditor/Range>
#include <KDebug>

namespace Kobby
{

CollabDocument::CollabDocument( QInfinity::Document &infDocument,
    KTextEditor::Document &document,
    QObject *parent )
    : m_infDocument( &infDocument )
    , m_kDocument( &document )
{
    setupActions();
}

CollabDocument::~CollabDocument()
{
    delete m_infDocument;
}

void CollabDocument::slotLocalTextInserted( KTextEditor::Document *document,
    const KTextEditor::Range &range )
{
    
}

void CollabDocument::slotInsertText( unsigned int pos,
    Infinity::TextChunk &textChunk,
    Infinity::User *user )
{
}

void CollabDocument::setupActions()
{
}

}

