/*
   Copyright 2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "User.h"
#include "Track.h"
#include "UrlBuilder.h"
#include "XmlQuery.h"
#include <QStringList>
#include "User.h"
#include <QAbstractNetworkCache>

using lastfm::User;
using lastfm::UserList;
using lastfm::User;
using lastfm::XmlQuery;
using lastfm::ImageSize;

User::User( const XmlQuery& xml ) 
     :AbstractType(),
     m_match( -1.0f ),
     m_age( 0 ),
     m_scrobbles( 0 ),
     m_registered( QDateTime() ),
     m_isSubscriber( false ),
     m_canBootstrap( false )
{
    m_name = xml["name"].text();
    m_images << xml["image size=small"].text()
             << xml["image size=medium"].text()
             << xml["image size=large"].text();
    m_realName = xml["realname"].text();

    QString type = xml["type"].text();

    if ( type == "subscriber" ) m_type = TypeSubscriber;
    else if ( type == "moderator" ) m_type = TypeModerator;
    else if ( type == "staff" ) m_type = TypeStaff;
    else if ( type == "alumni" ) m_type = TypeAlumni;
    else m_type = TypeUser;

    m_age = xml["age"].text().toUInt();
    m_scrobbles = xml["playcount"].text().toUInt();
    m_registered = QDateTime::fromTime_t(xml["registered"].attribute("unixtime").toUInt());
    m_country = xml["country"].text();
    m_isSubscriber = ( xml["subscriber"].text() == "1" );
    m_canBootstrap = ( xml["bootstrap"].text() == "1" );
    m_gender = xml["gender"].text();
    m_images << xml["image size=small"].text()
             << xml["image size=medium"].text()
             << xml["image size=large"].text()
             << xml["image size=extralarge"].text();
}

User::User()
    :AbstractType(),
    m_name( lastfm::ws::Username ),
    m_match( -1.0f ),
    m_age( 0 ),
    m_scrobbles( 0 ),
    m_registered( QDateTime() ),
    m_isSubscriber( false ),
    m_canBootstrap( false )
{}

User::User( const QString& name )
    :AbstractType(),
    m_name( name ),
    m_match( -1.0f ),
    m_age( 0 ),
    m_scrobbles( 0 ),
    m_registered( QDateTime() ),
    m_isSubscriber( false ),
    m_canBootstrap( false )
{}

lastfm::User&
User::operator=( const User& that )
{
    m_name = that.name();
    m_images = that.m_images;
    m_realName = that.m_realName;
    m_match = that.m_match;
    m_type = that.m_type;
    m_age = that.m_age;
    m_scrobbles = that.m_scrobbles;
    m_registered = that.m_registered;
    m_country = that.m_country;
    m_isSubscriber = that.m_isSubscriber;
    m_canBootstrap = that.m_canBootstrap;
    m_gender = that.m_gender;
    m_images = that.m_images;
    return *this;
}

QUrl 
User::imageUrl( ImageSize size, bool square ) const
{
    if( !square ) return m_images.value( size ); 

    QUrl url = m_images.value( size );
    QRegExp re( "/serve/(\\d*)s?/" );
    return QUrl( url.toString().replace( re, "/serve/\\1s/" ));
}


QMap<QString, QString>
User::params(const QString& method) const
{
    QMap<QString, QString> map;
    map["method"] = "user."+method;
    map["user"] = m_name;
    return map;
}


QNetworkReply*
User::getFriends( bool recentTracks, int limit, int page ) const
{
    QMap<QString, QString> map = params( "getFriends" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    if ( recentTracks ) map["recenttracks"] = "1";
    return ws::get( map );
}


QNetworkReply*
User::getFriendsListeningNow( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getFriendsListeningNow" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getLovedTracks( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getLovedTracks" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getTopTags() const
{
    return ws::get( params( "getTopTags" ) );
}


QNetworkReply*
User::getTopArtists( QString period, int limit, int page ) const
{
    QMap<QString, QString> map = params( "getTopArtists" );
    map["period"] = period;
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getRecentArtists() const
{
    return ws::get( params( "getRecentArtists" ) );
}


QNetworkReply*
User::getRecentTracks( int limit , int page ) const
{
    QMap<QString, QString> map = params( "getRecentTracks" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );

    QAbstractNetworkCache* cache = lastfm::nam()->cache();
    if ( cache )
        cache->remove( lastfm::ws::url( map ) );

    return ws::get( map );
}

QNetworkReply*
User::getRecentStations( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getRecentStations" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getRecommendedArtists( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getRecommendedArtists" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getNeighbours( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getNeighbours" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getPlaylists() const
{
    return ws::get( params( "getPlaylists" ) );
}


UserList //static
User::list( QNetworkReply* r )
{
    UserList users;

    XmlQuery lfm;

    if ( lfm.parse( r->readAll() ) )
    {
        foreach (XmlQuery e, lfm.children( "user" ))
        {
            User u( e );
            users += u;
        }

        users.total = lfm["friends"].attribute("total").toInt();
        users.page = lfm["friends"].attribute("page").toInt();
        users.perPage = lfm["friends"].attribute("perPage").toInt();
        users.totalPages = lfm["friends"].attribute("totalPages").toInt();
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }    
    return users;
}


QNetworkReply* //static
User::getInfo( const QString& username )
{
    QMap<QString, QString> map;
    map["method"] = "user.getInfo";
    map["user"] = username;
    return ws::post( map );
}




/*
QNetworkReply* //static
User::getRecommendedArtists()
{
    QMap<QString, QString> map;
    map["method"] = "user.getRecommendedArtists";
    return ws::post( map );
}
*/

QUrl
User::www() const
{ 
    return UrlBuilder( "user" ).slash( m_name ).url();
}


QString
User::getInfoString() const
{
    QString text;

    text = QObject::tr("%1").arg( m_realName.isEmpty() ? m_name : m_realName );
    if ( m_age ) text.append( QObject::tr(", %1").arg( m_age ) );
    if ( m_gender.known() ) text.append( QObject::tr(", %1").arg( m_gender.toString() ) );
    if ( !m_country.isEmpty() ) text.append( QObject::tr(", %1").arg( m_country ) );
    if ( m_scrobbles ) text.append( QObject::tr(", %L1 scrobbles").arg( m_scrobbles ) );

    return text;
}

void 
User::setScrobbleCount( quint32 scrobbleCount )
{
    m_scrobbles = scrobbleCount;
}


void
User::setDateRegistered( const QDateTime& date )
{
    m_registered = date;
}

void 
User::setImages( const QList<QUrl>& images )
{
    m_images = images;
}

void 
User::setRealName( const QString& realName )
{
    m_realName = realName;
}
void 
User::setAge( unsigned short age )
{
    m_age = age;
}

void 
User::setIsSubscriber( bool subscriber )
{
    m_isSubscriber = subscriber;
}

void 
User::setCanBootstrap( bool canBootstrap )
{
    m_canBootstrap = canBootstrap;
}

void 
User::setGender( const QString& s )
{
    m_gender = Gender( s );
}

void 
User::setCountry( const QString& country )
{
    m_country = country;
}

