// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

import { FlatBuffer } from './flat_buffer'

export class Flat {
	constructor(public buffer: FlatBuffer, public offset: number) { }
}

export function _alloc(buffer: FlatBuffer, vector_offset: number, n: number, element_size: number, align: number): number {
	if (n == 0) {
		buffer.dv.setUint32(vector_offset, 0, true);
		buffer.dv.setUint32(vector_offset + 4, 0, true);
		return 0;
	}

	let rem = buffer.offset % align;
	let offset = rem ? buffer.offset + (align - rem) : buffer.offset;
	{
		let added_size = n * element_size + (offset - buffer.offset);

		buffer.prepare(added_size);
		buffer.commit(added_size);
	}
	buffer.dv.setUint32(vector_offset, offset - vector_offset, true);
	buffer.dv.setUint32(vector_offset + 4, n, true);
	//	console.log({
	//		offset: offset,
	//		vector_offset: vector_offset,
	//		voffset: buffer.dv.getUint32(vector_offset,  true)
	//	});
	return offset;
}

export function _alloc1(buffer: FlatBuffer, flat_offset: number, element_size: number, align: number): number {
	let rem = buffer.offset % align;
	let offset = rem ? buffer.offset + (align - rem) : buffer.offset;
	{
		let added_size = element_size + (offset - buffer.offset);
		buffer.prepare(added_size);
		buffer.commit(added_size);
	}

	buffer.dv.setUint32(flat_offset, offset - flat_offset, true);

	return offset;
}

interface Container {
	readonly elements_offset: number;
	readonly elements_size: number;
}

interface NumberAccessor {
	at(ix: number): number;
	set(ix: number, value: number): void;
	copy_from_ts_array(arr: number[]): void;
}

interface BigIntAccessor {
	at(ix: number): bigint;
	set(ix: number, value: bigint): void;
	copy_from_ts_array(arr: bigint[]): void;
}

class Vector extends Flat implements Container {
	public get elements_offset(): number { return this.offset + this.buffer.dv.getUint32(this.offset, true); }
	public get elements_size(): number { return this.buffer.dv.getUint32(this.offset + 4, true); }
}

class FlatArray extends Flat implements Container {
	public get elements_offset() { return this.offset; }
	public get elements_size() { return this.size; }
	constructor(public buffer: FlatBuffer, public offset: number, public size: number) {
		super(buffer, offset);
	}
}

type FlatContainer = Flat & Container;
type FlatContainerOfNumbers = FlatContainer & NumberAccessor;
type FlatContainerOfBigInts = FlatContainer & BigIntAccessor;

type GConstructor<T = {}> = new (...args: any[]) => T;

function Iterable<TBase extends GConstructor<FlatContainerOfNumbers>>(Base: TBase) {
	return class _Iterable extends Base {
		[Symbol.iterator]() {
			let this_ = this;
			const size = this_.elements_size;
			let index = 0;
			return {
				next: () => {
					if (index < size) return { value: this_.at(index++), done: false }
					return { value: size, done: true }
				}
			};
		}
	};
}

function Iterable2<TBase extends GConstructor<FlatContainerOfBigInts>>(Base: TBase) {
	return class _Iterable extends Base {
		[Symbol.iterator]() {
			let this_ = this;
			const size = this_.elements_size;
			let index = 0;
			return {
				next: () => {
					if (index < size) return { value: this_.at(index++), done: false }
					return { value: size, done: true }
				}
			};
		}
	};
}

function Accessor_U8<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_U8 extends Base implements NumberAccessor {
		public at(ix: number): number { return this.buffer.dv.getUint8(this.elements_offset + ix); }
		public set(ix: number, value: number): void { this.buffer.dv.setUint8(this.elements_offset + ix, value); }
		public copy_from_ts_array(arr: number[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setUint8(offset, n);
				offset += 1;
			}
		}
		public get array_buffer(): ArrayBuffer {
			let offset: number = this.elements_offset;
			return (this.buffer as FlatBuffer).array_buffer.slice(offset, offset + this.elements_size);
		}
		
		public get data_view(): DataView {
			return new DataView(this.buffer.array_buffer, this.elements_offset);
		}
	};
}

function Accessor_I8<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_I8 extends Base implements NumberAccessor {
		public at(ix: number) { return this.buffer.dv.getInt8(this.elements_offset + ix); }
		public set(ix: number, value: number) { this.buffer.dv.setInt8(this.elements_offset + ix, value); }
		public copy_from_ts_array(arr: number[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setInt8(offset, n);
				offset += 1;
			}
		}
	};
}

function Accessor_U16<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_U16 extends Base implements NumberAccessor {
		public at(ix: number): number { return this.buffer.dv.getUint16(this.elements_offset + 2*ix, true); }
		public set(ix: number, value: number): void { this.buffer.dv.setUint16(this.elements_offset + 2*ix, value, true); }
		public copy_from_ts_array(arr: number[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setUint16(offset, n, true);
				offset += 2;
			}
		}
	};
}

function Accessor_I16<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_I16 extends Base implements NumberAccessor {
		public at(ix: number) { return this.buffer.dv.getInt16(this.elements_offset + 2*ix, true); }
		public set(ix: number, value: number) { this.buffer.dv.setInt16(this.elements_offset + 2*ix, value, true); }
		public copy_from_ts_array(arr: number[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setInt16(offset, n, true);
				offset += 2;
			}
		}
	};
}

function Accessor_U32<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_U32 extends Base implements NumberAccessor {
		public at(ix: number): number { return this.buffer.dv.getUint32(this.elements_offset + 4*ix, true); }
		public set(ix: number, value: number): void { this.buffer.dv.setUint32(this.elements_offset + 4*ix, value, true); }
		public copy_from_ts_array(arr: number[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setUint32(offset, n, true);
				offset += 4;
			}
		}
	};
}

function Accessor_I32<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_I32 extends Base implements NumberAccessor {
		public at(ix: number) { return this.buffer.dv.getInt32(this.elements_offset + 4*ix, true); }
		public set(ix: number, value: number) { this.buffer.dv.setInt32(this.elements_offset + 4*ix, value, true); }
		public copy_from_ts_array(arr: number[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setInt32(offset, n, true);
				offset += 4;
			}
		}
	};
}

function Accessor_U64<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_U64 extends Base implements BigIntAccessor {
		public at(ix: number) { return this.buffer.dv.getBigUint64(this.elements_offset + 8*ix, true); }
		public set(ix: number, value: bigint) { this.buffer.dv.setBigUint64(this.elements_offset + 8*ix, value, true); }
		public copy_from_ts_array(arr: bigint[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setBigUint64(offset, n, true);
				offset += 8;
			}
		}
	};
}

function Accessor_I64<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_I64 extends Base implements BigIntAccessor {
		public at(ix: number) { return this.buffer.dv.getBigInt64(this.elements_offset + 8*ix, true); }
		public set(ix: number, value: bigint) { this.buffer.dv.setBigInt64(this.elements_offset + 8*ix, value, true); }
		public copy_from_ts_array(arr: bigint[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setBigInt64(offset, n, true);
				offset += 8;
			}
		}
	};
}

function Accessor_F32<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_Float32 extends Base implements NumberAccessor {
		public at(ix: number) { return this.buffer.dv.getFloat32(this.elements_offset + 4*ix, true); }
		public set(ix: number, value: number) { this.buffer.dv.setFloat32(this.elements_offset + 4*ix, value, true); }
		public copy_from_ts_array(arr: number[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setFloat32(offset, n, true);
				offset += 4;
			}
		}
	};
}

function Accessor_F64<TBase extends GConstructor<FlatContainer>>(Base: TBase) {
	return class _Accessor_Float64 extends Base implements NumberAccessor {
		public at(ix: number) { return this.buffer.dv.getFloat64(this.elements_offset + 8*ix, true); }
		public set(ix: number, value: number) { this.buffer.dv.setFloat64(this.elements_offset + 8*ix, value, true); }
		public copy_from_ts_array(arr: number[]): void {
			let offset = this.elements_offset;
			for (let n of arr) {
				this.buffer.dv.setFloat64(offset, n, true);
				offset += 8;
			}
		}
	};
}

export const Vector_Direct1_u8 = Iterable(Accessor_U8(Vector));
export const Vector_Direct1_i8 = Iterable(Accessor_I8(Vector));
export const Vector_Direct1_u16 = Iterable(Accessor_U16(Vector));
export const Vector_Direct1_i16 = Iterable(Accessor_I16(Vector));
export const Vector_Direct1_u32 = Iterable(Accessor_U32(Vector));
export const Vector_Direct1_i32 = Iterable(Accessor_I32(Vector));
export const Vector_Direct1_u64 = Iterable2(Accessor_U64(Vector));
export const Vector_Direct1_i64 = Iterable2(Accessor_I64(Vector));
export const Vector_Direct1_f32 = Iterable(Accessor_F32(Vector));
export const Vector_Direct1_f64 = Iterable(Accessor_F64(Vector));

export class Vector_Direct2<T extends Flat> extends Vector {
	*[Symbol.iterator]() {
		let size = this.elements_size;
		for (let ix = 0; ix < size; ++ix) {
			yield new this.flat_struct_constructor(this.buffer, this.elements_offset + this.element_size * ix);
		}
	}
	constructor(buffer: FlatBuffer, offset: number, private element_size: number,
		private flat_struct_constructor: new (buffer: FlatBuffer, offset: number) => T) {
		super(buffer, offset);
	}
}

export const Array_Direct1_u8 = Iterable(Accessor_U8(FlatArray));
export const Array_Direct1_i8 = Iterable(Accessor_I8(FlatArray));
export const Array_Direct1_u16 = Iterable(Accessor_U16(FlatArray));
export const Array_Direct1_i16 = Iterable(Accessor_I16(FlatArray));
export const Array_Direct1_u32 = Iterable(Accessor_U32(FlatArray));
export const Array_Direct1_i32 = Iterable(Accessor_I32(FlatArray));
export const Array_Direct1_u64 = Iterable2(Accessor_U64(FlatArray));
export const Array_Direct1_i64 = Iterable2(Accessor_I64(FlatArray));
export const Array_Direct1_f32 = Iterable(Accessor_F32(FlatArray));
export const Array_Direct1_f64 = Iterable(Accessor_F64(FlatArray));

export class Array_Direct2<T extends Flat> extends FlatArray {
	*[Symbol.iterator]() {
		let size = this.elements_size;
		for (let ix = 0; ix < size; ++ix) {
			yield new this.flat_struct_constructor(this.buffer, this.elements_offset + this.element_size * ix);
		}
	}
	constructor(buffer: FlatBuffer, offset: number, private element_size: number,
		private flat_struct_constructor: new (buffer: FlatBuffer, offset: number) => T, size: number) {
		super(buffer, offset, size);
	}
}

export class String_Direct1 extends Iterable(Accessor_U8(Vector)) {
	public assign(str: string): void {
    let utf8_string = new TextEncoder().encode(str);
    let offset = _alloc(this.buffer, this.offset, utf8_string.length, 1, 1);
    new Uint8Array(this.buffer.array_buffer, offset).set(utf8_string);
	}
}

class Optional extends Flat {
	constructor(buffer: FlatBuffer, offset: number, /** @internal */ private ut_size: number, /** @internal */ private ut_align: number) { super(buffer, offset); }
	protected get value_offset(): number { return this.offset + this.buffer.dv.getUint32(this.offset, true); }
	public alloc(): void { _alloc1(this.buffer, this.offset, this.ut_size, this.ut_align); }
	public get has_value(): boolean { return this.buffer.dv.getUint32(this.offset, true) !== 0; }
	public set_nullopt(): void { this.buffer.dv.setUint32(this.offset, 0, true); }
}

export class Optional_Direct1_u8 extends Optional {
	public get value(): number { return this.buffer.dv.getUint8(this.value_offset); }
	public set value(value: number) { this.buffer.dv.setUint8(this.value_offset, value); }
}

export class Optional_Direct1_i8 extends Optional {
	public get value(): number { return this.buffer.dv.getInt8(this.value_offset); }
	public set value(value: number) { this.buffer.dv.setInt8(this.value_offset, value); }
}

export class Optional_Direct1_u16 extends Optional {
	public get value(): number { return this.buffer.dv.getUint16(this.value_offset, true); }
	public set value(value: number) { this.buffer.dv.setUint16(this.value_offset, value, true); }
}

export class Optional_Direct1_i16 extends Optional {
	public get value(): number { return this.buffer.dv.getInt16(this.value_offset, true); }
	public set value(value: number) { this.buffer.dv.setInt16(this.value_offset, value, true); }
}

export class Optional_Direct1_u32 extends Optional {
	public get value(): number { return this.buffer.dv.getUint32(this.value_offset, true); }
	public set value(value: number) { this.buffer.dv.setUint32(this.value_offset, value, true); }
}

export class Optional_Direct1_i32 extends Optional {
	public get value(): number { return this.buffer.dv.getInt32(this.value_offset, true); }
	public set value(value: number) { this.buffer.dv.setInt32(this.value_offset, value, true); }
}

export class Optional_Direct1_u64 extends Optional {
	public get value(): bigint { return this.buffer.dv.getBigUint64(this.value_offset, true); }
	public set value(value: bigint) { this.buffer.dv.setBigUint64(this.value_offset, value, true); }
}

export class Optional_Direct1_i64 extends Optional {
	public get value(): bigint { return this.buffer.dv.getBigInt64(this.value_offset, true); }
	public set value(value: bigint) { this.buffer.dv.setBigInt64(this.value_offset, value, true); }
}

export class Optional_Direct1_f32 extends Optional {
	public get value(): number { return this.buffer.dv.getFloat32(this.value_offset, true); }
	public set value(value: number) { this.buffer.dv.setFloat32(this.value_offset, value, true); }
}

export class Optional_Direct1_f64 extends Optional {
	public get value(): number { return this.buffer.dv.getFloat64(this.value_offset, true); }
	public set value(value: number) { this.buffer.dv.setFloat64(this.value_offset, value, true); }
}

export class Optional_Direct2<T extends Flat> extends Optional {
	constructor(
		buffer: FlatBuffer, 
		offset: number, 
		ut_size: number, 
		ut_align: number,
		private flat_struct_constructor: new (buffer: FlatBuffer, offset: number) => T) 
		{
		super(buffer, offset, ut_size, ut_align);
	}

	public get value(): T {
		return new this.flat_struct_constructor(this.buffer, this.value_offset);
	}
}