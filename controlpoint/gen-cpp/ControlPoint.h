/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef ControlPoint_H
#define ControlPoint_H

#include <thrift/TDispatchProcessor.h>
#include "controlpoint_types.h"

namespace doozy { namespace rpc {

class ControlPointIf {
 public:
  virtual ~ControlPointIf() {}
  virtual void GetRenderers(DeviceResponse& _return) = 0;
  virtual void GetServers(DeviceResponse& _return) = 0;
  virtual void Browse(BrowseResponse& _return, const BrowseRequest& req) = 0;
};

class ControlPointIfFactory {
 public:
  typedef ControlPointIf Handler;

  virtual ~ControlPointIfFactory() {}

  virtual ControlPointIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(ControlPointIf* /* handler */) = 0;
};

class ControlPointIfSingletonFactory : virtual public ControlPointIfFactory {
 public:
  ControlPointIfSingletonFactory(const boost::shared_ptr<ControlPointIf>& iface) : iface_(iface) {}
  virtual ~ControlPointIfSingletonFactory() {}

  virtual ControlPointIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(ControlPointIf* /* handler */) {}

 protected:
  boost::shared_ptr<ControlPointIf> iface_;
};

class ControlPointNull : virtual public ControlPointIf {
 public:
  virtual ~ControlPointNull() {}
  void GetRenderers(DeviceResponse& /* _return */) {
    return;
  }
  void GetServers(DeviceResponse& /* _return */) {
    return;
  }
  void Browse(BrowseResponse& /* _return */, const BrowseRequest& /* req */) {
    return;
  }
};


class ControlPoint_GetRenderers_args {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};

  ControlPoint_GetRenderers_args(const ControlPoint_GetRenderers_args&);
  ControlPoint_GetRenderers_args& operator=(const ControlPoint_GetRenderers_args&);
  ControlPoint_GetRenderers_args() {
  }

  virtual ~ControlPoint_GetRenderers_args() throw();

  bool operator == (const ControlPoint_GetRenderers_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const ControlPoint_GetRenderers_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ControlPoint_GetRenderers_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ControlPoint_GetRenderers_pargs {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};


  virtual ~ControlPoint_GetRenderers_pargs() throw();

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _ControlPoint_GetRenderers_result__isset {
  _ControlPoint_GetRenderers_result__isset() : success(false) {}
  bool success;
} _ControlPoint_GetRenderers_result__isset;

class ControlPoint_GetRenderers_result {
 public:

  static const char* ascii_fingerprint; // = "CBFF6EB1EDB1D2381B07A6FDC62945B3";
  static const uint8_t binary_fingerprint[16]; // = {0xCB,0xFF,0x6E,0xB1,0xED,0xB1,0xD2,0x38,0x1B,0x07,0xA6,0xFD,0xC6,0x29,0x45,0xB3};

  ControlPoint_GetRenderers_result(const ControlPoint_GetRenderers_result&);
  ControlPoint_GetRenderers_result& operator=(const ControlPoint_GetRenderers_result&);
  ControlPoint_GetRenderers_result() {
  }

  virtual ~ControlPoint_GetRenderers_result() throw();
  DeviceResponse success;

  _ControlPoint_GetRenderers_result__isset __isset;

  void __set_success(const DeviceResponse& val);

  bool operator == (const ControlPoint_GetRenderers_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const ControlPoint_GetRenderers_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ControlPoint_GetRenderers_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _ControlPoint_GetRenderers_presult__isset {
  _ControlPoint_GetRenderers_presult__isset() : success(false) {}
  bool success;
} _ControlPoint_GetRenderers_presult__isset;

class ControlPoint_GetRenderers_presult {
 public:

  static const char* ascii_fingerprint; // = "CBFF6EB1EDB1D2381B07A6FDC62945B3";
  static const uint8_t binary_fingerprint[16]; // = {0xCB,0xFF,0x6E,0xB1,0xED,0xB1,0xD2,0x38,0x1B,0x07,0xA6,0xFD,0xC6,0x29,0x45,0xB3};


  virtual ~ControlPoint_GetRenderers_presult() throw();
  DeviceResponse* success;

  _ControlPoint_GetRenderers_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class ControlPoint_GetServers_args {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};

  ControlPoint_GetServers_args(const ControlPoint_GetServers_args&);
  ControlPoint_GetServers_args& operator=(const ControlPoint_GetServers_args&);
  ControlPoint_GetServers_args() {
  }

  virtual ~ControlPoint_GetServers_args() throw();

  bool operator == (const ControlPoint_GetServers_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const ControlPoint_GetServers_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ControlPoint_GetServers_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ControlPoint_GetServers_pargs {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};


  virtual ~ControlPoint_GetServers_pargs() throw();

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _ControlPoint_GetServers_result__isset {
  _ControlPoint_GetServers_result__isset() : success(false) {}
  bool success;
} _ControlPoint_GetServers_result__isset;

class ControlPoint_GetServers_result {
 public:

  static const char* ascii_fingerprint; // = "CBFF6EB1EDB1D2381B07A6FDC62945B3";
  static const uint8_t binary_fingerprint[16]; // = {0xCB,0xFF,0x6E,0xB1,0xED,0xB1,0xD2,0x38,0x1B,0x07,0xA6,0xFD,0xC6,0x29,0x45,0xB3};

  ControlPoint_GetServers_result(const ControlPoint_GetServers_result&);
  ControlPoint_GetServers_result& operator=(const ControlPoint_GetServers_result&);
  ControlPoint_GetServers_result() {
  }

  virtual ~ControlPoint_GetServers_result() throw();
  DeviceResponse success;

  _ControlPoint_GetServers_result__isset __isset;

  void __set_success(const DeviceResponse& val);

  bool operator == (const ControlPoint_GetServers_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const ControlPoint_GetServers_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ControlPoint_GetServers_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _ControlPoint_GetServers_presult__isset {
  _ControlPoint_GetServers_presult__isset() : success(false) {}
  bool success;
} _ControlPoint_GetServers_presult__isset;

class ControlPoint_GetServers_presult {
 public:

  static const char* ascii_fingerprint; // = "CBFF6EB1EDB1D2381B07A6FDC62945B3";
  static const uint8_t binary_fingerprint[16]; // = {0xCB,0xFF,0x6E,0xB1,0xED,0xB1,0xD2,0x38,0x1B,0x07,0xA6,0xFD,0xC6,0x29,0x45,0xB3};


  virtual ~ControlPoint_GetServers_presult() throw();
  DeviceResponse* success;

  _ControlPoint_GetServers_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _ControlPoint_Browse_args__isset {
  _ControlPoint_Browse_args__isset() : req(false) {}
  bool req;
} _ControlPoint_Browse_args__isset;

class ControlPoint_Browse_args {
 public:

  static const char* ascii_fingerprint; // = "A756D3DBE614FB13F70BF7F7B6EB3D73";
  static const uint8_t binary_fingerprint[16]; // = {0xA7,0x56,0xD3,0xDB,0xE6,0x14,0xFB,0x13,0xF7,0x0B,0xF7,0xF7,0xB6,0xEB,0x3D,0x73};

  ControlPoint_Browse_args(const ControlPoint_Browse_args&);
  ControlPoint_Browse_args& operator=(const ControlPoint_Browse_args&);
  ControlPoint_Browse_args() {
  }

  virtual ~ControlPoint_Browse_args() throw();
  BrowseRequest req;

  _ControlPoint_Browse_args__isset __isset;

  void __set_req(const BrowseRequest& val);

  bool operator == (const ControlPoint_Browse_args & rhs) const
  {
    if (!(req == rhs.req))
      return false;
    return true;
  }
  bool operator != (const ControlPoint_Browse_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ControlPoint_Browse_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ControlPoint_Browse_pargs {
 public:

  static const char* ascii_fingerprint; // = "A756D3DBE614FB13F70BF7F7B6EB3D73";
  static const uint8_t binary_fingerprint[16]; // = {0xA7,0x56,0xD3,0xDB,0xE6,0x14,0xFB,0x13,0xF7,0x0B,0xF7,0xF7,0xB6,0xEB,0x3D,0x73};


  virtual ~ControlPoint_Browse_pargs() throw();
  const BrowseRequest* req;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _ControlPoint_Browse_result__isset {
  _ControlPoint_Browse_result__isset() : success(false) {}
  bool success;
} _ControlPoint_Browse_result__isset;

class ControlPoint_Browse_result {
 public:

  static const char* ascii_fingerprint; // = "CBFF6EB1EDB1D2381B07A6FDC62945B3";
  static const uint8_t binary_fingerprint[16]; // = {0xCB,0xFF,0x6E,0xB1,0xED,0xB1,0xD2,0x38,0x1B,0x07,0xA6,0xFD,0xC6,0x29,0x45,0xB3};

  ControlPoint_Browse_result(const ControlPoint_Browse_result&);
  ControlPoint_Browse_result& operator=(const ControlPoint_Browse_result&);
  ControlPoint_Browse_result() {
  }

  virtual ~ControlPoint_Browse_result() throw();
  BrowseResponse success;

  _ControlPoint_Browse_result__isset __isset;

  void __set_success(const BrowseResponse& val);

  bool operator == (const ControlPoint_Browse_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const ControlPoint_Browse_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ControlPoint_Browse_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _ControlPoint_Browse_presult__isset {
  _ControlPoint_Browse_presult__isset() : success(false) {}
  bool success;
} _ControlPoint_Browse_presult__isset;

class ControlPoint_Browse_presult {
 public:

  static const char* ascii_fingerprint; // = "CBFF6EB1EDB1D2381B07A6FDC62945B3";
  static const uint8_t binary_fingerprint[16]; // = {0xCB,0xFF,0x6E,0xB1,0xED,0xB1,0xD2,0x38,0x1B,0x07,0xA6,0xFD,0xC6,0x29,0x45,0xB3};


  virtual ~ControlPoint_Browse_presult() throw();
  BrowseResponse* success;

  _ControlPoint_Browse_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class ControlPointClient : virtual public ControlPointIf {
 public:
  ControlPointClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  ControlPointClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void GetRenderers(DeviceResponse& _return);
  void send_GetRenderers();
  void recv_GetRenderers(DeviceResponse& _return);
  void GetServers(DeviceResponse& _return);
  void send_GetServers();
  void recv_GetServers(DeviceResponse& _return);
  void Browse(BrowseResponse& _return, const BrowseRequest& req);
  void send_Browse(const BrowseRequest& req);
  void recv_Browse(BrowseResponse& _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class ControlPointProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<ControlPointIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (ControlPointProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_GetRenderers(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_GetServers(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_Browse(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  ControlPointProcessor(boost::shared_ptr<ControlPointIf> iface) :
    iface_(iface) {
    processMap_["GetRenderers"] = &ControlPointProcessor::process_GetRenderers;
    processMap_["GetServers"] = &ControlPointProcessor::process_GetServers;
    processMap_["Browse"] = &ControlPointProcessor::process_Browse;
  }

  virtual ~ControlPointProcessor() {}
};

class ControlPointProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  ControlPointProcessorFactory(const ::boost::shared_ptr< ControlPointIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< ControlPointIfFactory > handlerFactory_;
};

class ControlPointMultiface : virtual public ControlPointIf {
 public:
  ControlPointMultiface(std::vector<boost::shared_ptr<ControlPointIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~ControlPointMultiface() {}
 protected:
  std::vector<boost::shared_ptr<ControlPointIf> > ifaces_;
  ControlPointMultiface() {}
  void add(boost::shared_ptr<ControlPointIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void GetRenderers(DeviceResponse& _return) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->GetRenderers(_return);
    }
    ifaces_[i]->GetRenderers(_return);
    return;
  }

  void GetServers(DeviceResponse& _return) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->GetServers(_return);
    }
    ifaces_[i]->GetServers(_return);
    return;
  }

  void Browse(BrowseResponse& _return, const BrowseRequest& req) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->Browse(_return, req);
    }
    ifaces_[i]->Browse(_return, req);
    return;
  }

};

}} // namespace

#endif
