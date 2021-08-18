// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory
import {impl} from "./nprpc_base"

export class FlatBuffer {
	private capacity: number;
	private size_: number;
	public array_buffer: ArrayBuffer;
	public dv: DataView;

	public static from_array_buffer(array_buffer: ArrayBuffer): FlatBuffer {
		let b = new FlatBuffer();
		b.array_buffer = array_buffer;
		b.size_ = b.capacity = array_buffer.byteLength;
		b.dv = new DataView(b.array_buffer);
		return b;
	}

	public static create(initial_capacity: number = 512): FlatBuffer {
		let b = new FlatBuffer();
		b.capacity = initial_capacity;
		b.size_ = 0;
		b.array_buffer = new ArrayBuffer(initial_capacity);
		b.dv = new DataView(b.array_buffer);
		return b;
	}

	public prepare(n: number): void {
		if (this.size_ + n > this.capacity) {
			this.capacity = Math.max(this.capacity * 2, this.capacity + n);
			let new_buffer = new ArrayBuffer(this.capacity);
			new Uint8Array(new_buffer).set(new Uint8Array(this.array_buffer));
			this.array_buffer = new_buffer;
			this.dv = new DataView(this.array_buffer);
		}
	}

	public consume(n: number): void { this.size_ -= n; }
	public commit(n: number): void { this.size_ += n; }

	public get offset() { return this.size_; }
	public get size() { return this.size_; }

	public write_len(msg_len: number) {
		this.dv.setUint32(0, msg_len, true);
	}
	
	public write_msg_id(msg: impl.MessageId) {
		this.dv.setUint32(4, msg, true);
	}

	public read_msg_id(): impl.MessageId {
		return this.dv.getUint32(4, true) as impl.MessageId;
	}

	public write_msg_type(msg: impl.MessageType) {
		this.dv.setUint32(8, msg, true);
	}

	public read_msg_type(): impl.MessageType {
		return this.dv.getUint32(8, true) as impl.MessageType;
	}

	public read_exception_number(): number {
		return this.dv.getUint32(16, true);
	}

	public get writable_view(): DataView {
		return new DataView(this.array_buffer, 0, this.size);
	}

	public set_buffer(abuf: ArrayBuffer): void {
		this.array_buffer = abuf;
		this.size_ = this.capacity = abuf.byteLength;
		this.dv = new DataView(this.array_buffer);
	}

	public dump() {
		let s = new String();
		new Uint8Array(this.array_buffer, 0, this.size).forEach((x:number) => s += x.toString(16) + ' ');
		console.log(s);
	}
}