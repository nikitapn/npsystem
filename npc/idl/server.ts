import * as IdlUtils from '../include/npidl/npidl'

export class server_value_Direct {
  private buffer: IdlUtils.NetBuffer;
  private offset: number;

  constructor(buffer: IdlUtils.NetBuffer, offset: number) {
    this.buffer = buffer;
    this.offset = offset;
  }
}

export class RawDataDef_Direct {
  private buffer: IdlUtils.NetBuffer;
  private offset: number;

  constructor(buffer: IdlUtils.NetBuffer, offset: number) {
    this.buffer = buffer;
    this.offset = offset;
  }
}

export class DataDef_Direct {
  private buffer: IdlUtils.NetBuffer;
  private offset: number;

  constructor(buffer: IdlUtils.NetBuffer, offset: number) {
    this.buffer = buffer;
    this.offset = offset;
  }
}

