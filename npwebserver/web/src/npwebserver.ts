import * as NPRPC from './nprpc'

export interface WebItem {
  name: string;
  description: string;
  path: string;
  dev_addr: number/*u8*/;
  mem_addr: number/*u16*/;
  type: number/*u32*/;
}

export namespace Flat_npwebserver {
export class WebItem_Direct extends NPRPC.Flat.Flat {
  public get name() {
    let enc = new TextDecoder("utf-8");
    let v_begin = this.offset + 0;
    let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);
    let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));
    return enc.decode(bn);
  }
  public set name(str: string) {
    let enc = new TextEncoder();
    let bytes = enc.encode(str);
    let len = bytes.length;
    let offset = NPRPC.Flat._alloc(this.buffer, this.offset + 0, len, 1, 1);
    new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
  }
  public get description() {
    let enc = new TextDecoder("utf-8");
    let v_begin = this.offset + 8;
    let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);
    let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));
    return enc.decode(bn);
  }
  public set description(str: string) {
    let enc = new TextEncoder();
    let bytes = enc.encode(str);
    let len = bytes.length;
    let offset = NPRPC.Flat._alloc(this.buffer, this.offset + 8, len, 1, 1);
    new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
  }
  public get path() {
    let enc = new TextDecoder("utf-8");
    let v_begin = this.offset + 16;
    let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);
    let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));
    return enc.decode(bn);
  }
  public set path(str: string) {
    let enc = new TextEncoder();
    let bytes = enc.encode(str);
    let len = bytes.length;
    let offset = NPRPC.Flat._alloc(this.buffer, this.offset + 16, len, 1, 1);
    new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
  }
  public get dev_addr() { return this.buffer.dv.getUint8(this.offset+24); }
  public set dev_addr(value: number) { this.buffer.dv.setUint8(this.offset+24,value); }
  public get mem_addr() { return this.buffer.dv.getUint16(this.offset+26,true); }
  public set mem_addr(value: number) { this.buffer.dv.setUint16(this.offset+26,value,true); }
  public get type() { return this.buffer.dv.getUint32(this.offset+28,true); }
  public set type(value: number) { this.buffer.dv.setUint32(this.offset+28,value,true); }
}
} // namespace Flat 
export interface WebCategory {
  name: string;
  items: Array<WebItem>;
}

export namespace Flat_npwebserver {
export class WebCategory_Direct extends NPRPC.Flat.Flat {
  public get name() {
    let enc = new TextDecoder("utf-8");
    let v_begin = this.offset + 0;
    let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);
    let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));
    return enc.decode(bn);
  }
  public set name(str: string) {
    let enc = new TextEncoder();
    let bytes = enc.encode(str);
    let len = bytes.length;
    let offset = NPRPC.Flat._alloc(this.buffer, this.offset + 0, len, 1, 1);
    new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
  }
  public items(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 8, elements_size, 32, 4);
  }
  public items_vd() { return new NPRPC.Flat.Vector_Direct2<Flat_npwebserver.WebItem_Direct>(this.buffer, this.offset + 8, 32, Flat_npwebserver.WebItem_Direct); }
}
} // namespace Flat 
export class WebServer extends NPRPC.ObjectProxy {
  public static get servant_t(): new() => _IWebServer_Servant {
    return _IWebServer_Servant;
  }


  public async get_web_categories(cats: /*out*/Array<WebCategory>): Promise<void> {
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
  let out = new Flat_npwebserver.npwebserver_M1_Direct(buf, 16);
  {
  let vv = out._1_vd(), index_0 = 0;
  cats.length = vv.elements_size;
  for (let e of vv) {
    (cats[index_0] as any) = new Object();
  (cats[index_0] as any) = new Object();
  cats[index_0].name = e.name;
  {
  let vv = e.items_vd(), index_1 = 0;
  (cats[index_0].items as Array<any>) = new Array<any>(vv.elements_size)
  for (let e of vv) {
    (cats[index_0].items[index_1] as any) = new Object();
  (cats[index_0].items[index_1] as any) = new Object();
  cats[index_0].items[index_1].name = e.name;
  cats[index_0].items[index_1].description = e.description;
  cats[index_0].items[index_1].path = e.path;
  cats[index_0].items[index_1].dev_addr = e.dev_addr;
  cats[index_0].items[index_1].mem_addr = e.mem_addr;
  cats[index_0].items[index_1].type = e.type;
    ++index_1;
  }
  }
    ++index_0;
  }
  }
}

};

export interface IWebServer_Servant
{
  get_web_categories(cats: NPRPC.Flat.Vector_Direct2<Flat_npwebserver.WebCategory_Direct>): void;
}

export class _IWebServer_Servant extends NPRPC.ObjectServant {
  public static _get_class(): string { return "npwebserver/npwebserver.WebServer"; }
  public readonly get_class = () => { return _IWebServer_Servant._get_class(); }
  public readonly dispatch = (buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean) => {
    _IWebServer_Servant._dispatch(this, buf, remote_endpoint, from_parent);
  }
  static _dispatch(obj: _IWebServer_Servant, buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean): void {
  let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
  switch(__ch.function_idx) {
    case 0: {
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(152);
      obuf.commit(24);
      let oa = new Flat_npwebserver.npwebserver_M1_Direct(obuf,16);
      (obj as any).get_web_categories(oa._1_vd);
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    default:
      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Error_UnknownFunctionIdx);
  }
  }
}

export interface npwebserver_M1 {
  _1: Array<WebCategory>;
}

export namespace Flat_npwebserver {
export class npwebserver_M1_Direct extends NPRPC.Flat.Flat {
  public _1(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 0, elements_size, 16, 4);
  }
  public _1_vd() { return new NPRPC.Flat.Vector_Direct2<Flat_npwebserver.WebCategory_Direct>(this.buffer, this.offset + 0, 16, Flat_npwebserver.WebCategory_Direct); }
}
} // namespace Flat 
