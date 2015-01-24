//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "playqueue.h"

#include "utils/log.h"
#include "utils/fileoperations.h"
#include "utils/stringoperations.h"

#include "upnp/upnpxmlutils.h"
#include "upnp/upnphttpclient.h"

#include "audioconfig.h"
#include "audio/audiom3uparser.h"
#include "audio/audiometadata.h"

#include "image/image.h"
#include "image/imageloadstoreinterface.h"
#include "image/imagefactory.h"

#include <fstream>
#include <cstring>
#include <cassert>

using namespace utils;
using namespace upnp;
using namespace image;

namespace doozy
{
    
#ifndef HAVE_TAGLIB
    static_assert(false, "Audio library not compiled with metadata support");
#endif

PlayQueueItem::PlayQueueItem(const std::string& avTransportUri)
: m_TrackUri(avTransportUri)
, m_AVTransportUri(avTransportUri)
{
}

PlayQueueItem::PlayQueueItem(const std::string& trackUri, const std::string& avTransportUri)
: m_TrackUri(trackUri)
, m_AVTransportUri(avTransportUri)
{
}

std::string PlayQueueItem::getUri() const
{
    return m_TrackUri;
}

void PlayQueueItem::setItem(const upnp::ItemPtr& item)
{
    m_Item = item;
}

std::string PlayQueueItem::getAVTransportUri() const
{
    return m_AVTransportUri;
}

std::string PlayQueueItem::getMetadataString() const
{
    if (!m_Item)
    {
        return "";
    }

    return xml::utils::getItemDocument(*m_Item).toString();
}

const std::vector<uint8_t>& PlayQueueItem::getAlbumArt() const
{
    return m_AlbumArt;
}

const std::vector<uint8_t>& PlayQueueItem::getAlbumArtThumb() const
{
    return m_AlbumArtThumb;
}

void PlayQueueItem::setAlbumArtUri(const std::string& uri)
{
    m_Item->addMetaData(upnp::Property::AlbumArt, uri);
}

void PlayQueueItem::setAlbumArtUri(const std::string& uri, upnp::dlna::ProfileId profile)
{
    m_Item->setAlbumArt(profile, uri);
}

void PlayQueueItem::setAlbumArt(std::vector<uint8_t>&& data)
{
    m_AlbumArt = std::move(data);
}

void PlayQueueItem::setAlbumArt(const std::vector<uint8_t>& data)
{
    m_AlbumArt = data;
}

void PlayQueueItem::setAlbumArtThumb(std::vector<uint8_t>&& data)
{
    m_AlbumArtThumb = std::move(data);
}

void PlayQueueItem::setAlbumArtThumb(const std::vector<uint8_t>& data)
{
    m_AlbumArtThumb = data;
}

static void addMetaIfExists(Item& item, Property prop, uint32_t value)
{
    if (value != 0)
    {
        item.addMetaData(prop, std::to_string(value));
    }
}

static void addMetaIfExists(Item& item, Property prop, const std::string& value)
{
    if (!value.empty())
    {
        item.addMetaData(prop, value);
    }
}

static void convertImageToJpeg(std::vector<uint8_t>& data)
{
    try
    {
        auto image = Factory::createFromData(data);
        auto loadStore = Factory::createLoadStore(Type::Jpeg);
        
        data = loadStore->storeToMemory(*image);
    }
    catch (std::exception& e)
    {
        throw std::runtime_error(fmt::format("Failed to convert image: {}", e.what()));
    }
}

static void obtainMetadata(PlayQueueItemPtr qItem)
{
    try
    {
        audio::Metadata meta(qItem->getUri(), audio::Metadata::ReadAudioProperties::No);
        
        auto item = std::make_shared<Item>();
        addMetaIfExists(*item, Property::Title,          meta.getTitle());
        addMetaIfExists(*item, Property::Artist,         meta.getArtist());
        addMetaIfExists(*item, Property::Album,          meta.getAlbum());
        addMetaIfExists(*item, Property::Genre,          meta.getGenre());
        
        addMetaIfExists(*item, Property::TrackNumber,    meta.getTrackNr());
        addMetaIfExists(*item, Property::Date,           meta.getYear());
        
        qItem->setItem(item);
        
        auto art = meta.getAlbumArt();
        if (!art.data.empty())
        {
            try
            {
                if (art.format != audio::ImageFormat::Jpeg)
                {
                    convertImageToJpeg(art.data);
                }
            
                qItem->setAlbumArt(art.data);
            }
            catch (std::exception& e)
            {
                log::warn("Failed to set album art: {}", e.what());
            }
            
            try
            {
                auto image = Factory::createFromData(art.data, Type::Jpeg);
                image->resize(160, 160, ResizeAlgorithm::Bilinear);
                
                auto jpegStore = Factory::createLoadStore(Type::Jpeg);
                qItem->setAlbumArtThumb(jpegStore->storeToMemory(*image));
            }
            catch (std::exception& e)
            {
                log::warn("Failed to set album art thumbnail: {}", e.what());
            }
        }
    }
    catch (std::exception& e)
    {
        log::warn("Failed to obtain metadata for url: {}", qItem->getUri());
    }
}

PlayQueue::PlayQueue()
{
    m_Thread.start();
}

static std::vector<std::string> getTracksFromUri(const std::string& transportUri)
{
    std::vector<std::string> trackUris;

    const std::string extension = fileops::getFileExtension(transportUri);
    if (stringops::lowercase(extension) == "m3u")
    {
        upnp::HttpClient client(5);
        auto m3ufile = client.getText(transportUri);
        auto uris = audio::M3uParser::parseFileContents(m3ufile);
        
        for (auto& uri : uris)
        {
            trackUris.push_back(uri);
        }
    }
    else
    {
        trackUris.push_back(transportUri);
    }
    
    return trackUris;
}

void PlayQueue::setCurrentUri(const std::string& avTransportUri)
{
    // fetch the possible playlist on the callers thread so we can indicate
    // if it was successful
    auto uris = getTracksFromUri(avTransportUri);

    std::deque<PlayQueueItemPtr> items;
    bool first = true;
    for (auto& uri : uris)
    {
        log::debug("Add item: {} : {}", uri, avTransportUri);
        auto item = std::make_shared<PlayQueueItem>(uri, avTransportUri);
        items.emplace_back(item);
        
        if (first)
        {
            // fetch the metadata of the first item on the callers thread to
            // have it immediately avaiable
            
            first = false;
            obtainMetadata(item);
        }
        else
        {
            // the rest of the metadata is fetched on the worker thread as it can
            // take a while to fetch all the metadata over the network
            m_Thread.addJob([item] () { obtainMetadata(item); });
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(m_TracksMutex);
        m_CurrenURITracks = items;
    }
    
    CurrentTransportUriChanged(avTransportUri);
    QueueChanged();
}

void PlayQueue::setNextUri(const std::string& avTransportUri)
{
    // fetch the possible playlist on the callers thread so we can indicate
    // if it was successful
    auto uris = getTracksFromUri(avTransportUri);
    
    std::deque<PlayQueueItemPtr> items;
    for (auto& uri : uris)
    {
        log::debug("Add next item: {} : {}", uri, avTransportUri);
        auto item = std::make_shared<PlayQueueItem>(uri, avTransportUri);
        items.push_back(item);
        m_Thread.addJob([item] () { obtainMetadata(item); });
    }
    
    {
        std::lock_guard<std::mutex> lock(m_TracksMutex);
        m_NextURITracks = items;
    }
    
    NextTransportUriChanged(avTransportUri);
    QueueChanged();
}

std::string PlayQueue::getCurrentUri() const
{
    std::lock_guard<std::mutex> lock(m_TracksMutex);
    return m_CurrenURITracks.empty() ? "" : m_CurrenURITracks.front()->getAVTransportUri();
}

std::string PlayQueue::getNextUri() const
{
    std::lock_guard<std::mutex> lock(m_TracksMutex);
    return m_NextURITracks.empty() ? "" : m_NextURITracks.front()->getAVTransportUri();
}

void PlayQueue::clear()
{
    {
        std::lock_guard<std::mutex> lock(m_TracksMutex);
        m_CurrenURITracks.clear();
        m_NextURITracks.clear();
    }
    
    QueueChanged();
}

std::shared_ptr<audio::ITrack> PlayQueue::dequeueNextTrack()
{
    bool avTransportUriChange = false;
    PlayQueueItemPtr track;

    {
        std::lock_guard<std::mutex> lock(m_TracksMutex);
        if (m_CurrenURITracks.empty() && m_NextURITracks.empty())
        {
            return track;
        }
        else if (m_CurrenURITracks.empty() && !m_NextURITracks.empty())
        {
            std::swap(m_CurrenURITracks, m_NextURITracks);
            avTransportUriChange = true;
        }
        
        if (!m_CurrenURITracks.empty())
        {
            track = m_CurrenURITracks.front();
            m_CurrenURITracks.pop_front();
        }
    }
    
    if (avTransportUriChange)
    {
        CurrentTransportUriChanged(m_CurrenURITracks.front()->getAVTransportUri());
        NextTransportUriChanged("");
    }
    
    QueueChanged();
    
    return track;
}

size_t PlayQueue::getNumberOfTracks() const
{
    std::lock_guard<std::mutex> lock(m_TracksMutex);
    return m_CurrenURITracks.size();
}

}
