#ifndef DOOZY_DOOZYSCHEME_H
#define DOOZY_DOOZYSCHEME_H

#include <sqlpp11/table.h>
#include <sqlpp11/column_types.h>

namespace doozy
{
  namespace Objects_
  {
    struct Id
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Id"; }
        template<typename T>
        struct _member_t
          {
            T Id;
            T& operator()() { return Id; }
            const T& operator()() const { return Id; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct ObjectId
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "ObjectId"; }
        template<typename T>
        struct _member_t
          {
            T ObjectId;
            T& operator()() { return ObjectId; }
            const T& operator()() const { return ObjectId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct ParentId
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "ParentId"; }
        template<typename T>
        struct _member_t
          {
            T ParentId;
            T& operator()() { return ParentId; }
            const T& operator()() const { return ParentId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct RefId
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "RefId"; }
        template<typename T>
        struct _member_t
          {
            T RefId;
            T& operator()() { return RefId; }
            const T& operator()() const { return RefId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct Name
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Name"; }
        template<typename T>
        struct _member_t
          {
            T Name;
            T& operator()() { return Name; }
            const T& operator()() const { return Name; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
    struct Class
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Class"; }
        template<typename T>
        struct _member_t
          {
            T Class;
            T& operator()() { return Class; }
            const T& operator()() const { return Class; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct MetaData
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "MetaData"; }
        template<typename T>
        struct _member_t
          {
            T MetaData;
            T& operator()() { return MetaData; }
            const T& operator()() const { return MetaData; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
  }

  struct Objects: sqlpp::table_t<Objects,
               Objects_::Id,
               Objects_::ObjectId,
               Objects_::ParentId,
               Objects_::RefId,
               Objects_::Name,
               Objects_::Class,
               Objects_::MetaData>
  {
    using _value_type = sqlpp::no_value_t;
    struct _name_t
    {
      static constexpr const char* _get_name() { return "objects"; }
      template<typename T>
      struct _member_t
      {
        T objects;
        T& operator()() { return objects; }
        const T& operator()() const { return objects; }
      };
    };
  };
  namespace Metadata_
  {
    struct Id
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Id"; }
        template<typename T>
        struct _member_t
          {
            T Id;
            T& operator()() { return Id; }
            const T& operator()() const { return Id; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Album
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Album"; }
        template<typename T>
        struct _member_t
          {
            T Album;
            T& operator()() { return Album; }
            const T& operator()() const { return Album; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct Artist
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Artist"; }
        template<typename T>
        struct _member_t
          {
            T Artist;
            T& operator()() { return Artist; }
            const T& operator()() const { return Artist; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct Title
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Title"; }
        template<typename T>
        struct _member_t
          {
            T Title;
            T& operator()() { return Title; }
            const T& operator()() const { return Title; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct AlbumArtist
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "AlbumArtist"; }
        template<typename T>
        struct _member_t
          {
            T AlbumArtist;
            T& operator()() { return AlbumArtist; }
            const T& operator()() const { return AlbumArtist; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct Genre
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Genre"; }
        template<typename T>
        struct _member_t
          {
            T Genre;
            T& operator()() { return Genre; }
            const T& operator()() const { return Genre; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct Composer
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Composer"; }
        template<typename T>
        struct _member_t
          {
            T Composer;
            T& operator()() { return Composer; }
            const T& operator()() const { return Composer; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct Year
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Year"; }
        template<typename T>
        struct _member_t
          {
            T Year;
            T& operator()() { return Year; }
            const T& operator()() const { return Year; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct TrackNr
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "TrackNr"; }
        template<typename T>
        struct _member_t
          {
            T TrackNr;
            T& operator()() { return TrackNr; }
            const T& operator()() const { return TrackNr; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct DiscNr
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "DiscNr"; }
        template<typename T>
        struct _member_t
          {
            T DiscNr;
            T& operator()() { return DiscNr; }
            const T& operator()() const { return DiscNr; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct AlbumOrder
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "AlbumOrder"; }
        template<typename T>
        struct _member_t
          {
            T AlbumOrder;
            T& operator()() { return AlbumOrder; }
            const T& operator()() const { return AlbumOrder; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Duration
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Duration"; }
        template<typename T>
        struct _member_t
          {
            T Duration;
            T& operator()() { return Duration; }
            const T& operator()() const { return Duration; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct MimeType
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "MimeType"; }
        template<typename T>
        struct _member_t
          {
            T MimeType;
            T& operator()() { return MimeType; }
            const T& operator()() const { return MimeType; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct BitRate
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "BitRate"; }
        template<typename T>
        struct _member_t
          {
            T BitRate;
            T& operator()() { return BitRate; }
            const T& operator()() const { return BitRate; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct SampleRate
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "SampleRate"; }
        template<typename T>
        struct _member_t
          {
            T SampleRate;
            T& operator()() { return SampleRate; }
            const T& operator()() const { return SampleRate; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Channels
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Channels"; }
        template<typename T>
        struct _member_t
          {
            T Channels;
            T& operator()() { return Channels; }
            const T& operator()() const { return Channels; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct FileSize
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "FileSize"; }
        template<typename T>
        struct _member_t
          {
            T FileSize;
            T& operator()() { return FileSize; }
            const T& operator()() const { return FileSize; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct DateAdded
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "DateAdded"; }
        template<typename T>
        struct _member_t
          {
            T DateAdded;
            T& operator()() { return DateAdded; }
            const T& operator()() const { return DateAdded; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct ModifiedTime
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "ModifiedTime"; }
        template<typename T>
        struct _member_t
          {
            T ModifiedTime;
            T& operator()() { return ModifiedTime; }
            const T& operator()() const { return ModifiedTime; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Thumbnail
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "Thumbnail"; }
        template<typename T>
        struct _member_t
          {
            T Thumbnail;
            T& operator()() { return Thumbnail; }
            const T& operator()() const { return Thumbnail; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct FilePath
    {
      struct _name_t
      {
        static constexpr const char* _get_name() { return "FilePath"; }
        template<typename T>
        struct _member_t
          {
            T FilePath;
            T& operator()() { return FilePath; }
            const T& operator()() const { return FilePath; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
  }

  struct Metadata: sqlpp::table_t<Metadata,
               Metadata_::Id,
               Metadata_::Album,
               Metadata_::Artist,
               Metadata_::Title,
               Metadata_::AlbumArtist,
               Metadata_::Genre,
               Metadata_::Composer,
               Metadata_::Year,
               Metadata_::TrackNr,
               Metadata_::DiscNr,
               Metadata_::AlbumOrder,
               Metadata_::Duration,
               Metadata_::MimeType,
               Metadata_::BitRate,
               Metadata_::SampleRate,
               Metadata_::Channels,
               Metadata_::FileSize,
               Metadata_::DateAdded,
               Metadata_::ModifiedTime,
               Metadata_::Thumbnail,
               Metadata_::FilePath>
  {
    using _value_type = sqlpp::no_value_t;
    struct _name_t
    {
      static constexpr const char* _get_name() { return "metadata"; }
      template<typename T>
      struct _member_t
      {
        T metadata;
        T& operator()() { return metadata; }
        const T& operator()() const { return metadata; }
      };
    };
  };
}
#endif
