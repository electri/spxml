/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <typeinfo>

#include "spxmlparser.hpp"
#include "spxmlreader.hpp"
#include "spxmlutils.hpp"
#include "spxmlstag.hpp"
#include "spxmlevent.hpp"

//=========================================================

SP_XmlReader :: SP_XmlReader()
{
	mBuffer = new SP_XmlStringBuffer();
}

SP_XmlReader :: ~SP_XmlReader()
{
	delete mBuffer;
}

void SP_XmlReader :: changeReader(
		SP_XmlPullParser * parser, SP_XmlReader * reader )
{
	return parser->changeReader( reader );
}

SP_XmlReader * SP_XmlReader :: getReader( SP_XmlPullParser * parser, int type )
{
	return parser->getReader( type );
}

void SP_XmlReader :: setError( SP_XmlPullParser * parser, const char * error )
{
	parser->setError( error );
}

void SP_XmlReader :: reset()
{
	mBuffer->clean();
}

//=========================================================

SP_XmlDocDeclReader :: SP_XmlDocDeclReader()
{
}

SP_XmlDocDeclReader :: ~SP_XmlDocDeclReader()
{
}

void SP_XmlDocDeclReader :: read( SP_XmlPullParser * parser, char c )
{
	if( '>' == c ) {
		changeReader( parser, getReader( parser, SP_XmlReader::eCData ) );
	} else {
		mBuffer->append( c );
	}
}

SP_XmlPullEvent * SP_XmlDocDeclReader :: getEvent( SP_XmlPullParser * parser )
{
	SP_XmlDocDeclEvent * retEvent = NULL;

	SP_XmlSTagParser tagParser;

	tagParser.append( mBuffer->getBuffer(), mBuffer->getSize() );
	tagParser.append( " ", 1 );

	if( NULL == tagParser.getError() ) {
		SP_XmlStartTagEvent * event = tagParser.takeEvent();

		const char * version = event->getAttrValue( "version" );
		const char * encoding = event->getAttrValue( "encoding" );
		const char * standalone = event->getAttrValue( "standalone" );

		retEvent = new SP_XmlDocDeclEvent();
		retEvent->setVersion( NULL == version ? "" : version );
		retEvent->setEncoding( NULL == encoding ? "" : encoding );
		if( NULL != standalone && 0 == strcasecmp( "no", standalone ) ) {
			retEvent->setStandalone( 0 );
		}

		delete event;
	} else {
		setError( parser, tagParser.getError() );
	}

	return retEvent;
}

//=========================================================

SP_XmlStartTagReader :: SP_XmlStartTagReader()
{
	mIsQuot = 0;
}

SP_XmlStartTagReader :: ~SP_XmlStartTagReader()
{
}

void SP_XmlStartTagReader :: read( SP_XmlPullParser * parser, char c )
{
	if( '>' == c && 0 == mIsQuot ) {
		changeReader( parser, getReader( parser, SP_XmlReader::eCData ) );
	} else if( '/' == c && 0 == mIsQuot ) {
		SP_XmlReader * reader = getReader( parser, SP_XmlReader::eETag );
		const char * pos = mBuffer->getBuffer();
		for( ; isspace( *pos ); ) pos++;
		for( ; 0 == isspace( *pos ) && '\0' != *pos; pos++ ) {
			reader->read( parser, *pos );
		}
		changeReader( parser, reader );
	} else {
		mBuffer->append( c );
		if( '"' == c || '\'' == c ) mIsQuot = ( mIsQuot + 1 ) % 2;
	}
}

SP_XmlPullEvent * SP_XmlStartTagReader :: getEvent( SP_XmlPullParser * parser )
{
	SP_XmlStartTagEvent * retEvent = NULL;

	SP_XmlSTagParser tagParser;
	tagParser.append( mBuffer->getBuffer(), mBuffer->getSize() );
	tagParser.append( " ", 1 );

	if( NULL == tagParser.getError() ) {
		retEvent = tagParser.takeEvent();
	} else {
		setError( parser, tagParser.getError() );
	}

	return retEvent;
}

void SP_XmlStartTagReader :: reset()
{
	SP_XmlReader::reset();
	mIsQuot = 0;
}

//=========================================================

SP_XmlEndTagReader :: SP_XmlEndTagReader()
{
}

SP_XmlEndTagReader :: ~SP_XmlEndTagReader()
{
}

void SP_XmlEndTagReader :: read( SP_XmlPullParser * parser,	char c )
{
	if( '>' == c ) {
		changeReader( parser, getReader( parser, SP_XmlReader::eCData ) );
	} else {
		mBuffer->append( c );
	}
}

SP_XmlPullEvent * SP_XmlEndTagReader :: getEvent( SP_XmlPullParser * parser )
{
	SP_XmlEndTagEvent * retEvent = new SP_XmlEndTagEvent();
	retEvent->setText( mBuffer->getBuffer(), mBuffer->getSize() );

	return retEvent;
}

//=========================================================

SP_XmlCDataReader :: SP_XmlCDataReader()
{
}

SP_XmlCDataReader :: ~SP_XmlCDataReader()
{
}

void SP_XmlCDataReader :: read( SP_XmlPullParser * parser, char c )
{
	if( '<' == c ) {
		SP_XmlReader * reader = getReader( parser, SP_XmlReader::eLBracket );
		reader->read( parser, c );
		changeReader( parser, reader );
	} else {
		mBuffer->append( c );
	}
}

SP_XmlPullEvent * SP_XmlCDataReader :: getEvent( SP_XmlPullParser * parser )
{
	SP_XmlCDataEvent * retEvent = NULL;

	int ignoreWhitespace = 1;
	for( const char * pos = mBuffer->getBuffer(); '\0' != *pos; pos++ ) {
		if( !isspace( *pos ) ) {
			ignoreWhitespace = 0;
			break;
		}
	}

	if( 0 == ignoreWhitespace ) {
		retEvent = new SP_XmlCDataEvent();
		SP_XmlStringBuffer buffer;
		SP_XmlStringUtils::decode( mBuffer->getBuffer(), &buffer );
		retEvent->setText( buffer.getBuffer(), buffer.getSize() );
	}

	return retEvent;
}

//=========================================================

SP_XmlCommentReader :: SP_XmlCommentReader()
{
}

SP_XmlCommentReader :: ~SP_XmlCommentReader()
{
}

void SP_XmlCommentReader :: read( SP_XmlPullParser * parser, char c )
{
	if( '>' == c && mBuffer->getSize() >= 2 ) {
		int size = mBuffer->getSize();
		if( '-' == mBuffer->getBuffer()[ size - 1 ]
				&& '-' == mBuffer->getBuffer()[ size - 2 ] ) {
			changeReader( parser, getReader( parser, SP_XmlReader::eCData ) );
		} else {
			mBuffer->append( c );
		}
	} else {
		mBuffer->append( c );
	}
}

SP_XmlPullEvent * SP_XmlCommentReader :: getEvent( SP_XmlPullParser * parser )
{
	SP_XmlCommentEvent * retEvent = new SP_XmlCommentEvent();

	retEvent->setText( mBuffer->getBuffer(), mBuffer->getSize() - 2 );

	return retEvent;
}

//=========================================================

SP_XmlDocTypeReader :: SP_XmlDocTypeReader()
{
}

SP_XmlDocTypeReader :: ~SP_XmlDocTypeReader()
{
}

void SP_XmlDocTypeReader :: read( SP_XmlPullParser * parser, char c )
{
	if( '>' == c ) {
		changeReader( parser, getReader( parser, SP_XmlReader::eCData ) );
	} else {
		mBuffer->append( c );
	}
}

SP_XmlPullEvent * SP_XmlDocTypeReader :: getEvent( SP_XmlPullParser * parser )
{
	SP_XmlDocTypeEvent * retEvent = NULL;

	SP_XmlSTagParser tagParser;

	tagParser.append( "DOCTYPE ", strlen( "DOCTYPE " ) );
	tagParser.append( mBuffer->getBuffer(), mBuffer->getSize() );
	tagParser.append( " ", 1 );
	if( NULL == tagParser.getError() ) {
		SP_XmlStartTagEvent * event = tagParser.takeEvent();

		retEvent = new SP_XmlDocTypeEvent();

		for( int i = 0; i < event->getAttrCount(); i += 2 ) {
			const char * name = event->getAttr( i, NULL );
			if( 0 == strcmp( name, "DOCTYPE" ) ) {
				name = event->getAttr( i + 1, NULL );
				retEvent->setName( NULL == name ? "" : name );	
			} else if( 0 == strcmp( name, "PUBLIC" ) ) {
				name = event->getAttr( i + 1, NULL );
				retEvent->setPublicID( NULL == name ? "" : name );
			} else if( 0 == strcmp( name, "SYSTEM" ) ) {
				name = event->getAttr( i + 1, NULL );
				retEvent->setSystemID( NULL == name ? "" : name );
			} else if( NULL != strstr( name, ".dtd" ) ) {
				retEvent->setDTD( name );
			}
		}

		delete event;
	} else {
		setError( parser, tagParser.getError() );
	}

	return retEvent;
}

//=========================================================

SP_XmlLeftBracketReader :: SP_XmlLeftBracketReader()
{
	mHasReadBracket = 0;
}

SP_XmlLeftBracketReader :: ~SP_XmlLeftBracketReader()
{
}

void SP_XmlLeftBracketReader :: read( SP_XmlPullParser * parser, char c )
{
	if( 0 == mHasReadBracket ) {
		if( isspace( c ) ) {
			//skip
		} else if( '<' == c ) {
			mHasReadBracket = 1;
		}
	} else {
		if( '?' == c ) {
			changeReader( parser, getReader( parser, SP_XmlReader::eDocDecl ) );
		} else if( '/' == c ) {
			changeReader( parser, getReader( parser, SP_XmlReader::eETag ) );
		} else if( '!' == c ) {
			changeReader( parser, getReader( parser, SP_XmlReader::eSign ) );
		} else if( SP_XmlStringUtils::isNameChar( c ) ) {
			SP_XmlReader * reader = getReader( parser, SP_XmlReader::eSTag );
			reader->read( parser, c );
			changeReader( parser, reader );
		} else {
			setError( parser, "not well-formed" );
		}
	}
}

SP_XmlPullEvent * SP_XmlLeftBracketReader :: getEvent( SP_XmlPullParser * parser )
{
	return NULL;
}

void SP_XmlLeftBracketReader :: reset()
{
	SP_XmlReader::reset();
	mHasReadBracket = 0;
}

//=========================================================

SP_XmlSignReader :: SP_XmlSignReader()
{
}

SP_XmlSignReader :: ~SP_XmlSignReader()
{
}

void SP_XmlSignReader :: read( SP_XmlPullParser * parser, char c )
{
	if( '[' == c ) {
		changeReader( parser, getReader( parser, SP_XmlReader::eCData ) );
	} else if( '-' == c ) {
		changeReader( parser, getReader( parser, SP_XmlReader::eComment ) );
	} else if( isupper( c ) ) {
		SP_XmlReader * reader = getReader( parser, SP_XmlReader::eDocType );
		reader->read( parser, c );
		changeReader( parser, reader );
	} else {
		setError( parser, "not well-formed" );
	}
}

SP_XmlPullEvent * SP_XmlSignReader :: getEvent( SP_XmlPullParser * parser )
{
	return NULL;
}

//=========================================================

SP_XmlReaderPool :: SP_XmlReaderPool()
{
	mReaderList = (SP_XmlReader**)malloc( sizeof( void * ) * SP_XmlReader::MAX_READER );
	memset( mReaderList, 0, sizeof( void * ) * SP_XmlReader::MAX_READER );
}

SP_XmlReaderPool :: ~SP_XmlReaderPool()
{
	for( int i = 0; i < SP_XmlReader::MAX_READER; i++ ) {
		if( NULL != mReaderList[i] ) {
			delete mReaderList[i];
		}
	}
	free( mReaderList );
}

SP_XmlReader * SP_XmlReaderPool :: borrow( int type )
{
	SP_XmlReader * reader = NULL;

	if( type >= 0 && type < SP_XmlReader::MAX_READER ) {
		reader = mReaderList[ type ];
		if( NULL == reader ) {
			switch( type ) {
			case SP_XmlReader::eDocDecl: reader = new SP_XmlDocDeclReader(); break;
			case SP_XmlReader::eSTag: reader = new SP_XmlStartTagReader(); break;
			case SP_XmlReader::eETag: reader = new SP_XmlEndTagReader(); break;
			case SP_XmlReader::eCData: reader = new SP_XmlCDataReader(); break;
			case SP_XmlReader::eComment: reader = new SP_XmlCommentReader(); break;
			case SP_XmlReader::eDocType: reader = new SP_XmlDocTypeReader(); break;
			case SP_XmlReader::eLBracket: reader = new SP_XmlLeftBracketReader(); break;
			case SP_XmlReader::eSign: reader = new SP_XmlSignReader(); break;
			}
			mReaderList[ type ] = reader;
		}
	}

	//printf( "\nborrow change: %s\n", typeid( *reader ).name() );

	return reader;
}

void SP_XmlReaderPool :: save( SP_XmlReader * reader )
{
	//printf( "\nreturn change: %s\n", typeid( *reader ).name() );
	reader->reset();
}

//=========================================================
