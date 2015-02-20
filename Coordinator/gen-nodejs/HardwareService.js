//
// Autogenerated by Thrift Compiler (0.9.2)
//
// DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
//
var thrift = require('thrift');
var Thrift = thrift.Thrift;
var Q = thrift.Q;


var ttypes = require('./HardwareService_types');
//HELPER FUNCTIONS AND STRUCTURES

HardwareService_applyConfiguration_args = function(args) {
  this.jsonDocumentText = null;
  if (args) {
    if (args.jsonDocumentText !== undefined) {
      this.jsonDocumentText = args.jsonDocumentText;
    }
  }
};
HardwareService_applyConfiguration_args.prototype = {};
HardwareService_applyConfiguration_args.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
      if (ftype == Thrift.Type.STRING) {
        this.jsonDocumentText = input.readString();
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_applyConfiguration_args.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_applyConfiguration_args');
  if (this.jsonDocumentText !== null && this.jsonDocumentText !== undefined) {
    output.writeFieldBegin('jsonDocumentText', Thrift.Type.STRING, 1);
    output.writeString(this.jsonDocumentText);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_applyConfiguration_result = function(args) {
  this.ouch = null;
  if (args instanceof ttypes.InvalidOperation) {
    this.ouch = args;
    return;
  }
  if (args) {
    if (args.ouch !== undefined) {
      this.ouch = args.ouch;
    }
  }
};
HardwareService_applyConfiguration_result.prototype = {};
HardwareService_applyConfiguration_result.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
      if (ftype == Thrift.Type.STRUCT) {
        this.ouch = new ttypes.InvalidOperation();
        this.ouch.read(input);
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_applyConfiguration_result.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_applyConfiguration_result');
  if (this.ouch !== null && this.ouch !== undefined) {
    output.writeFieldBegin('ouch', Thrift.Type.STRUCT, 1);
    this.ouch.write(output);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_startPump_args = function(args) {
  this.pumpId = null;
  if (args) {
    if (args.pumpId !== undefined) {
      this.pumpId = args.pumpId;
    }
  }
};
HardwareService_startPump_args.prototype = {};
HardwareService_startPump_args.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
      if (ftype == Thrift.Type.I32) {
        this.pumpId = input.readI32();
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_startPump_args.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_startPump_args');
  if (this.pumpId !== null && this.pumpId !== undefined) {
    output.writeFieldBegin('pumpId', Thrift.Type.I32, 1);
    output.writeI32(this.pumpId);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_startPump_result = function(args) {
  this.ouch = null;
  if (args instanceof ttypes.InvalidOperation) {
    this.ouch = args;
    return;
  }
  if (args) {
    if (args.ouch !== undefined) {
      this.ouch = args.ouch;
    }
  }
};
HardwareService_startPump_result.prototype = {};
HardwareService_startPump_result.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
      if (ftype == Thrift.Type.STRUCT) {
        this.ouch = new ttypes.InvalidOperation();
        this.ouch.read(input);
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_startPump_result.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_startPump_result');
  if (this.ouch !== null && this.ouch !== undefined) {
    output.writeFieldBegin('ouch', Thrift.Type.STRUCT, 1);
    this.ouch.write(output);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_stopPump_args = function(args) {
  this.pumpId = null;
  if (args) {
    if (args.pumpId !== undefined) {
      this.pumpId = args.pumpId;
    }
  }
};
HardwareService_stopPump_args.prototype = {};
HardwareService_stopPump_args.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
      if (ftype == Thrift.Type.I32) {
        this.pumpId = input.readI32();
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_stopPump_args.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_stopPump_args');
  if (this.pumpId !== null && this.pumpId !== undefined) {
    output.writeFieldBegin('pumpId', Thrift.Type.I32, 1);
    output.writeI32(this.pumpId);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_stopPump_result = function(args) {
  this.success = null;
  this.ouch = null;
  if (args instanceof ttypes.InvalidOperation) {
    this.ouch = args;
    return;
  }
  if (args) {
    if (args.success !== undefined) {
      this.success = args.success;
    }
    if (args.ouch !== undefined) {
      this.ouch = args.ouch;
    }
  }
};
HardwareService_stopPump_result.prototype = {};
HardwareService_stopPump_result.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 0:
      if (ftype == Thrift.Type.STRUCT) {
        this.success = new ttypes.StopPumpResult();
        this.success.read(input);
      } else {
        input.skip(ftype);
      }
      break;
      case 1:
      if (ftype == Thrift.Type.STRUCT) {
        this.ouch = new ttypes.InvalidOperation();
        this.ouch.read(input);
      } else {
        input.skip(ftype);
      }
      break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_stopPump_result.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_stopPump_result');
  if (this.success !== null && this.success !== undefined) {
    output.writeFieldBegin('success', Thrift.Type.STRUCT, 0);
    this.success.write(output);
    output.writeFieldEnd();
  }
  if (this.ouch !== null && this.ouch !== undefined) {
    output.writeFieldBegin('ouch', Thrift.Type.STRUCT, 1);
    this.ouch.write(output);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_getServiceStatus_args = function(args) {
};
HardwareService_getServiceStatus_args.prototype = {};
HardwareService_getServiceStatus_args.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    input.skip(ftype);
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_getServiceStatus_args.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_getServiceStatus_args');
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_getServiceStatus_result = function(args) {
  this.success = null;
  if (args) {
    if (args.success !== undefined) {
      this.success = args.success;
    }
  }
};
HardwareService_getServiceStatus_result.prototype = {};
HardwareService_getServiceStatus_result.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 0:
      if (ftype == Thrift.Type.STRUCT) {
        this.success = new ttypes.ServiceStatus();
        this.success.read(input);
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_getServiceStatus_result.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_getServiceStatus_result');
  if (this.success !== null && this.success !== undefined) {
    output.writeFieldBegin('success', Thrift.Type.STRUCT, 0);
    this.success.write(output);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_getServiceStateJson_args = function(args) {
};
HardwareService_getServiceStateJson_args.prototype = {};
HardwareService_getServiceStateJson_args.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    input.skip(ftype);
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_getServiceStateJson_args.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_getServiceStateJson_args');
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_getServiceStateJson_result = function(args) {
  this.success = null;
  if (args) {
    if (args.success !== undefined) {
      this.success = args.success;
    }
  }
};
HardwareService_getServiceStateJson_result.prototype = {};
HardwareService_getServiceStateJson_result.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 0:
      if (ftype == Thrift.Type.STRING) {
        this.success = input.readString();
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_getServiceStateJson_result.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_getServiceStateJson_result');
  if (this.success !== null && this.success !== undefined) {
    output.writeFieldBegin('success', Thrift.Type.STRING, 0);
    output.writeString(this.success);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_fillVisibleContainerMillilitres_args = function(args) {
  this.amount = null;
  if (args) {
    if (args.amount !== undefined) {
      this.amount = args.amount;
    }
  }
};
HardwareService_fillVisibleContainerMillilitres_args.prototype = {};
HardwareService_fillVisibleContainerMillilitres_args.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
      if (ftype == Thrift.Type.I32) {
        this.amount = input.readI32();
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_fillVisibleContainerMillilitres_args.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_fillVisibleContainerMillilitres_args');
  if (this.amount !== null && this.amount !== undefined) {
    output.writeFieldBegin('amount', Thrift.Type.I32, 1);
    output.writeI32(this.amount);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_fillVisibleContainerMillilitres_result = function(args) {
  this.ouch = null;
  if (args instanceof ttypes.InvalidOperation) {
    this.ouch = args;
    return;
  }
  if (args) {
    if (args.ouch !== undefined) {
      this.ouch = args.ouch;
    }
  }
};
HardwareService_fillVisibleContainerMillilitres_result.prototype = {};
HardwareService_fillVisibleContainerMillilitres_result.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
      if (ftype == Thrift.Type.STRUCT) {
        this.ouch = new ttypes.InvalidOperation();
        this.ouch.read(input);
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_fillVisibleContainerMillilitres_result.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_fillVisibleContainerMillilitres_result');
  if (this.ouch !== null && this.ouch !== undefined) {
    output.writeFieldBegin('ouch', Thrift.Type.STRUCT, 1);
    this.ouch.write(output);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_emptyVisiableContainerMillilitres_args = function(args) {
  this.amount = null;
  if (args) {
    if (args.amount !== undefined) {
      this.amount = args.amount;
    }
  }
};
HardwareService_emptyVisiableContainerMillilitres_args.prototype = {};
HardwareService_emptyVisiableContainerMillilitres_args.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
      if (ftype == Thrift.Type.I32) {
        this.amount = input.readI32();
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_emptyVisiableContainerMillilitres_args.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_emptyVisiableContainerMillilitres_args');
  if (this.amount !== null && this.amount !== undefined) {
    output.writeFieldBegin('amount', Thrift.Type.I32, 1);
    output.writeI32(this.amount);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareService_emptyVisiableContainerMillilitres_result = function(args) {
  this.ouch = null;
  if (args instanceof ttypes.InvalidOperation) {
    this.ouch = args;
    return;
  }
  if (args) {
    if (args.ouch !== undefined) {
      this.ouch = args.ouch;
    }
  }
};
HardwareService_emptyVisiableContainerMillilitres_result.prototype = {};
HardwareService_emptyVisiableContainerMillilitres_result.prototype.read = function(input) {
  input.readStructBegin();
  while (true)
  {
    var ret = input.readFieldBegin();
    var fname = ret.fname;
    var ftype = ret.ftype;
    var fid = ret.fid;
    if (ftype == Thrift.Type.STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
      if (ftype == Thrift.Type.STRUCT) {
        this.ouch = new ttypes.InvalidOperation();
        this.ouch.read(input);
      } else {
        input.skip(ftype);
      }
      break;
      case 0:
        input.skip(ftype);
        break;
      default:
        input.skip(ftype);
    }
    input.readFieldEnd();
  }
  input.readStructEnd();
  return;
};

HardwareService_emptyVisiableContainerMillilitres_result.prototype.write = function(output) {
  output.writeStructBegin('HardwareService_emptyVisiableContainerMillilitres_result');
  if (this.ouch !== null && this.ouch !== undefined) {
    output.writeFieldBegin('ouch', Thrift.Type.STRUCT, 1);
    this.ouch.write(output);
    output.writeFieldEnd();
  }
  output.writeFieldStop();
  output.writeStructEnd();
  return;
};

HardwareServiceClient = exports.Client = function(output, pClass) {
    this.output = output;
    this.pClass = pClass;
    this._seqid = 0;
    this._reqs = {};
};
HardwareServiceClient.prototype = {};
HardwareServiceClient.prototype.seqid = function() { return this._seqid; }
HardwareServiceClient.prototype.new_seqid = function() { return this._seqid += 1; }
HardwareServiceClient.prototype.applyConfiguration = function(jsonDocumentText, callback) {
  this._seqid = this.new_seqid();
  if (callback === undefined) {
    var _defer = Q.defer();
    this._reqs[this.seqid()] = function(error, result) {
      if (error) {
        _defer.reject(error);
      } else {
        _defer.resolve(result);
      }
    };
    this.send_applyConfiguration(jsonDocumentText);
    return _defer.promise;
  } else {
    this._reqs[this.seqid()] = callback;
    this.send_applyConfiguration(jsonDocumentText);
  }
};

HardwareServiceClient.prototype.send_applyConfiguration = function(jsonDocumentText) {
  var output = new this.pClass(this.output);
  output.writeMessageBegin('applyConfiguration', Thrift.MessageType.CALL, this.seqid());
  var args = new HardwareService_applyConfiguration_args();
  args.jsonDocumentText = jsonDocumentText;
  args.write(output);
  output.writeMessageEnd();
  return this.output.flush();
};

HardwareServiceClient.prototype.recv_applyConfiguration = function(input,mtype,rseqid) {
  var callback = this._reqs[rseqid] || function() {};
  delete this._reqs[rseqid];
  if (mtype == Thrift.MessageType.EXCEPTION) {
    var x = new Thrift.TApplicationException();
    x.read(input);
    input.readMessageEnd();
    return callback(x);
  }
  var result = new HardwareService_applyConfiguration_result();
  result.read(input);
  input.readMessageEnd();

  if (null !== result.ouch) {
    return callback(result.ouch);
  }
  callback(null)
};
HardwareServiceClient.prototype.startPump = function(pumpId, callback) {
  this._seqid = this.new_seqid();
  if (callback === undefined) {
    var _defer = Q.defer();
    this._reqs[this.seqid()] = function(error, result) {
      if (error) {
        _defer.reject(error);
      } else {
        _defer.resolve(result);
      }
    };
    this.send_startPump(pumpId);
    return _defer.promise;
  } else {
    this._reqs[this.seqid()] = callback;
    this.send_startPump(pumpId);
  }
};

HardwareServiceClient.prototype.send_startPump = function(pumpId) {
  var output = new this.pClass(this.output);
  output.writeMessageBegin('startPump', Thrift.MessageType.CALL, this.seqid());
  var args = new HardwareService_startPump_args();
  args.pumpId = pumpId;
  args.write(output);
  output.writeMessageEnd();
  return this.output.flush();
};

HardwareServiceClient.prototype.recv_startPump = function(input,mtype,rseqid) {
  var callback = this._reqs[rseqid] || function() {};
  delete this._reqs[rseqid];
  if (mtype == Thrift.MessageType.EXCEPTION) {
    var x = new Thrift.TApplicationException();
    x.read(input);
    input.readMessageEnd();
    return callback(x);
  }
  var result = new HardwareService_startPump_result();
  result.read(input);
  input.readMessageEnd();

  if (null !== result.ouch) {
    return callback(result.ouch);
  }
  callback(null)
};
HardwareServiceClient.prototype.stopPump = function(pumpId, callback) {
  this._seqid = this.new_seqid();
  if (callback === undefined) {
    var _defer = Q.defer();
    this._reqs[this.seqid()] = function(error, result) {
      if (error) {
        _defer.reject(error);
      } else {
        _defer.resolve(result);
      }
    };
    this.send_stopPump(pumpId);
    return _defer.promise;
  } else {
    this._reqs[this.seqid()] = callback;
    this.send_stopPump(pumpId);
  }
};

HardwareServiceClient.prototype.send_stopPump = function(pumpId) {
  var output = new this.pClass(this.output);
  output.writeMessageBegin('stopPump', Thrift.MessageType.CALL, this.seqid());
  var args = new HardwareService_stopPump_args();
  args.pumpId = pumpId;
  args.write(output);
  output.writeMessageEnd();
  return this.output.flush();
};

HardwareServiceClient.prototype.recv_stopPump = function(input,mtype,rseqid) {
  var callback = this._reqs[rseqid] || function() {};
  delete this._reqs[rseqid];
  if (mtype == Thrift.MessageType.EXCEPTION) {
    var x = new Thrift.TApplicationException();
    x.read(input);
    input.readMessageEnd();
    return callback(x);
  }
  var result = new HardwareService_stopPump_result();
  result.read(input);
  input.readMessageEnd();

  if (null !== result.ouch) {
    return callback(result.ouch);
  }
  if (null !== result.success) {
    return callback(null, result.success);
  }
  return callback('stopPump failed: unknown result');
};
HardwareServiceClient.prototype.getServiceStatus = function(callback) {
  this._seqid = this.new_seqid();
  if (callback === undefined) {
    var _defer = Q.defer();
    this._reqs[this.seqid()] = function(error, result) {
      if (error) {
        _defer.reject(error);
      } else {
        _defer.resolve(result);
      }
    };
    this.send_getServiceStatus();
    return _defer.promise;
  } else {
    this._reqs[this.seqid()] = callback;
    this.send_getServiceStatus();
  }
};

HardwareServiceClient.prototype.send_getServiceStatus = function() {
  var output = new this.pClass(this.output);
  output.writeMessageBegin('getServiceStatus', Thrift.MessageType.CALL, this.seqid());
  var args = new HardwareService_getServiceStatus_args();
  args.write(output);
  output.writeMessageEnd();
  return this.output.flush();
};

HardwareServiceClient.prototype.recv_getServiceStatus = function(input,mtype,rseqid) {
  var callback = this._reqs[rseqid] || function() {};
  delete this._reqs[rseqid];
  if (mtype == Thrift.MessageType.EXCEPTION) {
    var x = new Thrift.TApplicationException();
    x.read(input);
    input.readMessageEnd();
    return callback(x);
  }
  var result = new HardwareService_getServiceStatus_result();
  result.read(input);
  input.readMessageEnd();

  if (null !== result.success) {
    return callback(null, result.success);
  }
  return callback('getServiceStatus failed: unknown result');
};
HardwareServiceClient.prototype.getServiceStateJson = function(callback) {
  this._seqid = this.new_seqid();
  if (callback === undefined) {
    var _defer = Q.defer();
    this._reqs[this.seqid()] = function(error, result) {
      if (error) {
        _defer.reject(error);
      } else {
        _defer.resolve(result);
      }
    };
    this.send_getServiceStateJson();
    return _defer.promise;
  } else {
    this._reqs[this.seqid()] = callback;
    this.send_getServiceStateJson();
  }
};

HardwareServiceClient.prototype.send_getServiceStateJson = function() {
  var output = new this.pClass(this.output);
  output.writeMessageBegin('getServiceStateJson', Thrift.MessageType.CALL, this.seqid());
  var args = new HardwareService_getServiceStateJson_args();
  args.write(output);
  output.writeMessageEnd();
  return this.output.flush();
};

HardwareServiceClient.prototype.recv_getServiceStateJson = function(input,mtype,rseqid) {
  var callback = this._reqs[rseqid] || function() {};
  delete this._reqs[rseqid];
  if (mtype == Thrift.MessageType.EXCEPTION) {
    var x = new Thrift.TApplicationException();
    x.read(input);
    input.readMessageEnd();
    return callback(x);
  }
  var result = new HardwareService_getServiceStateJson_result();
  result.read(input);
  input.readMessageEnd();

  if (null !== result.success) {
    return callback(null, result.success);
  }
  return callback('getServiceStateJson failed: unknown result');
};
HardwareServiceClient.prototype.fillVisibleContainerMillilitres = function(amount, callback) {
  this._seqid = this.new_seqid();
  if (callback === undefined) {
    var _defer = Q.defer();
    this._reqs[this.seqid()] = function(error, result) {
      if (error) {
        _defer.reject(error);
      } else {
        _defer.resolve(result);
      }
    };
    this.send_fillVisibleContainerMillilitres(amount);
    return _defer.promise;
  } else {
    this._reqs[this.seqid()] = callback;
    this.send_fillVisibleContainerMillilitres(amount);
  }
};

HardwareServiceClient.prototype.send_fillVisibleContainerMillilitres = function(amount) {
  var output = new this.pClass(this.output);
  output.writeMessageBegin('fillVisibleContainerMillilitres', Thrift.MessageType.CALL, this.seqid());
  var args = new HardwareService_fillVisibleContainerMillilitres_args();
  args.amount = amount;
  args.write(output);
  output.writeMessageEnd();
  return this.output.flush();
};

HardwareServiceClient.prototype.recv_fillVisibleContainerMillilitres = function(input,mtype,rseqid) {
  var callback = this._reqs[rseqid] || function() {};
  delete this._reqs[rseqid];
  if (mtype == Thrift.MessageType.EXCEPTION) {
    var x = new Thrift.TApplicationException();
    x.read(input);
    input.readMessageEnd();
    return callback(x);
  }
  var result = new HardwareService_fillVisibleContainerMillilitres_result();
  result.read(input);
  input.readMessageEnd();

  if (null !== result.ouch) {
    return callback(result.ouch);
  }
  callback(null)
};
HardwareServiceClient.prototype.emptyVisiableContainerMillilitres = function(amount, callback) {
  this._seqid = this.new_seqid();
  if (callback === undefined) {
    var _defer = Q.defer();
    this._reqs[this.seqid()] = function(error, result) {
      if (error) {
        _defer.reject(error);
      } else {
        _defer.resolve(result);
      }
    };
    this.send_emptyVisiableContainerMillilitres(amount);
    return _defer.promise;
  } else {
    this._reqs[this.seqid()] = callback;
    this.send_emptyVisiableContainerMillilitres(amount);
  }
};

HardwareServiceClient.prototype.send_emptyVisiableContainerMillilitres = function(amount) {
  var output = new this.pClass(this.output);
  output.writeMessageBegin('emptyVisiableContainerMillilitres', Thrift.MessageType.CALL, this.seqid());
  var args = new HardwareService_emptyVisiableContainerMillilitres_args();
  args.amount = amount;
  args.write(output);
  output.writeMessageEnd();
  return this.output.flush();
};

HardwareServiceClient.prototype.recv_emptyVisiableContainerMillilitres = function(input,mtype,rseqid) {
  var callback = this._reqs[rseqid] || function() {};
  delete this._reqs[rseqid];
  if (mtype == Thrift.MessageType.EXCEPTION) {
    var x = new Thrift.TApplicationException();
    x.read(input);
    input.readMessageEnd();
    return callback(x);
  }
  var result = new HardwareService_emptyVisiableContainerMillilitres_result();
  result.read(input);
  input.readMessageEnd();

  if (null !== result.ouch) {
    return callback(result.ouch);
  }
  callback(null)
};
HardwareServiceProcessor = exports.Processor = function(handler) {
  this._handler = handler
}
HardwareServiceProcessor.prototype.process = function(input, output) {
  var r = input.readMessageBegin();
  if (this['process_' + r.fname]) {
    return this['process_' + r.fname].call(this, r.rseqid, input, output);
  } else {
    input.skip(Thrift.Type.STRUCT);
    input.readMessageEnd();
    var x = new Thrift.TApplicationException(Thrift.TApplicationExceptionType.UNKNOWN_METHOD, 'Unknown function ' + r.fname);
    output.writeMessageBegin(r.fname, Thrift.MessageType.EXCEPTION, r.rseqid);
    x.write(output);
    output.writeMessageEnd();
    output.flush();
  }
}

HardwareServiceProcessor.prototype.process_applyConfiguration = function(seqid, input, output) {
  var args = new HardwareService_applyConfiguration_args();
  args.read(input);
  input.readMessageEnd();
  if (this._handler.applyConfiguration.length === 1) {
    Q.fcall(this._handler.applyConfiguration, args.jsonDocumentText)
      .then(function(result) {
        var result = new HardwareService_applyConfiguration_result({success: result});
        output.writeMessageBegin("applyConfiguration", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      }, function (err) {
        var result = new HardwareService_applyConfiguration_result(err);
        output.writeMessageBegin("applyConfiguration", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      });
  } else {
    this._handler.applyConfiguration(args.jsonDocumentText,  function (err, result) {
      var result = new HardwareService_applyConfiguration_result((err != null ? err : {success: result}));
      output.writeMessageBegin("applyConfiguration", Thrift.MessageType.REPLY, seqid);
      result.write(output);
      output.writeMessageEnd();
      output.flush();
    });
  }
}

HardwareServiceProcessor.prototype.process_startPump = function(seqid, input, output) {
  var args = new HardwareService_startPump_args();
  args.read(input);
  input.readMessageEnd();
  if (this._handler.startPump.length === 1) {
    Q.fcall(this._handler.startPump, args.pumpId)
      .then(function(result) {
        var result = new HardwareService_startPump_result({success: result});
        output.writeMessageBegin("startPump", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      }, function (err) {
        var result = new HardwareService_startPump_result(err);
        output.writeMessageBegin("startPump", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      });
  } else {
    this._handler.startPump(args.pumpId,  function (err, result) {
      var result = new HardwareService_startPump_result((err != null ? err : {success: result}));
      output.writeMessageBegin("startPump", Thrift.MessageType.REPLY, seqid);
      result.write(output);
      output.writeMessageEnd();
      output.flush();
    });
  }
}

HardwareServiceProcessor.prototype.process_stopPump = function(seqid, input, output) {
  var args = new HardwareService_stopPump_args();
  args.read(input);
  input.readMessageEnd();
  if (this._handler.stopPump.length === 1) {
    Q.fcall(this._handler.stopPump, args.pumpId)
      .then(function(result) {
        var result = new HardwareService_stopPump_result({success: result});
        output.writeMessageBegin("stopPump", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      }, function (err) {
        var result = new HardwareService_stopPump_result(err);
        output.writeMessageBegin("stopPump", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      });
  } else {
    this._handler.stopPump(args.pumpId,  function (err, result) {
      var result = new HardwareService_stopPump_result((err != null ? err : {success: result}));
      output.writeMessageBegin("stopPump", Thrift.MessageType.REPLY, seqid);
      result.write(output);
      output.writeMessageEnd();
      output.flush();
    });
  }
}

HardwareServiceProcessor.prototype.process_getServiceStatus = function(seqid, input, output) {
  var args = new HardwareService_getServiceStatus_args();
  args.read(input);
  input.readMessageEnd();
  if (this._handler.getServiceStatus.length === 0) {
    Q.fcall(this._handler.getServiceStatus)
      .then(function(result) {
        var result = new HardwareService_getServiceStatus_result({success: result});
        output.writeMessageBegin("getServiceStatus", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      }, function (err) {
        var result = new HardwareService_getServiceStatus_result(err);
        output.writeMessageBegin("getServiceStatus", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      });
  } else {
    this._handler.getServiceStatus( function (err, result) {
      var result = new HardwareService_getServiceStatus_result((err != null ? err : {success: result}));
      output.writeMessageBegin("getServiceStatus", Thrift.MessageType.REPLY, seqid);
      result.write(output);
      output.writeMessageEnd();
      output.flush();
    });
  }
}

HardwareServiceProcessor.prototype.process_getServiceStateJson = function(seqid, input, output) {
  var args = new HardwareService_getServiceStateJson_args();
  args.read(input);
  input.readMessageEnd();
  if (this._handler.getServiceStateJson.length === 0) {
    Q.fcall(this._handler.getServiceStateJson)
      .then(function(result) {
        var result = new HardwareService_getServiceStateJson_result({success: result});
        output.writeMessageBegin("getServiceStateJson", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      }, function (err) {
        var result = new HardwareService_getServiceStateJson_result(err);
        output.writeMessageBegin("getServiceStateJson", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      });
  } else {
    this._handler.getServiceStateJson( function (err, result) {
      var result = new HardwareService_getServiceStateJson_result((err != null ? err : {success: result}));
      output.writeMessageBegin("getServiceStateJson", Thrift.MessageType.REPLY, seqid);
      result.write(output);
      output.writeMessageEnd();
      output.flush();
    });
  }
}

HardwareServiceProcessor.prototype.process_fillVisibleContainerMillilitres = function(seqid, input, output) {
  var args = new HardwareService_fillVisibleContainerMillilitres_args();
  args.read(input);
  input.readMessageEnd();
  if (this._handler.fillVisibleContainerMillilitres.length === 1) {
    Q.fcall(this._handler.fillVisibleContainerMillilitres, args.amount)
      .then(function(result) {
        var result = new HardwareService_fillVisibleContainerMillilitres_result({success: result});
        output.writeMessageBegin("fillVisibleContainerMillilitres", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      }, function (err) {
        var result = new HardwareService_fillVisibleContainerMillilitres_result(err);
        output.writeMessageBegin("fillVisibleContainerMillilitres", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      });
  } else {
    this._handler.fillVisibleContainerMillilitres(args.amount,  function (err, result) {
      var result = new HardwareService_fillVisibleContainerMillilitres_result((err != null ? err : {success: result}));
      output.writeMessageBegin("fillVisibleContainerMillilitres", Thrift.MessageType.REPLY, seqid);
      result.write(output);
      output.writeMessageEnd();
      output.flush();
    });
  }
}

HardwareServiceProcessor.prototype.process_emptyVisiableContainerMillilitres = function(seqid, input, output) {
  var args = new HardwareService_emptyVisiableContainerMillilitres_args();
  args.read(input);
  input.readMessageEnd();
  if (this._handler.emptyVisiableContainerMillilitres.length === 1) {
    Q.fcall(this._handler.emptyVisiableContainerMillilitres, args.amount)
      .then(function(result) {
        var result = new HardwareService_emptyVisiableContainerMillilitres_result({success: result});
        output.writeMessageBegin("emptyVisiableContainerMillilitres", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      }, function (err) {
        var result = new HardwareService_emptyVisiableContainerMillilitres_result(err);
        output.writeMessageBegin("emptyVisiableContainerMillilitres", Thrift.MessageType.REPLY, seqid);
        result.write(output);
        output.writeMessageEnd();
        output.flush();
      });
  } else {
    this._handler.emptyVisiableContainerMillilitres(args.amount,  function (err, result) {
      var result = new HardwareService_emptyVisiableContainerMillilitres_result((err != null ? err : {success: result}));
      output.writeMessageBegin("emptyVisiableContainerMillilitres", Thrift.MessageType.REPLY, seqid);
      result.write(output);
      output.writeMessageEnd();
      output.flush();
    });
  }
}

