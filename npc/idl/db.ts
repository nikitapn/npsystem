import * as IdlUtils from '../include/npidl/npidl'

export class BatchOperation_Direct {
  private buffer: IdlUtils.NetBuffer;
  private offset: number;

  constructor(buffer: IdlUtils.NetBuffer, offset: number) {
    this.buffer = buffer;
    this.offset = offset;
  }
}

