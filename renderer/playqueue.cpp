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

#include "upnp/upnp.xml.parseutils.h"
#include "upnp/upnp.http.reader.h"

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
: m_trackUri(avTransportUri)
, m_avTransportUri(avTransportUri)
{
}

PlayQueueItem::PlayQueueItem(const std::string& trackUri, const std::string& avTransportUri)
: m_trackUri(trackUri)
, m_avTransportUri(avTransportUri)
{
}

std::string PlayQueueItem::getUri() const
{
    return m_trackUri;
}

void PlayQueueItem::setItem(const upnp::Item& item)
{
    m_item = item;
}

std::string PlayQueueItem::getAVTransportUri() const
{
    return m_avTransportUri;
}

std::string PlayQueueItem::getMetadataString() const
{
    if (m_item.getObjectId().empty())
    {
        return "";
    }

    return xml::getItemDocument(m_item);
}

const std::vector<uint8_t>& PlayQueueItem::getAlbumArt() const
{
    return m_albumArt;
}

const std::vector<uint8_t>& PlayQueueItem::getAlbumArtThumb() const
{
    return m_albumArtThumb;
}

void PlayQueueItem::setAlbumArtUri(const std::string& uri)
{
    m_item.addMetaData(upnp::Property::AlbumArt, uri);
}

void PlayQueueItem::setAlbumArtUri(const std::string& uri, upnp::dlna::ProfileId profile)
{
    m_item.setAlbumArt(profile, uri);
}

void PlayQueueItem::setAlbumArt(std::vector<uint8_t> data)
{
    m_albumArt = std::move(data);
}

void PlayQueueItem::setAlbumArtThumb(std::vector<uint8_t> data)
{
    m_albumArtThumb = std::move(data);
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

        auto item = Item();
        addMetaIfExists(item, Property::Title,          meta.getTitle());
        addMetaIfExists(item, Property::Artist,         meta.getArtist());
        addMetaIfExists(item, Property::Album,          meta.getAlbum());
        addMetaIfExists(item, Property::Genre,          meta.getGenre());

        addMetaIfExists(item, Property::TrackNumber,    meta.getTrackNr());
        addMetaIfExists(item, Property::Date,           meta.getYear());

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
    m_thread.start();
}

static std::vector<std::string> getTracksFromUri(const std::string& transportUri)
{
    std::vector<std::string> trackUris;

    const std::string extension = fileops::getFileExtension(transportUri);
    if (stringops::lowercase(extension) == "m3u")
    {
        upnp::http::Reader client;
        client.open(transportUri);

        auto m3ufile = client.readAllData();
        std::string m3uString(m3ufile.begin(), m3ufile.end());
        auto uris = audio::M3uParser::parseFileContents(m3uString);

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
            m_thread.addJob([item] () { obtainMetadata(item); });
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_tracksMutex);
        m_currenURITracks = items;
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
        m_thread.addJob([item] () { obtainMetadata(item); });
    }

    {
        std::lock_guard<std::mutex> lock(m_tracksMutex);
        m_nextURITracks = items;
    }

    NextTransportUriChanged(avTransportUri);
    QueueChanged();
}

std::string PlayQueue::getCurrentUri() const
{
    std::lock_guard<std::mutex> lock(m_tracksMutex);
    return m_currenURITracks.empty() ? "" : m_currenURITracks.front()->getAVTransportUri();
}

std::string PlayQueue::getNextUri() const
{
    std::lock_guard<std::mutex> lock(m_tracksMutex);
    return m_nextURITracks.empty() ? "" : m_nextURITracks.front()->getAVTransportUri();
}

void PlayQueue::clear()
{
    {
        std::lock_guard<std::mutex> lock(m_tracksMutex);
        m_currenURITracks.clear();
        m_nextURITracks.clear();
    }

    QueueChanged();
}

std::shared_ptr<audio::ITrack> PlayQueue::dequeueNextTrack()
{
    bool avTransportUriChange = false;
    PlayQueueItemPtr track;

    {
        std::lock_guard<std::mutex> lock(m_tracksMutex);
        if (m_currenURITracks.empty() && m_nextURITracks.empty())
        {
            return track;
        }
        else if (m_currenURITracks.empty() && !m_nextURITracks.empty())
        {
            std::swap(m_currenURITracks, m_nextURITracks);
            avTransportUriChange = true;
        }

        if (!m_currenURITracks.empty())
        {
            track = m_currenURITracks.front();
            m_currenURITracks.pop_front();
        }
    }

    if (avTransportUriChange)
    {
        CurrentTransportUriChanged(m_currenURITracks.front()->getAVTransportUri());
        NextTransportUriChanged("");
    }

    QueueChanged();

    return track;
}

size_t PlayQueue::getNumberOfTracks() const
{
    std::lock_guard<std::mutex> lock(m_tracksMutex);
    return m_currenURITracks.size();
}

}
