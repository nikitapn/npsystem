import * as NPRPC from 'nprpc'

const u8enc = new TextEncoder();
const u8dec = new TextDecoder();

export interface Data1 {
  f1: string;
  f2: number/*f32*/;
}

export namespace Flat_benchmark {
export class Data1_Direct extends NPRPC.Flat.Flat {
  public get f1() {
    const offset = this.offset + 0;
    const n = this.buffer.dv.getUint32(offset + 4, true);
    return n > 0 ? u8dec.decode(new DataView(this.buffer.array_buffer, offset + this.buffer.dv.getUint32(offset, true), n)) : ""
  }
  public set f1(str: string) {
    const bytes = u8enc.encode(str);
    const offset = NPRPC.Flat._alloc(this.buffer, this.offset + 0, bytes.length, 1, 1);
    new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
  }
  public get f2() { return this.buffer.dv.getFloat32(this.offset+8,true); }
  public set f2(value: number) { this.buffer.dv.setFloat32(this.offset+8,value,true); }
}
} // namespace Flat 
export interface Data {
  f1: string;
  f2: string;
  f3: number/*u8*/;
  f4: number/*u16*/;
  f5: number/*u32*/;
  f6: Array<Data1>;
}

export namespace Flat_benchmark {
export class Data_Direct extends NPRPC.Flat.Flat {
  public get f1() {
    const offset = this.offset + 0;
    const n = this.buffer.dv.getUint32(offset + 4, true);
    return n > 0 ? u8dec.decode(new DataView(this.buffer.array_buffer, offset + this.buffer.dv.getUint32(offset, true), n)) : ""
  }
  public set f1(str: string) {
    const bytes = u8enc.encode(str);
    const offset = NPRPC.Flat._alloc(this.buffer, this.offset + 0, bytes.length, 1, 1);
    new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
  }
  public get f2() {
    const offset = this.offset + 8;
    const n = this.buffer.dv.getUint32(offset + 4, true);
    return n > 0 ? u8dec.decode(new DataView(this.buffer.array_buffer, offset + this.buffer.dv.getUint32(offset, true), n)) : ""
  }
  public set f2(str: string) {
    const bytes = u8enc.encode(str);
    const offset = NPRPC.Flat._alloc(this.buffer, this.offset + 8, bytes.length, 1, 1);
    new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
  }
  public get f3() { return this.buffer.dv.getUint8(this.offset+16); }
  public set f3(value: number) { this.buffer.dv.setUint8(this.offset+16,value); }
  public get f4() { return this.buffer.dv.getUint16(this.offset+18,true); }
  public set f4(value: number) { this.buffer.dv.setUint16(this.offset+18,value,true); }
  public get f5() { return this.buffer.dv.getUint32(this.offset+20,true); }
  public set f5(value: number) { this.buffer.dv.setUint32(this.offset+20,value,true); }
  public f6(elements_size: number): void { 
    NPRPC.Flat._alloc(this.buffer, this.offset + 24, elements_size, 12, 4);
  }
  public f6_d() { return new NPRPC.Flat.Vector_Direct2<Flat_benchmark.Data1_Direct>(this.buffer, this.offset + 24, 12, Flat_benchmark.Data1_Direct); }
}
} // namespace Flat 
export class Benchmark extends NPRPC.ObjectProxy {
  public static get servant_t(): new() => _IBenchmark_Servant {
    return _IBenchmark_Servant;
  }


  public async rpc(): Promise<Data> {
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
      this.endpoint(), buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_benchmark.benchmark_M1_Direct(buf, 16);
  let __ret_value: Data;
  (__ret_value as any) = new Object();
  __ret_value.f1 = out._1.f1;
  __ret_value.f2 = out._1.f2;
  __ret_value.f3 = out._1.f3;
  __ret_value.f4 = out._1.f4;
  __ret_value.f5 = out._1.f5;
  {
  let vv = out._1.f6_d(), index_0 = 0;
  (__ret_value.f6 as Array<any>) = new Array<any>(vv.elements_size)
  for (let e of vv) {
    (__ret_value.f6[index_0] as any) = new Object();
  (__ret_value.f6[index_0] as any) = new Object();
  __ret_value.f6[index_0].f1 = e.f1;
  __ret_value.f6[index_0].f2 = e.f2;
    ++index_0;
  }
  }
  return __ret_value;
}

  public async json(): Promise<string> {
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
    __ch.function_idx = 1;
    buf.write_len(buf.size - 4);
    await NPRPC.rpc.call(
      this.endpoint(), buf, this.timeout
    );
    let std_reply = NPRPC.handle_standart_reply(buf);
    if (std_reply != -1) {
      console.log("received an unusual reply for function with output arguments");
      throw new NPRPC.Exception("Unknown Error");
    }
  let out = new Flat_benchmark.benchmark_M2_Direct(buf, 16);
  let __ret_value: string;
  __ret_value = out._1;
  return __ret_value;
}

};

export interface IBenchmark_Servant
{
  rpc(): Data;
  json(): string;
}

export class _IBenchmark_Servant extends NPRPC.ObjectServant {
  public static _get_class(): string { return "benchmark/benchmark.Benchmark"; }
  public readonly get_class = () => { return _IBenchmark_Servant._get_class(); }
  public readonly dispatch = (buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean) => {
    _IBenchmark_Servant._dispatch(this, buf, remote_endpoint, from_parent);
  }
  static _dispatch(obj: _IBenchmark_Servant, buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean): void {
  let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
  switch(__ch.function_idx) {
    case 0: {
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(176);
      obuf.commit(48);
      let oa = new Flat_benchmark.benchmark_M1_Direct(obuf,16);
let __ret_val: Data;
      __ret_val = (obj as any).rpc();
  oa._1.f1 = __ret_val.f1;
  oa._1.f2 = __ret_val.f2;
  oa._1.f3 = __ret_val.f3;
  oa._1.f4 = __ret_val.f4;
  oa._1.f5 = __ret_val.f5;
  oa._1.f6(__ret_val.f6.length);
  {
  let vv = oa._1.f6_d(), index = 0;
  for (let e of vv) {
          e.f1 = __ret_val.f6[index].f1;
      e.f2 = __ret_val.f6[index].f2;
    ++index;
  }
  }
      obuf.write_len(obuf.size - 4);
      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);
      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);
      break;
    }
    case 1: {
      let obuf = buf;
      obuf.consume(obuf.size);
      obuf.prepare(152);
      obuf.commit(24);
      let oa = new Flat_benchmark.benchmark_M2_Direct(obuf,16);
let __ret_val: string;
      __ret_val = (obj as any).json();
  oa._1 = __ret_val;
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

export interface benchmark_M1 {
  _1: Data;
}

export namespace Flat_benchmark {
export class benchmark_M1_Direct extends NPRPC.Flat.Flat {
  public get _1() { return new Data_Direct(this.buffer, this.offset + 0); }
}
} // namespace Flat 
export interface benchmark_M2 {
  _1: string;
}

export namespace Flat_benchmark {
export class benchmark_M2_Direct extends NPRPC.Flat.Flat {
  public get _1() {
    const offset = this.offset + 0;
    const n = this.buffer.dv.getUint32(offset + 4, true);
    return n > 0 ? u8dec.decode(new DataView(this.buffer.array_buffer, offset + this.buffer.dv.getUint32(offset, true), n)) : ""
  }
  public set _1(str: string) {
    const bytes = u8enc.encode(str);
    const offset = NPRPC.Flat._alloc(this.buffer, this.offset + 0, bytes.length, 1, 1);
    new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
  }
}
} // namespace Flat 
