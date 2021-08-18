import * as NPRPC from './nprpc'

export type discrete = boolean/*boolean*/;
export type oid_t = bigint/*u64*/;
export type uuid = Array<number/*u8*/>;
export type var_handle = bigint/*u64*/;
export type var_type = number/*u32*/;
export const enum controller_type { //u32
  CT_AVR5
}
export const enum var_status { //u32
  VST_DEVICE_RESPONDED,
  VST_DEVICE_NOT_RESPONDED,
  VST_UNKNOWN
}
export class NPNetCommunicationError extends NPRPC.Exception {
  constructor(public code?: number/*i32*/) { super("NPNetCommunicationError"); }
}

export namespace Flat_server {
export class NPNetCommunicationError_Direct extends NPRPC.Flat.Flat {
  public get __ex_id() { return this.buffer.dv.getUint32(this.offset+0,true); }
  public set __ex_id(value: number) { this.buffer.dv.setUint32(this.offset+0,value,true); }
  public get code() { return this.buffer.dv.getInt32(this.offset+4,true); }
  public set code(value: number) { this.buffer.dv.setInt32(this.offset+4,value,true); }
}
} // namespace Flat 
export interface server_value {
  h: var_handle;
  s: var_status;
  data: Array<number/*u8*/>;
}

export namespace Flat_server {
export class server_value_Direct extends NPRPC.Flat.Flat {
  public get h() { return this.buffer.dv.getBigUint64(this.offset+0,true); }
  public set h(value: bigint) { this.buffer.dv.setBigUint64(this.offset+0,value,true); }
  public get s() { return this.buffer.dv.getUint32(this.offset+8,true); }
  public set s(value: var_status) { this.buffer.dv.setUint32(this.offset+8,value,true); }
  public data_vd() { return new NPRPC.Flat.Array_Direct1_u8(this.buffer, this.offset + 12, 8); }
}
} // namespace Flat 
export interface RawDataDef {
  dev_addr: number/*u8*/;
  address: number/*u16*/;
  size: number/*u8*/;
}

export namespace Flat_server {
export class RawDataDef_Direct extends NPRPC.Flat.Flat {
  public get dev_addr() { return this.buffer.dv.getUint8(this.offset+0); }
  public set dev_addr(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get address() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set address(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public get size() { return this.buffer.dv.getUint8(this.offset+4); }
  public set size(value: number) { this.buffer.dv.setUint8(this.offset+4,value); }
}
} // namespace Flat 
export interface DataDef {
  dev_addr: number/*u8*/;
  mem_addr: number/*u16*/;
  type: var_type;
}

export namespace Flat_server {
export class DataDef_Direct extends NPRPC.Flat.Flat {
  public get dev_addr() { return this.buffer.dv.getUint8(this.offset+0); }
  public set dev_addr(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get mem_addr() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set mem_addr(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public get type() { return this.buffer.dv.getUint32(this.offset+4,true); }
  public set type(value: number) { this.buffer.dv.setUint32(this.offset+4,value,true); }
}
} // namespace Flat 
export class Pingable extends NPRPC.ObjectProxy {
  public static get servant_t(): new() => _IPingable_Servant {
    return _IPingable_Servant;
  }


  public async Ping(): Promise<void> {
    let interface_idx = (arguments.length == 0 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(32);
    buf.commit(32);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 0;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

};

export interface IPingable_Servant
{
  Ping(): void;
}

export class _IPingable_Servant extends NPRPC.ObjectServant {
  public static _get_class(): string { return "server/nps.Pingable"; }
  public readonly get_class = () => { return _IPingable_Servant._get_class(); }
  public readonly dispatch = (buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean) => {
    _IPingable_Servant._dispatch(this, buf, remote_endpoint, from_parent);
  }
  static _dispatch(obj: _IPingable_Servant, buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean): void {
  let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
  switch(__ch.function_idx) {
    case 0: {
      (obj as any).Ping();
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    default:
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Error_UnknownFunctionIdx);
  }
  }
}

export class DataCallBack extends NPRPC.ObjectProxy {
  public static get servant_t(): new() => _IDataCallBack_Servant {
    return _IDataCallBack_Servant;
  }

  // Pingable
  public async Ping(): Promise<void> {
    Pingable.prototype.Ping.bind(this,1)();
  }

  public async OnDataChanged(a: /*in*/Array<server_value>): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(168);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 0;
  let _ = new Flat_server.server_M1_Direct(buf,32);
  _._1(a.length);
  {
  let vv = _._1_vd(), index = 0;
  for (let e of vv) {
          e.h = a[index].h;
      e.s = a[index].s;
      e.data_vd().copy_from_ts_array(a[index].data); 
    ++index;
  }
  }
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

};

export interface IDataCallBack_Servant
 extends IPingable_Servant
{
  OnDataChanged(a: NPRPC.Flat.Vector_Direct2<Flat_server.server_value_Direct>): void;
}

export class _IDataCallBack_Servant extends NPRPC.ObjectServant {
  public static _get_class(): string { return "server/nps.DataCallBack"; }
  public readonly get_class = () => { return _IDataCallBack_Servant._get_class(); }
  public readonly dispatch = (buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean) => {
    _IDataCallBack_Servant._dispatch(this, buf, remote_endpoint, from_parent);
  }
  static _dispatch(obj: _IDataCallBack_Servant, buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean): void {
  let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
  if (from_parent == false) {
    switch(__ch.interface_idx) {
      case 0:
        break;
      case 1:
        _IPingable_Servant._dispatch(obj, buf, remote_endpoint, true);
        return;
      default:
        throw "unknown interface";
    }
  }
  switch(__ch.function_idx) {
    case 0: {
      let ia = new Flat_server.server_M1_Direct(buf, 32);
      (obj as any).OnDataChanged(ia._1_vd());
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    default:
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Error_UnknownFunctionIdx);
  }
  }
}

export class ItemManager extends NPRPC.ObjectProxy {
  public static get servant_t(): new() => _IItemManager_Servant {
    return _IItemManager_Servant;
  }

  // Pingable
  public async Ping(): Promise<void> {
    Pingable.prototype.Ping.bind(this,1)();
  }

  public async Activate(client: /*in*/NPRPC.detail.ObjectId): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(192);
    buf.commit(64);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 0;
  let _ = new Flat_server.server_M2_Direct(buf,32);
  _._1.object_id = client.object_id;
  _._1.ip4 = client.ip4;
  _._1.port = client.port;
  _._1.websocket_port = client.websocket_port;
  _._1.poa_idx = client.poa_idx;
  _._1.flags = client.flags;
  _._1.class_id = client.class_id;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Advise(a: /*in*/Array<DataDef>, h: /*out*/Array<var_handle>): Promise<void> {
    let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(168);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 1;
  let _ = new Flat_server.server_M3_Direct(buf,32);
  _._1(a.length);
  {
  let vv = _._1_vd(), index = 0;
  for (let e of vv) {
          e.dev_addr = a[index].dev_addr;
      e.mem_addr = a[index].mem_addr;
      e.type = a[index].type;
    ++index;
  }
  }
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M4_Direct(buf, 16);
  {
  let vv = out._1_vd(), index_0 = 0;
  h.length = vv.elements_size;
  for (let e of vv) {
  h[index_0] = e;
    ++index_0;
  }
  }
}

  public async UnAdvise(a: /*in*/Array<var_handle>): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(168);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 2;
  let _ = new Flat_server.server_M4_Direct(buf,32);
  _._1(a.length);
  _._1_vd().copy_from_ts_array(a); 
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

};

export interface IItemManager_Servant
 extends IPingable_Servant
{
  Activate(client: NPRPC.ObjectProxy): void;
  Advise(a: NPRPC.Flat.Vector_Direct2<Flat_server.DataDef_Direct>, h: NPRPC.Flat.Vector_Direct1_u64): void;
  UnAdvise(a: NPRPC.Flat.Vector_Direct1_u64): void;
}

export class _IItemManager_Servant extends NPRPC.ObjectServant {
  public static _get_class(): string { return "server/nps.ItemManager"; }
  public readonly get_class = () => { return _IItemManager_Servant._get_class(); }
  public readonly dispatch = (buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean) => {
    _IItemManager_Servant._dispatch(this, buf, remote_endpoint, from_parent);
  }
  static _dispatch(obj: _IItemManager_Servant, buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean): void {
  let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
  if (from_parent == false) {
    switch(__ch.interface_idx) {
      case 0:
        break;
      case 1:
        _IPingable_Servant._dispatch(obj, buf, remote_endpoint, true);
        return;
      default:
        throw "unknown interface";
    }
  }
  switch(__ch.function_idx) {
    case 0: {
      let ia = new Flat_server.server_M2_Direct(buf, 32);
      (obj as any).Activate(NPRPC.create_object_from_flat(ia._1, remote_endpoint.ip4));
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 1: {
      let ia = new Flat_server.server_M3_Direct(buf, 32);
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(152);
      obuf.commit(24);
      let oa = new Flat_server.server_M4_Direct(obuf,16);
      try {
      (obj as any).Advise(ia._1_vd(), oa._1_vd);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 2: {
      let ia = new Flat_server.server_M4_Direct(buf, 32);
      (obj as any).UnAdvise(ia._1_vd());
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    default:
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Error_UnknownFunctionIdx);
  }
  }
}

export class Server extends NPRPC.ObjectProxy {
  public static get servant_t(): new() => _IServer_Servant {
    return _IServer_Servant;
  }

  // Pingable
  public async Ping(): Promise<void> {
    Pingable.prototype.Ping.bind(this,1)();
  }

  public async GetNetworkStatus(network_status: /*out*/Array<number>): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(32);
    buf.commit(32);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 0;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M5_Direct(buf, 16);
  {
  let vv = out._1_vd(), index_1 = 0;
  network_status.length = vv.elements_size;
  for (let e of vv) {
  network_status[index_1] = e;
    ++index_1;
  }
  }
}

  public async CreateItemManager(im: /*out*/NPRPC.ref<NPRPC.ObjectProxy>): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(32);
    buf.commit(32);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 1;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M2_Direct(buf, 16);
  im.value = NPRPC.create_object_from_flat(out._1, this.data.ip4);
}

  public async SendRawData(data: /*in*/Array<number>): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(168);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 2;
  let _ = new Flat_server.server_M5_Direct(buf,32);
  _._1(data.length);
  _._1_vd().copy_from_ts_array(data); 
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Write_1(dev_addr: /*in*/number, mem_addr: /*in*/number, bit: /*in*/number, value: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(38);
    buf.commit(38);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 3;
  let _ = new Flat_server.server_M6_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3 = bit;
  _._4 = value;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Write_q1(dev_addr: /*in*/number, mem_addr: /*in*/number, bit: /*in*/number, value: /*in*/number, qvalue: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 5 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 4;
  let _ = new Flat_server.server_M7_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3 = bit;
  _._4 = value;
  _._5 = qvalue;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Write_8(dev_addr: /*in*/number, mem_addr: /*in*/number, value: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(38);
    buf.commit(38);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 5;
  let _ = new Flat_server.server_M8_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3 = value;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Write_q8(dev_addr: /*in*/number, mem_addr: /*in*/number, value: /*in*/number, qvalue: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(38);
    buf.commit(38);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 6;
  let _ = new Flat_server.server_M6_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3 = value;
  _._4 = qvalue;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Write_16(dev_addr: /*in*/number, mem_addr: /*in*/number, value: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(38);
    buf.commit(38);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 7;
  let _ = new Flat_server.server_M9_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3 = value;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Write_q16(dev_addr: /*in*/number, mem_addr: /*in*/number, value: /*in*/number, qvalue: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 8;
  let _ = new Flat_server.server_M10_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3 = value;
  _._4 = qvalue;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Write_32(dev_addr: /*in*/number, mem_addr: /*in*/number, value: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 9;
  let _ = new Flat_server.server_M11_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3 = value;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Write_q32(dev_addr: /*in*/number, mem_addr: /*in*/number, value: /*in*/number, qvalue: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(44);
    buf.commit(44);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 10;
  let _ = new Flat_server.server_M12_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3 = value;
  _._4 = qvalue;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async WriteBlock(dev_addr: /*in*/number, mem_addr: /*in*/number, data: /*in*/Array<number>): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(172);
    buf.commit(44);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 11;
  let _ = new Flat_server.server_M13_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3(data.length);
  _._3_vd().copy_from_ts_array(data); 
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async ReadByte(dev_addr: /*in*/number, addr: /*in*/number, value: /*out*/NPRPC.ref<number>): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(36);
    buf.commit(36);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 12;
  let _ = new Flat_server.server_M14_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = addr;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M15_Direct(buf, 16);
  value.value = out._1;
}

  public async ReadBlock(dev_addr: /*in*/number, addr: /*in*/number, len: /*in*/number, data: /*out*/Array<number>): Promise<void> {
    let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(38);
    buf.commit(38);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 13;
  let _ = new Flat_server.server_M8_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = addr;
  _._3 = len;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M5_Direct(buf, 16);
  {
  let vv = out._1_vd(), index_2 = 0;
  data.length = vv.elements_size;
  for (let e of vv) {
  data[index_2] = e;
    ++index_2;
  }
  }
}

  public async AVR_StopAlgorithm(dev_addr: /*in*/number, alg_addr: /*in*/number): Promise<boolean/*boolean*/> {
    let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(36);
    buf.commit(36);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 14;
  let _ = new Flat_server.server_M14_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = alg_addr;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M16_Direct(buf, 16);
  let __ret_value: boolean/*boolean*/;
  __ret_value = out._1;
  return __ret_value;
}

  public async AVR_ReinitIO(dev_addr: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(33);
    buf.commit(33);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 15;
  let _ = new Flat_server.server_M15_Direct(buf,32);
  _._1 = dev_addr;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async AVR_SendRemoteData(dev_addr: /*in*/number, page_num: /*in*/number, data: /*in*/Array<number>): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(172);
    buf.commit(44);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 16;
  let _ = new Flat_server.server_M13_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = page_num;
  _._3(data.length);
  _._3_vd().copy_from_ts_array(data); 
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async AVR_SendPage(dev_addr: /*in*/number, page_num: /*in*/number, data: /*in*/Array<number>): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(172);
    buf.commit(44);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 17;
  let _ = new Flat_server.server_M17_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = page_num;
  _._3(data.length);
  _._3_vd().copy_from_ts_array(data); 
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async AVR_RemoveAlgorithm(dev_addr: /*in*/number, alg_addr: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(36);
    buf.commit(36);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 18;
  let _ = new Flat_server.server_M14_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = alg_addr;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async AVR_ReplaceAlgorithm(dev_addr: /*in*/number, old_addr: /*in*/number, new_addr: /*in*/number): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(38);
    buf.commit(38);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 19;
  let _ = new Flat_server.server_M9_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = old_addr;
  _._3 = new_addr;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async AVR_WriteEeprom(dev_addr: /*in*/number, mem_addr: /*in*/number, data: /*in*/Array<number>): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(172);
    buf.commit(44);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 20;
  let _ = new Flat_server.server_M13_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = mem_addr;
  _._3(data.length);
  _._3_vd().copy_from_ts_array(data); 
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async AVR_WriteTwiTable(dev_addr: /*in*/number, page_num: /*in*/number, data: /*in*/Array<number>): Promise<void> {
    let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(172);
    buf.commit(44);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 21;
  let _ = new Flat_server.server_M17_Direct(buf,32);
  _._1 = dev_addr;
  _._2 = page_num;
  _._3(data.length);
  _._3_vd().copy_from_ts_array(data); 
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async AVR_V_GetFlash(device_id: /*in*/oid_t, data: /*out*/Array<number>): Promise<void> {
    let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 22;
  let _ = new Flat_server.server_M18_Direct(buf,32);
  _._1 = device_id;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M19_Direct(buf, 16);
  {
  let vv = out._1_vd(), index_3 = 0;
  data.length = vv.elements_size;
  for (let e of vv) {
  data[index_3] = e;
    ++index_3;
  }
  }
}

  public async AVR_V_StoreFlash(device_id: /*in*/oid_t): Promise<boolean/*boolean*/> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 23;
  let _ = new Flat_server.server_M18_Direct(buf,32);
  _._1 = device_id;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply == 1) {
      server_throw_exception(buf);
    }
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M16_Direct(buf, 16);
  let __ret_value: boolean/*boolean*/;
  __ret_value = out._1;
  return __ret_value;
}

  public async Notify_DeviceActivated(device_id: /*in*/oid_t): Promise<boolean/*boolean*/> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 24;
  let _ = new Flat_server.server_M18_Direct(buf,32);
  _._1 = device_id;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M16_Direct(buf, 16);
  let __ret_value: boolean/*boolean*/;
  __ret_value = out._1;
  return __ret_value;
}

  public async Notify_DeviceDeactivated(device_id: /*in*/oid_t): Promise<boolean/*boolean*/> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 25;
  let _ = new Flat_server.server_M18_Direct(buf,32);
  _._1 = device_id;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_server.server_M16_Direct(buf, 16);
  let __ret_value: boolean/*boolean*/;
  __ret_value = out._1;
  return __ret_value;
}

  public async Notify_ParameterRemoved(param_id: /*in*/oid_t): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 26;
  let _ = new Flat_server.server_M18_Direct(buf,32);
  _._1 = param_id;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async Notify_TypeOrVariableChanged(param_id: /*in*/oid_t): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 27;
  let _ = new Flat_server.server_M18_Direct(buf,32);
  _._1 = param_id;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async History_AddParameter(param_id: /*in*/oid_t): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 28;
  let _ = new Flat_server.server_M18_Direct(buf,32);
  _._1 = param_id;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

  public async History_RemoveParameter(param_id: /*in*/oid_t): Promise<void> {
    let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
    let buf = NPRPC.FlatBuffer.create();
    buf.prepare(40);
    buf.commit(40);
    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);
    buf.write_msg_type(NPRPC.impl.MessageType.Request);
    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
    __ch.object_id = this.data.object_id;
    __ch.poa_idx = this.data.poa_idx;
    __ch.interface_idx = interface_idx;
    __ch.function_idx = 29;
  let _ = new Flat_server.server_M18_Direct(buf,32);
  _._1 = param_id;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      {ip4: this.data.ip4, port: this.data.websocket_port}, buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != 0) {
      console.log("received an unusual reply for function with no output arguments");
    }
}

};

export interface IServer_Servant
 extends IPingable_Servant
{
  GetNetworkStatus(network_status: NPRPC.Flat.Vector_Direct1_u8): void;
  CreateItemManager(im: NPRPC.detail.Flat_nprpc_base.ObjectId_Direct): void;
  SendRawData(data: NPRPC.Flat.Vector_Direct1_u8): void;
  Write_1(dev_addr: number, mem_addr: number, bit: number, value: number): void;
  Write_q1(dev_addr: number, mem_addr: number, bit: number, value: number, qvalue: number): void;
  Write_8(dev_addr: number, mem_addr: number, value: number): void;
  Write_q8(dev_addr: number, mem_addr: number, value: number, qvalue: number): void;
  Write_16(dev_addr: number, mem_addr: number, value: number): void;
  Write_q16(dev_addr: number, mem_addr: number, value: number, qvalue: number): void;
  Write_32(dev_addr: number, mem_addr: number, value: number): void;
  Write_q32(dev_addr: number, mem_addr: number, value: number, qvalue: number): void;
  WriteBlock(dev_addr: number, mem_addr: number, data: NPRPC.Flat.Vector_Direct1_u8): void;
  ReadByte(dev_addr: number, addr: number, value: number): void;
  ReadBlock(dev_addr: number, addr: number, len: number, data: NPRPC.Flat.Vector_Direct1_u8): void;
  AVR_StopAlgorithm(dev_addr: number, alg_addr: number): boolean/*boolean*/;
  AVR_ReinitIO(dev_addr: number): void;
  AVR_SendRemoteData(dev_addr: number, page_num: number, data: NPRPC.Flat.Vector_Direct1_u8): void;
  AVR_SendPage(dev_addr: number, page_num: number, data: NPRPC.Flat.Vector_Direct1_u8): void;
  AVR_RemoveAlgorithm(dev_addr: number, alg_addr: number): void;
  AVR_ReplaceAlgorithm(dev_addr: number, old_addr: number, new_addr: number): void;
  AVR_WriteEeprom(dev_addr: number, mem_addr: number, data: NPRPC.Flat.Vector_Direct1_u8): void;
  AVR_WriteTwiTable(dev_addr: number, page_num: number, data: NPRPC.Flat.Vector_Direct1_u8): void;
  AVR_V_GetFlash(device_id: oid_t, data: NPRPC.Flat.Vector_Direct1_u16): void;
  AVR_V_StoreFlash(device_id: oid_t): boolean/*boolean*/;
  Notify_DeviceActivated(device_id: oid_t): boolean/*boolean*/;
  Notify_DeviceDeactivated(device_id: oid_t): boolean/*boolean*/;
  Notify_ParameterRemoved(param_id: oid_t): void;
  Notify_TypeOrVariableChanged(param_id: oid_t): void;
  History_AddParameter(param_id: oid_t): void;
  History_RemoveParameter(param_id: oid_t): void;
}

export class _IServer_Servant extends NPRPC.ObjectServant {
  public static _get_class(): string { return "server/nps.Server"; }
  public readonly get_class = () => { return _IServer_Servant._get_class(); }
  public readonly dispatch = (buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean) => {
    _IServer_Servant._dispatch(this, buf, remote_endpoint, from_parent);
  }
  static _dispatch(obj: _IServer_Servant, buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean): void {
  let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
  if (from_parent == false) {
    switch(__ch.interface_idx) {
      case 0:
        break;
      case 1:
        _IPingable_Servant._dispatch(obj, buf, remote_endpoint, true);
        return;
      default:
        throw "unknown interface";
    }
  }
  switch(__ch.function_idx) {
    case 0: {
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(152);
      obuf.commit(24);
      let oa = new Flat_server.server_M5_Direct(obuf,16);
      (obj as any).GetNetworkStatus(oa._1_vd);
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 1: {
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(176);
      obuf.commit(48);
      let oa = new Flat_server.server_M2_Direct(obuf,16);
      (obj as any).CreateItemManager(oa._1);
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 2: {
      let ia = new Flat_server.server_M5_Direct(buf, 32);
      try {
      (obj as any).SendRawData(ia._1_vd());
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 3: {
      let ia = new Flat_server.server_M6_Direct(buf, 32);
      try {
      (obj as any).Write_1(ia._1, ia._2, ia._3, ia._4);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 4: {
      let ia = new Flat_server.server_M7_Direct(buf, 32);
      try {
      (obj as any).Write_q1(ia._1, ia._2, ia._3, ia._4, ia._5);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 5: {
      let ia = new Flat_server.server_M8_Direct(buf, 32);
      try {
      (obj as any).Write_8(ia._1, ia._2, ia._3);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 6: {
      let ia = new Flat_server.server_M6_Direct(buf, 32);
      try {
      (obj as any).Write_q8(ia._1, ia._2, ia._3, ia._4);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 7: {
      let ia = new Flat_server.server_M9_Direct(buf, 32);
      try {
      (obj as any).Write_16(ia._1, ia._2, ia._3);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 8: {
      let ia = new Flat_server.server_M10_Direct(buf, 32);
      try {
      (obj as any).Write_q16(ia._1, ia._2, ia._3, ia._4);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 9: {
      let ia = new Flat_server.server_M11_Direct(buf, 32);
      try {
      (obj as any).Write_32(ia._1, ia._2, ia._3);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 10: {
      let ia = new Flat_server.server_M12_Direct(buf, 32);
      try {
      (obj as any).Write_q32(ia._1, ia._2, ia._3, ia._4);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 11: {
      let ia = new Flat_server.server_M13_Direct(buf, 32);
      try {
      (obj as any).WriteBlock(ia._1, ia._2, ia._3_vd());
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 12: {
      let _out_1: number/*u8*/;
      let ia = new Flat_server.server_M14_Direct(buf, 32);
      try {
      (obj as any).ReadByte(ia._1, ia._2, _out_1);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(17);
      obuf.commit(17);
      let oa = new Flat_server.server_M15_Direct(obuf,16);
  oa._1 = _out_1;
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 13: {
      let ia = new Flat_server.server_M8_Direct(buf, 32);
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(152);
      obuf.commit(24);
      let oa = new Flat_server.server_M5_Direct(obuf,16);
      try {
      (obj as any).ReadBlock(ia._1, ia._2, ia._3, oa._1_vd);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 14: {
      let ia = new Flat_server.server_M14_Direct(buf, 32);
let __ret_val: boolean/*boolean*/;
      try {
      __ret_val = (obj as any).AVR_StopAlgorithm(ia._1, ia._2);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(17);
      obuf.commit(17);
      let oa = new Flat_server.server_M16_Direct(obuf,16);
  oa._1 = __ret_val;
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 15: {
      let ia = new Flat_server.server_M15_Direct(buf, 32);
      try {
      (obj as any).AVR_ReinitIO(ia._1);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 16: {
      let ia = new Flat_server.server_M13_Direct(buf, 32);
      try {
      (obj as any).AVR_SendRemoteData(ia._1, ia._2, ia._3_vd());
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 17: {
      let ia = new Flat_server.server_M17_Direct(buf, 32);
      try {
      (obj as any).AVR_SendPage(ia._1, ia._2, ia._3_vd());
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 18: {
      let ia = new Flat_server.server_M14_Direct(buf, 32);
      try {
      (obj as any).AVR_RemoveAlgorithm(ia._1, ia._2);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 19: {
      let ia = new Flat_server.server_M9_Direct(buf, 32);
      try {
      (obj as any).AVR_ReplaceAlgorithm(ia._1, ia._2, ia._3);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 20: {
      let ia = new Flat_server.server_M13_Direct(buf, 32);
      try {
      (obj as any).AVR_WriteEeprom(ia._1, ia._2, ia._3_vd());
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 21: {
      let ia = new Flat_server.server_M17_Direct(buf, 32);
      try {
      (obj as any).AVR_WriteTwiTable(ia._1, ia._2, ia._3_vd());
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 22: {
      let ia = new Flat_server.server_M18_Direct(buf, 32);
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(152);
      obuf.commit(24);
      let oa = new Flat_server.server_M19_Direct(obuf,16);
      try {
      (obj as any).AVR_V_GetFlash(ia._1, oa._1_vd);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 23: {
      let ia = new Flat_server.server_M18_Direct(buf, 32);
let __ret_val: boolean/*boolean*/;
      try {
      __ret_val = (obj as any).AVR_V_StoreFlash(ia._1);
      }
      catch(e) {
        let obuf = buf;
        obuf.consume(obuf.size);
        obuf.prepare(24);
        obuf.commit(24);
        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf,16);
        oa.__ex_id = 0;
  oa.code = e.code;
        obuf.write_len(obuf.size - 4);
        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);
        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
        return;
      }
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(17);
      obuf.commit(17);
      let oa = new Flat_server.server_M16_Direct(obuf,16);
  oa._1 = __ret_val;
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 24: {
      let ia = new Flat_server.server_M18_Direct(buf, 32);
let __ret_val: boolean/*boolean*/;
      __ret_val = (obj as any).Notify_DeviceActivated(ia._1);
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(17);
      obuf.commit(17);
      let oa = new Flat_server.server_M16_Direct(obuf,16);
  oa._1 = __ret_val;
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 25: {
      let ia = new Flat_server.server_M18_Direct(buf, 32);
let __ret_val: boolean/*boolean*/;
      __ret_val = (obj as any).Notify_DeviceDeactivated(ia._1);
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(17);
      obuf.commit(17);
      let oa = new Flat_server.server_M16_Direct(obuf,16);
  oa._1 = __ret_val;
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 26: {
      let ia = new Flat_server.server_M18_Direct(buf, 32);
      (obj as any).Notify_ParameterRemoved(ia._1);
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 27: {
      let ia = new Flat_server.server_M18_Direct(buf, 32);
      (obj as any).Notify_TypeOrVariableChanged(ia._1);
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 28: {
      let ia = new Flat_server.server_M18_Direct(buf, 32);
      (obj as any).History_AddParameter(ia._1);
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    case 29: {
      let ia = new Flat_server.server_M18_Direct(buf, 32);
      (obj as any).History_RemoveParameter(ia._1);
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);
      break;
    }
    default:
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Error_UnknownFunctionIdx);
  }
  }
}


function server_throw_exception(buf: NPRPC.FlatBuffer): void { 
  switch( buf.read_exception_number() ) {
  case 0:
  {
    let ex_flat = new Flat_server.NPNetCommunicationError_Direct(buf, 16);
    let ex = new NPNetCommunicationError();
  ex.code = ex_flat.code;
    throw ex;
  }
  default:
    throw "unknown rpc exception";
  }
}
export interface server_M1 {
  _1: Array<server_value>;
}

export namespace Flat_server {
export class server_M1_Direct extends NPRPC.Flat.Flat {
  public _1(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 0, elements_size, 24, 8);
  }
  public _1_vd() { return new NPRPC.Flat.Vector_Direct2<Flat_server.server_value_Direct>(this.buffer, this.offset + 0, 24, Flat_server.server_value_Direct); }
}
} // namespace Flat 
export interface server_M2 {
  _1: NPRPC.detail.ObjectId;
}

export namespace Flat_server {
export class server_M2_Direct extends NPRPC.Flat.Flat {
  public get _1() { return new NPRPC.detail.Flat_nprpc_base.ObjectId_Direct(this.buffer, this.offset + 0); }
}
} // namespace Flat 
export interface server_M3 {
  _1: Array<DataDef>;
}

export namespace Flat_server {
export class server_M3_Direct extends NPRPC.Flat.Flat {
  public _1(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 0, elements_size, 8, 4);
  }
  public _1_vd() { return new NPRPC.Flat.Vector_Direct2<Flat_server.DataDef_Direct>(this.buffer, this.offset + 0, 8, Flat_server.DataDef_Direct); }
}
} // namespace Flat 
export interface server_M4 {
  _1: Array<var_handle>;
}

export namespace Flat_server {
export class server_M4_Direct extends NPRPC.Flat.Flat {
  public _1(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 0, elements_size, 8, 8);
  }
  public _1_vd() { return new NPRPC.Flat.Vector_Direct1_u64(this.buffer, this.offset + 0); }
}
} // namespace Flat 
export interface server_M5 {
  _1: Array<number/*u8*/>;
}

export namespace Flat_server {
export class server_M5_Direct extends NPRPC.Flat.Flat {
  public _1(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 0, elements_size, 1, 1);
  }
  public _1_vd() { return new NPRPC.Flat.Vector_Direct1_u8(this.buffer, this.offset + 0); }
}
} // namespace Flat 
export interface server_M6 {
  _1: number/*u8*/;
  _2: number/*u16*/;
  _3: number/*u8*/;
  _4: number/*u8*/;
}

export namespace Flat_server {
export class server_M6_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set _2(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public get _3() { return this.buffer.dv.getUint8(this.offset+4); }
  public set _3(value: number) { this.buffer.dv.setUint8(this.offset+4,value); }
  public get _4() { return this.buffer.dv.getUint8(this.offset+5); }
  public set _4(value: number) { this.buffer.dv.setUint8(this.offset+5,value); }
}
} // namespace Flat 
export interface server_M7 {
  _1: number/*u8*/;
  _2: number/*u16*/;
  _3: number/*u8*/;
  _4: number/*u8*/;
  _5: number/*u8*/;
}

export namespace Flat_server {
export class server_M7_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set _2(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public get _3() { return this.buffer.dv.getUint8(this.offset+4); }
  public set _3(value: number) { this.buffer.dv.setUint8(this.offset+4,value); }
  public get _4() { return this.buffer.dv.getUint8(this.offset+5); }
  public set _4(value: number) { this.buffer.dv.setUint8(this.offset+5,value); }
  public get _5() { return this.buffer.dv.getUint8(this.offset+6); }
  public set _5(value: number) { this.buffer.dv.setUint8(this.offset+6,value); }
}
} // namespace Flat 
export interface server_M8 {
  _1: number/*u8*/;
  _2: number/*u16*/;
  _3: number/*u8*/;
}

export namespace Flat_server {
export class server_M8_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set _2(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public get _3() { return this.buffer.dv.getUint8(this.offset+4); }
  public set _3(value: number) { this.buffer.dv.setUint8(this.offset+4,value); }
}
} // namespace Flat 
export interface server_M9 {
  _1: number/*u8*/;
  _2: number/*u16*/;
  _3: number/*u16*/;
}

export namespace Flat_server {
export class server_M9_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set _2(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public get _3() { return this.buffer.dv.getUint16(this.offset+4,true); }
  public set _3(value: number) { this.buffer.dv.setUint16(this.offset+4,value,true); }
}
} // namespace Flat 
export interface server_M10 {
  _1: number/*u8*/;
  _2: number/*u16*/;
  _3: number/*u16*/;
  _4: number/*u8*/;
}

export namespace Flat_server {
export class server_M10_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set _2(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public get _3() { return this.buffer.dv.getUint16(this.offset+4,true); }
  public set _3(value: number) { this.buffer.dv.setUint16(this.offset+4,value,true); }
  public get _4() { return this.buffer.dv.getUint8(this.offset+6); }
  public set _4(value: number) { this.buffer.dv.setUint8(this.offset+6,value); }
}
} // namespace Flat 
export interface server_M11 {
  _1: number/*u8*/;
  _2: number/*u16*/;
  _3: number/*u32*/;
}

export namespace Flat_server {
export class server_M11_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set _2(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public get _3() { return this.buffer.dv.getUint32(this.offset+4,true); }
  public set _3(value: number) { this.buffer.dv.setUint32(this.offset+4,value,true); }
}
} // namespace Flat 
export interface server_M12 {
  _1: number/*u8*/;
  _2: number/*u16*/;
  _3: number/*u32*/;
  _4: number/*u8*/;
}

export namespace Flat_server {
export class server_M12_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set _2(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public get _3() { return this.buffer.dv.getUint32(this.offset+4,true); }
  public set _3(value: number) { this.buffer.dv.setUint32(this.offset+4,value,true); }
  public get _4() { return this.buffer.dv.getUint8(this.offset+8); }
  public set _4(value: number) { this.buffer.dv.setUint8(this.offset+8,value); }
}
} // namespace Flat 
export interface server_M13 {
  _1: number/*u8*/;
  _2: number/*u16*/;
  _3: Array<number/*u8*/>;
}

export namespace Flat_server {
export class server_M13_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set _2(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
  public _3(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 4, elements_size, 1, 1);
  }
  public _3_vd() { return new NPRPC.Flat.Vector_Direct1_u8(this.buffer, this.offset + 4); }
}
} // namespace Flat 
export interface server_M14 {
  _1: number/*u8*/;
  _2: number/*u16*/;
}

export namespace Flat_server {
export class server_M14_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint16(this.offset+2,true); }
  public set _2(value: number) { this.buffer.dv.setUint16(this.offset+2,value,true); }
}
} // namespace Flat 
export interface server_M15 {
  _1: number/*u8*/;
}

export namespace Flat_server {
export class server_M15_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
}
} // namespace Flat 
export interface server_M16 {
  _1: boolean/*boolean*/;
}

export namespace Flat_server {
export class server_M16_Direct extends NPRPC.Flat.Flat {
  public get _1() { return (this.buffer.dv.getUint8(this.offset+0) === 0x01); }
  public set _1(value: boolean) { this.buffer.dv.setUint8(this.offset+0, value === true ? 0x01 : 0x00); }
}
} // namespace Flat 
export interface server_M17 {
  _1: number/*u8*/;
  _2: number/*u8*/;
  _3: Array<number/*u8*/>;
}

export namespace Flat_server {
export class server_M17_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getUint8(this.offset+0); }
  public set _1(value: number) { this.buffer.dv.setUint8(this.offset+0,value); }
  public get _2() { return this.buffer.dv.getUint8(this.offset+1); }
  public set _2(value: number) { this.buffer.dv.setUint8(this.offset+1,value); }
  public _3(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 4, elements_size, 1, 1);
  }
  public _3_vd() { return new NPRPC.Flat.Vector_Direct1_u8(this.buffer, this.offset + 4); }
}
} // namespace Flat 
export interface server_M18 {
  _1: oid_t;
}

export namespace Flat_server {
export class server_M18_Direct extends NPRPC.Flat.Flat {
  public get _1() { return this.buffer.dv.getBigUint64(this.offset+0,true); }
  public set _1(value: bigint) { this.buffer.dv.setBigUint64(this.offset+0,value,true); }
}
} // namespace Flat 
export interface server_M19 {
  _1: Array<number/*u16*/>;
}

export namespace Flat_server {
export class server_M19_Direct extends NPRPC.Flat.Flat {
  public _1(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 0, elements_size, 2, 2);
  }
  public _1_vd() { return new NPRPC.Flat.Vector_Direct1_u16(this.buffer, this.offset + 0); }
}
} // namespace Flat 
