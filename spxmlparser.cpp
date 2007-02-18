/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "spxmlparser.hpp"
#include "spxmlreader.hpp"
#include "spxmlutils.hpp"
#include "spxmlevent.hpp"

SP_XmlPullParser :: SP_XmlPullParser()
{
	mReaderPool = new SP_XmlReaderPool();
	mReader = getReader( SP_XmlReader::eLBracket );
	mEventQueue = new SP_XmlPullEventQueue();
	mEventQueue->enqueue( new SP_XmlStartDocEvent() );

	mRootTagState = eRootNone;
	mTagNameStack = new SP_XmlArrayList();
	mLevel = 0;

	mError = NULL;

	memset( mErrorSegment, 0, sizeof( mErrorSegment ) );
	mErrorIndex = 0;
	mRowIndex = mColIndex = 0;
}

SP_XmlPullParser :: ~SP_XmlPullParser()
{
	mReaderPool->save( mReader );

	delete mTagNameStack;

	delete mEventQueue;

	delete mReaderPool;

	if( NULL != mError ) free( mError );	
}

void SP_XmlPullParser :: append( const char * source, int len )
{
	if( NULL != mError ) return;

	for( int i = 0; i < len && NULL == mError; i++ ) {
		char c = source[ i ];

		mErrorSegment[ mErrorIndex++ % sizeof( mErrorSegment ) ] = c;
		mReader->read( this, c );
		if( '\n' == c ) {
			mRowIndex++;
			mColIndex = 0;
		} else {
			mColIndex++;
		}
	}
}

SP_XmlPullEvent * SP_XmlPullParser :: getNext()
{
	SP_XmlPullEvent * event = mEventQueue->dequeue();

	if( NULL != event ) {
		if( SP_XmlPullEvent::eStartTag == event->getEventType() ) mLevel++;
		if( SP_XmlPullEvent::eEndTag == event->getEventType() ) mLevel--;
	}

	return event;
}

int SP_XmlPullParser :: getLevel()
{
	return mLevel;
}

const char * SP_XmlPullParser :: getError()
{
	return mError;
}

void SP_XmlPullParser :: changeReader( SP_XmlReader * reader )
{
	SP_XmlPullEvent * event = mReader->getEvent( this );
	if( NULL != event ) {
		if( SP_XmlPullEvent::eStartTag == event->getEventType() ) {
			if( eRootNone == mRootTagState ) mRootTagState = eRootStart;
			const char * name = ((SP_XmlStartTagEvent*)event)->getName();
			mTagNameStack->append( name, strlen( name ) + 1 );
		}
		if( SP_XmlPullEvent::eEndTag == event->getEventType() ) {
			char error[ 256 ] = { 0 };

			const char * etag = ((SP_XmlEndTagEvent*)event)->getText();
			char * stag = (char*)mTagNameStack->takeItem( SP_XmlArrayList::LAST_INDEX );
			if( NULL != stag ) {
				if( 0 != strcmp( stag, etag ) ) {
					snprintf( error, sizeof( error ),
							"mismatched tag, start-tag <%s>, end-tag <%s>", stag, etag );
				}
				free( stag );
			} else {
				snprintf( error, sizeof( error ),
						"mismatched tag, start-tag <NULL>, end-tag <%s>", etag );
			}

			if( '\0' != *error ) {
				setError( error );
				delete event;
				event = NULL;
			}
		}

		if( NULL != event ) {
			mEventQueue->enqueue( event );
			if( mTagNameStack->getCount() <= 0 && eRootStart == mRootTagState ) {
				mRootTagState = eRootEnd;
				mEventQueue->enqueue( new SP_XmlEndDocEvent() );
			}
		}
	}

	mReaderPool->save( mReader );
	mReader = reader;
}

SP_XmlReader * SP_XmlPullParser :: getReader( int type )
{
	return mReaderPool->borrow( type );
}

void SP_XmlPullParser :: setError( const char * error )
{
	if( NULL != error ) {
		if( NULL != mError ) free( mError );

		char segment[ 2 * sizeof( mErrorSegment ) + 1 ];
		{
			memset( segment, 0, sizeof( segment ) );

			char temp[ sizeof( mErrorSegment ) + 1 ];
			memset( temp, 0, sizeof( temp ) );
			if( mErrorIndex < (int)sizeof( mErrorSegment ) ) {
				strncpy( temp, mErrorSegment, mErrorIndex );
			} else {
				int offset = mErrorIndex % sizeof( mErrorSegment );
				strncpy( temp, mErrorSegment + offset, sizeof( mErrorSegment ) - offset );
				strncpy( temp + sizeof( mErrorSegment ) - offset, mErrorSegment, offset );
			}

			for( char * pos = temp, * dest = segment; '\0' != *pos; pos++ ) {
				if( '\r' == *pos ) {
					*dest++ = '\\';
					*dest++ = 'r';
				} else if( '\n' == *pos ) {
					*dest++ = '\\';
					*dest++ = 'n';
				} else if( '\t' == *pos ) {
					*dest++ = '\\';
					*dest++ = 't';
				} else {
					*dest++ = *pos;
				}
			}
		}

		char msg[ 512 ];
		snprintf( msg, sizeof( msg), "%s ( occured at row(%d), col(%d) : %s )",
				error, mRowIndex + 1, mColIndex + 1, segment );

		mError = strdup( msg );
	}
}
